#include "AnimState.hpp"

AnimState::AnimState(std::string const& name, std::map<Direction, SpriteAnimDefinition*> const& anim)
	:m_name(name), m_directionalAnims(anim),m_curDirection(Direction::DOWN),m_stateEnum(0)
{
}

void AnimState::Enter()
{
	m_elapsedTime = 0.f;
}

int AnimState::Update(float deltaTime, const std::unordered_map<std::string, bool>& condition, Direction curDirection)
{
	m_elapsedTime += deltaTime;
	m_curDirection = curDirection;
	return m_stateEnum;
}

SpriteDefinition const& AnimState::GetCurSprite()
{
	return m_directionalAnims[m_curDirection]->GetSpriteDefAtTime(m_elapsedTime);
}

bool AnimState::IsFinished()
{
	return m_directionalAnims[m_curDirection]->IsPlayOnceFinished(m_elapsedTime);
}

std::string AnimState::GetName() const
{
	return m_name;
}
