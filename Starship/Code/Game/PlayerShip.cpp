#include "PlayerShip.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GameCommon.hpp"
#include "App.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"


//Rgba8 shipOriColor = Rgba8(102, 153, 204, 255);
Rgba8 shipOriColor = Rgba8(22, 212, 204, 255);
Rgba8 curShipColor = shipOriColor;
PlayerShip::PlayerShip(Game* game, float x, float y): Entity(game, x, y) 
{
	//can use initialize list
	m_orientationDegrees = 0;
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_velocity = Vec2{ 0.f,0.f };
	m_isDead = false;
	m_lives = 4;
	m_health = 80;
	m_thrustFraction = 0.f;
	InitializedVerts(vertices, curShipColor);
	m_Canon = new Canon(game, x, y, 3);
	m_laserExplosionCanon = new LaserExplosionCanon(game, x, y, 1);
	m_lightSaber = new LightSaber(game, x, y, 1);

	//tail verts
	tail_vertices[0]= Vertex_PCU(Vec3(-2.f, 1.f, 0.f), Rgba8(189,29,56), Vec2(0.f, 0.f));
	tail_vertices[1] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), Rgba8(189, 29, 56), Vec2(0.f, 0.f));
	tail_vertices[2] = Vertex_PCU(Vec3(-2.f,0.f, 0.f), Rgba8(71, 229, 180,10), Vec2(0.f, 0.f));
}

PlayerShip::~PlayerShip()
{
	if (m_Canon != nullptr)
	{
		delete m_Canon;
		m_Canon = nullptr;
	}
	if (m_laserExplosionCanon != nullptr)
	{
		delete m_laserExplosionCanon;
		m_laserExplosionCanon = nullptr;
	}
	if (m_lightSaber != nullptr)
	{
		delete m_lightSaber;
		m_lightSaber = nullptr;
	}
}

