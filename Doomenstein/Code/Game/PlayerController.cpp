#include "PlayerController.hpp"
#include "GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "ActorDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

extern Clock* g_systemClock;

PlayerController::PlayerController(Map* map):Controller(map)
{
	m_font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
// 	IntVec2 clientDimensions = g_theWindow->GetClientDimensions();
// 	AABB2 viewport = AABB2(Vec2(0.f, 0.f), Vec2(clientDimensions.x, clientDimensions.y*0.5f));
// 	m_playerCam.SetViewport(viewport);
// 
// 	m_playerCam.SetPerspectiveView(viewport.GetDimensions().x / viewport.GetDimensions().y, 60.f, 0.1f, 100.f);
// 	m_playerCam.SetCameraToRenderTransform(Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f)));
// 	m_playerCam.SetPositionAndOrientation(m_camPosition, m_camOrientation);
// 	//viewport.GetDimensions().x/viewport.GetDimensions().y

	AddVertsForAABB2D(m_clockBaseVerts, AABB2(-m_clockHalfExtent + m_clockBasePos, m_clockHalfExtent + m_clockBasePos), Rgba8::WHITE);
}

PlayerController::~PlayerController()
{
}

void PlayerController::Update(float deltaSeconds)
{
 	m_camVelocity = Vec3(0.f, 0.f, 0.f);

	if (!(GetActor()->m_map->m_game->m_playerController0 && GetActor()->m_map->m_game->m_playerController1))
	{
		if (g_theInput->WasKeyJustPressed('F'))
		{
			int newMode = ((int)m_playerMode + 1) % ((int)PlayerMode::COUNT);
			m_playerMode = PlayerMode(newMode);
			GetActor()->m_isVisible = !GetActor()->m_isVisible;
		}
	}
	
	if (m_playerMode==PlayerMode::FIRST_PLAYER)
	{
		GetActor()->m_isVisible = false;
	}
	else
	{
		GetActor()->m_isVisible = true;
	}

	UpdatePlayerInput(deltaSeconds);
	UpdateCamera(deltaSeconds);

	if (!m_curMap->m_isGold)
	{
		UpdateHUD();
	}
	else
	{
		UpdateGoldHUD();
	}
}

void PlayerController::UpdatePlayerInput(float deltaSeconds)
{
	if (m_playerMode == PlayerMode::FREE_CAMERA)
	{
		if (m_controllerID == -1)
		{
			UpdateKBInputCam(deltaSeconds);
		}
		else if (m_controllerID == 0)
		{
			UpdateControllerInputCam(deltaSeconds,0);
		}
		else if (m_controllerID == 1)
		{
			UpdateControllerInputCam(deltaSeconds,1);
		}
	}

	Actor* self = GetActor();
	if (m_playerMode == PlayerMode::FIRST_PLAYER)
	{
		if (!self->m_isDead)
		{
			if (m_controllerID == -1)
			{
				UpdateKBInputPlayer(deltaSeconds);
			}
			else if (m_controllerID == 0)
			{
				UpdateControllerInputPlayer(deltaSeconds,0);
			}
			else if (m_controllerID == 1)
			{
				UpdateControllerInputPlayer(deltaSeconds,1);
			}
		}
		else
		{
			float dropSpeed = self->m_actorDef->m_eyeHeight / self->m_actorDef->m_corpseLifetime;
			if (m_camPosition.z > 0.f)
			{
				m_camPosition.z -= deltaSeconds * dropSpeed;
			}
		}
	}
}

