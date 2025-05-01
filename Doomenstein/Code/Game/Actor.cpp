#include "Actor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Renderer/Renderer.hpp>
#include "ActorDefinition.hpp"
#include "PlayerController.hpp"
#include "AIController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Weapon.hpp"
#include "WeaponDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"
#include "GameCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"

extern Renderer* g_theRenderer;

Actor::Actor(int actorDefIndex, Map* curMap, Vec3 const& positon, EulerAngles const& orientaion, bool isProjectile)
{
	if (isProjectile)
	{
		m_actorDef = ActorDefinition::s_projectileActorDefinitions[actorDefIndex];
	}
	else
	{
		m_actorDef = ActorDefinition::s_actorDefinitions[actorDefIndex];
	}
	m_map = curMap;
	m_position = positon;
	m_orientation = orientaion;

	m_animClock = new Clock(*m_map->m_game->m_gameClock);

	InitializeFromDef();
}

Actor::~Actor()
{
	delete m_actorHandle;
	m_actorHandle = nullptr;

	for (Weapon* weapon : m_inventory)
	{
		delete weapon;
		weapon = nullptr;
	}

	if (m_aiController)
	{
		delete m_aiController;
		m_aiController = nullptr;
	}

	m_animClock->m_parent->RemoveChild(m_animClock);
	delete m_animClock;
	m_animClock = nullptr;
}

void Actor::InitializeFromDef()
{
	m_curHealth = (float)m_actorDef->m_health;
	m_isVisible = m_actorDef->m_isVisible;
	m_deadTimer = Timer(m_actorDef->m_corpseLifetime,m_map->m_game->m_gameClock);
	//collider
	m_physicCollider.m_center = Vec3(0.f, 0.f, 0.5f * m_actorDef->m_physicsHeight);
	m_physicCollider.m_halfHeight = 0.5f * m_actorDef->m_physicsHeight;
	m_physicCollider.m_radius = m_actorDef->m_physicsRadius;

	//add collider verts
	if (m_actorDef->m_isVisible)
	{
		if (m_actorDef->m_name == "Marine")
		{
			m_color = Rgba8::GREEN;
		}
		else if (m_actorDef->m_name == "Demon")
		{
			m_color = Rgba8::RED;
		}
		else
		{
			m_color = Rgba8::BLUE;
		}
// 		AddVertsForCylinder3D(m_vertexs, m_physicCollider.m_center - Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
// 			m_physicCollider.m_center + Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
// 			m_physicCollider.m_radius, m_color, AABB2::ZERO_TO_ONE, 16);
// 
// 		AddVertsForCylinder3D(m_vertexsWire, m_physicCollider.m_center - Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
// 			m_physicCollider.m_center + Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
// 			m_physicCollider.m_radius + 0.001f, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);
// 
// 		if (m_actorDef->m_name == "Marine" || m_actorDef->m_name == "Demon")
// 		{
// 			float eyeHeight = m_actorDef->m_eyeHeight;
// 			AddVertsForCone3D(m_vertexs, Vec3(m_physicCollider.m_radius, 0.f, eyeHeight),
// 				Vec3(m_physicCollider.m_radius, 0.f, eyeHeight) + Vec3(1.f, 0.f, 0.f) * 0.1f,
// 				0.1f, m_color, AABB2::ZERO_TO_ONE, 16);
// 
// 			AddVertsForCone3D(m_vertexsWire, Vec3(m_physicCollider.m_radius, 0.f, eyeHeight),
// 				Vec3(m_physicCollider.m_radius, 0.f, eyeHeight) + Vec3(1.f, 0.f, 0.f) * 0.101f,
// 				0.101f, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);
// 		}
	}

	m_faction = m_actorDef->m_faction;
	
	if (m_actorDef->m_aiEnabled)
	{
		m_aiController = new AIController(m_map);
		m_controller = m_aiController;
	}

	// weapon
	m_curWeaponIndex = 0;
	for (int i = 0; i < (int)m_actorDef->m_weaponName.size(); i++)
	{
		// create weapon
		for (int weaponIndex = 0; weaponIndex < (int)WeaponDefinition::s_weaponDefinitions.size(); weaponIndex++)
		{
			if (WeaponDefinition::s_weaponDefinitions[weaponIndex].m_name == m_actorDef->m_weaponName[i])
			{
				Weapon* newWeapon = new Weapon(weaponIndex,this);
				m_inventory.push_back(newWeapon);
			}
		}
	}
	EquipWeapon(m_curWeaponIndex);

	// add sprite animation
	if (m_actorDef->m_animGroupDef.size() > 0)
	{
		m_animTimer = Timer(m_actorDef->m_animGroupDef[0].m_animSet[0].m_anim.GetAnimTime(), m_animClock);
		//m_animTimer.Start();
	}

// 	if (m_weapon && m_weapon->m_def.m_animSheet.size()!=0)
// 	{
// 		auto animDef = m_weapon->m_def.m_animDefs.find("Idle");
// 		if (animDef != m_weapon->m_def.m_animDefs.end())
// 		{
// 			SpriteAnimDefinition* animIdleDef = animDef->second;
// 			m_weaponAnimTimer = Timer(animIdleDef->GetAnimTime(), m_animClock);
// 		}
// 	}
}

