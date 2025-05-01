#pragma once
#include "ActorHandle.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
class Map;
class Actor;

class Controller
{
public:
	Controller(Map* map) :m_curMap(map)
	{

	}
	~Controller() {};

	virtual void Update(float deltaSeconds) { UNUSED(deltaSeconds); };

	void Possess(ActorHandle actorHandle);
	Actor* GetActor();

	bool Attack(Vec3 const& dir);

	ActorHandle m_actorHandle = ActorHandle::INVALID;
	Map* m_curMap = nullptr;

	int m_kills = 0;
	int m_death = 0;
};