void PlayerController::UpdateKBInputCam(float deltaSeconds)
{
	Actor* curPlayer = GetActor();
	ActorDefinition* curActorDef = curPlayer->m_actorDef;
	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	m_playerCam.GetOrientation().GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	
	float curMoveSpeed;
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
	{
		curMoveSpeed = curActorDef->m_runSpeed;
	}
	else
	{
		curMoveSpeed = curActorDef->m_walkSpeed;
	}
	
	m_camVelocity = Vec3(0.f, 0.f, 0.f);
	Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
	if (g_theInput->IsKeyDown('W'))
	{
		aimDirection += fwdDirection;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		aimDirection -= fwdDirection;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		aimDirection += leftDirection;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		aimDirection -= leftDirection;
	}
 	if (g_theInput->IsKeyDown('Z'))
 	{
 		m_camPosition += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
 	}
 	if (g_theInput->IsKeyDown('C'))
 	{
		m_camPosition += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
 	}
	
	float rollChange = 0.f;
	if (g_theInput->IsKeyDown('Q'))
	{
		rollChange -= m_rollSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('E'))
	{
		rollChange += m_rollSpeed * deltaSeconds;
	}

	float yawChange = -g_theInput->GetCursorClientDelta().x * g_theWindow->GetClientDimensions().x * m_yawPitchRatio;
	float pitchChange = g_theInput->GetCursorClientDelta().y * g_theWindow->GetClientDimensions().y * m_yawPitchRatio;

	m_camOrientation.m_yawDegrees += yawChange;
	m_camOrientation.m_pitchDegrees += pitchChange;
	m_camOrientation.m_rollDegrees += rollChange;

	m_camOrientation.m_rollDegrees = GetClamped(m_camOrientation.m_rollDegrees, -45.f, 45.f);
	m_camOrientation.m_pitchDegrees = GetClamped(m_camOrientation.m_pitchDegrees, -85.f, 85.f);


	aimDirection.Normalized();
	aimDirection.SetLength(curMoveSpeed);
	m_camVelocity= aimDirection;
	
	m_camPosition += m_camVelocity * deltaSeconds;
	
	if (g_theInput->IsKeyDown('H'))
	{
		m_camOrientation.m_pitchDegrees = 0.f;
		m_camOrientation.m_rollDegrees = 0.f;
		m_camOrientation.m_yawDegrees = 0.f;
		m_camPosition = Vec3(0.f, 0.f, 0.f);
	}
}

void PlayerController::UpdateControllerInputCam(float deltaSeconds, int controllerID)
{
	if (controllerID != 0 && controllerID != 1)
	{
		ERROR_AND_DIE("Invaild controller ID for player");
	}
	Actor* curPlayer = GetActor();
	ActorDefinition* curActorDef = curPlayer->m_actorDef;
	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	m_playerCam.GetOrientation().GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);

	XboxController const& controller = g_theInput->GetController(controllerID);
	if (controller.IsConnected())
	{
		float curMoveSpeed;
		if (controller.IsButtonDown(XboxButtonID::A))
		{
			curMoveSpeed = curActorDef->m_runSpeed;
		}
		else
		{
			curMoveSpeed = curActorDef->m_walkSpeed;
		}

		m_camVelocity = Vec3(0.f, 0.f, 0.f);
		Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);

		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			aimDirection -= controller.GetLeftStick().GetPosition().x * leftDirection;
			aimDirection += controller.GetLeftStick().GetPosition().y * fwdDirection;
		}

		if (controller.IsButtonDown(XboxButtonID::LEFT_SHOULDER))
		{
			m_camPosition += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
		}
		if (controller.IsButtonDown(XboxButtonID::RIGHT_SHOULDER))
		{
			m_camPosition += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
		}


		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			float yawChange = -controller.GetRightStick().GetPosition().x * curPlayer->m_actorDef->m_turnSpeed * deltaSeconds;
			float pitchChange = -controller.GetRightStick().GetPosition().y * curPlayer->m_actorDef->m_turnSpeed * deltaSeconds;

			m_camOrientation.m_yawDegrees += yawChange;
			m_camOrientation.m_pitchDegrees += pitchChange;
		}

		m_camOrientation.m_pitchDegrees = GetClamped(m_camOrientation.m_pitchDegrees, -85.f, 85.f);

		if (aimDirection.GetLengthSquared() > 0.f)
		{
			aimDirection.Normalized();
			aimDirection.SetLength(curMoveSpeed);
		}
		m_camVelocity = aimDirection;

		m_camPosition += m_camVelocity * deltaSeconds;
	}
}

