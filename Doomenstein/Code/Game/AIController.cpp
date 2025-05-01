#include "AIController.hpp"
#include "ActorDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Weapon.hpp"
AIController::AIController(Map* map) : Controller(map)
{
}

AIController::~AIController()
{
}

void AIController::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Actor* selfActor = GetActor();
	if (selfActor)
	{
		if (m_targetActorHandle == ActorHandle::INVALID)
		{
			ActorHandle* targetHandle = FindNearestVisibleEnemy();
			if (targetHandle && *targetHandle != ActorHandle::INVALID)
			{
				m_targetActorHandle = *targetHandle;
			}
		}
		if (m_targetActorHandle != ActorHandle::INVALID)
		{
			Actor* target = m_curMap->GetActorByHandle(m_targetActorHandle);
			if (m_preTarget == nullptr && target)
			{
				selfActor->PlayAnimation("Walk");
			}
			if (target)
			{
				Vec3 dir = (target->m_position - selfActor->m_position).GetNormalized();
				if (GetDistanceSquared3D(selfActor->m_position, target->m_position) > 0.37f)
				{
					if (selfActor->m_animState != AnimState::PLAYING)
					{
						selfActor->PlayAnimation("Walk");
					}
					selfActor->MoveInDirection(dir, selfActor->m_actorDef->m_runSpeed);
				}

				selfActor->TurnInDirection(dir, selfActor->m_actorDef->m_turnSpeed);

				if (selfActor->m_weapon)
				{
					ActorHandle* targetActorHandle = selfActor->m_map->GetMeleeTarget(selfActor, Faction::GOOD, selfActor->m_weapon->m_def.m_meleeRange + 0.2f, selfActor->m_weapon->m_def.m_meleeArc );
					if (targetActorHandle)
					{
						selfActor->Attack(selfActor->m_orientation.GetForward_IFwd());
					}
					
				}
			}
			else
			{
				m_targetActorHandle = ActorHandle::INVALID;
			}
			m_preTarget = target;

		}

	}

}

void AIController::DamagedBy( ActorHandle const& attackerHandle)
{
	if(attackerHandle!=m_targetActorHandle)
	{
		m_targetActorHandle = attackerHandle;
	}
}

ActorHandle* AIController::FindNearestVisibleEnemy()
{
	Actor* controlledActor = GetActor();
	if (!controlledActor)
		return nullptr;

	Faction myFaction = controlledActor->m_faction;
	Vec3 myPosition = controlledActor->m_position;
	//float closestDistanceSq = m_sightRadius * m_sightRadius;

	Faction aimFaction = Faction::NEUTRAL;
	if (myFaction == Faction::GOOD)
	{
		aimFaction = Faction::EVIL;
	}
	else if (myFaction == Faction::EVIL)
	{
		aimFaction = Faction::GOOD;
	}

	return m_curMap->GetClosestVisibleEnemy(controlledActor, aimFaction);
}
