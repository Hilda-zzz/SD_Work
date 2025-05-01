#include "GamePachinko2D.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

constexpr float ARROW_SPEED = 180.f;
constexpr float	BALL_MIN_RADIUS = 5.f;
constexpr float	BALL_MAX_RADIUS = 25.f;

GamePachinko2D::GamePachinko2D()
{
	ParseGameConfig();

	g_theInput->SetCursorMode(CursorMode::POINTER);
	g_theWindow->SetCursorVisible(true);

	m_gameClock = new Clock();

	m_arrow = RaycastArrow(Vec2(100.f, 50.f), Vec2(2.f, 1.f), 1400.f);

	m_balls.reserve(2000);
	GenerateBumper2D();

}

GamePachinko2D::~GamePachinko2D()
{
	m_gameClock->m_parent->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void GamePachinko2D::Update()
{
	m_curDeltaTime = (float)m_gameClock->GetDeltaSeconds();
	if (g_theInput->IsKeyDown('T'))
	{
		m_curDeltaTime *= 0.05f;
	}

	UpdateCamera(m_curDeltaTime);
	//--------------------------------------------------------
	UpdateInput(m_curDeltaTime);

	if (m_isUseFixedUpdate)
	{
		m_phyOwe += m_curDeltaTime;
		while (m_phyOwe > m_fixedTimeStep)
		{
			m_phyOwe -= m_fixedTimeStep;
			UpdatePhysics(m_fixedTimeStep);
		}
	}
	else
	{
		UpdatePhysics(m_curDeltaTime);
	}


	m_ballVerts.clear();
	m_ballVerts.reserve(64 * 3 * m_balls.size());
	for (int i = 0; i < (int)m_balls.size(); i++)
	{
		AddVertsForDisc2D(m_ballVerts, m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_color);
	}
}

void GamePachinko2D::UpdatePhysics(float fixedTimeStep)
{
	//---------------------Physics--------------------------------
	// add gravity and move
	for (int i = 0; i < (int)m_balls.size(); i++)
	{
		m_balls[i].Update(fixedTimeStep);
	}
	// ball vs ball
	for (int i = 0; i <(int) m_balls.size(); i++)
	{
		for (int j =i+1; j <(int) m_balls.size(); j++)
		{
			if ((m_balls[i].m_radius + m_balls[j].m_radius) * (m_balls[i].m_radius + m_balls[j].m_radius) > GetDistanceSquared2D(m_balls[i].m_pos, m_balls[j].m_pos))
			{
				BounceDiscOffEachOther(m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_velocity, m_balls[i].m_elasticity,
					m_balls[j].m_pos, m_balls[j].m_radius, m_balls[j].m_velocity, m_balls[j].m_elasticity);
			}
		}
	}
	//ball vs bumper
 	for (int i = 0; i < (int)m_balls.size(); i++)
 	{
 		for (int j = 0; j <(int) m_bumpers.size(); j++)
 		{
 			if (m_bumpers[j].m_type == BumperType::DISC)
 			{
 				BounceDiscOffStaticDisc2D(m_bumpers[j].m_discPos, m_bumpers[j].m_discRadius,
 					m_balls[i].m_pos, m_balls[i].m_velocity, m_balls[i].m_radius, m_bumpers[j].m_elasticity, m_ballElasicity);
 			}
 			else if (m_bumpers[j].m_type == BumperType::CAPSULE2)
 			{
  				BounceDiscOffCapsule2D(m_bumpers[j].m_capsule,
  					m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_velocity, m_bumpers[j].m_elasticity, m_ballElasicity);
 			}
  			else if (m_bumpers[j].m_type == BumperType::OBB2)
  			{
  				BounceDiscOffOBB2D(m_bumpers[j].m_obbBox,
  					m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_velocity, m_bumpers[j].m_elasticity, m_ballElasicity);
  			}
 		}
 	}
 	//ball vs wall
	OBB2 groundBox = OBB2(Vec2(800.f, -25.f), Vec2(1.f, 0.f), Vec2(800.f, 25.f));
	OBB2 leftBox= OBB2(Vec2(-25.f, 400.f), Vec2(1.f, 0.f), Vec2(25.f, 700.f));
	OBB2 rightBox = OBB2(Vec2(1625.f, 400.f), Vec2(1.f, 0.f), Vec2(25.f, 700.f));
  	for (int i = 0; i <(int) m_balls.size(); i++)
  	{
  		if (m_isFloor)
  		{
  			if (m_balls[i].m_pos.y < m_balls[i].m_radius)
  			{
//   				//ground
//   				if (m_balls[i].m_velocity.y < 0.f)
//   				{
// 					m_balls[i].m_pos.y = m_balls[i].m_radius;
// 					m_balls[i].m_velocity.y = -m_balls[i].m_velocity.y * m_balls[i].m_elasticity * m_wallElasicity;
// //   				BounceDiscOffPoint(Vec2(m_balls[i].m_pos.x, 0.f),
// //   							m_balls[i].m_pos, m_balls[i].m_velocity, m_balls[i].m_radius, m_balls[i].m_elasticity * m_wallElasicity);
//   				
//   				}
				BounceDiscOffOBB2D(groundBox, m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_velocity, m_wallElasicity, m_ballElasicity);
  			}
  		}
  
  		if (m_balls[i].m_pos.x < m_balls[i].m_radius)
  		{
//   			//left wall
//   			if (m_balls[i].m_velocity.x < 0.f)
//   			{
// 				m_balls[i].m_pos.x = m_balls[i].m_radius;
// 				m_balls[i].m_velocity.x = -m_balls[i].m_velocity.x * m_balls[i].m_elasticity * m_wallElasicity;
// //   					BounceDiscOffPoint(Vec2(0.f, m_balls[i].m_pos.y),
// //   						m_balls[i].m_pos, m_balls[i].m_velocity, m_balls[i].m_radius, m_balls[i].m_elasticity * m_wallElasicity);
// 
//   			}
			BounceDiscOffOBB2D(leftBox, m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_velocity, m_wallElasicity, m_ballElasicity);
  		}
  
  		if (m_balls[i].m_pos.x > SCREEN_SIZE_X - m_balls[i].m_radius)
  		{
//   			//right wall
//   			if (m_balls[i].m_velocity.x > 0.f)
//   			{
// 				m_balls[i].m_pos.x = SCREEN_SIZE_X - m_balls[i].m_radius;
// 				m_balls[i].m_velocity.x = -m_balls[i].m_velocity.x * m_balls[i].m_elasticity * m_wallElasicity;
// // 				BounceDiscOffPoint(Vec2(SCREEN_SIZE_X, m_balls[i].m_pos.y),
// //   						m_balls[i].m_pos, m_balls[i].m_velocity, m_balls[i].m_radius, m_balls[i].m_elasticity * m_wallElasicity);
//   			}
			BounceDiscOffOBB2D(rightBox, m_balls[i].m_pos, m_balls[i].m_radius, m_balls[i].m_velocity, m_wallElasicity, m_ballElasicity);
  		}
  		
  	}
}

void GamePachinko2D::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	
	//-----------bumper-------------------
	g_theRenderer->DrawVertexArray(m_bumpersVerts);
	//--------------draw ball-------------------
	g_theRenderer->DrawVertexArray(m_ballVerts);
	//-----------draw title-------------------
	//g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	font->AddVertsForTextInBox2D(title, "Mode (F6/F7 for prev/next): Pachinko Machine(2D)", AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));

	std::string message = "F8 to randomize; LMB/RMB/ESDf/IJKL move; Hold T= slow; N/Space= ball; B= Toggle Floor; G/H= de/increase balls ela; [/]= de/increase fixed time step";
	font->AddVertsForTextInBox2D(title, message, AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));

	std::string gameModeMessage = "Play Mode(Press P) = Fixed Time Mode";
	std::string gameModeMessage2 = "Play Mode(Press P) = Actual Time Mode";
	if (m_isUseFixedUpdate)
	{
		font->AddVertsForTextInBox2D(title, gameModeMessage, AABB2(Vec2(10.f, 725.f), Vec2(1600.f, 745.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));
	}
	else
	{
		font->AddVertsForTextInBox2D(title, gameModeMessage2, AABB2(Vec2(10.f, 725.f), Vec2(1600.f, 745.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));
	}
	

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(title);
	//--------------arrow-----------------------
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	std::vector<Vertex_PCU> arrow_verts;
	AddVertsForArrow2D(arrow_verts, m_arrow.m_startPos, m_arrow.m_endPos_whole, 15.f, 2.f, Rgba8::WHITE);
	// 	g_theRenderer->SetModelConstants();
	// 	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(arrow_verts);
	DebugDrawRing(2.f, m_pachinkoMinBallRadius, Rgba8::HILDA, m_arrow.m_startPos);
	DebugDrawRing(2.f, m_pachinkoMaxBallRadius, Rgba8::HILDA, m_arrow.m_startPos);
	//-----------draw stats panel-------------------
	std::vector<Vertex_PCU> panelVerts;
	//AddVertsForAABB2D(panelVerts, AABB2(Vec2(10.f, 10.f), Vec2(600.f, 300.f)), Rgba8::HILDA);

	float curPhysicsTime;
	if (m_isUseFixedUpdate)
	{
		curPhysicsTime = m_fixedTimeStep;
	}
	else
	{
		curPhysicsTime = m_curDeltaTime;
	}

// 	std::string statsMessage = "Ball Count = " + std::to_string(m_ballCount)+'\n'
// 		+"Physics Time Step = " + std::to_string(curPhysicsTime*1000.f) + '\n'
// 		+"DT = " + std::to_string(m_curDeltaTime * 1000.f) + '\n'
// 		+"Framerate = " + std::to_string(1.f/m_curDeltaTime) + '\n';
	
	char buffer[256];
	sprintf_s(buffer, "Ball Count = %d\n"
		"Physics Time Step = %.2f\n"
		"DT = %.2f\n"
		"Framerate = %.2f\n",
		(int)m_ballCount,
		curPhysicsTime * 1000.f,
		m_curDeltaTime * 1000.f,
		1.f / m_curDeltaTime);
	std::string statsMessage(buffer);

	font->AddVertsForTextInBox2D(panelVerts, statsMessage, 
		AABB2(Vec2(10.f, 500.f), Vec2(600.f, 700.f)), 15.f, Rgba8::WHITE, 0.7f, Vec2(0.f, 1.f));

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(panelVerts);



	g_theRenderer->EndCamera(m_screenCamera);

	title.clear();
	panelVerts.clear();
}

void GamePachinko2D::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
		m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);
}

