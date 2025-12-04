#pragma once
#include <stdint.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <Engine/Renderer/Texture.hpp>
#include "Engine/Math/AABB2.hpp"

class SpriteDefinition;
class SpriteAnimDefinition;
class Texture;

enum class Nova2DParticleFlags : uint32_t {
	// ===== 物理标志 =====
	NOVA_FLAG_GRAVITY = 1 << 0,   // 受重力影响
	NOVA_FLAG_COLLISION = 1 << 1,   // 启用碰撞检测

	// ===== 视觉标志 =====
	NOVA_FLAG_FADE_OUT = 1 << 2,   // 淡出效果
	NOVA_FLAG_GROW = 1 << 3,   // 尺寸增长
	NOVA_FLAG_SHRINK = 1 << 4,   // 尺寸缩小
	NOVA_FLAG_ROTATE = 1 << 5,   // 旋转

	// ===== 预留位（游戏层可用）=====
	NOVA_FLAG_USER_0 = 1 << 16,  // 用户自定义标志 0
	NOVA_FLAG_USER_1 = 1 << 17,  // 用户自定义标志 1
	NOVA_FLAG_USER_2 = 1 << 18,  // 用户自定义标志 2
	// ... 可扩展到 1 << 31
};

// ===== 位运算操作符重载 =====
inline Nova2DParticleFlags operator|(Nova2DParticleFlags a, Nova2DParticleFlags b) {
	return static_cast<Nova2DParticleFlags>(
		static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
		);
}

inline Nova2DParticleFlags operator&(Nova2DParticleFlags a, Nova2DParticleFlags b) {
	return static_cast<Nova2DParticleFlags>(
		static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
		);
}

inline Nova2DParticleFlags operator~(Nova2DParticleFlags a) {
	return static_cast<Nova2DParticleFlags>(~static_cast<uint32_t>(a));
}

inline Nova2DParticleFlags& operator|=(Nova2DParticleFlags& a, Nova2DParticleFlags b) {
	a = a | b;
	return a;
}

inline Nova2DParticleFlags& operator&=(Nova2DParticleFlags& a, Nova2DParticleFlags b) {
	a = a & b;
	return a;
}

struct Nova2DParticle
{
	// 粒子数据结构（CPU/GPU共享）
	Vec2 m_position;
	Vec2 m_velocity;
	Rgba8 m_color;
	float m_lifetime;
	float m_maxLifetime;
	float m_size;
	float m_rotation;

	// Assets
	SpriteDefinition const* m_spriteDef = nullptr;
	SpriteAnimDefinition const* m_animDef = nullptr;
	float m_animStartTime = 0.0f;

	// ===== 标志位（控制行为）=====
	uint32_t m_flags;
	// ===== 用户扩展数据（游戏层可用）=====
	void* m_userData = nullptr;  // 指向游戏层自定义数据
	uint32_t m_userInt = 0;      // 快速整数数据（如类型 ID）

	// ------------------------------------------------------------

	bool IsAlive() const { return m_lifetime > 0.0f; }

	float GetLifetimeRatio() const
	{
		return m_lifetime / m_maxLifetime;
	}

	float GetPassedTime() const
	{
		return m_maxLifetime - m_lifetime;
	}

	AABB2 GetCurrentUVs(float currentTime) const;
	Texture* GetTexture() const;

	// 标志位操作
	bool HasFlag(Nova2DParticleFlags flag) const;

	void SetFlag(Nova2DParticleFlags flag, bool value);
};
