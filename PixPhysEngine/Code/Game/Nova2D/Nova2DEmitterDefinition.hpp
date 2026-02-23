// Nova2DEmitterDefinition.hpp
// GPU粒子系统 - Emitter Definition（发射器定义）
// 参考Matthew论文的Definition/Instance分离架构
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "Engine/Math/Vec2.hpp"

// 前向声明（避免循环依赖）
struct Vec2;
struct Rgba8;

//==========================================================================
// [Module 1] Emission Module（发射模块）
// GPU Size: 48 bytes
//==========================================================================
struct N2D_EmissionModule {
    // ===== Emission Rate (8 bytes) =====
    float m_emissionRate = 10.0f;         // particles/sec（CONTINUOUS模式）
    uint32_t m_numBursts = 1;             // BURST模式粒子数
    
    // ===== Lifetime (8 bytes) =====
    float m_lifetimeMin = 1.0f;
    float m_lifetimeMax = 2.0f;
    
    // ===== Shape (20 bytes) =====
    uint32_t m_emissionType = 0;          // 0=Point, 1=Circle, 2=Box
    float m_emissionRadius = 5.0f;        // Circle半径
    float m_boxDimensionX = 5.0f;         // Box宽度
    float m_boxDimensionY = 5.0f;         // Box高度
    uint32_t m_emissionMode = 0;          // 0=CONTINUOUS, 1=BURST
    
	// ===== BURST Mode Config (8 bytes) =====
	float m_burstInterval = 1.0f;         // 爆发间隔（秒）
	int32_t m_burstCycles = 1;            // 爆发次数（-1=无限循环，1=单次）

	float m_depthMin = 10.0f;   // 默认在Cell前面
	float m_depthMax = 40.0f;   // 可以跨越大范围

    // ===== Padding (12 bytes → 总计48，满足16字节对齐) =====
    uint32_t m_padding[3];
    
    // Total: 48 bytes (16-byte aligned)
};
static_assert(sizeof(N2D_EmissionModule) == 64, "N2D_EmissionModule must be 64 bytes");
static_assert(sizeof(N2D_EmissionModule) % 16 == 0, "Must be 16-byte aligned");

//==========================================================================
// [Module 2] Motion Module（运动模块）
// GPU Size: 32 bytes
//==========================================================================
struct N2D_MotionModule {
	// Basic (16 bytes)
	Vec2 m_velocityMin;
    Vec2 m_velocityMax;
	uint32_t m_velocityMode = 0;
	uint32_t m_orientToVelocity = 0;
	Vec2     m_linearForce = Vec2(0.0f, 0.0f);

	// Drag (16 bytes)
	float    m_drag = 0.0f;
	float    m_padding0[3] = {};

	// Point Force (32 bytes)
	float    m_pointForceStrength = 0.0f;
	//Vec2     m_pointForcePos = Vec2(0.0f, 0.0f);
	float    m_pointForceRadius = 100.0f;
	float    m_pointForceFalloff = 1.0f;
	uint32_t m_pointForceAttract = 1;
	uint32_t m_enablePointForce = 0;
    float    m_padding1[3];

	// Vortex Force (32 bytes)
	float    m_vortexStrength = 0.0f;
	//Vec2     m_vortexPos = Vec2(0.0f, 0.0f);
	float    m_vortexRadius = 100.0f;
	uint32_t m_enableVortex = 0;
	float    m_padding2;
};
static_assert(sizeof(N2D_MotionModule) == 96, "N2D_MotionModule must be 32 bytes");
static_assert(sizeof(N2D_MotionModule) % 16 == 0, "Must be 16-byte aligned");

//==========================================================================
// Color Keyframe（颜色关键帧）
// GPU Size: 16 bytes
//==========================================================================
struct N2D_ColorKeyframe {
    float m_colorPacked[4];               // RGBA packed (Rgba8转换)
    float m_time;                         // 0.0 ~ 1.0（归一化生命周期）
    float m_padding[3];                // 对齐到16字节
    
    // Total: 16 bytes
};
static_assert(sizeof(N2D_ColorKeyframe) == 32, "N2D_ColorKeyframe must be 16 bytes");

//==========================================================================
// [Module 3] Appearance Module（外观模块）
// GPU Size: 176 bytes
//==========================================================================
struct N2D_AppearanceModule {
    // ===== Sprite Sheet (16 bytes) =====
    uint32_t m_spriteSheetDimensionsX = 1;
    uint32_t m_spriteSheetDimensionsY = 1;
    uint32_t m_spriteStartIndex = 0;
    uint32_t m_spriteEndIndex = 0;