void GamePachinko2D::UpdateInput(float deltaTime)
{
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
		m_arrow.m_startPos = m_worldUV.GetPointAtUV(mousePos);
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
		m_arrow.m_endPos_whole = m_worldUV.GetPointAtUV(mousePos);
	}
	UpdateKBInputForArrow(deltaTime, m_arrow, ARROW_SPEED);

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE)||g_theInput->IsKeyDown('N'))
	{
		GenerateEachBall();
	}

	if (g_theInput->WasKeyJustPressed('B'))
	{
		m_isFloor = !m_isFloor;
	}

	if (g_theInput->WasKeyJustPressed('G'))
	{
		ChangeBallElasicity(-0.05f);
	}

	if (g_theInput->WasKeyJustPressed('H'))
	{
		ChangeBallElasicity(0.05f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTBRACKET))
	{
		m_fixedTimeStep /= 1.1f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTBRACKET))
	{
		m_fixedTimeStep *= 1.1f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		ReRandomObject();
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_isUseFixedUpdate = !m_isUseFixedUpdate;
		m_phyOwe = 0.f;
	}
}

void GamePachinko2D::UpdateKBInputForArrow(float& deltaTime, RaycastArrow& arrow, float speed)
{
	if (g_theInput->IsKeyDown('E'))
	{
		arrow.m_startPos.y += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		arrow.m_startPos.x -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		arrow.m_startPos.y -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('F'))
	{
		arrow.m_startPos.x += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('I'))
	{
		arrow.m_endPos_whole.y += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		arrow.m_endPos_whole.x -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		arrow.m_endPos_whole.y -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		arrow.m_endPos_whole.x += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		arrow.m_startPos.y += speed * deltaTime;
		arrow.m_endPos_whole.y += speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		arrow.m_startPos.x -= speed * deltaTime;
		arrow.m_endPos_whole.x -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		arrow.m_startPos.y -= speed * deltaTime;
		arrow.m_endPos_whole.y -= speed * deltaTime;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		arrow.m_startPos.x += speed * deltaTime;
		arrow.m_endPos_whole.x += speed * deltaTime;
	}
}

