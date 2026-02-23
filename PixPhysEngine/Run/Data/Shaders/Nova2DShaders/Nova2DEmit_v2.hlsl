#include "Nova2DShaders/Nova2DParticleStruct.hlsli"
#include "Nova2DShaders/Nova2DCommon.hlsli"

//==========================================================================
// GPU Buffers
//==========================================================================
RWStructuredBuffer<GPUParticle2D> g_Particles : register(u0);
RWStructuredBuffer<uint> g_DeadList : register(u1);
RWStructuredBuffer<uint> g_Counters : register(u2);
RWStructuredBuffer<uint> g_AliveList : register(u3);

StructuredBuffer<Nova2DEmitterDefinitionGPU> g_Definitions : register(t5);
StructuredBuffer<Nova2DEmitterInstanceGPU> g_Instances : register(t6);

//==========================================================================
// Constant Buffer
//==========================================================================
cbuffer EmitParams : register(b0)
{
    uint g_instanceCount;      
    uint g_randomSeed;          
    uint g_maxParticles;        
    uint g_padding;
};


float2 SampleEmissionPosition(N2D_EmissionModule emission, uint rngSeed)
{
    float2 localPos = float2(0, 0);
    
    if (emission.m_emissionType == 0)  // Point
    {

    }
    else if (emission.m_emissionType == 1)  // Circle
    {
        float angle = Random01(rngSeed, 0) * TWO_PI;
        float radius = Random01(rngSeed, 1) * emission.m_emissionRadius;
        localPos = float2(cos(angle), sin(angle)) * radius;
    }
    else if (emission.m_emissionType == 2)  // Box
    {
        float halfWidth = emission.m_boxDimensionX * 0.5;
        float halfHeight = emission.m_boxDimensionY * 0.5;
        localPos = float2(
            RandomRange(rngSeed, 0, -halfWidth, halfWidth),
            RandomRange(rngSeed, 1, -halfHeight, halfHeight)
        );
    }
    
    return localPos;
}

float2 SampleVelocity(N2D_MotionModule motion, uint rngSeed)
{
    return float2(
        RandomRange(rngSeed, 2, motion.m_velocityMinX, motion.m_velocityMaxX),
        RandomRange(rngSeed, 3, motion.m_velocityMinY, motion.m_velocityMaxY)
    );
}

float4 GetStartColor(N2D_AppearanceModule appearance)
{
    return (appearance.m_numColorKeyframes > 0) 
        ? appearance.m_colorKeyframes[0].m_colorPacked 
        : float4(1.0, 1.0, 1.0, 1.0);
}

//==========================================================================
float GetStartSize(Nova2DEmitterDefinitionGPU def)
{
    // TODO:def.m_curves
    return 5.0f;
}

//==========================================================================
// Main Compute Shader
//==========================================================================
[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint instanceID = DTid.x;
    
    if (instanceID >= g_instanceCount)
        return;
    
    Nova2DEmitterInstanceGPU inst = g_Instances[instanceID];
    
    if (inst.m_isActive == 0 || inst.m_killFlag != 0)
        return;
    
    Nova2DEmitterDefinitionGPU def = g_Definitions[inst.m_definitionIndex];
    
    uint emitCount = 0;
    
    if (def.m_emission.m_emissionMode == 0)  // CONTINUOUS
    {
        // accumulator Instance::UpdateCPU
        emitCount = uint(inst.m_emissionAccumulator);
    }
    else if (def.m_emission.m_emissionMode == 1)  // BURST
    {
        emitCount = def.m_emission.m_numBursts;
    }
    
    if (emitCount == 0)
        return;

    // TODO Phase 4: each particle one thread
    
    for (uint i = 0; i < emitCount; ++i)
    {
        uint currentDeadCount = g_Counters[1];
        if (currentDeadCount == 0) 
        {
            break;
        }
        
        uint oldDeadCount;
        InterlockedAdd(g_Counters[1], -1, oldDeadCount);

        if (oldDeadCount == 0)
        {
            InterlockedAdd(g_Counters[1], 1);
            break;
        }
        
        if (oldDeadCount == 0)
        {
            break;
        }
        
        uint deadListIndex = oldDeadCount - 1;
        uint particleIndex = g_DeadList[deadListIndex];
        
        if (particleIndex >= g_maxParticles)
            continue;
        
        uint rngSeed = g_randomSeed + instanceID * 10000 + i;
        
        float2 localPos = SampleEmissionPosition(def.m_emission, rngSeed);
        
        float2 scaledPos = localPos * inst.m_scale;
        float2 rotatedPos = Rotate2D(scaledPos, inst.m_rotation);
        float2 worldPos = float2(inst.m_positionX, inst.m_positionY) + rotatedPos;
        
        float2 velocity = SampleVelocity(def.m_motion, rngSeed);
        
        if (def.m_motion.m_velocityMode == 1)
        {
            float2 dir = normalize(localPos);
            if (length(localPos) < 0.001f)
            {
                float angle = Random01(rngSeed, 10) * TWO_PI;
                dir = float2(cos(angle), sin(angle));
            }
            float speed = length(velocity);
            velocity = dir * speed;
        }
        
        float lifetime = RandomRange(rngSeed, 4, 
            def.m_emission.m_lifetimeMin, 
            def.m_emission.m_lifetimeMax);
        
        float size = GetStartSize(def);
        
        float4 color = GetStartColor(def.m_appearance);
        
        float rotation = 0.0f;
        
        uint flags = 0;
        if (def.m_motion.m_gravityScale > 0.001f)
        {
            flags |= PARTICLE_FLAG_GRAVITY;
        }

        for (uint curveIdx = 0; curveIdx < def.m_numCurves; ++curveIdx)
        {
            if (def.m_curves[curveIdx].m_type == 5)  // CURL_NOISE = 5
            {
                flags |= 0x10;  // PARTICLE_FLAG_CURL_NOISE
                break;
            }
        }
        
        GPUParticle2D p;
        p.position = worldPos;
        p.velocity = velocity;
        p.lifetime = lifetime;
        p.maxLifetime = lifetime;
        p.size = size;
        p.rotation = rotation;
        p.color = color;
        p.flags = flags;
        p.spriteIndex = def.m_appearance.m_spriteStartIndex;
        p.animFrame = 0;
        p.animTime = 0.0f;
        p.instanceID=instanceID;
        p.padding=float3(0.0,0.0,0.0);
        
        g_Particles[particleIndex] = p;
        
        uint aliveIndex;
        InterlockedAdd(g_Counters[0], 1, aliveIndex);
        
        if (aliveIndex < g_maxParticles)
        {
            g_AliveList[aliveIndex] = particleIndex;
        }
        else
        {
            InterlockedAdd(g_Counters[0], -1);
            
            uint returnDeadIndex;
            InterlockedAdd(g_Counters[1], 1, returnDeadIndex);
            if (returnDeadIndex < g_maxParticles)
            {
                g_DeadList[returnDeadIndex] = particleIndex;
            }
            
            break;
        }
    }
}