    float m_animationFPS = 30.0f;
    uint32_t m_textureIndex = 0;
	float m_textureUVScaleX = 1.0f;  // 原始宽度/POT宽度
	float m_textureUVScaleY = 1.0f;  // 原始高度/POT高度

    // ===== Color Over Lifetime (144 bytes: 16 + 128) =====
    uint32_t m_numColorKeyframes = 1;     // 当前有效关键帧数
    //uint32_t m_padding0[1];               // 对齐到16字节
    N2D_ColorKeyframe m_colorKeyframes[8]; // 最多8个关键帧 = 8×16 = 128 bytes
    
    // ===== Padding (16 bytes → 总计176) =====
    float m_emissionStrength = 20.0f;
    uint32_t m_padding1[2];
    
    // Total: 176 bytes (16-byte aligned)
};
//static_assert(sizeof(N2D_AppearanceModule) == 176, "N2D_AppearanceModule must be 176 bytes");
static_assert(sizeof(N2D_AppearanceModule) % 16 == 0, "Must be 16-byte aligned");

//==========================================================================
// Float Curve Type（曲线类型）
//==========================================================================
enum class N2D_FloatCurveType : uint32_t {
    SIZE = 0,              // 尺寸变化
    ROTATION_SPEED = 1,    // 旋转速度
    ALPHA = 2,             // 透明度
    GRAVITY = 3,           // 重力强度
    // [Future] 预留
    DRAG = 4,
    CURL_NOISE = 5,
};

//==========================================================================
// Float Keyframe（浮点关键帧）
// GPU Size: 16 bytes
//==========================================================================
struct N2D_FloatKeyframe {
    float m_value;                        // 值
    float m_time;                         // 0.0 ~ 1.0
    uint32_t m_easingFunction = 0;        // 0=Linear, 1=EaseIn, 2=EaseOut
    uint32_t m_padding;
    
    // Total: 16 bytes
};
static_assert(sizeof(N2D_FloatKeyframe) == 16, "N2D_FloatKeyframe must be 16 bytes");

//==========================================================================
// Float Curve（浮点曲线）
// GPU Size: 144 bytes
//==========================================================================
struct N2D_FloatCurve {
    uint32_t m_type;                      // N2D_FloatCurveType（作为uint32_t存储）
    uint32_t m_numKeyframes = 1;          // 当前有效关键帧数
    uint32_t m_padding0[2];               // 对齐
    N2D_FloatKeyframe m_keyframes[8];     // 最多8个关键点 = 128 bytes
    
    // Total: 144 bytes (16-byte aligned)
};
static_assert(sizeof(N2D_FloatCurve) == 144, "N2D_FloatCurve must be 144 bytes");
static_assert(sizeof(N2D_FloatCurve) % 16 == 0, "Must be 16-byte aligned");

//==========================================================================
// [Module 4] Emitter Properties（发射器属性）
// GPU Size: 16 bytes
//==========================================================================
struct N2D_EmitterProperties {
    float m_lifetime = -1.0f;             // -1表示无限
    float m_startDelay = 0.0f;
    uint32_t m_worldSimulation = 1;       // 0=Local, 1=World
    uint32_t m_padding;
    
    // Total: 16 bytes
};
static_assert(sizeof(N2D_EmitterProperties) == 16, "N2D_EmitterProperties must be 16 bytes");

//==========================================================================
// Collision Response Type
//==========================================================================
enum class N2D_CollisionResponse : uint32_t {
	NONE = 0,      // 穿透
	BOUNCE = 1,    // 反弹
	DIE = 2,       // 消失
	STICK = 3,     // 粘附（速度归零）
	SLOW = 4,      // 减速（只在进入时触发一次）
};

//==========================================================================
// Collision Rule（单条规则）
// GPU Size: 32 bytes
//==========================================================================
struct N2D_CollisionRule {
	float m_targetColor[4];        // 目标颜色RGBA（归一化0-1）
	uint32_t m_response;           // N2D_CollisionResponse
	float m_bounceDamping;         // 反弹衰减（0.8 = 保留80%速度）
	float m_slowFactor;            // 减速因子（0.5 = 速度变为50%）
	uint32_t m_maxBounces;

	// Total: 32 bytes
};
static_assert(sizeof(N2D_CollisionRule) == 32, "Must be 32 bytes");

