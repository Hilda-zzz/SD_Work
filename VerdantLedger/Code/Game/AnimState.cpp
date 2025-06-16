#include "AnimState.hpp"

AnimState::AnimState(const std::string& name, SpriteAnimDefinition* anim):m_name(name),m_anim(anim)
{
}

void AnimState::Enter()
{
	m_elapsedTime = 0.f;
}

int AnimState::Update(float deltaTime, const std::unordered_map<std::string, bool>& condition)
{
	m_elapsedTime += deltaTime;
	return m_stateEnum;
}

SpriteDefinition const& AnimState::GetCurSprite() const
{
	return m_anim->GetSpriteDefAtTime(m_elapsedTime);
}

bool AnimState::IsFinished() const
{
	return m_anim->IsPlayOnceFinished(m_elapsedTime);
}

std::string AnimState::GetName() const
{
	return m_name;
}