void PlayerShip::Update(float deltaTime) 
{
	UpdateFromController(deltaTime);
	if (m_isDead == false)
	{
		//DetectAsteroids();
		//--------------------------------------------------------------------------------------
		//left rotate
		m_isShipRotateLeft = g_theInput->IsKeyDown('S');
		//right rotate
		m_isShipRotateRight = g_theInput->IsKeyDown('F');
		//accelerate
		m_isAccelerate = g_theInput->IsKeyDown('E');
		//shoot
		//m_isShoot = g_theInput->WasKeyJustPressed(32);
		m_isKeepShoot = g_theInput->IsKeyDown(32);
		m_isJustShoot = g_theInput->WasKeyJustPressed(32);
		//respawn
		//light saber

		m_isSaber = g_theInput->IsKeyDown('J');
		m_isChangeState = g_theInput->WasKeyJustPressed('C');
		if (m_isChangeState)
		{
			g_theAudio->StartSound(m_game->playerChangeWeaponSound);
			if (m_FirstWeaponState == 1)
			{
				m_FirstWeaponState = 2;
			}
			else
			{
				m_FirstWeaponState = 1;
			}
		}
		if (m_isShipRotateLeft)
		{
			if(m_FirstWeaponState==1)
				m_orientationDegrees += PLAYER_SHIP_TURN_SPEED * deltaTime;
			else
				m_orientationDegrees += PLAYER_SHIP_TURN_SPEED_SLOW * deltaTime;
		}

		if (m_isShipRotateRight)
		{
			if (m_FirstWeaponState == 1)
				m_orientationDegrees -= PLAYER_SHIP_TURN_SPEED * deltaTime;
			else
				m_orientationDegrees -= PLAYER_SHIP_TURN_SPEED_SLOW * deltaTime;
		}

		if (m_thrustFraction > 0.f)
		{
			m_thrustFraction = m_thrustFraction;
		}
		else
		{
			if (m_isAccelerate)
			{
				m_thrustFraction = 1.0f;
				//m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, PLAYER_SHIP_ACCELERATION * deltaTime);
			}
			else
			{
				m_thrustFraction =0.0f;
			}
		}
		if (m_isAccelerate)
		{
			//m_thrustFraction = 1.0f;
			m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, PLAYER_SHIP_ACCELERATION * deltaTime);
		}


		if (m_velocity.GetLength() > 0)
		{
			Vec2 deltaVelocity = -m_velocity;
			deltaVelocity.SetLength(0.02f);
			m_velocity += deltaVelocity;
		}
		BounceOffWall();

		if (m_isStartQuickDash)
		{
			if (GetCurrentTimeSeconds() - m_startQuickDashTime > 0.2)
			{
				m_isStartQuickDash = false;
				m_velocity= Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_previousVelocity.GetLength());
			}
			else
			{
				//add force
				//m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, 60.f * deltaTime);
				m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_quickDashRadius  / 0.2f);
			}
		}

		m_position.x += m_velocity.x * deltaTime;
		m_position.y += m_velocity.y * deltaTime;

		if (m_isKeepShoot&&m_FirstWeaponState==1)
		{
			m_game->PlayerShipKeepShoot();
		}

		if (m_isJustShoot && m_FirstWeaponState == 2)
		{
			m_game->PlayerShipJustShoot();
		}

		if (m_isInvincibleState)
		{
			double curTime = GetCurrentTimeSeconds();
			if (curTime - m_invincibleStartTime > 3.0)
			{
				m_isInvincibleState = false;
				curShipColor = shipOriColor;
				return;
			}
			float temp = RangeMap(SinDegrees((float)curTime * 500.f), -1.f, 1.f, 0.4f, 1.f);
			curShipColor = Rgba8(110, 47, 117, (unsigned char)(255.f * temp));
		}

		if (m_FirstWeaponState == 1)
		{
			m_Canon->Update(deltaTime);
		}
		else if (m_FirstWeaponState == 2)
		{
			m_laserExplosionCanon->Update(deltaTime);
		}
		if (m_magicPoint < m_maxMagicPoint)
		{
			m_magicPoint += 0.04f;
		}
	
		if (m_isSaber==true && m_magicPoint > MIN_SABER_MP)
		{
			m_lightSaber->Update(deltaTime);
			m_magicPoint -= 0.3f;
		}

		if (g_theInput->WasKeyJustPressed('J'))
		{
			g_theAudio->StartSound(m_game->playerSaberSound);
		}

		
		//trail tail
		float tailLen = RangeMapClamped(m_thrustFraction, 0.f, 1.f, 5.f, 12.f);
		float offset=m_game->m_rng->RollRandomFloatInRange(-0.5f, 0.5f);
		tail_vertices[2] = Vertex_PCU(Vec3(-tailLen+ offset, 0.f, 0.f), Rgba8(214, 116, 207, 10), Vec2(0.f, 0.f));
	}
	
	if (m_isDead == true && g_theInput->WasKeyJustPressed('N') && m_lives > 1)
	{
		RespawnShip();
	}

	if (m_isDead && m_lives <= 1&&m_isUseUpLives==false)
	{
		//wait to return menu
		g_theAudio->StartSound(m_game->finalDefeat);
		m_isUseUpLives = true;
		m_deadTime= (float)GetCurrentTimeSeconds();
	}
	if (m_isUseUpLives == true)
	{
		float curTime= (float)GetCurrentTimeSeconds();
		if (curTime - m_deadTime >= 2)
		{
			//m_game->m_isArractMode = true;
			m_game->m_flagToReset = true;
		}
	}
}

