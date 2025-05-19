#include "Game3DTestShapes.hpp"
#include "Sphere.hpp"
#include "Box3D.hpp"
#include "Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/GameCommon.hpp"
#include <Engine/Core/DebugRenderSystem.hpp>
#include "ZCylinder.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Window/Window.hpp"
#include "Game/ShapePlane3D.hpp"
#include "Engine/Math/OBB3.hpp"
#include "ShapeOBB3D.hpp"

//#include "Engine/Math/Plane3.hpp"
Game3DTestShapes::Game3DTestShapes()
{
	m_gameClock = new Clock();
	m_player = new Player(this);
	Texture* tex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");

	Sphere* sphere = new Sphere(Vec3(0.f, 0.f, 0.f), 1.f,tex,this, false);
	m_shapes.push_back(sphere);
	Sphere* sphere2 = new Sphere(Vec3(4.f, 4.f, 4.f), 1.f, tex, this, false);
	m_shapes.push_back(sphere2);

	AABB3 box = AABB3(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f));
	Box3D* box3D = new Box3D(Vec3(2.f, 2.f, 2.f), box,tex, this, false);
	m_shapes.push_back(box3D);
	AABB3 box2 = AABB3(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f));
	Box3D* box3D2 = new Box3D(Vec3(2.f, 2.f, 2.f), box, tex, this, false);
	m_shapes.push_back(box3D2);

	ZCylinder* zCylinder =new ZCylinder(Vec3(-2.f, -2.f, -2.f), 1.f, 1.f, tex, this, false);
	m_shapes.push_back(zCylinder);
	ZCylinder* zCylinder2 = new ZCylinder(Vec3(-2.f, -2.f, -2.f), 1.f, 1.f, tex, this, false);
	m_shapes.push_back(zCylinder2);

	Sphere* sphereWire = new Sphere(Vec3(0.f, 0.f, 0.f), 1.f, nullptr, this, true);
	m_shapes.push_back(sphereWire);
	Sphere* sphereWire2 = new Sphere(Vec3(4.f, 4.f, 4.f), 1.f, nullptr, this, true);
	m_shapes.push_back(sphereWire2);

	ZCylinder* zCylinderWire = new ZCylinder(Vec3(-2.f, -2.f, -2.f), 1.f, 1.f, nullptr, this, true);
	m_shapes.push_back(zCylinderWire);
	ZCylinder* zCylinderWire2 = new ZCylinder(Vec3(-2.f, -2.f, -2.f), 1.f, 1.f, nullptr, this, true);
	m_shapes.push_back(zCylinderWire2);

	AABB3 boxWire = AABB3(Vec3(-3.f, -3.f, -3.f), Vec3(1.f, 1.f, 1.f));
	Box3D* box3DWire = new Box3D(Vec3(-2.f, 2.f, 2.f), boxWire, nullptr, this, true);
	m_shapes.push_back(box3DWire);
	AABB3 boxWire2 = AABB3(Vec3(-3.f, -3.f, -3.f), Vec3(1.f, 1.f, 1.f));
	Box3D* box3DWire2 = new Box3D(Vec3(-2.f, 2.f, 2.f), boxWire2, nullptr, this, true);
	m_shapes.push_back(box3DWire2);

 	Plane3 plane =Plane3(Vec3(2.f,1.f,1.f).GetNormalized(), 5.f);
 	ShapePlane3D* shapePlane = new ShapePlane3D(plane, Vec3(1.f, 0.f, 0.f), nullptr, this, false);
 	m_shapes.push_back(shapePlane);

	OBB3 obbBox = OBB3(Vec3(1.f,1.f, 1.f).GetNormalized(), Vec3(1.5, 1.f, 1.f), Vec3(0.f, 0.f, 0.f));
	ShapeOBB3D* shapeOBB = new ShapeOBB3D(Vec3(0.f, 0.f, 0.f), obbBox, tex, this, false);
	m_shapes.push_back(shapeOBB);

	OBB3 obbBoxWire = OBB3(Vec3(1.f, 1.f, 1.f).GetNormalized(), Vec3(1.5, 1.f, 1.f), Vec3(0.f, 0.f, 0.f));
	ShapeOBB3D* shapeOBBWire = new ShapeOBB3D(Vec3(0.f, 0.f, 0.f), obbBoxWire, tex, this, true);
	m_shapes.push_back(shapeOBBWire);

	Rerandom3DShapes();
}

