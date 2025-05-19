#include "Weapon.hpp"
#include "Actor.hpp"
#include "Map.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/PlayerController.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "FieldObject.hpp"
Weapon::Weapon(int defIndex, Actor* owner) 
	:m_weaponDefIndex(defIndex), m_def(WeaponDefinition::s_weaponDefinitions[defIndex]),m_owner(owner)
{
	m_refireTimer = Timer(m_def.m_refireTime, owner->m_map->m_game->m_gameClock);
}

Weapon::~Weapon()
{
}

bool Weapon::Fire(Vec3 const& dir,PlayerController* curPlayerController)
{
	if (m_refireTimer.IsStopped()||m_refireTimer.GetElapsedTime() > m_refireTimer.m_period)
	{
		//can fire

		if (m_def.m_fireSoundPath != "")
		{
			Vec3 animDirection3 = m_owner->m_position - curPlayerController->m_playerCam.GetPosition();
			Vec2 animDirection2 = Vec2(animDirection3.x, animDirection3.y);
			Mat44 localMat = m_owner->GetModelMat();
			localMat.Transpose();
			localMat.AppendTranslation2D(animDirection2);
			SoundID fireSoundID = g_theAudio->CreateOrGetSound(m_def.m_fireSoundPath, true);
			g_theAudio->StartSoundAt(fireSoundID, localMat.GetTranslation3D(), false);
		}

		for (int i = 0; i < m_def.m_rayCount; i++)
		{
			Vec3 shootDir;
			if (m_def.m_rayCone > 0.01f)
			{
				shootDir = GetRandomDirectionInCone(dir, m_def.m_rayCone);
			}
			else
			{
				shootDir = dir;
			}
			Vec3 rayRenderStart = m_owner->m_position + Vec3(0.f, 0.f, m_owner->m_actorDef->m_eyeHeight - 0.1f);
				//+shootDir.GetNormalized() *0.2f;
			Vec3 rayEnd = rayRenderStart + m_def.m_rayRange * shootDir.GetNormalized();
// 			DebugAddWorldLine(rayRenderStart, rayEnd,
// 				0.01f,1.f,Rgba8::BLUE,Rgba8::WHITE,DebugRenderMode::X_RAY);

			//plant
			if (m_def.m_name == "ManDig")
			{
				// spawn field
				if (m_owner->m_map->RaycastAllFields(rayRenderStart, shootDir.GetNormalized(), m_def.m_rayRange)==nullptr)
				{
					RaycastResult3D raycastResultWithGroud = m_owner->m_map->RaycastWorldZ(rayRenderStart, shootDir.GetNormalized(), m_def.m_rayRange);
					if (raycastResultWithGroud.m_didImpact)
					{
						IntVec2 coords = m_owner->m_map->GetTileCoordsFromPoint(Vec2(raycastResultWithGroud.m_impactPos.x, raycastResultWithGroud.m_impactPos.y));
						std::string tileName = m_owner->m_map->GetTileTypeNameFromCoords(coords.x, coords.y);
						if (tileName == "PlantGrass")
						{
							m_owner->m_map->SpawnPlantField(coords);
							curPlayerController->m_coinCount--;
						}
					}
				}
			}
			else if (m_def.m_name == "ManWater")
			{
				FieldObject* curField = m_owner->m_map->RaycastAllFields(rayRenderStart, shootDir.GetNormalized(), m_def.m_rayRange);
				if(curField)
				{
					curField->PourWater();
				}
			}
			else if (m_def.m_name == "ManBasket")
			{
				FieldObject* curField = m_owner->m_map->RaycastAllFields(rayRenderStart, shootDir.GetNormalized(), m_def.m_rayRange);
				if (curField)
				{
					curField->Harvest(curPlayerController);
				}
			}

			m_owner->m_map->RaycastAll(rayRenderStart, shootDir.GetNormalized(), m_def.m_rayRange,m_owner->m_actorHandle);
		}
		for (int i = 0; i < m_def.m_projectileCount; i++)
		{
			Vec3 shootDir;
			if (m_def.m_projectileCone > 0.01f)
			{
				shootDir = GetRandomDirectionInCone(dir, m_def.m_projectileCone);
			}
			else
			{
				shootDir = dir;
			}

			SpawnInfo projectileInfo = SpawnInfo(m_def.m_projectileActor,m_owner->m_faction, 
				m_owner->m_position + Vec3(0.f, 0.f, m_owner->m_actorDef->m_eyeHeight), m_owner->m_orientation);
			ActorHandle* newProjectileHandle=m_owner->m_map->SpawnProjectile(projectileInfo,m_owner->m_actorHandle);
			Actor* newProjectile = m_owner->m_map->GetActorByHandle(*newProjectileHandle);
			newProjectile->AddImpulse(shootDir * m_def.m_projectileSpeed);
			newProjectile->m_owner = m_owner->m_actorHandle;
		}
		for (int i = 0; i < m_def.m_meleeCount; i++)
		{
			Faction aimFaction;
			if (m_owner->m_faction == Faction::GOOD)
			{
				aimFaction = Faction::EVIL;
			}
			else if (m_owner->m_faction == Faction::EVIL)
			{
				aimFaction = Faction::GOOD;
			}

			ActorHandle* targetActorHandle=m_owner->m_map->GetMeleeTarget(m_owner, aimFaction, m_def.m_meleeRange + m_owner->m_physicCollider.m_radius + 0.2f
				,m_def.m_meleeArc);

			if (targetActorHandle != nullptr)
			{
				Actor* targetActor = m_owner->m_map->GetActorByHandle(*targetActorHandle);
				if (targetActor)
				{
					targetActor->AddImpulse(dir * m_def.m_meleeImpulse);
					float curDamage = m_owner->m_map->m_game->m_rng.RollRandomFloatInRange(m_def.m_meleeDamage.m_min, m_def.m_meleeDamage.m_max);
					PlayerController* listener = curPlayerController;
					targetActor->Damage(curDamage, *m_owner->m_actorHandle,listener);
				}
			}

			
		}
		// start the timer
		m_refireTimer.Stop();
		m_refireTimer.Start();
		return true;
	}
	return false;
}

Vec3 Weapon::GetRandomDirectionInCone(Vec3 const& dir,float coneDegree)
{
	Vec3 fwd=dir.GetNormalized();

	float phi = m_owner->m_map->m_game->m_rng.RollRandomFloatInRange(0.0f, 360.0f);
	float cosTheta = CosDegrees(coneDegree);
	float u = m_owner->m_map->m_game->m_rng.RollRandomFloatInRange(cosTheta, 1.0f);
	float sinTheta = sqrtf(1.0f - u * u);

	Vec3 randomDir(
		sinTheta * CosDegrees(phi),
		sinTheta * SinDegrees(phi),
		u
	);

	Vec3 zAxis = fwd;
	Vec3 xAxis, yAxis;


	if (fabsf(zAxis.z) < 0.999f)
		xAxis = CrossProduct3D(Vec3(0, 0, 1), zAxis).GetNormalized();
	else
		xAxis = CrossProduct3D(Vec3(1, 0, 0), zAxis).GetNormalized();

	yAxis = CrossProduct3D(zAxis, xAxis);

	return xAxis * randomDir.x + yAxis * randomDir.y + zAxis * randomDir.z;
}
