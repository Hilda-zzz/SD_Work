//#pragma once
//#include <stdint.h>
//#include "Engine/Math/Vec2.hpp"
//#include "Engine/Core/Rgba8.hpp"
//
//enum NovaParticleType : uint32_t {
//	NOVA_PARTICLE_DEFAULT = 0,
//	NOVA_PARTICLE_SPARK,
//	NOVA_PARTICLE_SMOKE,
//	NOVA_PARTICLE_BLOOD,
//	NOVA_PARTICLE_DEBRIS,
//	// 游戏层可扩展
//};
//
//enum NovaParticleFlags : uint32_t {
//	NOVA_FLAG_GRAVITY = 1 << 0,  // 受重力影响
//	NOVA_FLAG_COLLISION = 1 << 1,  // 启用碰撞检测
//	NOVA_FLAG_FADE_OUT = 1 << 2,  // 淡出效果
//	NOVA_FLAG_GROW = 1 << 3,  // 尺寸增长
//	NOVA_FLAG_SHRINK = 1 << 4,  // 尺寸缩小
//	NOVA_FLAG_ROTATE = 1 << 5,  // 旋转
//};
//
//struct NovaParticle2D
//{
//	// 粒子数据结构（CPU/GPU共享）
//	Vec2 m_position;
//	Vec2 m_velocity;
//	Rgba8 m_color;
//	float m_lifetime;
//	float m_maxLifetime;
//	float m_size;
//	float m_rotation;
//	NovaParticleType m_type;
//	NovaParticleFlags m_flags;
//
//	bool IsAlive() const { return m_lifetime > 0.0f; }
//
//	float GetLifetimeRatio() const
//	{
//		return m_lifetime / m_maxLifetime;
//	}
//};
