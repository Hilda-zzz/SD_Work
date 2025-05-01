//#include "Player.hpp"
//#include "Engine/Input/InputSystem.hpp"
//#include "Game/GameCommon.hpp"
//#include <Engine/Math/MathUtils.hpp>
//#include "Engine/Window/Window.hpp"
//#include "Game/Map.hpp"
//#include <Engine/Core/VertexUtils.hpp>
//#include "ActorDefinition.hpp"
//
//extern Window* g_theWindow;
//Player::Player(int actorDefIndex, Map* curMap):Actor(actorDefIndex,curMap)
//{
//	m_color = Rgba8::GREEN;
//
//	m_position = Vec3(3.f, 7.f, 0.f);
//	m_orientation = EulerAngles(0.f, 0.f, 0.f);
//	m_playerCam = Camera();
//	m_camPosition = m_position;
//	m_camOrientation = m_orientation;
//	
//	m_eyeHeight = ActorDefinition::s_actorDefinitions[actorDefIndex].m_eyeHeight;
//	m_cameraFOV = ActorDefinition::s_actorDefinitions[actorDefIndex].m_cameraFOVDegrees;
//
//	m_camPosition = m_position + Vec3(0.f, 0.f, m_eyeHeight);
//	m_camOrientation = m_orientation;
//	m_playerCam.SetCameraToRenderTransform(Mat44(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f)));
//	m_playerCam.SetPositionAndOrientation(m_camPosition, m_camOrientation);
//	
//	m_playerCam.SetPerspectiveView(2.f, m_cameraFOV, 0.1f, 100.f);
//
//	AddVertsForCylinder3D(m_vertexs, m_physicCollider.m_center - Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
//		m_physicCollider.m_center + Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
//		m_physicCollider.m_radius, m_color, AABB2::ZERO_TO_ONE, 16);
//
//	AddVertsForCylinder3D(m_vertexsWire, m_physicCollider.m_center - Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
//		m_physicCollider.m_center + Vec3(0.f, 0.f, m_physicCollider.m_halfHeight),
//		m_physicCollider.m_radius + 0.001f, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);
//
//	AddVertsForCone3D(m_vertexs,  Vec3(m_physicCollider.m_radius, 0.f, m_eyeHeight),
//		Vec3(m_physicCollider.m_radius, 0.f, m_eyeHeight) + m_orientation.GetForward_IFwd()*0.1f,
//		0.1f, m_color, AABB2::ZERO_TO_ONE, 16);
//
//	AddVertsForCone3D(m_vertexsWire, Vec3(m_physicCollider.m_radius, 0.f, m_eyeHeight),
//		Vec3(m_physicCollider.m_radius, 0.f, m_eyeHeight) + m_orientation.GetForward_IFwd() * 0.101f,
//		0.101f,Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);
//}
//
//void Player::Update(float deltaSeconds)
//{
//	m_velocity = Vec3(0.f, 0.f, 0.f);
//	m_angularVelocity = EulerAngles(0.f, 0.f, 0.f);
//	Mat44 rollMat = Mat44();
//
//	if (g_theInput->WasKeyJustPressed('F'))
//	{
//		int newMode = ((int)m_playerMode + 1) % ((int)PlayerMode::COUNT);
//		m_playerMode = PlayerMode(newMode);
//	}
//
//	//--------------------------------------------------------
//	
//	if (m_playerMode == PlayerMode::FREE_CAMERA)
//	{
//		UpdateKBInputCam(deltaSeconds);
//		UpdateControllerInputCam(deltaSeconds);
//	}
//
//	UpdateCamera();
//}
//
//void Player::Render() const
//{
//	Actor::Render();
//}
//
//Player::~Player()
//{
//}
//
//void Player::UpdateKBInputCam(float deltaSeconds)
//{
//	Vec3 fwdDirection;
//	Vec3 leftDirection;
//	Vec3 upDirection;
//	m_camOrientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
//
//	float curMoveSpeed;
//	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
//	{
//		curMoveSpeed=m_moveSpeed*10.f;
//	}
//	else
//	{
//		curMoveSpeed = m_moveSpeed;
//	}
//
//	m_velocity = Vec3(0.f, 0.f, 0.f);
//	Vec3 aimDirection = Vec3(0.f, 0.f, 0.f);
//	if (g_theInput->IsKeyDown('W'))
//	{
//		aimDirection += fwdDirection;
//	}
//	if (g_theInput->IsKeyDown('S'))
//	{
//		aimDirection -= fwdDirection;
//	}
//	if (g_theInput->IsKeyDown('A'))
//	{
//		aimDirection += leftDirection;
//	}
//	if (g_theInput->IsKeyDown('D'))
//	{
//		aimDirection -= leftDirection;
//	}
//	if (g_theInput->IsKeyDown('Z'))
//	{
//		m_camPosition += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
//	}
//	if (g_theInput->IsKeyDown('C'))
//	{
//		if (m_playerMode == PlayerMode::FREE_CAMERA)
//		{
//			m_camPosition += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
//		}
//	}
//
//	m_angularVelocity.m_rollDegrees = 0.f;
//	if (g_theInput->IsKeyDown('Q'))
//	{
//		m_angularVelocity.m_rollDegrees += -m_rollSpeed*deltaSeconds;
//	}
//	if (g_theInput->IsKeyDown('E'))
//	{
//		m_angularVelocity.m_rollDegrees += m_rollSpeed * deltaSeconds;
//	}
//
//	m_angularVelocity.m_yawDegrees = -g_theInput->GetCursorClientDelta().x * g_theWindow->GetClientDimensions().x;
//	m_angularVelocity.m_pitchDegrees = g_theInput->GetCursorClientDelta().y * g_theWindow->GetClientDimensions().y;
//
//	m_angularVelocity.m_yawDegrees *= m_yawPitchRatio;
//	m_angularVelocity.m_pitchDegrees *= m_yawPitchRatio;
//	m_camOrientation = m_camOrientation + m_angularVelocity;
//	m_camOrientation.m_rollDegrees = GetClamped(m_camOrientation.m_rollDegrees, -45.f, 45.f);
//	m_camOrientation.m_pitchDegrees = GetClamped(m_camOrientation.m_pitchDegrees, -85.f, 85.f);
//
//	aimDirection.Normalized();
//	aimDirection.SetLength(curMoveSpeed);
//	m_velocity = aimDirection;
//
//
//	m_camPosition += m_velocity * deltaSeconds;
//
//	if (g_theInput->IsKeyDown('H'))
//	{
//		m_camOrientation.m_pitchDegrees = 0.f;
//		m_camOrientation.m_rollDegrees = 0.f;
//		m_camOrientation.m_yawDegrees = 0.f;
//		m_camPosition = Vec3(0.f, 0.f, 0.f);
//	}
//}
//
// void Player::UpdateControllerInputCam(float deltaSeconds)
// {
// 	Vec3 fwdDirection;
// 	Vec3 leftDirection;
// 	Vec3 upDirection;
// 	m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwdDirection, leftDirection, upDirection);
// 
// 	XboxController const& controller = g_theInput->GetController(0);
// 	if (controller.IsConnected())
// 	{
// 		float curMoveSpeed;
// 		if (controller.IsButtongDown(XboxButtonID::A))
// 		{
// 			curMoveSpeed = m_moveSpeed * 10.f;
// 		}
// 		else
// 		{
// 			curMoveSpeed = m_moveSpeed;
// 		}
// 
// 		Vec3 moveDirection = Vec3();
// 		float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
// 		if (leftStickMagnitude > 0.f)
// 		{
// 			m_velocity = Vec3(0.f, 0.f, 0.f);
// 			moveDirection -= controller.GetLeftStick().GetPosition().x * leftDirection;
// 			moveDirection += controller.GetLeftStick().GetPosition().y * fwdDirection;
// 
// 		}
// 		if (controller.IsButtongDown(XboxButtonID::LEFT_SHOULDER))
// 		{
// 			m_position += Vec3(0.f, 0.f, 1.f) * curMoveSpeed * deltaSeconds;
// 		}
// 		if (controller.IsButtongDown(XboxButtonID::RIGHT_SHOULDER))
// 		{
// 			m_position += Vec3(0.f, 0.f, -1.f) * curMoveSpeed * deltaSeconds;
// 		}
// 		moveDirection.Normalized();
// 		m_velocity = moveDirection * (leftStickMagnitude * curMoveSpeed);
// 
// 		float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
// 		if (rightStickMagnitude > 0.f)
// 		{
// 			m_angularVelocity.m_yawDegrees = -controller.GetRightStick().GetPosition().x * m_yawSpeed;
// 			m_angularVelocity.m_pitchDegrees = -controller.GetRightStick().GetPosition().y * m_pitchSpeed;
// 		}
// 
// 		m_angularVelocity.m_rollDegrees = 0.f;
// 		if (controller.GetLeftTrigger() > 0.f)
// 		{
// 			m_angularVelocity.m_rollDegrees += -m_rollSpeed * controller.GetLeftTrigger();
// 		}
// 		if (controller.GetRightTrigger() > 0.f)
// 		{
// 			m_angularVelocity.m_rollDegrees += m_rollSpeed * controller.GetRightTrigger();
// 		}
// 
// 		m_orientation = m_orientation + m_angularVelocity * deltaSeconds;
// 		m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
// 		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
// 		m_position += m_velocity * deltaSeconds;
// 
// 		if (controller.IsButtongDown(XboxButtonID::START))
// 		{
// 			m_orientation.m_pitchDegrees = 0.f;
// 			m_orientation.m_rollDegrees = 0.f;
// 			m_orientation.m_yawDegrees = 0.f;
// 			m_position = Vec3(0.f, 0.f, 0.f);
// 		}
// 	}
// }
//
//void Player::UpdateCamera()
//{
//	m_playerCam.SetPosition(m_camPosition);
//	m_playerCam.SetOrientation(m_camOrientation);
//}
