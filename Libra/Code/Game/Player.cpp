#include "Game/Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"

Player::Player(EntityType type,EntityFaction faction) :Entity(type,faction)
{
	m_cosmeticRadius = PLAYER_COS_RADIUS;
	m_physicsRadius = PLAYER_PHY_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("playerTurnRate",360.f);
	m_speed = g_gameConfigBlackboard.GetValue("playerDriveSpeed", 3.f);
	m_aimDirection = Vec2{ 0.f,0.f };
	m_type = ENTITY_TYPE_GOOD_PLAYER;
	m_turretAngularVelocity = g_gameConfigBlackboard.GetValue("playerGunTurnRate", 720.f); 
	m_previous_orientation = 0.f;
	m_turretOrientationDegrees = m_orientationDegrees;
	m_health = g_gameConfigBlackboard.GetValue("playerMaxHealth", 1000.f);
	m_totalHealth = m_health;
	Vec2 turret_direction = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
	Vec2 body_direction = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	m_verts_body.reserve(6);
	m_verts_turret.reserve(6);
	m_turretBox = OBB2(Vec2(0.f, 0.f), turret_direction, Vec2(0.6f, 0.6f));
	m_bodyBox = OBB2(Vec2(0.f, 0.f), body_direction, Vec2(0.6f, 0.6f));
	AddVertsForOBB2D(m_verts_body, m_bodyBox, Rgba8(255, 255, 255, 255));
	AddVertsForOBB2D(m_verts_turret, m_turretBox, Rgba8(255, 255, 255, 255));

	m_texPlayerTankBase = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	m_texPlayerTankTurret = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
	m_texPlayerTankBullet= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBullet.png");

	m_curPlayerTileCoorPos = IntVec2(1, 1);

	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_scale = 0.f;
}

Player::~Player()
{
}

void Player::Update(float deltaTime)
{
	XboxController& controller = g_theInput->GetControllerAndSet(0);
	if (CheckOverlapVsRubble())
	{
		m_speedRatio = 0.5f;
		g_theGame->ShakeScreen();
		if (controller.IsConnected())
		{
			controller.SetVibrates(5000,5000);
		}
	}
	else
	{
		m_speedRatio = 1.f;
		if (controller.IsConnected())
		{
			controller.SetVibrates(0, 0);
		}
	}
	if (!g_theGame->m_isStartMap && !g_theGame->m_isChangeMap)
	{
		UpdateKBMoveInput(deltaTime);
		UpdateKBTurretInput(deltaTime);
		UpdateFromController(deltaTime);

		if (!m_isDead)
		{
			UpdateMovement(deltaTime);

			if (m_isShooting)
			{
				if (!m_isShootCoolDown)
				{
					m_isShootCoolDown = true;
					m_lastShootTime = GetCurrentTimeSeconds();
					Vec2 tip = m_position + Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees) * 0.4f;
					float bulletStartOri = m_turretOrientationDegrees + g_theGame->m_rng.RollRandomFloatInRange(-15.f, 15.f);
					if (m_weaponState == 1)
					{
						m_map->AgentShooting(ENTITY_TYPE_GOOD_BULLET, m_faction, tip, bulletStartOri, 0.2f);
					}
					else
					{
						m_map->AgentShooting(ENTITY_TYPE_GOOD_FLAMEBULLET, m_faction, tip, bulletStartOri, 0.1f);
					}
					g_theAudio->StartSound(g_theGame->m_playerShoot);
				}
			}
			if (GetCurrentTimeSeconds() - m_lastShootTime > g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 0.1f))
			{
				m_isShootCoolDown = false;
			}
		}
	}
	if (g_theGame->m_isChangeMap)
	{
		m_scale -= 1.f * deltaTime / g_theGame->m_transitionTime;
		float accelerate = (360.f * 4.f) / (g_theGame->m_transitionTime * g_theGame->m_transitionTime);
		m_curSpinSpeed += accelerate * deltaTime;
		m_orientationDegrees += m_curSpinSpeed * deltaTime;
		m_turretOrientationDegrees += m_curSpinSpeed * deltaTime;
	}
	else if (g_theGame->m_isStartMap)
	{
		m_scale += 1.f * deltaTime / g_theGame->m_transitionTime;
		float accelerate = (360.f * 4.f) / (g_theGame->m_transitionTime * g_theGame->m_transitionTime);
		m_curSpinSpeed += accelerate * deltaTime;
		m_orientationDegrees += m_curSpinSpeed * deltaTime;
		m_turretOrientationDegrees += m_curSpinSpeed * deltaTime;
	}
}

