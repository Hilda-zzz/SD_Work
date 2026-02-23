#include "Nova2DShaders/Nova2DParticleStruct.hlsli"
#include "Nova2DShaders/Nova2DCommon.hlsli"

// ===== Buffers =====
RWStructuredBuffer<GPUParticle2D> Particles : register(u0);
RWStructuredBuffer<uint> DeadList : register(u1);
RWStructuredBuffer<uint> Counters : register(u2);  // [aliveCount, deadCount, emitCount, padding]
RWStructuredBuffer<uint> AliveList1 : register(u3);  // In read-only
RWStructuredBuffer<uint> AliveList2 : register(u4);  // Out write- only

StructuredBuffer<Nova2DEmitterDefinitionGPU> g_Definitions : register(t5);
StructuredBuffer<Nova2DEmitterInstanceGPU> g_Instances : register(t6);

Texture2D<float4> CollisionTexture : register(t10);
//SamplerState CollisionSampler : register(s0);

cbuffer UpdateParams : register(b0)
{
    float deltaTime;
    float gravity;
    float currentTime;
    uint maxParticles;

    float2 viewportMin;
    float2 viewportMax;

    float2 textureSize;
    float2 padding;
};

//==========================================================================
// Collision detection and response
// Returns: true if collision hit
//==========================================================================
bool ProcessCollision(inout GPUParticle2D particle, 
                      Nova2DEmitterDefinitionGPU def)
{
    bool result = false;
    
    // Collision disabled
    if (def.m_collision.m_enableCollision == 0)
    {
        return result;
    }
    else
    {
        // World position -> Normalized viewport coords (0-1)
        float2 viewportSize = viewportMax - viewportMin;
        float2 normalizedPos = (particle.position - viewportMin) / viewportSize;
        normalizedPos.y = 1.0 - normalizedPos.y; 
    
        // Boundary check
        if (normalizedPos.x < 0 || normalizedPos.x > 1 || 
            normalizedPos.y < 0 || normalizedPos.y > 1)
        {
            return result;
        }
        else
        {
            int2 pixelCoord = int2(normalizedPos.x * textureSize.x, 
                                       normalizedPos.y * textureSize.y);
            float4 collisionColor = CollisionTexture.Load(int3(pixelCoord, 0));     
    
            // Iterate through all rules
            for (uint i = 0; i < def.m_collision.m_numRules; i++)
            {
                N2D_CollisionRule rule = def.m_collision.m_rules[i];
        
                // Color matching
                if (!ColorMatch(collisionColor, rule.m_targetColor))
                    continue;
        
                // Hit! Execute response
                result = true;

                if (rule.m_maxBounces > 0 && particle.bounceCount >= rule.m_maxBounces)
                {
                    particle.lifetime = 0;
                    break;
                }
                particle.bounceCount++;
                switch (rule.m_response)
                {
                    case N2D_COLLISION_NONE:
                        // Pass through, no action
                        break;
                
                    case N2D_COLLISION_BOUNCE:
                    {
                        // Simple bounce: reverse dominant velocity component
                        float2 vel = particle.velocity;
    
                        // Determine which component is dominant
                        if (abs(vel.x) > abs(vel.y))
                        {
                            // Hit vertical wall -> reverse X
                            particle.velocity.x = -vel.x * rule.m_bounceDamping;
                            particle.velocity.y = vel.y * rule.m_bounceDamping;
                        }
                        else
                        {
                            // Hit horizontal surface -> reverse Y
                            particle.velocity.y = -vel.y * rule.m_bounceDamping;
                            particle.velocity.x = vel.x * rule.m_bounceDamping;
                        }
                        
                        int particleIndex = int(particle.position.x * 1000.0 + particle.position.y);
                        uint seedTime = uint(currentTime * 1000.0);
    
                        // Directly get angle in range [-15°, +15°]
                        float randomAngle = GetFloatNoiseInRange(-0.262, 0.262, particleIndex, seedTime);
    
                        // Rotate velocity
                        float cosA = cos(randomAngle);
                        float sinA = sin(randomAngle);
                        particle.velocity = float2(
                            particle.velocity.x * cosA - particle.velocity.y * sinA,
                            particle.velocity.x * sinA + particle.velocity.y * cosA
                        );
    
                        break;
                    }
                
                    case N2D_COLLISION_DIE:
                    {
                        // Die: kill particle
                        particle.lifetime = 0;
                        break;
                    }
                
                    case N2D_COLLISION_STICK:
                    {
                        // Stick: zero velocity
                        particle.velocity = float2(0, 0);
                        break;
                    }
                
                    case N2D_COLLISION_SLOW:
                    {
                        // Slow (only on first touch)
                        if (particle.collisionState == 0)
                        {
                            particle.velocity *= rule.m_slowFactor;
                        }
                        break;
                    }
                }
        
                // Only match first rule
                break;
            }
    
            return result;
   
        }
    
    }
    
}