void GamePachinko2D::ParseGameConfig()
{
	XmlDocument gameConfigDefsDoc;
	char const* filePath = "Data/GameConfig.xml";
	XmlResult result = gameConfigDefsDoc.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required game config file \"%s\"", filePath));

	XmlElement* rootElement = gameConfigDefsDoc.RootElement();
	if (!rootElement)
	{
		GUARANTEE_OR_DIE(rootElement, "Actor definitions file has no root element");
		return;
	}

	//XmlElement* gameDefElement = rootElement->FirstChildElement();
	XmlElement* gameDefElement = rootElement;
	m_pachinkoMinBallRadius = ParseXmlAttribute(gameDefElement, "pachinkoMinBallRadius", m_pachinkoMinBallRadius);
	m_pachinkoMaxBallRadius = ParseXmlAttribute(gameDefElement, "pachinkoMaxBallRadius", m_pachinkoMaxBallRadius);
	m_pachinkoNumDiscBumpers = ParseXmlAttribute(gameDefElement, "pachinkoNumDiscBumpers", m_pachinkoNumDiscBumpers);
	m_pachinkoMinDiscBumperRadius = ParseXmlAttribute(gameDefElement, "pachinkoMinDiscBumperRadius", m_pachinkoMinDiscBumperRadius);
	m_pachinkoMaxDiscBumperRadius = ParseXmlAttribute(gameDefElement, "pachinkoMaxDiscBumperRadius", m_pachinkoMaxDiscBumperRadius);
	m_pachinkoNumCapsuleBumpers = ParseXmlAttribute(gameDefElement, "pachinkoNumCapsuleBumpers", m_pachinkoNumCapsuleBumpers);
	m_pachinkoMinCapsuleBumperLength = ParseXmlAttribute(gameDefElement, "pachinkoMinCapsuleBumperLength", m_pachinkoMinCapsuleBumperLength);
	m_pachinkoMaxCapsuleBumperLength = ParseXmlAttribute(gameDefElement, "pachinkoMaxCapsuleBumperLength", m_pachinkoMaxCapsuleBumperLength);
	m_pachinkoMinCapsuleBumperRadius = ParseXmlAttribute(gameDefElement, "pachinkoMinCapsuleBumperRadius", m_pachinkoMinCapsuleBumperRadius);
	m_pachinkoMaxCapsuleBumperRadius = ParseXmlAttribute(gameDefElement, "pachinkoMaxCapsuleBumperRadius", m_pachinkoMaxCapsuleBumperRadius);
	m_pachinkoNumObbBumpers = ParseXmlAttribute(gameDefElement, "pachinkoNumObbBumpers", m_pachinkoNumObbBumpers);
	m_pachinkoMinObbBumperWidth = ParseXmlAttribute(gameDefElement, "pachinkoMinObbBumperWidth", m_pachinkoMinObbBumperWidth);
	m_pachinkoMaxObbBumperWidth = ParseXmlAttribute(gameDefElement, "pachinkoMaxObbBumperWidth", m_pachinkoMaxObbBumperWidth);
	m_pachinkoWallElasticity = ParseXmlAttribute(gameDefElement, "pachinkoWallElasticity", m_pachinkoWallElasticity);
	m_pachinkoMinBumperElasticity = ParseXmlAttribute(gameDefElement, "pachinkoMinBumperElasticity", m_pachinkoMinBumperElasticity);
	m_pachinkoMaxBumperElasticity = ParseXmlAttribute(gameDefElement, "pachinkoMaxBumperElasticity", m_pachinkoMaxBumperElasticity);
	m_pachinkoExtraWarpHeight = ParseXmlAttribute(gameDefElement, "pachinkoExtraWarpHeight", m_pachinkoExtraWarpHeight);
}