void Actor::Update(float deltaSeconds)
{
	if (m_deadTimer.GetElapsedTime() > m_deadTimer.m_period)
	{
		if (m_actorDef->m_spriteSheet&&m_animTimer.GetElapsedFraction() > 1.f)
		{
			m_isDestroy = true;
			m_deadTimer.Stop();
			return;
		}
		else
		{
			m_isDestroy = true;
			m_deadTimer.Stop();
			return;
		}
	}
	if (m_controller&&m_controller == m_aiController && !m_isDead)
	{
		m_controller->Update(deltaSeconds);
	}
	//UpdateAnimation(deltaSeconds);
}
void Actor::UpdateAnimation(PlayerController* curPlayer)
{
	if (!m_actorDef->m_spriteSheet)
		return;

	m_vertexsSprite.clear();
	m_vertexsSpriteNorm.clear();

	if (m_animState != AnimState::FINISHED)
	{
		if (m_curAnimGroupIndex != m_nextAnimGroupIndex)
		{
			m_animTimer.Stop();
			m_curAnimGroupIndex = m_nextAnimGroupIndex;
			m_animTimer.m_period = m_actorDef->m_animGroupDef[m_curAnimGroupIndex].m_animSet[0].m_anim.GetAnimTime();

			if (m_animState != AnimState::IDLE)
			{
				m_animTimer.Start();
				m_animState = AnimState::PLAYING;
			}
		}
	}

	switch (m_animState)
	{
	case AnimState::IDLE:
		break;

	case AnimState::PLAYING:
		if (m_animTimer.GetElapsedFraction() > 1.f)
		{
			if (m_actorDef->m_animGroupDef[m_curAnimGroupIndex].m_playbackMode == SpriteAnimPlaybackType::LOOP)
			{
				m_animTimer.Stop();
				m_animTimer.Start();
			}
			else
			{
				m_animState = AnimState::FINISHED;
				m_nextAnimGroupIndex =0; 
				m_animTimer.Stop();
			}
		}
		break;

	case AnimState::FINISHED:
		if (m_curAnimGroupIndex != m_nextAnimGroupIndex)
		{
			m_curAnimGroupIndex = m_nextAnimGroupIndex;
			m_animState = AnimState::IDLE;
		}
		break;
	}

	float curTime;
	if (m_animState == AnimState::IDLE)
	{
		curTime = 0.f;
	}
	else if (m_animTimer.IsStopped())
	{
		curTime = 0.f;
	}
	else
	{
		curTime = (float)m_animTimer.GetElapsedTime();
	}


	Vec3 animDirection3 = m_position - curPlayer->m_playerCam.GetPosition();
	Vec2 animDirection2 = Vec2(animDirection3.x, animDirection3.y);

	Mat44 localMat=GetModelMat();
	localMat.Transpose();
	localMat.AppendTranslation2D(animDirection2);

	SpriteAnimationGroupDefinition const& curAnimGroup = m_actorDef->m_animGroupDef[m_curAnimGroupIndex];

	float dpResult = FLT_MIN;
	int setIndex = 0;
	for (int i = 0; i < (int)curAnimGroup.m_animSet.size(); i++)
	{
		float curDp = DotProduct2D(localMat.GetTranslation2D(), Vec2(curAnimGroup.m_animSet[i].m_direction.x, curAnimGroup.m_animSet[i].m_direction.y));
		if (curDp> dpResult)
		{
			dpResult = curDp;
			setIndex = i;
		}
	}

	SpriteDefinition spriteDef = m_actorDef->m_animGroupDef[m_curAnimGroupIndex].m_animSet[setIndex].m_anim.GetSpriteDefAtTime(curTime);

 	if (m_actorDef->m_animGroupDef[m_curAnimGroupIndex].m_scaleBySpeed)
 	{
		float curSpeed = m_velocity.GetLength();
		float speedRatio = curSpeed / m_actorDef->m_runSpeed;
		if (curSpeed > 1.f)
		{
			m_animClock->SetTimeScale(speedRatio);
		}
 		
 	}
 	else
 	{
 		m_animClock->SetTimeScale(1.f);
 	}

	if (m_actorDef->m_isRenderLit)
	{
		if (m_actorDef->m_isRenderRounded)
		{
			AddVertsForRoundedQuad3D(m_vertexsSpriteNorm, Vec3(0.f, 0.f, 0.f), Vec3(0.f, m_actorDef->m_spriteSize.x, 0.f),
				Vec3(0.f, m_actorDef->m_spriteSize.x, m_actorDef->m_spriteSize.y), Vec3(0.f, 0.f, m_actorDef->m_spriteSize.y),
				Rgba8::WHITE, spriteDef.GetUVs());
		}
		else
		{
			AddVertsForQuad3D_WithTBN(m_vertexsSpriteNorm, Vec3(0.f, 0.f, 0.f), Vec3(0.f, m_actorDef->m_spriteSize.x, 0.f),
				Vec3(0.f, m_actorDef->m_spriteSize.x, m_actorDef->m_spriteSize.y), Vec3(0.f, 0.f, m_actorDef->m_spriteSize.y),
				Rgba8::WHITE, spriteDef.GetUVs());
		}
	}
	else
	{
		AddVertsForQuad3D(m_vertexsSprite, Vec3(0.f, 0.f, 0.f), Vec3(0.f, m_actorDef->m_spriteSize.x, 0.f),
			Vec3(0.f, m_actorDef->m_spriteSize.x, m_actorDef->m_spriteSize.y), Vec3(0.f, 0.f, m_actorDef->m_spriteSize.y),
			Rgba8::WHITE, spriteDef.GetUVs());
	}
}

