#include "GameBox2DTest.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/OBB2.hpp"
#include <ThirdParty/box2d/include/box2d/types.h>
#include <ThirdParty/box2d/include/box2d/box2d.h>


GameBox2DTest::GameBox2DTest()
{
	g_theInput->SetCursorMode(CursorMode::POINTER);
	g_theWindow->SetCursorVisible(true);

	m_gameClock = new Clock();

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = { 0.0f, -9.8f }; 
	m_worldId = b2CreateWorld(&worldDef);

	// Ground
	{
		b2BodyDef groundBodyDef = b2DefaultBodyDef();
		groundBodyDef.type = b2_staticBody;
		groundBodyDef.position = { SCREEN_SIZE_X * 0.5f * METERS_PER_PIXEL,
								   2.0f * METERS_PER_PIXEL };
		m_groundBodyId = b2CreateBody(m_worldId, &groundBodyDef);

		b2Polygon groundBox = b2MakeBox(SCREEN_SIZE_X * 0.5f * METERS_PER_PIXEL, 2.0f);

		b2ShapeDef groundShapeDef = b2DefaultShapeDef();
		groundShapeDef.material.friction = 0.5f;
		groundShapeDef.material.restitution = 0.3f;
		b2CreatePolygonShape(m_groundBodyId, &groundShapeDef, &groundBox);
	}

	m_selectedBodyId = b2_nullBodyId;

	for (int i = 0; i < 5; ++i)
	{
		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = { (400.f + i * 150.f) * METERS_PER_PIXEL,
							(400.f + i * 80.f) * METERS_PER_PIXEL };
		bodyDef.rotation = b2MakeRot(0.2f * i);  

		b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

		b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);  // 1x1 meter

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.material.friction = 0.3f;
		shapeDef.material.restitution = 0.5f;
		b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

		m_dynamicBoxes.push_back(bodyId);
	}
}

GameBox2DTest::~GameBox2DTest()
{
	// cleanup box2d world
	if (B2_IS_NON_NULL(m_worldId))
	{
		b2DestroyWorld(m_worldId);
	}

	if (m_gameClock)
	{
		m_gameClock->m_parent->RemoveChild(m_gameClock);
		delete m_gameClock;
		m_gameClock = nullptr;
	}
}

void GameBox2DTest::Update()
{
	m_curDeltaTime = (float)m_gameClock->GetDeltaSeconds();

	float deltaTime = m_curDeltaTime;
	if (g_theInput->IsKeyDown('T'))
	{
		deltaTime *= 0.1f;
	}

	UpdateCamera(deltaTime);
	UpdateInput(deltaTime);

	m_physicsAccumulator += deltaTime;
	while (m_physicsAccumulator >= m_fixedTimeStep)
	{
		m_physicsAccumulator -= m_fixedTimeStep;
		UpdatePhysics(m_fixedTimeStep);
	}
}

void GameBox2DTest::UpdatePhysics(float fixedTimeStep)
{
	if (m_isDragging && B2_IS_NON_NULL(m_selectedBodyId))
	{
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		Vec2 mouseWorldPos = m_worldUV.GetPointAtUV(mouseUV);

		b2Vec2 targetPos = { mouseWorldPos.x * METERS_PER_PIXEL,
							mouseWorldPos.y * METERS_PER_PIXEL };

		b2Vec2 bodyPos = b2Body_GetPosition(m_selectedBodyId);
		b2Vec2 bodyVel = b2Body_GetLinearVelocity(m_selectedBodyId);

		b2Vec2 displacement = { targetPos.x - bodyPos.x, targetPos.y - bodyPos.y };
		b2Vec2 springForce = { displacement.x * DRAG_STIFFNESS,
							  displacement.y * DRAG_STIFFNESS };

		b2Vec2 dampingForce = { -bodyVel.x * DRAG_DAMPING, -bodyVel.y * DRAG_DAMPING };

		b2Vec2 totalForce = { springForce.x + dampingForce.x,
							 springForce.y + dampingForce.y };

		float mass = b2Body_GetMass(m_selectedBodyId);
		b2Body_ApplyForceToCenter(m_selectedBodyId,
			{ totalForce.x * mass, totalForce.y * mass },
			true);

		// 减少角速度，让拖拽时物体不会旋转太多
		b2Body_SetAngularVelocity(m_selectedBodyId,
			b2Body_GetAngularVelocity(m_selectedBodyId) * 0.9f);
	}

	// Update
	b2World_Step(m_worldId, fixedTimeStep, 4);  // 4是子步数
}