Game3DTestShapes::~Game3DTestShapes()
{
	delete m_player;
	m_player = nullptr;

	for (Shape* shape : m_shapes)
	{
		delete shape;
		shape = nullptr;
	}

	g_systemClock->RemoveChild(m_gameClock);
	delete m_gameClock;
	m_gameClock = nullptr;
}

void Game3DTestShapes::Update()
{
	float deltaTime = (float)m_gameClock->GetDeltaSeconds();
	m_player->Update(deltaTime);
	//----------------------------------------------------------------------------------------
	UpdateIsStationary();
	//----------------------------------------------------------------------------------------
	float nearestDistSquare = FLT_MAX;
	int nearestIndex = 0;
	int curIndex = 0;
	float raycastDist= FLT_MAX;
	m_isHitSth = false;
	int raycastIndex = 0;
	RaycastResult3D curRaycastResult;
	
	for (Shape* shape : m_shapes)
	{
		shape->m_nearestPoint = shape->GetNearestPoint(m_curReferPoint);
		float curNearDistSquare = GetDistanceSquared3D(shape->m_nearestPoint, m_curReferPoint);
		if (curNearDistSquare < nearestDistSquare)
		{
			nearestIndex = curIndex;
			nearestDistSquare = curNearDistSquare;
		}
		//----------------------------------------------------------------------------------------
		RaycastResult3D raycastResult = shape->GetRaycastResult(m_curReferPoint, m_curOrientation.GetForward_IFwd(),10.f);
 		if (raycastResult.m_didImpact)
 		{
			m_isHitSth = true;
			if (raycastResult.m_impactDist < raycastDist)
			{
				shape->m_isRayHit = true;
				curRaycastResult = raycastResult;
				raycastIndex = curIndex;
				raycastDist = raycastResult.m_impactDist;
			}
			else
			{
				shape->m_isRayHit = false;
			}
 		}
		else
		{
			shape->m_isRayHit =false;
		}
		shape->Update(deltaTime);
		curIndex++;
	}
	//----------------------------------------------------------------------------------------
	if (m_isHitSth)
	{
		DebugAddWorldPoint(curRaycastResult.m_impactPos, 0.05f, 0.f, Rgba8::WHITE, Rgba8::WHITE);
		Vec3 normalEnd = curRaycastResult.m_impactPos + curRaycastResult.m_impactNormal;
		DebugAddWorldArrow(curRaycastResult.m_impactPos, normalEnd, 0.05f, 0.f, Rgba8::YELLOW, Rgba8::YELLOW);
		if (m_isStationary)
		{
			DebugAddWorldArrow(curRaycastResult.m_rayStartPos, curRaycastResult.m_impactPos, 0.05f, 0.f, Rgba8::RED, Rgba8::RED);
		}

		DebugAddWorldArrow(curRaycastResult.m_impactPos,
			curRaycastResult.m_rayStartPos + curRaycastResult.m_rayFwdNormal * curRaycastResult.m_rayMaxLength,
			0.05f, 0.f, Rgba8(150, 150, 150), Rgba8(150, 150, 150));
	}
	else
	{
		if (m_isStationary)
		{
			DebugAddWorldArrow(m_curReferPoint,
				m_curReferPoint + m_curOrientation.GetForward_IFwd() * 10.f,
				0.05f, 0.f, Rgba8::GREEN , Rgba8::GREEN);
		}
	}
	//----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		if (m_isHitSth)
		{
			m_isGrabSth = !m_isGrabSth;
		}

		if (!m_isHitSth && m_isGrabSth)
		{
			m_isGrabSth = !m_isGrabSth;
		}
		
		if (m_isGrabSth && m_isHitSth)
		{
			m_grabIndex = raycastIndex;
			Mat44 transToCam = m_player->m_playerCam.GetWorldToCameraTransform();
			m_grabObjRelativePos = transToCam.TransformPosition3D(m_shapes[m_grabIndex]->m_center);
			m_shapes[m_grabIndex]->m_isGrab = true;
		}
		else
		{
			if (m_grabIndex != -1)
			{
				m_shapes[m_grabIndex]->m_isGrab = false;
				m_grabIndex = -1;
			}
		}
	}
	if (m_isGrabSth)
	{
		Mat44 transToWorld = m_player->m_playerCam.GetCameraToWorldTransform();
		m_shapes[m_grabIndex]->m_center = transToWorld.TransformPosition3D(m_grabObjRelativePos);

		if (m_shapes[m_grabIndex]->m_shapeType == ShapeType::TYPE_OBB3D)
		{
			ShapeOBB3D* obb = static_cast<ShapeOBB3D*> (m_shapes[m_grabIndex]);
			if (g_theInput->WasKeyJustPressed('O'))
			{
				obb->m_euler.m_yawDegrees -= 10.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);

				Vec3 i, j, k;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(i, j, k);
				OBB3 newBox = OBB3(i, j, k, obb->m_box.m_halfDimensions, Vec3(0.f, 0.f, 0.f));
				//OBB3 newBox = OBB3(i,Vec3(xHalfLen, yHalfLen, zHalfLen), Vec3(0.f, 0.f, 0.f));
				obb->m_box = newBox;
			}

			if (g_theInput->WasKeyJustPressed('I'))
			{
				obb->m_euler.m_yawDegrees += 10.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);
			}

			if (g_theInput->WasKeyJustPressed('J'))
			{
				obb->m_euler.m_pitchDegrees -= 10.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);
			}

			if (g_theInput->WasKeyJustPressed('K'))
			{
				obb->m_euler.m_pitchDegrees += 10.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);
			}

			if (g_theInput->WasKeyJustPressed('N'))
			{
				obb->m_euler.m_rollDegrees -= 10.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);
			}

			if (g_theInput->WasKeyJustPressed('M'))
			{
				obb->m_euler.m_rollDegrees += 10.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);
			}

			if (g_theInput->WasKeyJustPressed('U'))
			{
				obb->m_euler.m_yawDegrees = 0.f;
				obb->m_euler.m_pitchDegrees = 0.f;
				obb->m_euler.m_rollDegrees = 0.f;
				obb->m_euler.GetAsVectors_IFwd_JLeft_KUp(obb->m_box.m_iBasis, obb->m_box.m_jBasis, obb->m_box.m_kBasis);
			}
		}
	}
	//----------------------------------------------------------------------------------------
	for (int i = 0; i < (int)m_shapes.size(); i++)
	{
		if (i != nearestIndex)
		{
			DebugAddWorldPoint(m_shapes[i]->m_nearestPoint, 0.05f, 0.f, Rgba8(233, 113, 50), Rgba8(233, 113, 50));
		}
		else
		{
			DebugAddWorldPoint(m_shapes[i]->m_nearestPoint, 0.05f, 0.f, Rgba8(71, 212, 90), Rgba8(71, 212, 90));
		}
	}
	//----------------------------------------------------------------------------------------
	UpdateOverlap();
	//----------------------------------------------------------------------------------------
	Vec3 basisPos = m_player->m_position + m_player->m_orientation.GetForward_IFwd() * 0.2f;
	Mat44 basisTransform = Mat44::MakeTranslation3D(basisPos);
	DebugAddWorldBasis(basisTransform, 0.f, DebugRenderMode::USE_DEPTH, 0.01f);

	DebugAddWorldBasis(Mat44(), 0.f, DebugRenderMode::USE_DEPTH, 1.f);
	//----------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		Rerandom3DShapes();
	}

	UpdateCamera(deltaTime);
}

