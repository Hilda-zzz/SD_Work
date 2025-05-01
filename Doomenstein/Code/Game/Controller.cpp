#include "Controller.hpp"
#include "Game/Map.hpp"
#include "Game/ActorHandle.hpp"
#include "Actor.hpp"

void Controller::Possess(ActorHandle actorHandle)
{
	if (m_actorHandle.IsValid())
	{
		//un possess previous one 
		Actor* curActor=GetActor();
		if (curActor)
		{
			curActor->OnUnpossessed();
		}
		
	}

	m_actorHandle = actorHandle;
	Actor* curActor = GetActor();
	if (curActor)
	{
		curActor->OnPossessed(this);
	}

}

Actor* Controller::GetActor()
{
	return  m_curMap->GetActorByHandle(m_actorHandle);
}

bool Controller::Attack(Vec3 const& dir)
{
	if (GetActor()->Attack(dir))
	{
		return true;
	}
	return false;
}
