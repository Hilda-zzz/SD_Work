#include "Nova2DParticle.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

AABB2 Nova2DParticle::GetCurrentUVs(float currentTime) const
{
	if (m_anim)
	{
		// 动画模式：根据经过的时间获取当前帧
		float animTime = currentTime - m_animStartTime;
		SpriteDefinition const& frame = m_anim->GetSpriteDefAtTime(animTime);
		return frame.GetUVs();
	}
	else if (m_sprite)
	{
		// 静态精灵模式
		return m_sprite->GetSpriteUVs(0);
	}
	// 默认：全纹理
	return AABB2(Vec2::ZERO, Vec2::ONE);
}

Texture* Nova2DParticle::GetTexture() const
{
	if (m_anim) 
	{
		return &m_anim->GetSpriteDefAtTime(0.0f).GetTexture();
	}
	else if (m_sprite) 
	{
		return &m_sprite->GetTexture();
	}
	return nullptr;
}

bool Nova2DParticle::HasFlag(Nova2DParticleFlags flag) const
{
	return (m_flags & static_cast<uint32_t>(flag)) != 0;
}

void Nova2DParticle::SetFlag(Nova2DParticleFlags flag, bool value)
{
	uint32_t flagBit = static_cast<uint32_t>(flag);
	if (value) {
		m_flags |= flagBit;  // 设置位
	}
	else {
		m_flags &= ~flagBit; // 清除位
	}
}

