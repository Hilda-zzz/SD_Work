#include "Nova2DShaders/Nova2DParticleStruct.hlsli"

//struct GPUParticle2D
//{
//    float2 position;
//    float2 velocity;
//    float lifetime;
//    float maxLifetime;
//    
//    float size;
//    float rotation;
//    uint color;
//    uint flags;
//    
//    uint spriteIndex;
//    uint animFrame;
//    float animTime;
//    uint padding;
//};
//
//#define PARTICLE_FLAG_GRAVITY    0x1
//#define PARTICLE_FLAG_COLLISION  0x2
//#define PARTICLE_FLAG_FADE_OUT   0x4
//#define PARTICLE_FLAG_ROTATE     0x8


// ===== Buffers =====
RWStructuredBuffer<GPUParticle2D> Particles : register(u0);
RWStructuredBuffer<uint> DeadList : register(u1);
RWStructuredBuffer<uint> Counters : register(u2);
RWStructuredBuffer<uint> AliveList : register(u3); // write in this list

cbuffer EmitParams : register(b0)
{
    float2 emitterPosition;    // 8 bytes
    float2 velocityMin;        // 8 bytes
    float2 velocityMax;        // 8 bytes
    
    float lifetimeMin;         // 4 bytes
    float lifetimeMax;         // 4 bytes
    float sizeMin;             // 4 bytes
    float sizeMax;             // 4 bytes
    
    uint shapeType;            // 4 bytes (0=Point, 1=Circle, 2=Box)
    float shapeParam1;         // 4 bytes (Circle: radius, Box: width)
    float shapeParam2;         // 4 bytes (Box: height)
    uint colorPacked;          // 4 bytes 
    
    uint spriteIndex;          // 4 bytes
    uint emitCount;            // 4 bytes
    uint flags;                // 4 bytes
    uint randomSeed;           // 4 bytes
    
    uint maxParticles;         // 4 bytes
    uint padding;             // 12 bytes
    
    // 80 bytes
};

// ===== RNG =====
uint Hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float Random01(uint seed, uint index)
{
    return float(Hash(seed + index * 747796405u)) / 4294967296.0;
}

float RandomRange(uint seed, uint index, float minVal, float maxVal)
{
    return lerp(minVal, maxVal, Random01(seed, index));
}

[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint threadIndex = DTid.x;
    
    if (threadIndex >= emitCount)
        return;
    
    // ===== draw a particle in dead list =====
    uint oldDeadCount;
    InterlockedAdd(Counters[1], -1, oldDeadCount);
    
    if (oldDeadCount == 0)
        return; 
    
    uint deadListIndex = oldDeadCount - 1;
    
    uint particleIndex = DeadList[deadListIndex];
    
    if (particleIndex >= maxParticles)
        return;
    
    uint rngSeed = randomSeed + threadIndex;
    
    float2 position = emitterPosition;
    
    if (shapeType == 1)  // Circle
    {
        float angle = Random01(rngSeed, 0) * 6.28318530718;  // 0 to 2?
        float radius = Random01(rngSeed, 1) * shapeParam1;
        position += float2(cos(angle), sin(angle)) * radius;
    }
    else if (shapeType == 2)  // Box
    {
        float halfWidth = shapeParam1 * 0.5;
        float halfHeight = shapeParam2 * 0.5;
        float x = RandomRange(rngSeed, 0, -halfWidth, halfWidth);
        float y = RandomRange(rngSeed, 1, -halfHeight, halfHeight);
        position += float2(x, y);
    }
    
    float2 velocity = float2(
        RandomRange(rngSeed, 2, velocityMin.x, velocityMax.x),
        RandomRange(rngSeed, 3, velocityMin.y, velocityMax.y)
    );
    
    float lifetime = RandomRange(rngSeed, 4, lifetimeMin, lifetimeMax);
    
    float size = RandomRange(rngSeed, 5, sizeMin, sizeMax);
    
    float rotation = 0.0f;
    if (flags & PARTICLE_FLAG_ROTATE)
    {
        rotation = Random01(rngSeed, 6) * 6.28318530718; 
    }
    
    GPUParticle2D p;
    p.position = position;
    p.velocity = velocity;
    p.lifetime = lifetime;
    p.maxLifetime = lifetime;
    p.size = size;
    p.rotation = rotation;
    p.color = colorPacked;
    p.flags = flags;
    p.spriteIndex = spriteIndex;
    p.animFrame = 0;
    p.animTime = 0.0f;
    p.instanceID=0;
    p.padding=float3(0.0,0.0,0.0);
    
    Particles[particleIndex] = p;
    
    uint aliveIndex;
    InterlockedAdd(Counters[0], 1, aliveIndex);
    AliveList[aliveIndex] = particleIndex;
}