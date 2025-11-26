#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

//class SpriteDefinition;
class SpriteAnimDefinition;
class SpriteSheet;

//----------------------------------------------------------------------
// Emitter Shape
//----------------------------------------------------------------------
enum class n2d_EmitterShape {
	POINT,
	CIRCLE,
	BOX,
	// CONE,    
	// LINE,    
};

//----------------------------------------------------------------------
// Emission Mode 
//----------------------------------------------------------------------
enum class n2d_EmissionMode {
	CONTINUOUS,           // 持续发射
	BURST,                // 一次性爆发
	// BURST_REPEAT,      // [进阶] 重复爆发
	// EMIT_LIMITED_TIME, // [进阶] 限时发射
};

//----------------------------------------------------------------------
// Simulation Space 
//----------------------------------------------------------------------
enum class n2d_SimulationSpace {
	WORLD,    // 世界空间（粒子发射后独立于发射器）
	LOCAL,    // 局部空间（粒子跟随发射器运动）
};

//==========================================================================
// [Module 1] Emitter Properties
//==========================================================================
struct n2d_EmitterPropertiesConfig {
	// ===== MVP 核心参数 =====
	bool m_hasLifetime = false;              // 发射器是否有生命周期
	float m_emitterLifetime = 5.0f;          // 发射器生命周期（秒）
	float m_startDelay = 0.0f;               // 发射延迟（秒）
	n2d_SimulationSpace m_simulationSpace = n2d_SimulationSpace::WORLD;

	// ===== [进阶] 待实现 =====
	// Vec2 emitterVelocity = Vec2::ZERO;  // 发射器自身速度
	// bool emitSubEmitters = false;       // 是否发射子发射器
	// std::vector<ParticleEmitter*> childEmitters;  // 子发射器列表
};

//==========================================================================
// [Module 2] Emission Module 
//==========================================================================
struct n2d_EmissionConfig {
	// ===== MVP 核心参数 =====
	n2d_EmissionMode m_mode = n2d_EmissionMode::CONTINUOUS;
	float m_emissionRate = 10.0f;            // 粒子/秒（Continuous 模式）
	int m_burstCount = 50;                   // Burst 数量（Burst 模式）

	// Lifetime Range 
	float m_lifetimeMin = 1.0f;              // 最小生命周期（秒）
	float m_lifetimeMax = 2.0f;              // 最大生命周期（秒）

	// Emission Shape 
	n2d_EmitterShape m_shape = n2d_EmitterShape::POINT;
	float m_shapeRadius = 10.0f;             // Circle 半径
	Vec2 m_shapeBoxSize = Vec2(20, 20);      // Box 尺寸

	// ===== [进阶] 待实现 =====
	// bool repeatBurst = false;           // 是否重复 Burst
	// float burstInterval = 1.0f;         // Burst 间隔
	// float coneAngle = 45.0f;            // Cone 角度（如果实现 Cone）
};

//==========================================================================
// [Module 3] Motion Module 
//==========================================================================
struct n2d_MotionConfig {
	// ===== Start Velocity =====
	Vec2 m_startVelocityMin = Vec2(0, 50);   // 最小起始速度
	Vec2 m_startVelocityMax = Vec2(0, 150);  // 最大起始速度

	// ===== 速度控制 =====
	bool m_inheritEmitterVelocity = false;   // 是否继承发射器速度
	float m_maxSpeed = -1.0f;                // 最大速度限制（<0 表示无限制）

	// ===== [进阶] Rotation =====
	// bool orientToVelocity = false;      // 朝向速度方向旋转
	// float rotationSpeed = 0.0f;         // 旋转速度（度/秒）

	// ===== [进阶] Forces - 力场系统（暂时注释） =====
	// bool enableDrag = false;
	// float dragCoefficient = 0.5f;
	// 
	// bool enableLinearForce = false;
	// Vec2 linearForce = Vec2(0, -100);    // 类似重力
	// 
	// bool enablePointForce = false;
	// Vec2 pointForcePosition = Vec2::ZERO;
	// float pointForceStrength = 100.0f;
	// float pointForceRadius = 100.0f;
	// bool pointForceAttract = true;       // true=吸引，false=排斥
	// 
	// bool enableVortexForce = false;
	// Vec2 vortexPosition = Vec2::ZERO;
	// float vortexStrength = 100.0f;
	// float vortexRadius = 100.0f;
	// 
	// bool enableReturnForce = false;
	// float returnForceStrength = 50.0f;
	// float returnForceDelay = 1.0f;

	// ===== [进阶] Curl Noise =====
	// bool enableCurlNoise = false;
	// float curlNoiseStrength = 100.0f;
	// float curlNoiseScale = 0.1f;
	// int curlNoiseOctaves = 3;
};

//==========================================================================
// [Module 4] Appearance Module
//==========================================================================
struct n2d_AppearanceConfig {
	// ===== Size =====
	float m_sizeMin = 2.0f;                  
	float m_sizeMax = 5.0f;

	// ===== Color =====
	Rgba8 m_colorStart = Rgba8::WHITE;       // 起始颜色

	// ===== [进阶] Color Over Lifetime =====
	// Rgba8 colorEnd = Rgba8::WHITE;
	// bool fadeOut = false;                // 淡出效果

	// ===== [进阶] Size Over Lifetime =====
	// float sizeGrowthRate = 0.0f;        // 每秒增长率
	// float sizeShrinkRate = 0.0f;        // 每秒缩小率

	// ===== [进阶] Sprite Sheet Animation =====
	// int spriteSheetWidth = 1;
	// int spriteSheetHeight = 1;
	// int spriteStartIndex = 0;
	// int spriteEndIndex = 0;