void Actor::UpdatePhysics(float deltaSeconds)
{
	AddForce(m_actorDef->m_drag* (-m_velocity));
	m_velocity += m_acceleration * deltaSeconds;
	if (!m_actorDef->m_flying)
	{
		m_velocity.z = 0.f;
	}
	m_position += m_velocity * deltaSeconds;
	m_acceleration = Vec3(0.0f, 0.0f, 0.0f);
}

void Actor::AddForce(Vec3 const& force)
{
	m_acceleration += force;
}

void Actor::AddImpulse(Vec3 const& impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollideOtherActor(ActorHandle* source)
{
	// source add impulse
	Actor* sourceActor=m_map->GetActorByHandle(*source);
	AddImpulse(sourceActor->m_actorDef->m_impulseOnCollide * sourceActor->m_orientation.GetForward_IFwd());
	//source cause damage
	if (m_faction != sourceActor->m_faction && m_faction != Faction::NEUTRAL)
	{
		float randomDamage = m_map->m_game->m_rng.RollRandomFloatInRange(sourceActor->m_actorDef->m_damageOnCollide.m_min,
			sourceActor->m_actorDef->m_damageOnCollide.m_max);
		if(randomDamage>0.1f)
		{
			PlayerController* listener = GetListener();
			if (sourceActor->m_isProjectile)
			{
				Damage(randomDamage, *sourceActor->m_owner,listener);
			}
			else
			{
				Damage(randomDamage, *source,listener);
			}
		}

		
	}
	// if dead on collide
	if (m_actorDef->m_dieOnCollide)
	{
		Die(GetListener());
	}
}

void Actor::OnCollideWorld()
{
	if (m_actorDef->m_dieOnCollide)
	{
		Die(GetListener());
	}
}

void Actor::OnCollideRay(ActorHandle* source,Vec3 const& rayDirection)
{
	PlayerController* listener = GetListener();
	// source add impulse
	Actor* sourceActor = m_map->GetActorByHandle(*source);
	AddImpulse(sourceActor->m_weapon->m_def.m_rayImpulse * rayDirection);
	//source cause damage
	float randomDamage = m_map->m_game->m_rng.RollRandomFloatInRange(sourceActor->m_weapon->m_def.m_rayDamage.m_min,
		sourceActor->m_weapon->m_def.m_rayDamage.m_max);
	Damage(randomDamage, *source, listener);
}

void Actor::MoveInDirection(Vec3 const& direction, float speed)
{
	Vec3 targetVelocity = direction * speed;
	Vec3 forceNeeded = targetVelocity * m_actorDef->m_drag;
	AddForce(forceNeeded);
}

void Actor::TurnInDirection(Vec3 const& direction, float maxDeg)
{
	float curDeg = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, 
		Vec2(direction.x, direction.y).GetOrientationDegrees(), maxDeg);
	m_orientation.m_yawDegrees = curDeg;
}