void PlayerController::UpdateKBInputPlayer(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Actor* curPlayer = GetActor();
	ActorDefinition* curActorDef = curPlayer->m_actorDef;
	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	curPlayer->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);

	m_camPosition = curPlayer->m_position + Vec3(0.f, 0.f, curActorDef->m_eyeHeight);

	float curMoveSpeed;
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
	{
		curMoveSpeed = curActorDef->m_runSpeed;
	}
	else
	{
		curMoveSpeed = curActorDef->m_walkSpeed;
	}

	Vec3 moveVelocity = Vec3(0.f, 0.f, 0.f);
	Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
	if (g_theInput->IsKeyDown('W'))
	{
		aimDirection += fwdDirection;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		aimDirection -= fwdDirection;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		aimDirection += leftDirection;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		aimDirection -= leftDirection;
	}

	float yawChange = -g_theInput->GetCursorClientDelta().x * g_theWindow->GetClientDimensions().x * m_yawPitchRatio;
	float pitchChange = g_theInput->GetCursorClientDelta().y * g_theWindow->GetClientDimensions().y * m_yawPitchRatio;

	curPlayer->m_orientation.m_yawDegrees += yawChange;

	m_camOrientation.m_yawDegrees =curPlayer->m_orientation.m_yawDegrees;
	m_camOrientation.m_pitchDegrees += pitchChange;
	m_camOrientation.m_rollDegrees =0.f;
	m_camOrientation.m_pitchDegrees = GetClamped(m_camOrientation.m_pitchDegrees, -85.f, 85.f);

	aimDirection.Normalized();
	curPlayer->MoveInDirection(aimDirection, curMoveSpeed);

	if (!m_isWalking)
	{
		if (curPlayer->m_velocity.GetLengthSquared() > 0.1f)
		{
			curPlayer->PlayAnimation("Walk");
			m_isWalking = true;
		}
	}
	else
	{
		if (curPlayer->m_velocity.GetLengthSquared() < 0.1f)
		{
			curPlayer->m_animState = AnimState::FINISHED;
			m_isWalking = false;
		}
	}


	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		if (Attack(m_camOrientation.GetForward_IFwd()))
		{
			PlayWeaponAnimationHud("Attack");
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		if (GetActor()->m_curWeaponIndex > 0)
		{
			GetActor()->m_curWeaponIndex--;
		}
		else
		{
			GetActor()->m_curWeaponIndex = (int)GetActor()->m_inventory.size() - 1;
		}
		GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		SetWeaponAnimDef();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		if (GetActor()->m_curWeaponIndex < (int)GetActor()->m_inventory.size() - 1)
		{
			GetActor()->m_curWeaponIndex++;
		}
		else
		{
			GetActor()->m_curWeaponIndex = 0;
		}
		GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		SetWeaponAnimDef();

	}
	if (g_theInput->WasKeyJustPressed('1'))
	{
		GetActor()->m_curWeaponIndex = 0;
		GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		SetWeaponAnimDef();
	}

	if (g_theInput->WasKeyJustPressed('2'))
	{
		GetActor()->m_curWeaponIndex = 1;
		GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		SetWeaponAnimDef();
	}
}