void Game3DTestShapes::Renderer() const
{
	DebugRenderWorld(m_player->m_playerCam);

	g_theRenderer->BeginCamera(m_player->m_playerCam);
	for (Shape* shape : m_shapes)
	{
		shape->Render();
	}
	g_theRenderer->EndCamera(m_player->m_playerCam);

	//--------------Title--------------------------------------------------
	g_theRenderer->BeginCamera(m_screenCamera);
	std::vector<Vertex_PCU> title;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	std::string secLine;
	std::string thirdLine="";
	font->AddVertsForTextInBox2D(title, "Mode (F6/F7 for prev/next): 3D Shapes (3D)", AABB2(Vec2(10.f, 770.f), Vec2(1600.f, 790.f)), 15.f, Rgba8(200, 200, 0), 0.7f, Vec2(0.f, 0.f));
	if (m_isHitSth)
	{
		secLine = "F8 to randomize; WASD = fly; ZC = fly vertical, Space = lock raycast, hold T = slow, LMB = grab the thing";
	}
	else
	{
		secLine = "F8 to randomize; WASD = fly; ZC = fly vertical, Space = lock raycast, hold T = slow";
	}
	if (m_isGrabSth)
	{
		secLine = "F8 to randomize; WASD = fly; ZC = fly vertical, Space = lock raycast, hold T = slow, LMB = release the thing";
		if (m_shapes[m_grabIndex]->m_shapeType == ShapeType::TYPE_OBB3D)
		{
			ShapeOBB3D* obb = static_cast<ShapeOBB3D*>(m_shapes[m_grabIndex]);
			thirdLine = "Selected OBB has Yaw= " + std::to_string((int)obb->m_euler.m_yawDegrees) + " Pitch= " + std::to_string((int)obb->m_euler.m_pitchDegrees) + " Roll= " + std::to_string((int)obb->m_euler.m_rollDegrees) +
				"----Modify Yaw[O/I] Pitch[J/K] Roll[N/M] Reset[U]";
		}
	}
	font->AddVertsForTextInBox2D(title, secLine, AABB2(Vec2(10.f, 745.f), Vec2(1600.f, 765.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	font->AddVertsForTextInBox2D(title, thirdLine, AABB2(Vec2(10.f, 715.f), Vec2(1600.f, 735.f)), 15.f, Rgba8(0, 200, 200), 0.7f, Vec2(0.f, 0.f));
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexArray(title);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game3DTestShapes::Rerandom3DShapes()
{
	for (int i = 0; i < (int)m_shapes.size(); i++)
	{
		//if (m_shapes[i]->m_shapeType != ShapeType::TYPE_PLANE)
		//{
			float posX = m_rng.RollRandomFloatInRange(-10.f, 10.f);
			float posY = m_rng.RollRandomFloatInRange(-10.f, 10.f);
			float posZ = m_rng.RollRandomFloatInRange(-3.f, 3.f);
			m_shapes[i]->m_center = Vec3(posX, posY, posZ);
			m_shapes[i]->RandomSize();
		//}
	}
}

void Game3DTestShapes::UpdateCamera(float deltaTime)
{
	UNUSED(deltaTime);
	IntVec2 dimensions = g_theWindow->GetClientDimensions();
	m_player->m_playerCam.SetViewport(AABB2(Vec2::ZERO, Vec2((float)dimensions.x, (float)dimensions.y)));
	m_player->m_playerCam.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);

	m_screenCamera.SetViewport(AABB2(Vec2::ZERO, Vec2((float)dimensions.x, (float)dimensions.y)));
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
}

void Game3DTestShapes::UpdateIsStationary()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		ToggleReferPointMode();
	}

	if (!m_isStationary)
	{
		m_curReferPoint = m_player->m_position;
		m_curOrientation = m_player->m_orientation;
	}
}

void Game3DTestShapes::ToggleReferPointMode()
{
	m_isStationary = !m_isStationary;
}

void Game3DTestShapes::UpdateOverlap()
{
	for (Shape* shape : m_shapes)
	{
		shape->m_isOverlap = false;
	}

	for (Shape* shapeA : m_shapes)
	{
		for (Shape* shapeB : m_shapes)
		{
			bool isOverlapping = false;
			if (shapeA == shapeB)
				continue;
			if (shapeA->m_shapeType == ShapeType::TYPE_SHPERE)
			{
				Sphere* sphereA = static_cast<Sphere*>(shapeA);
				if (shapeB->m_shapeType == ShapeType::TYPE_SHPERE)
				{
					Sphere* sphereB = static_cast<Sphere*>(shapeB);
					isOverlapping = DoSpheresOverlap3D(sphereA->m_center, sphereA->m_radius,
						sphereB->m_center, sphereB->m_radius);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_BOX3D)
				{
					Box3D* boxB = static_cast<Box3D*>(shapeB);
					AABB3 curBoxB = AABB3(boxB->m_center + boxB->m_box.m_mins, boxB->m_center + boxB->m_box.m_maxs);
					isOverlapping = DoSphereAndAABBOverlap3D(sphereA->m_center, sphereA->m_radius, curBoxB);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_ZCYLINDER)
				{
					ZCylinder* cylinderB = static_cast<ZCylinder*>(shapeB);
					isOverlapping = DoZCylinderAndShpereOVerlap3D(cylinderB->m_center, cylinderB->m_radius, cylinderB->m_halfHeight,
						sphereA->m_center, sphereA->m_radius);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_PLANE)
				{
					ShapePlane3D* planeB = static_cast<ShapePlane3D*>(shapeB);
					isOverlapping = DoSphereAndPlane3Overlap3D(sphereA->m_center, sphereA->m_radius,planeB->m_plane);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_OBB3D)
				{
					ShapeOBB3D* obbB = static_cast<ShapeOBB3D*>(shapeB);
					isOverlapping = DoOBB3AndSphereOverlap3D(OBB3(obbB->m_box.m_iBasis, obbB->m_box.m_halfDimensions,obbB->m_center), 
						sphereA->m_center,sphereA->m_radius);
				}
			}
			else if (shapeA->m_shapeType == ShapeType::TYPE_BOX3D)
			{
				Box3D* boxA = static_cast<Box3D*>(shapeA);
				AABB3 curBoxA = AABB3(boxA->m_center + boxA->m_box.m_mins, boxA->m_center + boxA->m_box.m_maxs);
				if (shapeB->m_shapeType == ShapeType::TYPE_SHPERE)
				{
					Sphere* sphereB = static_cast<Sphere*>(shapeB);
					isOverlapping = DoSphereAndAABBOverlap3D(sphereB->m_center, sphereB->m_radius, curBoxA);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_BOX3D)
				{
					Box3D* boxB = static_cast<Box3D*>(shapeB);
					AABB3 curBoxB = AABB3(boxB->m_center + boxB->m_box.m_mins, boxB->m_center + boxB->m_box.m_maxs);
					isOverlapping = DoAABBsOverlap3D(curBoxB, curBoxA);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_ZCYLINDER)
				{
					ZCylinder* cylinderB = static_cast<ZCylinder*>(shapeB);
					isOverlapping = DoZCylinderAndAABBOverlap3D(cylinderB->m_center, cylinderB->m_radius,
						cylinderB->m_halfHeight, curBoxA);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_PLANE)
				{
					ShapePlane3D* planeB = static_cast<ShapePlane3D*>(shapeB);
					isOverlapping = DoAABB3AndPlane3Overlap3D(curBoxA,planeB->m_plane);
				}
			}
			else if (shapeA->m_shapeType == ShapeType::TYPE_ZCYLINDER)
			{
				ZCylinder* cylinderA = static_cast<ZCylinder*>(shapeA);
				if (shapeB->m_shapeType == ShapeType::TYPE_SHPERE)
				{
					Sphere* sphereB = static_cast<Sphere*>(shapeB);
					isOverlapping = DoZCylinderAndShpereOVerlap3D(cylinderA->m_center, cylinderA->m_radius,
						cylinderA->m_halfHeight,
						sphereB->m_center, sphereB->m_radius);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_BOX3D)
				{
					Box3D* boxB = static_cast<Box3D*>(shapeB);
					AABB3 curBoxB = AABB3(boxB->m_center + boxB->m_box.m_mins, boxB->m_center + boxB->m_box.m_maxs);
					isOverlapping = DoZCylinderAndAABBOverlap3D(cylinderA->m_center, cylinderA->m_radius,
						cylinderA->m_halfHeight, curBoxB);
				}
				else if (shapeB->m_shapeType == ShapeType::TYPE_ZCYLINDER)
				{
					ZCylinder* cylinderB = static_cast<ZCylinder*>(shapeB);
					isOverlapping = DoZCylindersOverlap3D(cylinderA->m_center, cylinderA->m_radius, cylinderA->m_halfHeight,
						cylinderB->m_center, cylinderB->m_radius, cylinderB->m_halfHeight);
				}
			}
			else if (shapeA->m_shapeType == ShapeType::TYPE_OBB3D)
			{
				ShapeOBB3D* obbA= static_cast<ShapeOBB3D*>(shapeA);
				if (shapeB->m_shapeType == ShapeType::TYPE_PLANE)
				{
					ShapePlane3D* planeB = static_cast<ShapePlane3D*>(shapeB);
					isOverlapping = DoOBB3AndPlane3Overlap3D(OBB3(obbA->m_box.m_iBasis, obbA->m_box.m_halfDimensions, obbA->m_center), 
						planeB->m_plane);
				}
			}
			if (isOverlapping)
			{
				shapeA->m_isOverlap = true;
				shapeB->m_isOverlap = true;
			}
		}
	}
}