void Player::Render() const
{
	if (!m_isDead)
	{
		std::vector<Vertex_PCU> temp_verts;
		for (int i = 0; i < static_cast<int>(m_verts_body.size()); i++)
		{
			temp_verts.push_back(m_verts_body[i]);
		}
		TransformVertexArrayXY3D(static_cast<int>(m_verts_body.size()), temp_verts, m_scale, m_orientationDegrees, m_position);
		g_theRenderer->BindTexture(m_texPlayerTankBase);
		g_theRenderer->DrawVertexArray(temp_verts);

		std::vector<Vertex_PCU> temp_verts_turret;
		for (int i = 0; i < static_cast<int>(m_verts_turret.size()); i++)
		{
			temp_verts_turret.push_back(m_verts_turret[i]);
		}
		TransformVertexArrayXY3D(static_cast<int>(m_verts_turret.size()), temp_verts_turret, m_scale, m_turretOrientationDegrees, m_position);
		g_theRenderer->BindTexture(m_texPlayerTankTurret);
		g_theRenderer->DrawVertexArray(temp_verts_turret);
	}
}

void Player::Die()
{
	g_theAudio->StartSound(g_theGame->m_playerDead);
	m_isDead = true;
	m_map->SpawnExplosion(m_position,0.7f,g_explosionAnimDef_Slow);
}

void Player::DrawDebugMode() const
{
	g_theRenderer->BindTexture(nullptr);
	Entity::DrawDebugMode();
	DebugDrawLine(m_position + m_aimDirection.GetNormalized() * m_cosmeticRadius, 
		m_position + m_aimDirection.GetNormalized() * (m_cosmeticRadius + 0.05f),
		0.1f, Rgba8::RED);
	DebugDrawLine(m_position + m_turret_aimDirection.GetNormalized() * m_cosmeticRadius,
		m_position + m_turret_aimDirection.GetNormalized() * (m_cosmeticRadius + 0.05f),
		2.f*PLAYER_TURRET_HDIM_X, Rgba8::BLUE);

	OBB2 turret_shape_box = OBB2(m_position + m_turretBox.m_iBasisNormal.GetRotated90Degrees() * m_cosmeticRadius*0.5f,
		m_turretBox.m_iBasisNormal, Vec2(PLAYER_TURRET_HDIM_X, m_cosmeticRadius * 0.5f));
	std::vector<Vertex_PCU> obb_verts;
	AddVertsForOBB2D(obb_verts, turret_shape_box, Rgba8::BLUE);
	g_theRenderer->DrawVertexArray(obb_verts);
}

void Player::UpdateKBMoveInput(float deltaTime)
{
	UNUSED(deltaTime);
	m_isMove = false;
	Vec2 aimFrameDirection = Vec2{ 0.f,0.f };
	m_previous_orientation = m_orientationDegrees;
	if (g_theInput->WasKeyJustPressed('C'))
	{
		if (m_weaponState == 1)	m_weaponState = 2;
		else
		{
			m_weaponState = 1;
		}
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_isMove = true;
		aimFrameDirection += Vec2{ -1.f,0.f };
	}
	if (g_theInput->IsKeyDown('F'))
	{
		m_isMove = true;
		aimFrameDirection += Vec2{ 1.f,0.f };
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_isMove = true;
		aimFrameDirection += Vec2{ 0.f,1.f };
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_isMove = true;
		aimFrameDirection += Vec2{ 0.f,-1.f };
	}
	
	if ((aimFrameDirection!=Vec2(0.f,0.f))&&m_isMove)
	{
		m_aimDirection = aimFrameDirection;
		m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, g_gameConfigBlackboard.GetValue("playerDriveSpeed", 3.f)*m_speedRatio);
	}
	else
	{
		m_velocity = Vec2{ 0.f,0.f };
	}

	m_isShooting = g_theInput->IsKeyDown(KEYCODE_SPACE);
}