void PlayerShip::UpdateFromController(float deltaSeconds)
{
	//UNUSED(deltaSceonds);
	XboxController const& controller = g_theInput->GetController(0);
	if (controller.IsConnected())
	{
		m_previousRightTrigger = controller.GetRightTrigger();
		if (m_isDead)
		{
			if (controller.WasButtonJustPressed(XboxButtonID::START) && m_lives > 1)
			{
				RespawnShip();
			}
			return;
		}

		//move
		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f && !m_isShipRotateLeft && !m_isShipRotateRight)
		{
			m_thrustFraction = leftStickMagnitude;
			m_orientationDegrees = controller.GetLeftStick().GetOrientationDegrees();
			m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, PLAYER_SHIP_ACCELERATION * deltaSeconds * m_thrustFraction);
		}
		else
		{
			m_thrustFraction = 0.f;
		}
		//float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		//if (leftStickMagnitude > 0.f&&!m_isShipRotateLeft&&!m_isShipRotateRight)
		//{
		//	//m_isAccelerate = true;
		//	m_isControllerAccelerate = true;
		//	m_thrustFraction = leftStickMagnitude;
		//	m_orientationDegrees = controller.GetLeftStick().GetOrientationDegrees();
		//	m_velocity += Vec2::MakeFromPolarDegrees(m_orientationDegrees, PLAYER_SHIP_ACCELERATION * deltaSeconds*m_thrustFraction);
		//}
		//else
		//{
		//	//m_isAccelerate = false;
		//	m_isControllerAccelerate = false;
		//	m_thrustFraction = 0.f;
		//}
		//fire
		if (controller.WasButtonJustPressed(XboxButtonID::A)&& m_FirstWeaponState == 2)
		{
			m_game->PlayerShipJustShoot();
		}
		if (controller.IsButtongDown(XboxButtonID::A) && m_FirstWeaponState == 1)
		{
			m_game->PlayerShipKeepShoot();
		}
		//change weapon
		if (controller.WasButtonJustPressed(XboxButtonID::B))
		{
			g_theAudio->StartSound(m_game->playerChangeWeaponSound);
			if (m_FirstWeaponState == 1)
			{
				m_FirstWeaponState = 2;
			}
			else
			{
				m_FirstWeaponState = 1;
			}
		}
		//light saber
		if (controller.IsButtongDown(XboxButtonID::Y) && m_magicPoint > MIN_SABER_MP)
		{
			m_isControllerSaber = true;
			m_lightSaber->Update(deltaSeconds);
			m_magicPoint -= 0.3f;
		}
		else
		{
			m_isControllerSaber = false;
		}
		if (controller.WasButtonJustPressed(XboxButtonID::Y) && m_magicPoint > MIN_SABER_MP)
		{
			g_theAudio->StartSound(m_game->playerSaberSound);
		}
		//bullet time
		if (controller.GetRightTrigger()>0.f)
		{
			if (m_previousRightTrigger <= 0.1f)
			{
				g_theAudio->StartSound(m_game->bulletTimeStart);
			}
			//trigger bullet time
			if (m_magicPoint > 3.f)
			{
				TriggerBulletTime(controller.GetRightTrigger());
				if (controller.WasButtonJustPressed(XboxButtonID::X))
				{
					TriggerQuickDash();
					g_theAudio->StartSound(m_game->setBulletTimeMove);
				}
			}
			else
			{
				m_isTriggerQuickDash = false;
				m_game->m_isBulletTime = false;
			}
		}
		else
		{
			if (m_isTriggerQuickDash)
			{
				g_theAudio->StartSound(m_game->dash);
				m_game->ShakeScreen();
				m_isTriggerQuickDash = false;
				//add dash
				m_isStartQuickDash = true;
				m_startQuickDashTime = GetCurrentTimeSeconds();
				m_previousVelocity = m_velocity;
			}
			
			m_game->m_isBulletTime = false;
		}

		if (m_isTriggerQuickDash)
		{
			UpdateQuickDashUI(leftStickMagnitude);
		}
	}

}
void PlayerShip::TriggerBulletTime(float rightTrigger)
{
	m_game->m_deltaTimeRatio = RangeMapClamped(rightTrigger, 0.f, 1.f, 1.f, 0.3f);
	m_game->m_isBulletTime = true;
	m_magicPoint -= 0.1f;
}

void PlayerShip::TriggerQuickDash()
{
	m_isTriggerQuickDash = true;
}

void PlayerShip::RenderQuickDashUI() const
{
	DebugDrawHighCircle(m_quickDashRadius, m_position, Rgba8(0, 0, 255, 100));

	Vertex_PCU temp_vertices[NUM_SHIP_VERTS];
	for (int i = 0; i < NUM_SHIP_VERTS; i++)
	{
		temp_vertices[i] = vertices[i];
		temp_vertices[i].m_color =Rgba8(22, 212, 204,100);
	}
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position+Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_quickDashRadius));
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, temp_vertices);

}

void PlayerShip::UpdateQuickDashUI(float leftMagnitute)
{
	m_quickDashRadius = RangeMapClamped(leftMagnitute, 0.f, 1.f, 5.f, 40.f);
}

