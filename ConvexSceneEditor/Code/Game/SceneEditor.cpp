#include "Game/SceneEditor.hpp"
#include "Game/ConvexScene.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Window/Window.hpp"
#include <Engine/Core/Vertex_PCU.hpp>
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/Renderer.hpp"
#include "ThirdParty/imgui/imgui.h"
#include <Engine/Core/Time.hpp>
#include "Engine/Math/BVHTree2D.hpp"
#include "Engine/Math/QuadTree2D.hpp"

extern Window* g_theWindow;
extern Renderer* g_theRenderer;

//-----------------------------------------------------------------------------------------------
SceneEditor::SceneEditor(ConvexScene* scene, Camera* camera)
	: m_scene(scene)
	, m_camera(camera)
{
	m_rng = new RandomNumberGenerator();
}

//-----------------------------------------------------------------------------------------------
SceneEditor::~SceneEditor()
{
	delete m_rng;
	m_rng = nullptr;
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::Update(float deltaSeconds)
{
	UpdateMouseWorldPosition();
	UpdateInput(deltaSeconds);

	UpdateVisibleRay();

	if (m_scene->GetOptimizationMode() == OptimizationMode::BVH)
	{
		m_scene->RebuildBVHTree(m_bvhMaxObjectsPerLeaf, m_bvhMaxTreeDepth);
	}
	if (m_scene->GetOptimizationMode() == OptimizationMode::QUAD_TREE)
	{
		m_scene->RebuildQuadTree(m_bvhMaxObjectsPerLeaf, m_bvhMaxTreeDepth);
	}

	RenderUI();
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::Render() const
{
	RenderVisibleRay();
	RenderDebugInfo();
	if (m_scene->GetOptimizationMode() == OptimizationMode::BVH)
	{
		if (m_scene->m_bvhTree && m_showSpatialStructure)
			m_scene->m_bvhTree->DebugRender(10);
	}
	else if (m_scene->GetOptimizationMode() == OptimizationMode::QUAD_TREE
		&& m_scene->m_quadTree)
	{
		m_scene->m_quadTree->DebugRenderWithObjectColoring(8);
	}
	DebugDrawCircle(5.f, m_mouseWorldPos, Rgba8(192, 192, 192));
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::RenderVisibleRay() const
{
	std::vector<Vertex_PCU> arrow_verts;
	AddVertsForArrow2D(arrow_verts, m_visibleRayStart, m_visibleRayEnd, 10.f, RAY_THICKNESS, Rgba8(224, 224, 224));

	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(arrow_verts);
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::RenderDebugInfo() const
{
	RenderSingleObjectDebug();

	if (m_visibleRayResult.m_didImpact)
	{
		std::vector<Vertex_PCU> arrow_verts;

		Vec2 impactPos = m_visibleRayResult.m_impactPos;
		Vec2 impactNormal = m_visibleRayResult.m_impactNormal;

		// 碰撞点（洋红色圆）
		DebugDrawCircle(4.f, impactPos, Rgba8::MAGNETA);
		Vec2 normalEnd = impactPos + impactNormal * IMPACT_NORMAL_LENGTH;
		AddVertsForArrow2D(arrow_verts, impactPos, normalEnd, 10.f, RAY_THICKNESS, Rgba8::MAGNETA);
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(arrow_verts);
	}
}

void SceneEditor::RenderUI()
{
	ImGui::Begin("Convex Scene Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// ===== 场景信息 =====
	ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Scene Info");
	ImGui::Separator();

	ImGui::Text("Invisible Rays: %d", m_numInvisibleRays);
	ImGui::SameLine();
	if (ImGui::Button("M (Double)"))
	{
		DoubleRaycasts();
	}
	ImGui::SameLine();
	if (ImGui::Button("N (Halve)"))
	{
		HalveRaycasts();
	}

	ImGui::Text("Object Count: %d", m_scene->GetObjectCount());
	ImGui::SameLine();
	if (ImGui::Button("Y (Double)"))
	{
		DoubleCovexObjects();
	}
	ImGui::SameLine();
	if (ImGui::Button("U (Halve)"))
	{
		HalveConvexObjects();
	}
	ImGui::SameLine();
	if (ImGui::Button("F8 (Randomize)"))
	{
		RegenerateConvexObjects();
	}

	ImGui::Spacing();

	// ===== 渲染模式 =====
	ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "Render Settings");
	ImGui::Separator();

	// 绘制模式选择
	const char* drawModeItems[] = { "Composite (edges then fills)", "Individual (per object)" };
	int currentDrawMode = m_drawMode;
	if (ImGui::Combo("Draw Mode (F2)", &currentDrawMode, drawModeItems, 2))
	{
		m_drawMode = currentDrawMode;
	}

	// 调试显示开关
	bool showDis = m_showBoundingDiscs;
	if (ImGui::Checkbox("Show Objects Bounding Disc (F1)", &showDis))
	{
		m_showBoundingDiscs = showDis;
	}

	bool showSpatial = m_showSpatialStructure;
	if (ImGui::Checkbox("Show Current Spatial Structure (F4)", &showSpatial))
	{
		m_showSpatialStructure = showSpatial;
	}

	ImGui::Spacing();

	// ===== 优化设置 =====
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Optimization");
	ImGui::Separator();

	bool useDiscCheck = m_useBoundingDiscCheck;
	if (ImGui::Checkbox("Use Bounding Disc Check", &useDiscCheck))
	{
		m_useBoundingDiscCheck = useDiscCheck;
	}

	// 优化模式选择
	const char* optModeItems[] = {
		"None (Brute Force)",
		"BVH",
		"Quad Tree",
	};
	int currentOptMode = (int)m_scene->GetOptimizationMode();
	if (ImGui::Combo("Optimization Mode (F9)", &currentOptMode, optModeItems, 3))
	{
		m_scene->SetOptimizationMode((OptimizationMode)currentOptMode);
	}

	ImGui::Text("Current: %s", m_scene->GetOptimizationModeName());

	// ===== BVH 配置 (仅在 BVH 模式下显示) =====
	if (m_scene->GetOptimizationMode() == OptimizationMode::BVH)
	{
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "BVH Configuration");
		ImGui::Separator();

		bool bvhConfigChanged = false;

		if (ImGui::SliderInt("Max Objects Per Leaf", &m_bvhMaxObjectsPerLeaf, 1, 32))
		{
			bvhConfigChanged = true;
		}

		if (ImGui::SliderInt("Max Tree Depth", &m_bvhMaxTreeDepth, 1, 32))
		{
			bvhConfigChanged = true;
		}

		if (bvhConfigChanged)
		{
			// 配置改变时自动重建BVH树
			m_scene->RebuildBVHTree(m_bvhMaxObjectsPerLeaf, m_bvhMaxTreeDepth);
		}

		// 显示当前BVH统计信息
		if (m_scene->m_bvhTree)
		{
			ImGui::Text("Tree Depth: %d", m_scene->m_bvhTree->GetDepth());
			ImGui::Text("Node Count: %d", m_scene->m_bvhTree->GetNodeCount());
		}
	}
	if (m_scene->GetOptimizationMode() == OptimizationMode::QUAD_TREE)
	{
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "QuadTree Configuration");
		ImGui::Separator();

		bool quadTreeConfigChanged = false;

		if (ImGui::SliderInt("Max Objects Per Leaf##qt", &m_quadTreeMaxObjectsPerLeaf, 1, 32))
		{
			quadTreeConfigChanged = true;
		}

		if (ImGui::SliderInt("Max Tree Depth##qt", &m_quadTreeMaxTreeDepth, 1, 32))
		{
			quadTreeConfigChanged = true;
		}

		if (quadTreeConfigChanged)
		{
			m_scene->RebuildQuadTree(m_quadTreeMaxObjectsPerLeaf, m_quadTreeMaxTreeDepth);
		}

		if (m_scene->m_quadTree)
		{
			ImGui::Text("Tree Depth: %d", m_scene->m_quadTree->GetDepth());
			ImGui::Text("Node Count: %d", m_scene->m_quadTree->GetNodeCount());
		}
	}

	ImGui::Spacing();

	// ===== 性能测试 =====
	ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Performance Test");
	ImGui::Separator();

	if (ImGui::Button("T - Run Raycast Test"))
	{
		PerformBatchRaycastTest();
	}

	ImGui::Text("Last Test Time: %.2f ms", m_lastRaycastTimeMs);

	ImGui::Spacing();

	// ===== 控制说明 =====
	ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Controls");
	ImGui::Separator();
	ImGui::BulletText("S/E or LMB/RMB: Drag ray start/end");
	ImGui::BulletText("W/R: Rotate object around mouse");
	ImGui::BulletText("K/L: Scale object around mouse");
	ImGui::BulletText("LMB Drag: Move selected object");
	ImGui::BulletText("ESC: Back to attract mode");

	ImGui::End();
}

//-----------------------------------------------------------------------------------------------
int SceneEditor::GetNumObjects() const
{
	return m_scene->GetObjectCount();
}

//-----------------------------------------------------------------------------------------------
OptimizationMode SceneEditor::GetOptimizationMode() const
{
	return m_scene->GetOptimizationMode();
}

//-----------------------------------------------------------------------------------------------
bool SceneEditor::ShouldShowSingleObjectDebug() const
{
	return m_scene->GetObjectCount() == 1;
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::UpdateInput(float deltaSeconds)
{
	HandleRaycastDragging();
	
	HandleObjectSelection();
	HandleObjectDragging();
	HandleObjectRotation(deltaSeconds);
	HandleObjectScaling(deltaSeconds);

	HandleSceneControls();
	HandleDebugToggles();
	HandleOptimizationCycling();
	HandleBatchRaycastTest();
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::UpdateMouseWorldPosition()
{
	m_lastMouseWorldPos = m_mouseWorldPos;

	m_worldUV = AABB2(m_camera->GetOrthoBottomLeft().x, m_camera->GetOrthoBottomLeft().y,
		m_camera->GetOrthoTopRight().x, m_camera->GetOrthoTopRight().y);
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	m_mouseWorldPos = m_worldUV.GetPointAtUV(mouseUV);
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleRaycastDragging()
{
	bool holdingS = g_theInput->IsKeyDown('S');
	bool holdingE = g_theInput->IsKeyDown('E');
	bool holdingLMB = g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
	bool holdingRMB = g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE);

	if (holdingS) // || holdingLMB
	{
		m_visibleRayStart = m_mouseWorldPos;
		m_mode = EditorMode::RAYCAST_START_DRAG;
	}

	else if (holdingE) // || holdingRMB
	{
		m_visibleRayEnd = m_mouseWorldPos;
		m_mode = EditorMode::RAYCAST_END_DRAG;
	}
	else
	{
		// Not dragging anymore
		if (m_mode == EditorMode::RAYCAST_START_DRAG || m_mode == EditorMode::RAYCAST_END_DRAG)
		{
			m_mode = EditorMode::NORMAL;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleObjectSelection()
{
	ConvexObject2* objectAtMouse = m_scene->GetObjectAtPoint(m_mouseWorldPos);

	// 清除之前的高亮状态
	if (m_currentObject && m_currentObject != m_lockedObject)
	{
		m_currentObject->SetRenderState(ConvexObject2::RenderState::NORMAL);
	}

	// 更新当前对象
	if (!m_lockedObject && !m_isDragging)
	{
		m_currentObject = objectAtMouse;
		m_scene->SetTopObject(m_currentObject);
		// 设置高亮状态
		if (m_currentObject)
		{
			m_currentObject->SetRenderState(ConvexObject2::RenderState::HIGHLIGHTED);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleObjectRotation(float deltaSeconds)
{
	if (!m_currentObject)
		return;

	bool rotatingCW = g_theInput->IsKeyDown('W');   // Clockwise
	bool rotatingCCW = g_theInput->IsKeyDown('R');  // Counter-clockwise

	if (rotatingCW || rotatingCCW)
	{
		// Lock the object while rotating
		if (!m_lockedObject)
		{
			m_lockedObject = m_currentObject;
		}

		float rotationAmount = m_rotationSpeed * deltaSeconds;
		if (rotatingCW)
			rotationAmount = -rotationAmount;  // Negative for clockwise

		// Rotate around mouse position
		m_lockedObject->RotateAroundPoint(m_mouseWorldPos, rotationAmount);
	}
	else
	{
		// Release lock when keys released
		if (m_lockedObject && !m_isDragging)
		{
			m_lockedObject = nullptr;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleObjectScaling(float deltaSeconds)
{
	if (!m_currentObject)
		return;

	bool scalingUp = g_theInput->IsKeyDown('K');    // Scale up
	bool scalingDown = g_theInput->IsKeyDown('L');  // Scale down

	if (scalingUp || scalingDown)
	{
		// Lock the object while scaling
		if (!m_lockedObject)
		{
			m_lockedObject = m_currentObject;
		}

		float scaleMultiplier = 1.0f + (m_scaleSpeed * deltaSeconds);
		if (scalingDown)
			scaleMultiplier = 1.0f / scaleMultiplier;  // Invert for shrinking

		// Scale around mouse position
		m_lockedObject->ScaleAroundPoint(m_mouseWorldPos, scaleMultiplier);
	}
	else
	{
		// Release lock when keys released
		if (m_lockedObject && !m_isDragging)
		{
			m_lockedObject = nullptr;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleObjectDragging()
{
	bool isLMBDown = g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
	bool wasLMBJustPressed = g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE);

	// Start dragging
	if (wasLMBJustPressed && m_currentObject)
	{
		// Only start drag if we're not in raycast drag mode
		if (m_mode == EditorMode::NORMAL)
		{
			m_isDragging = true;
			m_lockedObject = m_currentObject;
			m_lockedObject->SetRenderState(ConvexObject2::RenderState::DRAGGING);
			m_mode = EditorMode::OBJECT_DRAG;
		}
	}

	// Continue dragging - translate by mouse delta
	if (m_isDragging && isLMBDown && m_lockedObject)
	{
		Vec2 mouseDelta = m_mouseWorldPos - m_lastMouseWorldPos;
		m_lockedObject->Translate(mouseDelta);
	}

	// End dragging
	if (!isLMBDown && m_isDragging&&m_lockedObject)
	{
		m_lockedObject->SetRenderState(ConvexObject2::RenderState::HIGHLIGHTED);
		m_isDragging = false;
		m_lockedObject = nullptr;
		m_mode = EditorMode::NORMAL;
	}

	if (!m_lockedObject)
	{
		m_isDragging = false;
		m_mode = EditorMode::NORMAL;
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleSceneControls()
{
	if (g_theInput->WasKeyJustPressed('Y'))
	{
		DoubleCovexObjects();
	}
	if (g_theInput->WasKeyJustPressed('U'))
	{
		HalveConvexObjects();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RegenerateConvexObjects();
	}
	
	if (g_theInput->WasKeyJustPressed('M'))
	{
		DoubleRaycasts();
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		HalveRaycasts();
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleDebugToggles()
{
	// F1 - Toggle bounding discs
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_showBoundingDiscs = !m_showBoundingDiscs;
	}

	// F2 - Toggle draw mode (composite vs individual)
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_drawMode = (m_drawMode + 1) % 2;  // Toggle between 0 and 1
	}

	// F4 - Toggle spatial structure visualization
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_showSpatialStructure = !m_showSpatialStructure;
	}
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleOptimizationCycling()
{
	// F9 - Cycle through optimization modes
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		OptimizationMode currentMode = m_scene->GetOptimizationMode();
		int modeInt = (int)currentMode;
		modeInt = (modeInt + 1) % (int)OptimizationMode::NUM_MODES;
		m_scene->SetOptimizationMode((OptimizationMode)modeInt);
	}
}

void SceneEditor::DoubleCovexObjects()
{
	if (m_currentObject) m_currentObject->SetRenderState(ConvexObject2::RenderState::NORMAL);

	int currentCount = m_scene->GetObjectCount();
	int numToAdd = currentCount;  // 加倍 = 添加与当前数量相同的新对象

	for (int i = 0; i < numToAdd; ++i)
	{
		ConvexObject2* obj = new ConvexObject2();

		// Random number of vertices
		int numVertices = m_rng->RollRandomIntInRange(3, 8);

		// Random center position
		Vec2 center(
			m_rng->RollRandomFloatInRange(100.f, 1500.f),
			m_rng->RollRandomFloatInRange(100.f, 700.f)
		);

		// Generate the object
		obj->GenerateRandom(numVertices, center, 20.f, 80.f, *m_rng);

		// Set random color
		m_scene->AddObject(obj);
	}

	m_currentObject = nullptr;
	m_lockedObject = nullptr;
	m_scene->SetTopObject(nullptr);
	m_isDragging = false;
	m_dragOffset = Vec2::ZERO;

	if (m_scene->m_bvhTree)
		m_scene->RebuildBVHTree(m_bvhMaxObjectsPerLeaf, m_bvhMaxTreeDepth);
	if (m_scene->m_quadTree)
		m_scene->RebuildQuadTree(m_quadTreeMaxObjectsPerLeaf, m_quadTreeMaxTreeDepth);
}

void SceneEditor::HalveConvexObjects()
{
	if (m_currentObject) m_currentObject->SetRenderState(ConvexObject2::RenderState::NORMAL);

	int currentCount = m_scene->GetObjectCount();
	int targetCount = currentCount / 2;
	if (targetCount < 1)
		targetCount = 1;

	int numToRemove = currentCount - targetCount;

	// Remove from the end
	std::vector<ConvexObject2*> const& objects = m_scene->GetAllObjects();
	for (int i = 0; i < numToRemove; ++i)
	{
		int lastIndex = m_scene->GetObjectCount() - 1;
		if (lastIndex >= 0)
		{
			ConvexObject2* objToRemove = objects[lastIndex];
			m_scene->RemoveObject(objToRemove);
			delete objToRemove;
		}
	}

	m_currentObject = nullptr;
	m_lockedObject = nullptr;
	m_scene->SetTopObject(nullptr);
	m_isDragging = false;
	m_dragOffset = Vec2::ZERO;

	if (m_scene->m_bvhTree)
		m_scene->RebuildBVHTree(m_bvhMaxObjectsPerLeaf, m_bvhMaxTreeDepth);
	if (m_scene->m_quadTree)
		m_scene->RebuildQuadTree(m_quadTreeMaxObjectsPerLeaf, m_quadTreeMaxTreeDepth);
}

void SceneEditor::RegenerateConvexObjects()
{
	m_currentObject = nullptr;
	m_lockedObject = nullptr;
	int currentCount = m_scene->GetObjectCount();
	m_scene->RegenerateScene(currentCount, *m_rng);
}

void SceneEditor::DoubleRaycasts()
{
	m_numInvisibleRays *= 2;
}

void SceneEditor::HalveRaycasts()
{
	m_numInvisibleRays /= 2;
	if (m_numInvisibleRays < 1)
		m_numInvisibleRays = 1;
}

//-----------------------------------------------------------------------------------------------
void SceneEditor::HandleBatchRaycastTest()
{
	// T - Perform batch raycast test
	if (g_theInput->WasKeyJustPressed('T'))
	{
		OptimizationMode mode = m_scene->GetOptimizationMode();
		//m_lastRaycastTimeMs = m_scene->PerformBatchRaycast(m_numInvisibleRays, mode, *m_rng);
		PerformBatchRaycastTest();
	}
}

void SceneEditor::UpdateVisibleRay()
{
	m_visibleRayResult.m_didImpact = false;

	std::vector<ConvexObject2*> const& objects = m_scene->GetAllObjects();
	float closestDist = 1000000.f;

	for (int i = 0; i < (int)objects.size(); ++i)
	{
		Vec2 rayDisp = m_visibleRayEnd - m_visibleRayStart;
		float rayLength = rayDisp.GetLength();
		rayDisp.Normalize();

		RaycastResult2D result = objects[i]->Raycast(m_visibleRayStart, rayDisp, rayLength);

		if (result.m_didImpact && result.m_impactDist < closestDist)
		{
			closestDist = result.m_impactDist;
			m_visibleRayResult = result;
		}
	}
}

void SceneEditor::PerformBatchRaycastTest()
{
	// 获取场景边界
	Vec2 sceneMin(100.f, 100.f);
	Vec2 sceneMax(1500.f, 700.f);

	// ===== 预先生成所有随机射线（不计时） =====
	std::vector<Ray> rays;
	rays.reserve(m_numInvisibleRays);

	for (int i = 0; i < m_numInvisibleRays; ++i)
	{
		Ray ray;
		ray.m_rayStartPos = Vec2(
			m_rng->RollRandomFloatInRange(sceneMin.x, sceneMax.x),
			m_rng->RollRandomFloatInRange(sceneMin.y, sceneMax.y)
		);

		Vec2 end = Vec2(
			m_rng->RollRandomFloatInRange(sceneMin.x, sceneMax.x),
			m_rng->RollRandomFloatInRange(sceneMin.y, sceneMax.y)
		);

		Vec2 disp = end - ray.m_rayStartPos;
		ray.m_rayMaxLength = disp.GetLength();
		ray.m_rayFwdNormal = disp / ray.m_rayMaxLength;  // Normalize

		rays.push_back(ray);
	}
	
	RaycastModeConfig config;
	config.m_mode = m_scene->GetOptimizationMode();
	config.m_useDiscCheck = m_useBoundingDiscCheck;

	// ===== 开始计时 =====
	double startTime = GetCurrentTimeSeconds();

	SceneRaycastResult res = m_scene->RaycastScene(rays, config);

	// ===== 结束计时 =====
	double endTime = GetCurrentTimeSeconds();

	m_lastRaycastTimeMs = (float)((endTime - startTime) * 1000.0);
}

//void SceneEditor::RaycastScecne(std::vector<Ray> const& rays)
//{
//	// 执行所有射线测试
//
//	std::vector<ISpatialObject2D*> spatialCandidates;
//	m_scene->m_bvhTree->QueryRay(rayStart, rayDir, maxDist, spatialCandidates);
//
//	std::vector<ConvexObject2*> const& objects = m_scene->GetAllObjects();
//	for (int i = 0; i < m_numInvisibleRays; ++i)
//	{
//		for (int j = 0; j < (int)objects.size(); ++j)
//		{
//			// Per-Object Optimization Check
//			if (m_useBoundingDiscCheck)
//			{
//				RaycastResult2D discResult = RaycastVsDisc2D(rays[i].m_rayStartPos, rays[i].m_rayFwdNormal, rays[i].m_rayMaxLength,
//					objects[i]->GetGenerateCenter(), objects[i]->GetGenerateRadius());
//				if (!discResult.m_didImpact)
//					continue;
//			}
//
//			objects[j]->Raycast(rays[i].m_rayStartPos, rays[i].m_rayFwdNormal, rays[i].m_rayMaxLength);
//		}
//	}
//}

//-----------------------------------------------------------------------------------------------
bool SceneEditor::IsOnlyOneObjectInScene() const
{
	return m_scene->GetObjectCount() == 1;
}

void SceneEditor::RenderSingleObjectDebug() const
{
	if (m_scene->GetObjectCount() != 1)
		return;

	ConvexObject2* obj = m_scene->GetAllObjects()[0];
	std::vector<Plane2> const& planes = obj->GetHull().GetPlanes();
	std::vector<Vec2> const& vertices = obj->GetPoly().GetVertices();
	int vertexCount = (int)vertices.size();

	Vec2 rayDisp = m_visibleRayEnd - m_visibleRayStart;
	float rayLength = rayDisp.GetLength();

	float realEnterT = 0.f;
	float realExitT = 1.f;
	int enterPlaneIndex = -1;
	int exitPlaneIndex = -1;

	// ===== 2. 逐 plane 求交，记录交点信息 =====
	struct PlaneIntersection
	{
		float t;
		Vec2 point;
		int planeIndex;
		bool isEnter;  // true = front->behind（入口），false = behind->front（出口）
		bool isParallel;
		bool isOutside;  // 平行且在外侧
	};

	std::vector<PlaneIntersection> intersections;

	for (int i = 0; i < (int)planes.size(); ++i)
	{
		Plane2 const& plane = planes[i];
		float distStart = plane.GetSignedDistance(m_visibleRayStart);
		float distEnd = plane.GetSignedDistance(m_visibleRayEnd);
		float denom = distEnd - distStart;

		PlaneIntersection info;
		info.planeIndex = i;
		info.isParallel = false;
		info.isOutside = false;
		info.isEnter = false;
		info.t = 0.f;
		info.point = Vec2::ZERO;

		// 平面颜色默认灰色
		Rgba8 planeColor = Rgba8(100, 100, 100, 255);

		if (fabsf(denom) < 0.0001f)
		{
			info.isParallel = true;
			info.isOutside = (distStart > 0.f);
			intersections.push_back(info);
		}
		else
		{
			info.t = -distStart / denom;
			info.point = m_visibleRayStart + rayDisp * info.t;

			float dot = DotProduct2D(rayDisp, plane.m_normal);

			if (dot < 0.f)
			{
				// 异向 - 入口 - 红色
				info.isEnter = true;
				planeColor = Rgba8(204, 0, 102, 255);
				if (info.t > realEnterT&&info.t>0&&info.t<1.f)
				{
					realEnterT = info.t;
					enterPlaneIndex = i;
				}
			}
			else
			{
				// 同向 - 出口 - 绿色
				info.isEnter = false;
				planeColor = Rgba8(0, 153, 153, 255);
				if (info.t < realExitT && info.t>0 && info.t < 1.f)
				{
					realExitT = info.t;
					exitPlaneIndex = i;
				}
			}

			intersections.push_back(info);
		}

		// 画平面线和法线箭头
		Vec2 pointOnPlane = plane.m_normal * plane.m_distance;
		Vec2 planeDir = Vec2(-plane.m_normal.y, plane.m_normal.x);
		DebugDrawLine(pointOnPlane - planeDir * 3000.f, pointOnPlane + planeDir * 3000.f, 2.5f, planeColor);
	}

	// ===== 画最远入口和最近出口（大圆） =====
	// realEnterT 默认 0，realExitT 默认 1
	// 如果没有实际入口/出口交点落在 [0,1] 内，就画在起点/终点
	Vec2 realEnterPos = m_visibleRayStart + rayDisp * realEnterT;
	Vec2 realExitPos = m_visibleRayStart + rayDisp * realExitT;

	Vec2 midPoint = (realEnterPos + realExitPos) * 0.5f;

	bool isIntersecting = (realExitT >= realEnterT) && obj->GetHull().ContainsPoint(midPoint);

	// 画中点
	if (isIntersecting)
	{
		DebugDrawCircle(4.f, midPoint, Rgba8::YELLOW);
		DebugDrawLine(realEnterPos, realExitPos, RAY_THICKNESS, Rgba8::YELLOW);
		DebugDrawLine(m_visibleRayStart, realEnterPos, RAY_THICKNESS, Rgba8(0,153,76));
	}
	else
	{
		// 不相交 - 中点黄色
		DebugDrawCircle(4.f, midPoint, Rgba8::HILDA);
		DebugDrawLine(realEnterPos, realExitPos, RAY_THICKNESS, Rgba8::HILDA);
	}

	// ===== 画交点 =====
	for (int i = 0; i < (int)intersections.size(); ++i)
	{
		PlaneIntersection const& info = intersections[i];

		if (info.isParallel)
			continue;

		// 只画 t 在 [0, 1] 范围内的交点
		if (info.t < 0.f || info.t > 1.f)
			continue;

		if (info.isEnter)
		{
			DebugDrawCircle(4.f, info.point, Rgba8(204, 0, 102, 255));  // 入口小红圆
		}
		else
		{
			DebugDrawCircle(4.f, info.point, Rgba8(0, 153, 153, 255));  // 出口小绿圆
		}
	}

	DebugDrawCircle(7.f, realEnterPos, Rgba8(204, 0, 102, 255));  // 大红圆 - 最远入口
	DebugDrawCircle(7.f, realExitPos, Rgba8(0, 153, 153, 255));   // 大绿圆 - 最近出口
}
