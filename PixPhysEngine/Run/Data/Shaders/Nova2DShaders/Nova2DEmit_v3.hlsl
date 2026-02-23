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
StructuredBuffer<uint> g_EmitOffsets : register(t7); 

//==========================================================================
// Constant Buffer
//==========================================================================
cbuffer EmitParams : register(b0)
{
    uint g_totalEmitCount;    
    uint g_instanceCount;    
    uint g_randomSeed;
    uint g_maxParticles;
};

uint BinarySearchInstance(uint globalParticleIndex)
{
    uint left = 0;
    uint right = g_instanceCount;
    
    if (g_instanceCount == 1) 
    {
        return left;
    }
    else
    {
        while (left < right - 1) 
        {
            uint mid = (left + right) / 2;
            if (globalParticleIndex >= g_EmitOffsets[mid]) 
            {
                left = mid;
            } 
            else 
            {
                right = mid;
            }
        }
        return left;
    }
    
      // O(log n)
}


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
        RandomRange(rngSeed, 2, motion.m_velocityMin.x, motion.m_velocityMax.x),
        RandomRange(rngSeed, 3, motion.m_velocityMin.y, motion.m_velocityMax.y)
    );
}

float4 GetStartColor(N2D_AppearanceModule appearance)
{
    return (appearance.m_numColorKeyframes > 0) 
        ? appearance.m_colorKeyframes[0].m_colorPacked 
        : float4(1.0, 1.0, 1.0, 1.0);
}

float GetStartSize(Nova2DEmitterDefinitionGPU def)
{
    return 5.0f; 
}

[numthreads(64, 1, 1)] 
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint globalParticleIndex = DTid.x;
    
    if (globalParticleIndex >= g_totalEmitCount)
        return;
    
    uint instanceID = BinarySearchInstance(globalParticleIndex);
    
    if (instanceID >= g_instanceCount)
        return;  
    
    Nova2DEmitterInstanceGPU inst = g_Instances[instanceID];
    
    if (inst.m_isActive == 0 || inst.m_killFlag != 0)
        return;
    
    Nova2DEmitterDefinitionGPU def = g_Definitions[inst.m_definitionIndex];
    
    uint currentDeadCount = g_Counters[1];
    if (currentDeadCount == 0) {
        return;  
    }
    
    uint oldDeadCount;
    InterlockedAdd(g_Counters[1], -1, oldDeadCount);
    
    if (oldDeadCount == 0) {
        InterlockedAdd(g_Counters[1], 1); 
        return;
    }
    
    uint deadListIndex = oldDeadCount - 1;
    uint particleIndex = g_DeadList[deadListIndex];
    
    if (particleIndex >= g_maxParticles) {
        InterlockedAdd(g_Counters[1], 1); 
        return;
    }
    
    uint rngSeed = g_randomSeed + instanceID * 10000 + globalParticleIndex;
    
    float2 localPos = SampleEmissionPosition(def.m_emission, rngSeed);
    float2 scaledPos = localPos * inst.m_scale;
    float2 rotatedPos = Rotate2D(scaledPos, inst.m_rotation);
    float2 worldPos = float2(inst.m_positionX, inst.m_positionY) + rotatedPos;
    
    float2 velocity = SampleVelocity(def.m_motion, rngSeed);
    
    if (def.m_motion.m_velocityMode == 1)  // Radial
    {
        float2 dir = normalize(localPos);
        if (length(localPos) < 0.001f) {
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
    
    for (uint curveIdx = 0; curveIdx < def.m_numCurves; ++curveIdx) {
        if (def.m_curves[curveIdx].m_type == 5) {  
            flags |=PARTICLE_FLAG_CURL_NOISE ;
            break;
        }
        if (def.m_curves[curveIdx].m_type == 1) {  
            flags |= PARTICLE_FLAG_ROTATE;
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
    p.instanceID = instanceID;
    p.textureIndex=0;

    p.depth = RandomRange(rngSeed, 4, def.m_emission.m_depthMin,def.m_emission.m_depthMax);
    p.emission = def.m_appearance.m_emissionStrength;

    p.collisionState = 0;
    p.bounceCount=0;
    
    p.padding =float2(0.f,0.f);
    
    g_Particles[particleIndex] = p;
    
    uint aliveIndex;
    InterlockedAdd(g_Counters[0], 1, aliveIndex);
    
    if (aliveIndex < g_maxParticles) {
        g_AliveList[aliveIndex] = particleIndex;
    } else {
        InterlockedAdd(g_Counters[0], -1);
        
        uint returnDeadIndex;
        InterlockedAdd(g_Counters[1], 1, returnDeadIndex);
        if (returnDeadIndex < g_maxParticles) {
            g_DeadList[returnDeadIndex] = particleIndex;
        }
    }
}