void PlayerController::UpdateGoldHUD()
{
	m_hudBaseVerts.clear();
	m_hudWeaponVerts.clear();
	m_rectileVerts.clear();
	m_blackVerts.clear();
	if (GetActor()->m_isDead)
	{
		AABB2 blackBox = AABB2(Vec2::ZERO, Vec2(1600.f, 800.f));
		AddVertsForAABB2D(m_hudBaseVerts, blackBox, Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f), Vec2(1.f, 1.f));
	}
	if (GetActor()->m_weapon->m_def.m_animDefs.size() != 0)
	{
		if (m_weaponAnimDef->GetPlaybackType() == SpriteAnimPlaybackType::ONCE && m_weaponAnimTimer.GetElapsedFraction() > 1.f)
		{
			PlayWeaponAnimationHud("Idle");
		}
		SpriteDefinition weaponSpriteDef = m_weaponAnimDef->GetSpriteDefAtTime((float)m_weaponAnimTimer.GetElapsedTime());


		AABB2 hudBaseBox = AABB2(Vec2::ZERO, Vec2(1600.f, 166.f));
		AddVertsForAABB2D(m_hudBaseVerts, hudBaseBox, Rgba8::WHITE, Vec2(0.f, 0.f), Vec2(1.f, 1.f));


		IntVec2 weaponSpriteSize = GetActor()->m_weapon->m_def.m_spriteSize;
		IntVec2 rectileSpriteSize = GetActor()->m_weapon->m_def.m_reticleSize;
		AABB2 weaponBox;
		AABB2 rectileBox;

		if (GetActor()->m_map->m_game->m_playerController0 && GetActor()->m_map->m_game->m_playerController1)
		{
			weaponBox = AABB2(Vec2(800.f, 255.f) - Vec2(0.5f * (float)weaponSpriteSize.x, (float)weaponSpriteSize.y),
				Vec2(800.f, 220.f) + Vec2(0.5f * (float)weaponSpriteSize.x, (float)weaponSpriteSize.y));
			rectileBox = AABB2(Vec2(800.f, 400.f) - Vec2(0.5f * (float)rectileSpriteSize.x, (float)rectileSpriteSize.y),
				Vec2(800.f, 400.f) + Vec2(0.5f * (float)rectileSpriteSize.x, (float)rectileSpriteSize.y));
		}
		else
		{
			weaponBox = AABB2(Vec2(800.f, 235.f) - Vec2(0.5f * (float)weaponSpriteSize.x, 0.5f * (float)weaponSpriteSize.y),
				Vec2(800.f, 220.f) + Vec2(0.5f * (float)weaponSpriteSize.x, 0.5f * (float)weaponSpriteSize.y));
			rectileBox = AABB2(Vec2(800.f, 400.f) - Vec2(0.5f * (float)rectileSpriteSize.x, 0.5f * (float)rectileSpriteSize.y),
				Vec2(800.f, 400.f) + Vec2(0.5f * (float)rectileSpriteSize.x, 0.5f * (float)rectileSpriteSize.y));
		}

		AddVertsForAABB2D(m_hudWeaponVerts, weaponBox, Rgba8::WHITE, weaponSpriteDef.GetUVs().m_mins, weaponSpriteDef.GetUVs().m_maxs);
		AddVertsForAABB2D(m_rectileVerts, rectileBox, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE);
	}
	int healthInInt = (int)GetActor()->m_curHealth;
	std::string curPlayerHealth = std::to_string(healthInInt);
	std::string curCoin = std::to_string(m_coinCount);

	m_textVerts.clear();
	m_font->AddVertsForTextInBox2D(m_textVerts, curPlayerHealth, AABB2(Vec2(700.f, 15.f), Vec2(850.f, 50.f)), 30.f);
	m_font->AddVertsForTextInBox2D(m_textVerts, curCoin, AABB2(Vec2(700.f, 45.f), Vec2(850.f, 100.f)), 30.f);
// 	m_font->AddVertsForTextInBox2D(m_textVerts, std::to_string(GetActor()->m_controller->m_kills), AABB2(Vec2(50.f, 50.f), Vec2(150.f, 100.f)), 50.f);
// 	m_font->AddVertsForTextInBox2D(m_textVerts, std::to_string(GetActor()->m_controller->m_death), AABB2(Vec2(1450.f, 50.f), Vec2(1550.f, 100.f)), 50.f);

	m_sunIconVerts.clear();
	float sunIconAngle = -(m_curMap->m_curDayTime / m_curMap->m_totalDayTime) * 720.f-90.f;
	Vec2 sunDirection=Vec2::MakeFromPolarDegrees(sunIconAngle);
	sunDirection.Normalize();
	m_sunIconPos = m_clockBasePos + 45.f * sunDirection;
	AABB2 sunIconBox = AABB2(m_sunIconPos - m_sunHalfExtent, m_sunIconPos + m_sunHalfExtent);
	AddVertsForAABB2D(m_sunIconVerts, sunIconBox, Rgba8::WHITE);
}

