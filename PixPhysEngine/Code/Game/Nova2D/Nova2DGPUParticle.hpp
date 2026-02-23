#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <cstdint>

//==========================================================================
// GPU粒子标志位 (用于控制粒子行为)
//==========================================================================
enum class Nova2DGPUParticleFlags : uint32_t 
{
	NONE = 0,
	GRAVITY = 1 << 0,   // 受重力影响
	COLLISION = 1 << 1,   // 启用碰撞检测
	FADE_OUT = 1 << 2,   // 淡出效果
	ROTATE = 1 << 3,   // 旋转
	CURL_NOISE = 1 << 4,   // 受Curl Noise 影响

	// 预留位（游戏层可用）
	USER_0 = 1 << 16,
	USER_1 = 1 << 17,
	USER_2 = 1 << 18,
};

//==========================================================================
// GPU粒子数据结构
// 注意：必须与 Shader 中的定义**完全一致**
// 注意：必须满足 GPU 对齐要求（16字节对齐）
//==========================================================================
struct Nova2DGPUParticle
{
	// ===== 物理属性 (24 bytes) =====
	Vec2 m_position;          // 8 bytes - 当前位置
	Vec2 m_velocity;          // 8 bytes - 当前速度
	float m_lifetime;         // 4 bytes - 剩余生命
	float m_maxLifetime;      // 4 bytes - 最大生命（用于计算生命比例）

	// ===== 视觉属性 (16 bytes) =====
	float m_size;             // 4 bytes - 粒子尺寸
	float m_rotation;         // 4 bytes - 旋转角度（弧度）
	float m_color[4];            // 4 bytes - 颜色（RGBA）
	uint32_t m_flags;         // 4 bytes - 行为标志位

	// ===== 渲染属性 (8 bytes) =====
	uint32_t m_spriteIndex;   // 2 bytes - 精灵图集索引
	uint32_t m_animFrame;     // 2 bytes - 当前动画帧
	float m_animTime;         // 4 bytes - 动画播放时间

	uint32_t m_instanceID;

	uint32_t textureIndex;  // ← 新增：指向Texture2DArray的索引

	float depth;

	float emission;     // ← 调整padding

	uint32_t collisionState;
	uint32_t bounceCount;
	uint32_t padding[2];
	// ===== 对齐填充 =====
	// 总计：48 bytes（满足16字节对齐）
};

// 编译期检查对齐
static_assert(sizeof(Nova2DGPUParticle) % 16 == 0,
	"GPUParticle2D must be 16-byte aligned for GPU");
//static_assert(sizeof(Nova2DGPUParticle) == 48,
//	"GPUParticle2D size should be 48 bytes");

//==========================================================================
// GPU计数器结构（用于原子操作）
//==========================================================================
struct Nova2DGPUParticleCounters 
{
	uint32_t m_aliveCount;    // 当前存活粒子数
	uint32_t m_deadCount;     // 死亡列表中的粒子数
	uint32_t m_emitCount;     // 本帧需要发射的粒子数
	uint32_t m_aliveCountAfterSim;       // 对齐到16字节
};

static_assert(sizeof(Nova2DGPUParticleCounters) == 16);

//==========================================================================
// GPU发射参数（CPU传给GPU的发射命令）
//==========================================================================
struct Nova2DGPUEmitParams
{
	Vec2 m_emitterPosition;   // 8 bytes
	Vec2 m_velocityMin;       // 8 bytes
	Vec2 m_velocityMax;       // 8 bytes

	float m_lifetimeMin;      // 4 bytes
	float m_lifetimeMax;      // 4 bytes
	float m_sizeMin;          // 4 bytes
	float m_sizeMax;          // 4 bytes

	uint32_t m_shapeType;     // 4 bytes
	float m_shapeParam1;      // 4 bytes
	float m_shapeParam2;      // 4 bytes
	Rgba8 m_color;            // 4 bytes

	uint32_t m_spriteIndex;   // 4 bytes
	uint32_t m_emitCount;     // 4 bytes
	uint32_t m_flags;         // 4 bytes
	uint32_t m_randomSeed;    // 4 bytes

	uint32_t m_maxParticles;  // ✅ 添加这个字段
	uint32_t m_padding[1];    // ✅ 改为3个 = 12 bytes

	// 总计：80 bytes
};

static_assert(sizeof(Nova2DGPUEmitParams) % 16 == 0);