void GameBox2DTest::UpdateInput(float deltaTime)
{
	UNUSED(deltaTime);

	HandleMouseInput();

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		Vec2 mousePos = g_theWindow->GetNormalizedMouseUV();
		Vec2 worldPos = m_worldUV.GetPointAtUV(mousePos);

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = { worldPos.x * METERS_PER_PIXEL, worldPos.y * METERS_PER_PIXEL };

		b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

		b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.material.friction = 0.3f;
		shapeDef.material.restitution = 0.5f;
		b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

		m_dynamicBoxes.push_back(bodyId);
	}

	// R reset
	if (g_theInput->WasKeyJustPressed('R'))
	{
		// delete all boxes
		for (b2BodyId bodyId : m_dynamicBoxes)
		{
			if (B2_IS_NON_NULL(bodyId))
			{
				b2DestroyBody(bodyId);
			}
		}
		m_dynamicBoxes.clear();
		m_selectedBodyId = b2_nullBodyId;
		m_isDragging = false;

		for (int i = 0; i < 5; ++i)
		{
			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = b2_dynamicBody;
			bodyDef.position = { (400.f + i * 150.f) * METERS_PER_PIXEL,
								(400.f + i * 80.f) * METERS_PER_PIXEL };

			b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

			b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = 1.0f;
			shapeDef.material.friction = 0.3f;
			shapeDef.material.restitution = 0.5f;
			b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

			m_dynamicBoxes.push_back(bodyId);
		}
	}
}

void GameBox2DTest::HandleMouseInput()
{
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	Vec2 mouseWorldPos = m_worldUV.GetPointAtUV(mouseUV);

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		if (!m_isDragging)
		{
			b2BodyId bodyId = GetBodyAtMousePosition(mouseWorldPos);
			if (B2_IS_NON_NULL(bodyId))
			{
				m_selectedBodyId = bodyId;
				m_isDragging = true;

				b2Vec2 bodyPos = b2Body_GetPosition(bodyId);
				m_dragOffset.x = mouseWorldPos.x * METERS_PER_PIXEL - bodyPos.x;
				m_dragOffset.y = mouseWorldPos.y * METERS_PER_PIXEL - bodyPos.y;

				// 唤醒物体
				b2Body_SetAwake(bodyId, true);
			}
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE))
	{
		m_isDragging = false;
		m_selectedBodyId = b2_nullBodyId;
	}
}

b2BodyId GameBox2DTest::GetBodyAtMousePosition(const Vec2& worldPos)
{
	b2Vec2 point = { worldPos.x * METERS_PER_PIXEL, worldPos.y * METERS_PER_PIXEL };

	// 使用世界查询来找到点击位置的物体
	// 创建一个很小的圆形查询区域
	b2Circle queryCircle;
	queryCircle.center = point;
	queryCircle.radius = 0.01f;  // 很小的查询半径

	// 遍历所有动态盒子，手动检测点是否在其内部
	for (b2BodyId bodyId : m_dynamicBoxes)
	{
		if (B2_IS_NON_NULL(bodyId))
		{
			b2Vec2 bodyPos = b2Body_GetPosition(bodyId);
			b2Rot bodyRot = b2Body_GetRotation(bodyId);

			// 将点转换到物体的本地坐标系
			b2Vec2 localPoint = b2InvRotateVector(bodyRot, { point.x - bodyPos.x, point.y - bodyPos.y });

			// 检查点是否在1x1米的盒子内
			if (fabsf(localPoint.x) <= 1.0f && fabsf(localPoint.y) <= 1.0f)
			{
				return bodyId;
			}
		}
	}
	return b2_nullBodyId;
}