void PlayerController::SetWeaponAnimDef()
{
	auto spriteAnimDef = GetActor()->m_weapon->m_def.m_animDefs.find("Idle");
	if (spriteAnimDef != GetActor()->m_weapon->m_def.m_animDefs.end())
	{
		m_weaponAnimTimer = Timer(spriteAnimDef->second->GetAnimTime(), GetActor()->m_map->m_game->m_gameClock);
		PlayWeaponAnimationHud("Idle");
	}
}

void PlayerController::UpdateControllerInputPlayer(float deltaSeconds, int controllerID)
{
	if (controllerID != 0 && controllerID != 1)
	{
		ERROR_AND_DIE("Invaild controller ID for player");
	}
	Actor* curPlayer = GetActor();
	ActorDefinition* curActorDef = curPlayer->m_actorDef;
	Vec3 fwdDirection;
	Vec3 leftDirection;
	Vec3 upDirection;
	curPlayer->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
	m_camPosition = curPlayer->m_position + Vec3(0.f, 0.f, curActorDef->m_eyeHeight);

	XboxController const& controller = g_theInput->GetController(controllerID);
	if (controller.IsConnected())
	{
		float curMoveSpeed;
		if (controller.IsButtonDown(XboxButtonID::A))
		{
			curMoveSpeed = curActorDef->m_runSpeed;
		}
		else
		{
			curMoveSpeed = curActorDef->m_walkSpeed;
		}

		Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
		if (leftStickMagnitude > 0.f)
		{
			aimDirection -= controller.GetLeftStick().GetPosition().x * leftDirection;
			aimDirection += controller.GetLeftStick().GetPosition().y * fwdDirection;
		}

		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
		if (rightStickMagnitude > 0.f)
		{
			float yawChange = -controller.GetRightStick().GetPosition().x * curPlayer->m_actorDef->m_turnSpeed*deltaSeconds;
			float pitchChange = -controller.GetRightStick().GetPosition().y * curPlayer->m_actorDef->m_turnSpeed * deltaSeconds;

			curPlayer->m_orientation.m_yawDegrees += yawChange;
			m_camOrientation.m_yawDegrees = curPlayer->m_orientation.m_yawDegrees;
			m_camOrientation.m_pitchDegrees += pitchChange;
		}

		m_camOrientation.m_rollDegrees = 0.f;
		m_camOrientation.m_pitchDegrees = GetClamped(m_camOrientation.m_pitchDegrees, -85.f, 85.f);

		if (aimDirection.GetLengthSquared() > 0.f)
		{
			aimDirection.Normalized();
		}

		curPlayer->MoveInDirection(aimDirection, curMoveSpeed);

		if (controller.GetRightTrigger() > 0.5f)
		{
			if (Attack(m_camOrientation.GetForward_IFwd()))
			{
				PlayWeaponAnimationHud("Attack");
			}
		}

		if (controller.WasButtonJustPressed(XboxButtonID::DPAD_UP))
		{
			if (GetActor()->m_curWeaponIndex > 0)
			{
				GetActor()->m_curWeaponIndex--;
			}
			else
			{
				GetActor()->m_curWeaponIndex = (int)GetActor()->m_inventory.size() - 1;
			}
			GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::DPAD_DOWN))
		{
			if (GetActor()->m_curWeaponIndex < (int)GetActor()->m_inventory.size() - 1)
			{
				GetActor()->m_curWeaponIndex++;
			}
			else
			{
				GetActor()->m_curWeaponIndex = 0;
			}
			GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::X))
		{
			GetActor()->m_curWeaponIndex = 0;
			GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::Y))
		{
			GetActor()->m_curWeaponIndex = 1;
			GetActor()->EquipWeapon(GetActor()->m_curWeaponIndex);
		}

		if (!m_isWalking)
		{
			if (curPlayer->m_velocity.GetLengthSquared() > 0.1f)
			{
				curPlayer->PlayAnimation("Walk");
				m_isWalking = true;
			}
		}
		else
		{
			if (curPlayer->m_velocity.GetLengthSquared() < 0.1f)
			{
				curPlayer->m_animState = AnimState::FINISHED;
				m_isWalking = false;
			}
		}
	}
}