void PlayerShip::Render() const
{
	if (m_isDead == false)
	{
		if (m_isTriggerQuickDash)
		{
			RenderQuickDashUI();
		}
		if ((m_isSaber||m_isControllerSaber) && m_magicPoint > MIN_SABER_MP)
		{
			m_lightSaber->Render();
		}
		
		Vertex_PCU temp_vertices[NUM_SHIP_VERTS];
		for (int i = 0; i < NUM_SHIP_VERTS; i++)
		{
			temp_vertices[i] = vertices[i];
			temp_vertices[i].m_color = curShipColor;
		}
		TransformVertexArrayXY3D(NUM_SHIP_VERTS, temp_vertices, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, temp_vertices); //NUM_SHIP_VERTS

		if (m_FirstWeaponState == 1)
		{
			m_Canon->Render();
		}
		else if (m_FirstWeaponState == 2)
		{
			m_laserExplosionCanon->Render();
		}

		Vertex_PCU temp_tail_vertices[3];
		for (int i = 0; i < 3; i++)
		{
			temp_tail_vertices[i] = tail_vertices[i];
			//temp_tail_vertices[i].m_color = curShipColor;
		}
		TransformVertexArrayXY3D(3, temp_tail_vertices, 1.f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(3, temp_tail_vertices); //NUM_SHIP_VERTS

	}	
}

void PlayerShip::Die() 
{
	m_isInvincibleState = false;
	m_game->ShakeScreen();
	m_game->GenerateDebris(m_position, Rgba8(102, 153, 204, 130), 8, 15.f, 35.f, 1.5f, 2.f, m_velocity);
	m_isDead = true;
	g_theAudio->StartSound(m_game->playerDiedSound);
	//m_game->SpawnNewDebrisCluster(10,m_position,m_velocity,10.f,10.f,m_color)
}

void PlayerShip::GetHurt(float hurtHp)
{
	m_health -= hurtHp;
	m_isInvincibleState = true;
	m_invincibleStartTime = GetCurrentTimeSeconds();
	g_theAudio->StartSound(m_game->playerHurtSound);
}

void PlayerShip::BounceOffWall()
{
	if (m_position.x < WORLDSPACE_SIZE_BL_X+PLAYER_SHIP_PHYSICS_RADIUS )
	{
		m_position.x = WORLDSPACE_SIZE_BL_X + PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.x > WORLDSPACE_SIZE_TR_X- PLAYER_SHIP_PHYSICS_RADIUS)
	{
		m_position.x = WORLDSPACE_SIZE_TR_X - PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.x = -m_velocity.x;
	}
	if (m_position.y < WORLDSPACE_SIZE_BL_Y+PLAYER_SHIP_PHYSICS_RADIUS )
	{
		m_position.y = WORLDSPACE_SIZE_BL_Y + PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.y = -m_velocity.y;
	}
	if (m_position.y > WORLDSPACE_SIZE_TR_Y - PLAYER_SHIP_PHYSICS_RADIUS)
	{
		m_position.y = WORLDSPACE_SIZE_TR_Y - PLAYER_SHIP_PHYSICS_RADIUS;
		m_velocity.y = -m_velocity.y;
	}
}


void PlayerShip::RespawnShip()
{
	g_theAudio->StartSound(m_game->playerRespawn);
	m_isInvincibleState = true;
	m_invincibleStartTime = GetCurrentTimeSeconds();
	m_health = m_maxHealth;
	//if (m_lives >= 1)
	//{
		m_lives--;
	//}
	m_orientationDegrees = 0;
	m_velocity.x = 0;
	m_velocity.y = 0;
	m_position.x = WORLD_CENTER_X;
	m_position.y = WORLD_CENTER_Y;
	m_isDead = false;
}

void PlayerShip::InitializedVerts(Vertex_PCU* vertsToFillIn, Rgba8 const& color)
{
	vertsToFillIn[0] = Vertex_PCU(Vec3(-2, 1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[1] = Vertex_PCU(Vec3(0, 2, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[2] = Vertex_PCU(Vec3(2, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[3] = Vertex_PCU(Vec3(-2, 1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[4] = Vertex_PCU(Vec3(-2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[5] = Vertex_PCU(Vec3(0, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[6] = Vertex_PCU(Vec3(-2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[7] = Vertex_PCU(Vec3(0, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[8] = Vertex_PCU(Vec3(0, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[9] = Vertex_PCU(Vec3(0, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[10] = Vertex_PCU(Vec3(1, 0, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[11] = Vertex_PCU(Vec3(0, 1, 0.f), color, Vec2(0.f, 0.f));

	vertsToFillIn[12] = Vertex_PCU(Vec3(-2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[13] = Vertex_PCU(Vec3(2, -1, 0.f), color, Vec2(0.f, 0.f));
	vertsToFillIn[14] = Vertex_PCU(Vec3(0, -2, 0.f), color, Vec2(0.f, 0.f));
}