	// ===== [进阶] Blending =====
	// float alphaObscurance = 1.0f;       // Alpha vs Additive (1=alpha, 0=additive)
	// float emissiveStrength = 0.0f;      // 自发光强度
};

//==========================================================================
// Emitter Config
//==========================================================================
struct Nova2DEmitterConfig 
{
	Vec2 m_position = Vec2::ZERO;          

	// 4 Modules
	n2d_EmitterPropertiesConfig		m_propertiesConfig;
	n2d_EmissionConfig				m_emissionConfig;
	n2d_MotionConfig				m_motionConfig;
	n2d_AppearanceConfig			m_appearanceConfig;

	// ===== Assets =====
	SpriteSheet const*			m_sprite = nullptr;       
	SpriteAnimDefinition const*		m_animation = nullptr; 

	// ===== Physics =====
	bool	m_enableGravity = true;             
	float	m_gravityScale = 1.0f;             // 重力缩放

	// ===== [进阶] 碰撞检测 =====
	// bool enableCollision = false;
	// float collisionDamping = 0.5f;       // 碰撞阻尼
};

//==========================================================================
// 辅助工具：预设配置生成器
//==========================================================================
namespace EmitterPresets 
{
	// 火焰效果
	inline Nova2DEmitterConfig CreateFireConfig() 
	{
		Nova2DEmitterConfig config;

		// Emission
		config.m_emissionConfig.m_mode = n2d_EmissionMode::CONTINUOUS;
		config.m_emissionConfig.m_emissionRate = 50.0f;
		config.m_emissionConfig.m_lifetimeMin = 0.5f;
		config.m_emissionConfig.m_lifetimeMax = 1.5f;
		config.m_emissionConfig.m_shape = n2d_EmitterShape::CIRCLE;
		config.m_emissionConfig.m_shapeRadius = 10.0f;

		// Motion
		config.m_motionConfig.m_startVelocityMin = Vec2(0, 100);
		config.m_motionConfig.m_startVelocityMax = Vec2(0, 200);

		// Appearance
		config.m_appearanceConfig.m_sizeMin = 3.0f;
		config.m_appearanceConfig.m_sizeMax = 8.0f;
		config.m_appearanceConfig.m_colorStart = Rgba8(255, 200, 50, 255); // 黄橙色

		// Physics
		config.m_enableGravity = false;  // 火焰向上飘

		return config;
	}

	// 烟雾效果
	inline Nova2DEmitterConfig CreateSmokeConfig() 
	{
		Nova2DEmitterConfig config;

		config.m_emissionConfig.m_mode = n2d_EmissionMode::CONTINUOUS;
		config.m_emissionConfig.m_emissionRate = 20.0f;
		config.m_emissionConfig.m_lifetimeMin = 1.0f;
		config.m_emissionConfig.m_lifetimeMax = 3.0f;
		config.m_emissionConfig.m_shape = n2d_EmitterShape::CIRCLE;
		config.m_emissionConfig.m_shapeRadius = 15.0f;

		config.m_motionConfig.m_startVelocityMin = Vec2(0, 30);
		config.m_motionConfig.m_startVelocityMax = Vec2(0, 80);

		config.m_appearanceConfig.m_sizeMin = 5.0f;
		config.m_appearanceConfig.m_sizeMax = 15.0f;
		config.m_appearanceConfig.m_colorStart = Rgba8(100, 100, 100, 150); // 灰色半透明

		config.m_enableGravity = false;

		return config;
	}

	// 爆炸效果（Burst 模式）
	inline Nova2DEmitterConfig CreateExplosionConfig() 
	{
		Nova2DEmitterConfig config;

		config.m_emissionConfig.m_mode = n2d_EmissionMode::BURST;
		config.m_emissionConfig.m_burstCount = 200;
		config.m_emissionConfig.m_lifetimeMin = 0.3f;
		config.m_emissionConfig.m_lifetimeMax = 1.0f;
		config.m_emissionConfig.m_shape = n2d_EmitterShape::POINT;

		config.m_motionConfig.m_startVelocityMin = Vec2(-200, -200);
		config.m_motionConfig.m_startVelocityMax = Vec2(200, 200);
		config.m_motionConfig.m_maxSpeed = 300.0f;

		config.m_appearanceConfig.m_sizeMin = 2.0f;
		config.m_appearanceConfig.m_sizeMax = 6.0f;
		config.m_appearanceConfig.m_colorStart = Rgba8(255, 150, 50, 255);

		config.m_enableGravity = true;
		config.m_gravityScale = 0.5f;

		return config;
	}

	// 火花飞溅
	inline Nova2DEmitterConfig CreateSparkConfig()
	{
		Nova2DEmitterConfig config;

		config.m_emissionConfig.m_mode = n2d_EmissionMode::BURST;
		config.m_emissionConfig.m_burstCount = 50;
		config.m_emissionConfig.m_lifetimeMin = 0.2f;
		config.m_emissionConfig.m_lifetimeMax = 0.8f;
		config.m_emissionConfig.m_shape = n2d_EmitterShape::CIRCLE;
		config.m_emissionConfig.m_shapeRadius = 5.0f;

		config.m_motionConfig.m_startVelocityMin = Vec2(-150, 50);
		config.m_motionConfig.m_startVelocityMax = Vec2(150, 250);

		config.m_appearanceConfig.m_sizeMin = 1.0f;
		config.m_appearanceConfig.m_sizeMax = 3.0f;
		config.m_appearanceConfig.m_colorStart = Rgba8(255, 200, 100, 255);

		config.m_enableGravity = true;

		return config;
	}
}