void PlayerController::Possess(ActorHandle actorHandle, int viewPortType)
{
	Controller::Possess(actorHandle);
	Actor* curPlayer = GetActor();
	if (curPlayer)
	{
		ActorDefinition* curActorDef = curPlayer->m_actorDef;
		m_camPosition = curPlayer->m_position + Vec3(0.f, 0.f, curActorDef->m_eyeHeight);
		m_camOrientation = curPlayer->m_orientation;
		
		IntVec2 clientDimensions = g_theWindow->GetClientDimensions(); 
		AABB2 viewport;
		if (viewPortType == 0)
		{
			viewport = AABB2(Vec2(0.f, (float)clientDimensions.y * 0.5f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
		}
		else if (viewPortType == 1)
		{
			viewport = AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y * 0.5f));
		}
		else if (viewPortType == -1)
		{
			viewport= AABB2(Vec2(0.f, 0.f), Vec2((float)clientDimensions.x, (float)clientDimensions.y));
		}
		m_playerCam.SetViewport(viewport);
		m_playerCam.SetPerspectiveView(viewport.GetDimensions().x / viewport.GetDimensions().y, 60.f, 0.1f, 100.f);
		m_playerCam.SetCameraToRenderTransform(Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f)));
		m_playerCam.SetPositionAndOrientation(m_camPosition, m_camOrientation);

		m_hudCam.SetViewport(viewport);
		m_hudCam.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
		m_hudCam.SetPositionAndOrientation(Vec3(0.f, 0.f, 0.f), EulerAngles(0.f, 0.f, 0.f));

		auto spriteAnimDef = GetActor()->m_weapon->m_def.m_animDefs.find("Idle");
		if (spriteAnimDef != GetActor()->m_weapon->m_def.m_animDefs.end())
		{
			m_weaponAnimTimer = Timer(spriteAnimDef->second->GetAnimTime(), GetActor()->m_map->m_game->m_gameClock);
			PlayWeaponAnimationHud("Idle");
		}
	}
}

Actor* PlayerController::GetActor() const
{
	return m_curMap->GetActorByHandle(m_actorHandle);
}

bool PlayerController::PlayWeaponAnimationHud(std::string name)
{
	auto spriteAnimDef = GetActor()->m_weapon->m_def.m_animDefs.find(name);
	if (spriteAnimDef != GetActor()->m_weapon->m_def.m_animDefs.end())
	{
		m_weaponAnimDef = spriteAnimDef->second;
		
		m_weaponAnimTimer.m_period = spriteAnimDef->second->GetAnimTime();
		m_weaponAnimTimer.Stop();
		m_weaponAnimTimer.Start();

		auto sheet = GetActor()->m_weapon->m_def.m_animSheet.find(name);
		if (sheet != GetActor()->m_weapon->m_def.m_animSheet.end())
		{
			m_curWeaponSheet = sheet->second;
		}
		return true;
	}
	return false;
}

void PlayerController::SetPlayerIndexAndControllerID(int playerIndex, int controllerIndex)
{
	m_playerIndex = playerIndex;
	m_controllerID = controllerIndex;
}

int PlayerController::GetPlayerControllerID()
{
	return m_controllerID;
}

void PlayerController::UpdateCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_playerCam.SetPosition(m_camPosition);
	m_playerCam.SetOrientation(m_camOrientation);
}