//==========================================================================
// Collision Module（碰撞模块）
// GPU Size: 272 bytes
//==========================================================================
struct N2D_CollisionModule {
    N2D_CollisionRule m_rules[8];  // 最多8条规则 = 8×32 = 256 bytes
	uint32_t m_enableCollision;    // 0=关闭, 1=开启
	uint32_t m_numRules;           // 当前规则数（0-8）
	uint32_t m_padding[2];
	// Total: 16 + 256 = 272 bytes
};
static_assert(sizeof(N2D_CollisionModule) == 272, "Must be 272 bytes");
static_assert(sizeof(N2D_CollisionModule) % 16 == 0, "Must be 16-byte aligned");

//==========================================================================
// GPU Definition（完整定义）
// GPU Size: 880 bytes
//==========================================================================
struct Nova2DEmitterDefinitionGPU {
    // ===== 核心模块 (272 bytes) =====
    N2D_EmissionModule m_emission;        // 48 bytes
    N2D_MotionModule m_motion;            // 32 bytes
    N2D_AppearanceModule m_appearance;    // 176 bytes
    N2D_EmitterProperties m_properties;   // 16 bytes
    N2D_CollisionModule m_collision;
    // ===== Curves（最多支持4条曲线，592 bytes） =====
    uint32_t m_numCurves = 0;             // 当前有效曲线数
    uint32_t m_padding0[3];               // 对齐
    N2D_FloatCurve m_curves[4];           // 4 × 144 = 576 bytes
    
    // Total: 48+32+176+16+16+576 = 864 bytes
    // Aligned: 864 bytes (already 16-byte aligned)
};
static_assert(sizeof(Nova2DEmitterDefinitionGPU) % 16 == 0, 
              "Nova2DEmitterDefinitionGPU must be 16-byte aligned");

//==========================================================================
// CPU Definition Manager（CPU封装类）
//==========================================================================
class Nova2DEmitterDefinition {
public:
    Nova2DEmitterDefinition();
    ~Nova2DEmitterDefinition();
    
    // ===== ID管理 =====
    uint32_t GetID() const { return m_definitionID; }
    void SetID(uint32_t id) { m_definitionID = id; }
    
    // ===== Dirty Flag =====
    bool IsDirty() const { return m_isDirty; }
    void MarkDirty() { m_isDirty = true; }
    void ClearDirty() { m_isDirty = false; }
    
    // ===== 数据访问 =====
    Nova2DEmitterDefinitionGPU& GetGPUData() { MarkDirty(); return m_gpuData; }
    Nova2DEmitterDefinitionGPU const& GetGPUDataReadOnly() const { return m_gpuData; }
    
    // ===== 便捷修改API =====
    void SetEmissionRate(float rate);
    void SetLifetimeRange(float min, float max);
    void SetVelocityRange(float minX, float minY, float maxX, float maxY);
    void SetEmissionShape(uint32_t type, float param1, float param2 = 0.0f);
    void SetLinearForce(Vec2 const force);
    // Color Over Lifetime
    void ClearColorKeyframes();
    void AddColorKeyframe(Rgba8 color, float time);
    
    // Float Curves
    void AddFloatCurve(N2D_FloatCurveType type, float constantValue);
    void AddFloatCurveKeyframe(uint32_t curveIndex, float value, float time);

	// BURST Mode Config
	void SetBurstConfig(uint32_t numBursts, float interval, int32_t cycles);

	// Sprite Sheet Config
	void SetStaticSprite();
	void SetSpriteSheet(uint32_t dimX, uint32_t dimY, 
        uint32_t startIdx, uint32_t endIdx,float fps = 12.0f);

	// Texture Config
	void SetTexturePath(const char* path);
	std::string const& GetTexturePath() const { return m_texturePath; }
    
	// Collision
	void EnableCollision(bool enable);
	void AddCollisionRule(Rgba8 targetColor, N2D_CollisionResponse response,
		float bounceDamping = 0.8f, float slowFactor = 0.5f, uint32_t maxBounces=0);
	void ClearCollisionRules();

    // Orient to velocity
    void SetOrientToVelocity(bool enable);

	void SetPointForce(float radius, float strength, float falloff, bool attract);
	void SetVortexForce(float radius, float strength);
	void EnablePointForce(bool enable);
	void EnableVortex(bool enable);
private:
    uint32_t m_definitionID = 0;
    bool m_isDirty = true;
    Nova2DEmitterDefinitionGPU m_gpuData;

    std::string m_texturePath = "Data/Images/Particles/ENGINE_DEFAULT_WHITE.png";
};

//==========================================================================
// Size Report（编译期大小报告）
//==========================================================================
// EmissionModule:    48 bytes
// MotionModule:      32 bytes
// AppearanceModule: 160 bytes
// EmitterProperties: 16 bytes
// FloatCurve:       144 bytes
// Total Definition: ~864 bytes
//==========================================================================