void GamePachinko2D::GenerateEachBall()
{
	float radius = m_rng.RollRandomFloatInRange(m_pachinkoMinBallRadius, m_pachinkoMaxBallRadius);
	float colorRatio = m_rng.RollRandomFloatZeroToOne();
	Rgba8 curColor = Interpolate(Rgba8::WHITE, Rgba8(0,50,150), colorRatio);
	Ball newBall = Ball(m_arrow.m_startPos, radius, m_ballElasicity,curColor);

	float arrowLen = (m_arrow.m_endPos_whole - m_arrow.m_startPos).GetLength();
	Vec2 dir = (m_arrow.m_endPos_whole - m_arrow.m_startPos).GetNormalized();
	// add impulse
	newBall.m_velocity += 3.f * arrowLen * dir;
	m_balls.push_back(newBall);
	++m_ballCount;
}

void GamePachinko2D::GenerateBumper2D()
{
	m_bumpers.reserve(30);
	m_bumpersVerts.reserve(1500);
	for (int i = 0; i < m_pachinkoNumDiscBumpers; i++)
	{
		//disc
		float x=m_rng.RollRandomFloatInRange(100.f, 1550.f);
		float y = m_rng.RollRandomFloatInRange(30.f, 770.f);
		float radius = m_rng.RollRandomFloatInRange(m_pachinkoMinDiscBumperRadius, m_pachinkoMaxDiscBumperRadius);
		float elaticity = m_rng.RollRandomFloatInRange(m_pachinkoMinBumperElasticity, m_pachinkoMaxBumperElasticity);

		Rgba8 curColor = Interpolate(Rgba8(75, 0, 130), Rgba8(255, 223, 0), elaticity);

		Bumper2D newDiscBumper = Bumper2D(BumperType::DISC, Vec2(x, y), radius,elaticity);
		m_bumpers.push_back(newDiscBumper);

		AddVertsForDisc2D(m_bumpersVerts, Vec2(x, y), radius, curColor);
	}
	for (int i = 0; i < m_pachinkoNumCapsuleBumpers; i++)
	{
		//capsule
 		Vec2 cap_start = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
 			m_rng.RollRandomFloatInRange(50.f, 750.f));
 		Vec2 cap_end = Vec2(cap_start.x + m_rng.RollRandomFloatInRange(10.f, 80.f),
 			cap_start.y + m_rng.RollRandomFloatInRange(10.f, 80.f));
 		float cap_radius = m_rng.RollRandomFloatInRange(m_pachinkoMinCapsuleBumperRadius, m_pachinkoMaxCapsuleBumperRadius);
 		float elaticity = m_rng.RollRandomFloatInRange(0.01f, 0.99f);
 		Rgba8 curColor = Interpolate(Rgba8(75, 0, 130), Rgba8(255, 223, 0), elaticity);
 		Capsule2 capsule = Capsule2(LineSegment2(cap_start, cap_end), cap_radius);
 		Bumper2D newCapBumper = Bumper2D(BumperType::CAPSULE2, capsule, elaticity);
 		m_bumpers.push_back(newCapBumper);
 
 		AddVertsForCapsule2D(m_bumpersVerts, capsule.m_bone.m_start, capsule.m_bone.m_end,capsule.m_radius, curColor);
	}
	for (int i = 0; i < 10; i++)
	{
		//obb2
		Vec2 obb_center = Vec2(m_rng.RollRandomFloatInRange(100.f, 1400.f),
			m_rng.RollRandomFloatInRange(50.f, 750.f));
		Vec2 obb_halfDimensions = Vec2(m_rng.RollRandomFloatInRange(20.f, 90.f),
			m_rng.RollRandomFloatInRange(20.f, 90.f));
		Vec2 obb_iBasisNormal = Vec2::MakeFromPolarDegrees(m_rng.RollRandomFloatInRange(0.f, 360.f));
		float elaticity = m_rng.RollRandomFloatInRange(0.01f, 0.99f);
		Rgba8 curColor = Interpolate(Rgba8(75, 0, 130), Rgba8(255, 223, 0), elaticity);
		OBB2 box = OBB2(obb_center, obb_iBasisNormal, obb_halfDimensions);
		Bumper2D newOBBBumper = Bumper2D(BumperType::OBB2, box, elaticity);
		m_bumpers.push_back(newOBBBumper);
		AddVertsForOBB2D(m_bumpersVerts,box, curColor);
	}
}

void GamePachinko2D::ReRandomObject()
{
	m_ballCount = 0;
	m_balls.clear();
	m_ballVerts.clear();
	m_bumpers.clear();
	m_bumpersVerts.clear();
	GenerateBumper2D();
}

void GamePachinko2D::ChangeBallElasicity(float change)
{
	m_ballElasicity += change;
	m_ballElasicity=GetClampedZeroToOne(m_ballElasicity);
	for (Ball ball : m_balls)
	{
		ball.m_elasticity = m_ballElasicity;
	}
}