void PlayerController::UpdateHUD()
{
	m_hudBaseVerts.clear();
	m_hudWeaponVerts.clear();
	m_rectileVerts.clear();
	m_blackVerts.clear();
	if (GetActor()->m_isDead)
	{
		AABB2 blackBox = AABB2(Vec2::ZERO, Vec2(1600.f, 800.f));
		AddVertsForAABB2D(m_hudBaseVerts, blackBox, Rgba8::TRANSP_BLACK, Vec2(0.f, 0.f), Vec2(1.f, 1.f));
	}
	if (GetActor()->m_weapon->m_def.m_animDefs.size() != 0)
	{
		if (m_weaponAnimDef->GetPlaybackType() == SpriteAnimPlaybackType::ONCE&&m_weaponAnimTimer.GetElapsedFraction()>1.f)
		{
			PlayWeaponAnimationHud("Idle");
		}
		SpriteDefinition weaponSpriteDef=m_weaponAnimDef->GetSpriteDefAtTime((float)m_weaponAnimTimer.GetElapsedTime());


		AABB2 hudBaseBox = AABB2(Vec2::ZERO, Vec2(1600.f, 117.f));
		AddVertsForAABB2D(m_hudBaseVerts, hudBaseBox, Rgba8::WHITE, Vec2(0.f, 0.f), Vec2(1.f, 1.f));


		IntVec2 weaponSpriteSize = GetActor()->m_weapon->m_def.m_spriteSize;
		IntVec2 rectileSpriteSize = GetActor()->m_weapon->m_def.m_reticleSize;
		AABB2 weaponBox;
		AABB2 rectileBox;

		if (GetActor()->m_map->m_game->m_playerController0 && GetActor()->m_map->m_game->m_playerController1)
		{
			weaponBox = AABB2(Vec2(800.f, 255.f) - Vec2(0.5f * (float)weaponSpriteSize.x, (float)weaponSpriteSize.y),
				Vec2(800.f, 220.f) + Vec2(0.5f * (float)weaponSpriteSize.x, (float)weaponSpriteSize.y));
			rectileBox = AABB2(Vec2(800.f, 400.f) - Vec2(0.5f * (float)rectileSpriteSize.x, (float)rectileSpriteSize.y),
				Vec2(800.f, 400.f) + Vec2(0.5f * (float)rectileSpriteSize.x, (float)rectileSpriteSize.y));
		}
		else
		{
			weaponBox = AABB2(Vec2(800.f, 235.f) - Vec2(0.5f * (float)weaponSpriteSize.x, 0.5f * (float)weaponSpriteSize.y),
				Vec2(800.f, 220.f) + Vec2(0.5f * (float)weaponSpriteSize.x, 0.5f * (float)weaponSpriteSize.y));
			rectileBox = AABB2(Vec2(800.f, 400.f) - Vec2(0.5f * (float)rectileSpriteSize.x, 0.5f * (float)rectileSpriteSize.y),
				Vec2(800.f, 400.f) + Vec2(0.5f * (float)rectileSpriteSize.x, 0.5f * (float)rectileSpriteSize.y));
		}

		AddVertsForAABB2D(m_hudWeaponVerts, weaponBox, Rgba8::WHITE, weaponSpriteDef.GetUVs().m_mins, weaponSpriteDef.GetUVs().m_maxs);
		AddVertsForAABB2D(m_rectileVerts, rectileBox, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE);
	}
	int healthInInt = (int)GetActor()->m_curHealth;
	std::string curPlayerHealth = std::to_string(healthInInt);

	m_textVerts.clear();
	m_font->AddVertsForTextInBox2D(m_textVerts, curPlayerHealth, AABB2(Vec2(400.f, 50.f), Vec2(550.f, 100.f)), 50.f);
	m_font->AddVertsForTextInBox2D(m_textVerts, std::to_string(GetActor()->m_controller->m_kills), AABB2(Vec2(50.f, 50.f), Vec2(150.f, 100.f)), 50.f);
	m_font->AddVertsForTextInBox2D(m_textVerts, std::to_string(GetActor()->m_controller->m_death), AABB2(Vec2(1450.f, 50.f), Vec2(1550.f, 100.f)), 50.f);
}