void Player::UpdateKBTurretInput(float deltaTime)
{
	UNUSED(deltaTime);
	m_isRotateTurret = false;
	Vec2 aimDirection = Vec2{ 0.f,0.f };
	if (g_theInput->IsKeyDown('J'))
	{
		m_isRotateTurret = true;
		aimDirection += Vec2{ -1.f,0.f };
	}
	if (g_theInput->IsKeyDown('L'))
	{
		m_isRotateTurret = true;
		aimDirection += Vec2{ 1.f,0.f };
	}
	if (g_theInput->IsKeyDown('I'))
	{
		m_isRotateTurret = true;
		aimDirection += Vec2{ 0.f,1.f };
	}
	if (g_theInput->IsKeyDown('K'))
	{
		m_isRotateTurret = true;
		aimDirection += Vec2{ 0.f,-1.f };
	}

	if ((aimDirection != Vec2(0.f, 0.f)) && m_isRotateTurret)
	{
		m_turret_aimDirection = aimDirection;
	}

}

void Player::UpdateFromController(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController const& controller = g_theInput->GetController(0);
	if (controller.IsConnected())
	{
		if (controller.WasButtonJustPressed(XboxButtonID::X))
		{
			if (m_weaponState == 1)	m_weaponState = 2;
			else
			{
				m_weaponState = 1;
			}
		}
		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			m_isMove = true;
			float aimOrientation = controller.GetLeftStick().GetOrientationDegrees();
			m_aimDirection = Vec2::MakeFromPolarDegrees(aimOrientation);
			m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, g_gameConfigBlackboard.GetValue("playerDriveSpeed", 3.f) * leftStickMagnitude * m_speedRatio);
		}

		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			m_isRotateTurret = true;
			float aimTurretOrientation = controller.GetRightStick().GetOrientationDegrees();
			m_turret_aimDirection = Vec2::MakeFromPolarDegrees(aimTurretOrientation);
		}

		m_isShooting = g_theInput->IsKeyDown(KEYCODE_SPACE)||controller.GetRightTrigger()>0.f;
	}
}
void Player::UpdateMovement(float deltaSeconds)
{
	if (m_isMove)
	{
		//player move
		m_position += m_velocity * deltaSeconds;
		//player rotate
		float curDeg = GetTurnedTowardDegrees(m_orientationDegrees, m_aimDirection.GetOrientationDegrees(), m_angularVelocity * deltaSeconds);
		m_orientationDegrees = curDeg;
	}
	
	//turret
	if (m_isRotateTurret)
	{
		float curDeg = GetTurnedTowardDegrees(m_turretOrientationDegrees, m_turret_aimDirection.GetOrientationDegrees(), 360.f * deltaSeconds);
		m_turretOrientationDegrees = curDeg;
	}
	else
	{
		float newTurretOri = m_orientationDegrees - m_previous_orientation + m_turretOrientationDegrees;
		m_turretOrientationDegrees = newTurretOri;
	}

	Vec2 turret_direction = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
	m_turretBox.m_center = m_position + turret_direction * PLAYER_TURRET_HDIM_Y;
	m_turretBox.m_iBasisNormal = turret_direction.GetRotatedMinus90Degrees();
}

void Player::ResetHealth()
{
	m_health = m_totalHealth;
}



