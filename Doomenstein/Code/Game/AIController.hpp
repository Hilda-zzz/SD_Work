#pragma once
#include "Controller.hpp"

class Actor;
class Map;

class AIController : public Controller
{
public:
	AIController(Map* map);
	virtual ~AIController();

	void Update(float deltaSeconds);

	void DamagedBy(ActorHandle const& attackerHandle);

	ActorHandle* FindNearestVisibleEnemy();


private:
	ActorHandle m_targetActorHandle = ActorHandle::INVALID;

	float m_sightRadius = 10.0f;       
	float m_sightAngle = 120.0f;       
	float m_meleeAttackRange = 1.0f;   
	float m_targetForgetTime = 5.0f;   
	float m_timeSinceTargetSeen = 0.0f; 

	Actor* m_preTarget = nullptr;
};