void PlayerController::RenderPlayerHUD() const
{
	Actor* actor = GetActor();
	Texture* hudBase=g_theRenderer->CreateOrGetTextureFromFile(actor->m_weapon->m_def.m_baseTexturePath.c_str());
	Texture* rectile = g_theRenderer->CreateOrGetTextureFromFile(actor->m_weapon->m_def.m_reticleTexturePath.c_str());
	Texture* m_weaponAnimTexture=nullptr;
	auto it = actor->m_weapon->m_def.m_animSheet.find("Idle");
	if (it != actor->m_weapon->m_def.m_animSheet.end()) 
	{
		SpriteSheet* weaponAnimSheet = it->second;
		m_weaponAnimTexture = &weaponAnimSheet->GetTexture();
	}
	if (hudBase)
	{
		g_theRenderer->BeginCamera(m_hudCam);

		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(m_blackVerts);
		
		g_theRenderer->BindTexture(m_weaponAnimTexture);
		g_theRenderer->DrawVertexArray(m_hudWeaponVerts);
		g_theRenderer->BindTexture(hudBase);
		g_theRenderer->DrawVertexArray(m_hudBaseVerts);
		g_theRenderer->BindTexture(rectile);
		g_theRenderer->DrawVertexArray(m_rectileVerts);

		g_theRenderer->BindTexture(&m_font->GetTexture());
		g_theRenderer->DrawVertexArray(m_textVerts);

		g_theRenderer->EndCamera(m_hudCam);
	}


}

void PlayerController::RenderGoldHUD() const
{
	Actor* actor = GetActor();
	Texture* hudBase = g_theRenderer->CreateOrGetTextureFromFile(actor->m_weapon->m_def.m_baseTexturePath.c_str());
	Texture* rectile = g_theRenderer->CreateOrGetTextureFromFile(actor->m_weapon->m_def.m_reticleTexturePath.c_str());
	Texture* m_weaponAnimTexture = nullptr;
	auto it = actor->m_weapon->m_def.m_animSheet.find("Idle");
	if (it != actor->m_weapon->m_def.m_animSheet.end())
	{
		SpriteSheet* weaponAnimSheet = it->second;
		m_weaponAnimTexture = &weaponAnimSheet->GetTexture();
	}
	if (hudBase)
	{
		g_theRenderer->BeginCamera(m_hudCam);

		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(m_blackVerts);

		g_theRenderer->BindTexture(m_weaponAnimTexture);
		g_theRenderer->DrawVertexArray(m_hudWeaponVerts);
		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DoomUIBot.png"));
		g_theRenderer->DrawVertexArray(m_hudBaseVerts);
		g_theRenderer->BindTexture(rectile);
		g_theRenderer->DrawVertexArray(m_rectileVerts);

		g_theRenderer->BindTexture(&m_font->GetTexture());
		g_theRenderer->DrawVertexArray(m_textVerts);

		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ClockBase.png"));
		g_theRenderer->DrawVertexArray(m_clockBaseVerts);

		if (m_curMap->m_isSun)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SunIcon.png"));
		}
		else
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MoonIcon.png"));
		}
		
		g_theRenderer->DrawVertexArray(m_sunIconVerts);

		g_theRenderer->EndCamera(m_hudCam);
	}


}



//XmlElement* actorDefElement = rootElement->FirstChildElement();
//while (actorDefElement)
//{
//	std::string elementName = actorDefElement->Name();
//	GUARANTEE_OR_DIE(elementName == "ActorDefinition", Stringf("Root child element in %s was <%s>, must be <MapDefinition>!", filePath, elementName.c_str()));
//	ActorDefinition newMapDef = ActorDefinition(actorDefElement);
//	s_actorDefinitions.push_back(newMapDef);
//	actorDefElement = actorDefElement->NextSiblingElement();
//}