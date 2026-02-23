#ifndef NOVA2D_STRUCTURES_HLSLI
#define NOVA2D_STRUCTURES_HLSLI

struct GPUParticle2D
{
    float2 position;
    float2 velocity;
    float lifetime;
    float maxLifetime;
    
    float size;
    float rotation;
    float4 color;
    uint flags;
    
    uint spriteIndex;
    uint animFrame;
    float animTime;
    
    uint instanceID;
    uint textureIndex;
    
    float depth;
    
    float emission;
    
    uint collisionState;
    uint bounceCount;
    float2 padding;
};

// Particle Flags
#define PARTICLE_FLAG_GRAVITY    0x1
#define PARTICLE_FLAG_COLLISION  0x2
#define PARTICLE_FLAG_FADE_OUT   0x4
#define PARTICLE_FLAG_ROTATE     0x8
#define PARTICLE_FLAG_CURL_NOISE   0x10
//==========================================================================
// Emission Module (48 bytes)
//==========================================================================
struct N2D_EmissionModule
{
    float m_emissionRate;
    uint m_numBursts;
    float m_lifetimeMin;
    float m_lifetimeMax;
    
    uint m_emissionType; // 0=Point, 1=Circle, 2=Box
    float m_emissionRadius;
    float m_boxDimensionX;
    float m_boxDimensionY;
    
    uint m_emissionMode; // 0=CONTINUOUS, 1=BURST
    float m_burstInterval;
    int m_burstCycles; // -1 for infinite
    
    float m_depthMin;
    float m_depthMax;
    uint m_padding[3];
};

//==========================================================================
// Motion Module (32 bytes)
//==========================================================================
struct N2D_MotionModule
{
// Basic (16 bytes)
    float2 m_velocityMin;
    float2 m_velocityMax;
    uint m_velocityMode;
    uint m_orientToVelocity;
    float2 m_linearForce;

// Drag (16 bytes)
    float m_drag;
    float m_padding0[3];

// Point Force (32 bytes)
    float m_pointForceStrength;
    //float2 m_pointForcePos;
    float m_pointForceRadius;
    float m_pointForceFalloff;
    uint m_pointForceAttract;
    uint m_enablePointForce;
    float3 m_padding1;

// Vortex Force (32 bytes)
    float m_vortexStrength;
    //float2 m_vortexPos;
    float m_vortexRadius;
    uint m_enableVortex;
    float m_padding2[1];
};

//==========================================================================
// Color Keyframe (32 bytes)
//==========================================================================
struct N2D_ColorKeyframe
{
    float4 m_colorPacked;
    float m_time;
    float m_padding[3];
};

//==========================================================================
// Appearance Module (176 bytes)
//==========================================================================
struct N2D_AppearanceModule
{
    uint m_spriteSheetDimensionsX;
    uint m_spriteSheetDimensionsY;
    uint m_spriteStartIndex;
    uint m_spriteEndIndex;
    float m_animationFPS;
    
    uint m_textureIndex;
    float m_textureUVScaleX;
    float m_textureUVScaleY;
    
    uint m_numColorKeyframes;
    //uint m_padding0[2];
    N2D_ColorKeyframe m_colorKeyframes[8]; 
    float m_emissionStrength;
    uint m_padding1[2];
};

//==========================================================================
// Float Keyframe (16 bytes)
//==========================================================================
struct N2D_FloatKeyframe
{
    float m_value;
    float m_time;
    uint m_easingFunction;
    uint m_padding;
};

//==========================================================================
// Float Curve (144 bytes)
//==========================================================================
struct N2D_FloatCurve
{
    uint m_type;
    uint m_numKeyframes;
    uint m_padding0[2];
    N2D_FloatKeyframe m_keyframes[8];
};

//==========================================================================
// Emitter Properties (16 bytes)
//==========================================================================
struct N2D_EmitterProperties
{
    float m_lifetime;
    float m_startDelay;
    uint m_worldSimulation;
    uint m_padding;
};

//==========================================================================
// Collision Response Type
//==========================================================================
#define N2D_COLLISION_NONE   0
#define N2D_COLLISION_BOUNCE 1
#define N2D_COLLISION_DIE    2
#define N2D_COLLISION_STICK  3
#define N2D_COLLISION_SLOW   4

//==========================================================================
// Collision Rule
// GPU Size 32 bytes
//==========================================================================
struct N2D_CollisionRule
{
    float4 m_targetColor;
    uint m_response;
    float m_bounceDamping;
    float m_slowFactor;
    uint m_maxBounces;
    
    // Total 32 bytes
};

//==========================================================================
// Collision Module
// GPU Size 272 bytes
//==========================================================================
struct N2D_CollisionModule
{
    N2D_CollisionRule m_rules[8];
    uint m_enableCollision;
    uint m_numRules;
    float2 m_padding;    
};

//==========================================================================
// Definition (864 bytes)
//==========================================================================
struct Nova2DEmitterDefinitionGPU
{
    N2D_EmissionModule m_emission;
    N2D_MotionModule m_motion;
    N2D_AppearanceModule m_appearance;
    N2D_EmitterProperties m_properties;
    N2D_CollisionModule m_collision;
    
    uint m_numCurves;
    uint m_padding0[3];
    N2D_FloatCurve m_curves[4];
};

//==========================================================================
// Instance (64 bytes)
//==========================================================================
struct Nova2DEmitterInstanceGPU
{
    // Transform (16 bytes)
    float m_positionX;
    float m_positionY;
    float m_rotation;
    float m_scale;
    
    // Particle Ownership (16 bytes)
    uint m_definitionIndex;
    uint m_particleStartIndex;
    uint m_particleCount;
    uint m_maxParticles;
    
    // Emission State (16 bytes)
    float m_emissionAccumulator;
    float m_elapsedTime;
    uint m_isActive;
    uint m_killFlag;
    
    float m_burstTimer;
    int m_burstCycleCount;
    
    float2 m_pointForceOffset;
    float2 m_vortexOffset;
    
    float2 m_padding;
};

#endif // NOVA2D_STRUCTURES_HLSLI