Vec2 GameBox2DTest::ScreenToWorld(const Vec2& screenPos) const
{
	return m_worldUV.GetPointAtUV(screenPos);
}

void GameBox2DTest::Renderer() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);

	m_worldVerts.clear();

	if (B2_IS_NON_NULL(m_groundBodyId))
	{
		b2Vec2 pos = b2Body_GetPosition(m_groundBodyId);
		b2Rot rot = b2Body_GetRotation(m_groundBodyId);
		float angle = b2Rot_GetAngle(rot);

		Vec2 center(pos.x / METERS_PER_PIXEL, pos.y / METERS_PER_PIXEL);
		Vec2 halfDims(SCREEN_SIZE_X * 0.5f, 2.0f / METERS_PER_PIXEL);

		OBB2 groundOBB(center, Vec2::MakeFromPolarRadians(angle), halfDims);
		AddVertsForOBB2D(m_worldVerts, groundOBB, Rgba8(100, 100, 100));
	}

	for (b2BodyId bodyId : m_dynamicBoxes)
	{
		if (B2_IS_NON_NULL(bodyId))
		{
			b2Vec2 pos = b2Body_GetPosition(bodyId);
			b2Rot rot = b2Body_GetRotation(bodyId);
			float angle = b2Rot_GetAngle(rot);

			Vec2 center(pos.x / METERS_PER_PIXEL, pos.y / METERS_PER_PIXEL);
			Vec2 halfDims(1.0f / METERS_PER_PIXEL, 1.0f / METERS_PER_PIXEL);

			bool isSelected = (bodyId.index1 == m_selectedBodyId.index1 &&
				bodyId.world0 == m_selectedBodyId.world0);
			Rgba8 color = isSelected ? Rgba8(255, 200, 0) : Rgba8(150, 200, 255);

			OBB2 boxOBB(center, Vec2::MakeFromPolarRadians(angle), halfDims);
			AddVertsForOBB2D(m_worldVerts, boxOBB, color);
		}
	}

	// 如果正在拖拽，绘制连接线
	//if (m_isDragging && B2_IS_NON_NULL(m_selectedBodyId))
	//{
	//	b2Vec2 bodyPos = b2Body_GetPosition(m_selectedBodyId);

	//	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	//	Vec2 mouseWorldPos = m_worldUV.GetPointAtUV(mouseUV);

	//	Vec2 start(bodyPos.x / METERS_PER_PIXEL, bodyPos.y / METERS_PER_PIXEL);
	//	Vec2 end = mouseWorldPos;

	//	AddVertsForLineSegment2D(m_worldVerts, start, end, 2.f, Rgba8(255, 255, 0));
	//}

	g_theRenderer->DrawVertexArray(m_worldVerts);

	std::vector<Vertex_PCU> textVerts;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	font->AddVertsForTextInBox2D(textVerts,
		"Mode (F6/F7 for prev/next): Box2D Physics Test",
		AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)),
		15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));

	std::string instructions = "LMB: Drag boxes | Space: Add box | R: Reset | T: Slow motion";
	font->AddVertsForTextInBox2D(textVerts, instructions,
		AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)),
		15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));

	char buffer[256];
	sprintf_s(buffer, "Box Count: %d\nPhysics Time Step: %.2f ms\nDT: %.2f ms\nFPS: %.1f",
		(int)m_dynamicBoxes.size(),
		m_fixedTimeStep * 1000.f,
		m_curDeltaTime * 1000.f,
		1.f / m_curDeltaTime);

	font->AddVertsForTextInBox2D(textVerts, buffer,
		AABB2(Vec2(10.f, 500.f), Vec2(400.f, 700.f)),
		15.f, Rgba8::WHITE, 0.7f, Vec2(0.f, 1.f));

	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);

	g_theRenderer->EndCamera(m_screenCamera);
}

void GameBox2DTest::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);

	m_worldUV = AABB2(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y,
		m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y);

	IntVec2 dimensions = g_theWindow->GetClientDimensions();
	m_screenCamera.SetViewport(AABB2(Vec2::ZERO, Vec2((float)dimensions.x, (float)dimensions.y)));
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
}