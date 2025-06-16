#pragma once
#include <string>
#include <unordered_map>
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

class AnimState
{
public:
	AnimState(const std::string& name, SpriteAnimDefinition* anim);
	virtual ~AnimState(){}
	virtual void Enter();
	virtual void Exit() {};
	virtual int Update(float deltaTime, const std::unordered_map<std::string, bool>& condition);
	SpriteDefinition const& GetCurSprite() const;
	bool IsFinished() const;
	std::string GetName() const;

protected:
	std::string m_name;
	SpriteAnimDefinition* m_anim;
	float m_elapsedTime=0.f;
	int m_stateEnum;
};