void Actor::Render(PlayerController* curPlayer) const
{
	Mat44 modelMatt = GetModelMat();
	Rgba8 tintColor = Rgba8::WHITE;
// 	if (m_isDead)
// 	{
// 		tintColor = Rgba8(100,100,100);
// 	}
	g_theRenderer->SetModelConstants(modelMatt, tintColor);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	//g_theRenderer->DrawVertexArray(m_vertexs);

	g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
	g_theRenderer->DrawVertexArray(m_vertexsWire);

	Vec2 relativePos = -Vec2(m_actorDef->m_spritePivot.x * m_actorDef->m_spriteSize.x,
		m_actorDef->m_spritePivot.y * m_actorDef->m_spriteSize.y);
	Mat44 relativeMatt= Mat44::MakeTranslation3D(Vec3(0.f, relativePos.x, relativePos.y));

	Mat44 billboardMatrix = Mat44();
	switch (m_actorDef->m_billboardType)
	{
	case BillboardType::NONE:
		billboardMatrix = modelMatt;
		break;
	case BillboardType::WORLD_UP_FACING:
		billboardMatrix = GetBillboardMatrix(BillboardType::WORLD_UP_FACING,
			curPlayer->m_playerCam.GetOrientation().GetAsMatrix_IFwd_JLeft_KUp(),
			curPlayer->m_playerCam.GetPosition(),
			m_position);
		break;
	case BillboardType::WORLD_UP_OPPOSING:
		billboardMatrix = GetBillboardMatrix(BillboardType::WORLD_UP_OPPOSING,
			curPlayer->m_playerCam.GetOrientation().GetAsMatrix_IFwd_JLeft_KUp(),
			curPlayer->m_playerCam.GetPosition(),
			m_position);
		break;
	case BillboardType::FULL_OPPOSING:
		billboardMatrix = GetBillboardMatrix(BillboardType::FULL_OPPOSING,
			curPlayer->m_playerCam.GetOrientation().GetAsMatrix_IFwd_JLeft_KUp(),
			curPlayer->m_playerCam.GetPosition(),
			m_position);
		break;
	}

	billboardMatrix.Append(relativeMatt);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(&m_actorDef->m_spriteSheet->GetTexture());
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants(billboardMatrix, tintColor);
	g_theRenderer->DrawVertexArray(m_vertexsSprite);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(&m_actorDef->m_spriteSheet->GetTexture());
	g_theRenderer->BindShader(m_actorDef->m_shader);
	g_theRenderer->SetModelConstants(billboardMatrix, tintColor);
	g_theRenderer->DrawVertexArray_WithTBN(m_vertexsSpriteNorm);

}

