#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

enum class Direction
{
	DOWN,
	UP,
	LEFT,
	RIGHT
};


class AnimState
{
public:
	AnimState(std::string const& name, std::map<Direction, SpriteAnimDefinition*> const& anim);
	virtual ~AnimState(){}
	virtual void Enter();
	virtual void Exit() {};
	virtual int Update(float deltaTime, const std::unordered_map<std::string, bool>& condition,Direction curDirection);
	SpriteDefinition const& GetCurSprite();
	bool IsFinished();
	std::string GetName() const;

protected:
	std::string m_name;
	std::map<Direction, SpriteAnimDefinition*> m_directionalAnims;
	Direction m_curDirection;
	float m_elapsedTime=0.f;
	int m_stateEnum = 0;
};