[numthreads(128, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;

    if (index >= Counters[0])
        return;
    
    uint particleIndex = AliveList1[index];
    GPUParticle2D p = Particles[particleIndex];

    Nova2DEmitterInstanceGPU inst = g_Instances[p.instanceID];
    Nova2DEmitterDefinitionGPU def = g_Definitions[inst.m_definitionIndex];
    
    // ================== Point Force ==================
    if (def.m_motion.m_enablePointForce != 0)
    {
        float2 pointForcePosWorld = float2(inst.m_positionX, inst.m_positionY) + inst.m_pointForceOffset;
        float2 toPoint = pointForcePosWorld - p.position;
        float dist = length(toPoint);
        if (dist < def.m_motion.m_pointForceRadius && dist > 0.001f)
        {
            float t2 = 1.0f - (dist / def.m_motion.m_pointForceRadius);
            float falloff = pow(t2, def.m_motion.m_pointForceFalloff);
            float2 dir = normalize(toPoint);
            if (def.m_motion.m_pointForceAttract == 0) dir = -dir;
            p.velocity += dir * def.m_motion.m_pointForceStrength * falloff * deltaTime;
        }
    }

    // ================== Vortex Force ==================
    if (def.m_motion.m_enableVortex != 0)
    {
        float2 vortexPosWorld = float2(inst.m_positionX, inst.m_positionY) + inst.m_vortexOffset;
        float2 toParticle = p.position - vortexPosWorld;
        float dist = length(toParticle);
        if (dist < def.m_motion.m_vortexRadius && dist > 0.001f)
        {
            float2 tangent = normalize(float2(-toParticle.y, toParticle.x));
            p.velocity += tangent * def.m_motion.m_vortexStrength * deltaTime;
        }
    }

    // linear force
    p.velocity+= def.m_motion.m_linearForce* deltaTime;

    
    // ======================= Collision ================
    bool hitCollision = ProcessCollision(p, def);
    if (hitCollision)
        p.collisionState = 1;
    else
        p.collisionState = 0;

    // ===== Color/Size/Rotation Over Lifetime =====
    float t = 1.0 - (p.lifetime / p.maxLifetime);

    if (p.flags & PARTICLE_FLAG_ROTATE)
    {
        float rotationSpeed = EvaluateRotationSpeedCurve(def, t);
        p.rotation += rotationSpeed * deltaTime;
    }

    p.color = EvaluateColorCurve(def.m_appearance, t);
    p.size = EvaluateSizeCurve(def, t);

    // ================== Curl Noise Force Field ================
    // Check if particle has CURL_NOISE flag enabled
    if (p.flags & PARTICLE_FLAG_CURL_NOISE)  // PARTICLE_FLAG_CURL_NOISE = 0x10 (1 << 4)
    {
        // Find Curl Noise curve in definition
        float curlStrength = 0.0;
        for (uint i = 0; i < def.m_numCurves; ++i)
        {
            if (def.m_curves[i].m_type == 5)  // CURL_NOISE = 5
            {
                curlStrength = EvaluateFloatCurve(def.m_curves[i], t);
                break;
            }
        }
    
        // Apply curl force if strength > 0
        if (curlStrength > 0.001)
        {
            // Compute curl force (x, y, time)
            float scale = 0.01;  // Noise scale (lower = larger swirls)
            float2 curlForce = ComputeCurlForce2D(p.position, currentTime, scale, curlStrength);
        
            // Apply force to velocity
            p.velocity += curlForce * deltaTime;
        }
    }

    // ================== Drag ===============
    if (def.m_motion.m_drag > 0.0f)
    {
        p.velocity *= max(0.0f, 1.0f - def.m_motion.m_drag * deltaTime);
    }
    // ==========Orient to Velocity===========
    if (def.m_motion.m_orientToVelocity != 0)
    {
        if (dot(p.velocity, p.velocity) > 0.0001f)
            p.rotation = atan2(p.velocity.y, p.velocity.x);
    }
    

    p.position += p.velocity * deltaTime;

    // ==============sprite sheet animation===============
    p.animTime += deltaTime;

    uint sheetDimX = def.m_appearance.m_spriteSheetDimensionsX;
    uint sheetDimY = def.m_appearance.m_spriteSheetDimensionsY;
    uint startIndex = def.m_appearance.m_spriteStartIndex;
    uint endIndex = def.m_appearance.m_spriteEndIndex;
    uint frameCount = (endIndex - startIndex + 1);

    float animFPS = def.m_appearance.m_animationFPS; 
    if (frameCount > 0)
    {
        float frameDuration = 1.0f / animFPS;
        p.animFrame = (uint)(p.animTime / frameDuration) % frameCount;
    }
    else
    {
        p.animFrame = 0;
    }

    // ------------------------------------------
    // Dead or Alive
    
    p.lifetime -= deltaTime;
    if (p.lifetime <= 0.0f)
    {
        uint deadIndex;
        InterlockedAdd(Counters[1], 1, deadIndex);
        
        if (deadIndex < maxParticles)
        {
            DeadList[deadIndex] =  particleIndex;
        }
        Particles[particleIndex] = p;
        
    }
    else
    {
        uint newAliveIndex;
        InterlockedAdd(Counters[3], 1, newAliveIndex);
        if (newAliveIndex < maxParticles)
        {
            AliveList2[newAliveIndex] = particleIndex;
        }
        else
        {
            InterlockedAdd(Counters[3], -1);     
            p.lifetime = -1.0f;
        
            uint deadIndex;
            InterlockedAdd(Counters[1], 1, deadIndex);
            if (deadIndex < maxParticles)
            {
                DeadList[deadIndex] = particleIndex;
            }
        }
    
        Particles[particleIndex] = p;
    }
}