Mat44 Actor::GetModelMat() const
{
	Mat44 translation = Mat44::MakeTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	translation.Append(orientation);
	return translation;
}

void Actor::OnPossessed(Controller* newController)
{
	m_controller = newController;
}

void Actor::OnUnpossessed()
{
	if (m_aiController)
	{
		m_controller = m_aiController;
	}
	else
	{
		m_controller = nullptr;
	}
}

void Actor::Damage(float damage, ActorHandle const& sourceHandle,PlayerController* curPlayerController)
{
	m_curHealth -= damage;

	PlayAnimation("Hurt");

	Vec3 animDirection3 = m_position - curPlayerController->m_playerCam.GetPosition();
	Vec2 animDirection2 = Vec2(animDirection3.x, animDirection3.y);
	Mat44 localMat = GetModelMat();
	localMat.Transpose();
	localMat.AppendTranslation2D(animDirection2);
	SoundID hurtSoundID=g_theAudio->CreateOrGetSound(m_actorDef->m_hurtSoundPath,true);
	g_theAudio->StartSoundAt(hurtSoundID,localMat.GetTranslation3D(),false);

	if (m_curHealth <= 0.f&&!m_isDead)
	{
		if (m_map->GetActorByHandle(sourceHandle)->m_controller)
		{
			if (m_actorDef->m_name=="Marine")
			{
				m_map->GetActorByHandle(sourceHandle)->m_controller->m_kills++;
			}
		}
		
		Die(curPlayerController);
	}

 	if (m_controller==m_aiController&&m_controller&&m_aiController)
 	{
		m_aiController->DamagedBy(sourceHandle);
 	}
}

void Actor::Die(PlayerController* curPlayerController)
{
	m_isDead = true;
	m_deadTimer.Start();
	PlayAnimation("Death");
	if (m_controller)
	{
		m_controller->m_death++;
	}
	
	if (m_actorDef->m_deathSoundPath != "")
	{
		Vec3 animDirection3 = m_position - curPlayerController->m_playerCam.GetPosition();
		Vec2 animDirection2 = Vec2(animDirection3.x, animDirection3.y);
		Mat44 localMat = GetModelMat();
		localMat.Transpose();
		localMat.AppendTranslation2D(animDirection2);
		SoundID deathSoundID = g_theAudio->CreateOrGetSound(m_actorDef->m_deathSoundPath, true);
		g_theAudio->StartSoundAt(deathSoundID, localMat.GetTranslation3D(), false);
	}

}

bool Actor::Attack(Vec3 const& dir)
{
	if (m_weapon->Fire(dir, GetListener()))
	{
		PlayAnimation("Attack");
		return true;
	}
	return false;
}

PlayerController* Actor::GetListener()
{
	if (m_map->m_game->m_playerController0)
	{
		return m_map->m_game->m_playerController0;
	}
	else
	{
		return m_map->m_game->m_playerController1;
	}
}

void Actor::EquipWeapon(int weaponIndex)
{
	if ((int)m_inventory.size()>0&&weaponIndex >= 0 && weaponIndex <= (int)m_inventory.size() - 1)
	{
		m_weapon = m_inventory[weaponIndex];
	}
}

bool Actor::PlayAnimation(std::string const& animName)
{
	for (int i=0;i< (int)m_actorDef->m_animGroupDef.size();i++)
	{
		if (m_actorDef->m_animGroupDef[i].m_name == animName)
		{
			m_nextAnimGroupIndex = i;
			m_animState = AnimState::PLAYING;
			m_animTimer.Start();
			return true;
		}
	}
	m_animState = AnimState::IDLE;
	return false;
}

// bool Actor::PlayWeaponAnimation(std::string const& animName)
// {
// 	auto weaponAnimDef=m_weapon->m_def.m_animDefs.find(animName);
// 	if (weaponAnimDef != m_weapon->m_def.m_animDefs.end())
// 	{
// 		m_animTimer.m_period = weaponAnimDef->second->GetAnimTime();
// 		m_animTimer.Start();
// 		return true;
// 	}
// 	return false;
// }


