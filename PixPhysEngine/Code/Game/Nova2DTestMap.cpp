#include "Nova2DTestMap.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Game.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "WangTileMap.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Game/SandboxPlayer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/ResourceManager/ResourceManager.hpp"
#include "Nova2D/Nova2DEmitterDefinition.hpp"
#include "Nova2D/Nova2DEmitterInstance.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

extern Nova2DSystem* g_nova2D;
extern Window* g_theWindow;
extern ResourceManager* g_theResourceManager;

//==========================================================================
// 构造与析构
//==========================================================================
Nova2DTestMap::Nova2DTestMap(SandboxPlayer* player)
	:BaseMap(IntVec2(WANG_MAP_CHUNKS_X* WANG_CHUNK_SIZE, WANG_MAP_CHUNKS_Y* WANG_CHUNK_SIZE), WANG_CHUNK_SIZE)
{
	m_player = player;
	//Initialize();
}

Nova2DTestMap::~Nova2DTestMap() 
{
	// 清理发射器
	for (Nova2DEmitter* emitter : m_emitters) 
	{
		g_nova2D->UnregisterEmitter(emitter);
		delete emitter;
	}
	m_emitters.clear();

}

//==========================================================================
// 生命周期
//==========================================================================
void Nova2DTestMap::Initialize() 
{
	LoadTextureResources();
	CreatePresetEmitters();

	TestDefInstance();
}

void Nova2DTestMap::LoadTextureResources() 
{
	// 加载火花纹理
	Texture* sparkTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Spark.png");
	m_sparkSheet = g_theResourceManager->CreateOrGetSpriteSheet("SparkParticle", "Data/Images/Particles/Spark.png", IntVec2(5, 5));
	
	//// 加载圆形纹理
	//Texture* circleTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Circle.png");
	//if (circleTex) {
	//	m_circleSheet = new SpriteSheet(*circleTex, IntVec2(1, 1));
	//}

	//// 加载爆炸动画（假设是 4x4 的 SpriteSheet）
	//Texture* explosionTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Spark.png");
	//if (explosionTex) {
	//	SpriteSheet* m_explosionSheet = new SpriteSheet(*explosionTex, IntVec2(5, 5));
	//	m_explosionAnim = new SpriteAnimDefinition(
	//		*m_explosionSheet,
	//		0, 23,  // 16 帧动画
	//		30.0f,  // 30 FPS
	//		SpriteAnimPlaybackType::ONCE
	//	);
	//}
}

void Nova2DTestMap::CreatePresetEmitters() 
{
	// 预设 1：火焰发射器
	Nova2DEmitter* fireEmitter = new Nova2DEmitter(EmitterPresets::CreateFireConfig());
	fireEmitter->SetPosition(Vec2(400, 300));
	fireEmitter->SetSpriteDef(&m_sparkSheet->GetSpriteDef(0));
	m_emitters.push_back(fireEmitter);
	g_nova2D->RegisterEmitter(fireEmitter);

	// 预设 2：爆炸发射器
	Nova2DEmitter* explosionEmitter = new Nova2DEmitter(EmitterPresets::CreateExplosionConfig());
	explosionEmitter->SetPosition(Vec2(600, 300));
	SpriteAnimDefinition* explosionAnim = g_theResourceManager->CreateOrGetSpriteAnim("Explosion", 
		"SparkParticle", 
		0, 23, 30.f, SpriteAnimPlaybackType::ONCE);
	explosionEmitter->SetAnimation(explosionAnim);
	m_emitters.push_back(explosionEmitter);
	g_nova2D->RegisterEmitter(explosionEmitter);

	// 预设 3：烟雾发射器
	Nova2DEmitter* smokeEmitter = new Nova2DEmitter(EmitterPresets::CreateSmokeConfig());
	smokeEmitter->SetPosition(Vec2(800, 300));
	//smokeEmitter->SetSpriteDef(m_circleSheet);
	m_emitters.push_back(smokeEmitter);
	g_nova2D->RegisterEmitter(smokeEmitter);

	// 默认播放第一个
	if (!m_emitters.empty()) {
		m_emitters[0]->Play();
		m_editingConfig = m_emitters[0]->GetConfig();
	}
}

void Nova2DTestMap::Update(float deltaTime)
{
	g_nova2D->Update(deltaTime, m_player->m_camera,nullptr);

	//// 鼠标点击发射（测试用）
	//if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE)) 
	//{
	//	g_nova2D->GetDefinition(0)->SetEmissionRate(800);

	//	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	//	Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(), m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);

	//	Nova2DEmitter* currentEmitter = GetCurrentEmitter();
	//	if (currentEmitter) 
	//	{
	//		currentEmitter->SetPosition(mousePosInWorld);
	//		currentEmitter->Reset();
	//		currentEmitter->Play();
	//	}
	//}

	// 如果配置有修改，同步到 Emitter
	if (m_configDirty) 
	{
		ApplyConfigToEmitter();
		m_configDirty = false;
	}

	// 渲染 UI
	RenderUI();
}

void Nova2DTestMap::Render() const 
{
	g_theRenderer->BeginCamera(m_player->m_camera);
	//// 渲染粒子系统
	g_nova2D->Render(m_player->GetCamera());
	g_theRenderer->EndCamera(m_player->m_camera);
}

//==========================================================================
// 辅助方法
//==========================================================================
Nova2DEmitter* Nova2DTestMap::GetCurrentEmitter() const
{
	if (m_selectedEmitterIndex >= 0 && m_selectedEmitterIndex < (int)m_emitters.size()) 
	{
		return m_emitters[m_selectedEmitterIndex];
	}
	return nullptr;
}

void Nova2DTestMap::ApplyConfigToEmitter() 
{
	Nova2DEmitter* emitter = GetCurrentEmitter();
	if (emitter) 
	{
		emitter->SetConfig(m_editingConfig);
	}
}

void Nova2DTestMap::TestDefInstance()
{
	// ========== Test 1: Fire Effect (橙→红→透明) ==========
	auto* fireDef = new Nova2DEmitterDefinition();
	Nova2DEmitterDefinitionGPU& gpu = fireDef->GetGPUData();

	// Emission Settings
	gpu.m_emission.m_emissionRate = 80.0f;
	gpu.m_emission.m_lifetimeMin = 3.f;
	gpu.m_emission.m_lifetimeMax = 4.0f;
	gpu.m_emission.m_emissionType = 1;  // Circle
	gpu.m_emission.m_emissionRadius = 40.0f;
	gpu.m_emission.m_emissionMode = 0;  // CONTINUOUS

	//fireDef->SetBurstConfig(20, 1.0f, -1);
	fireDef->SetTexturePath("Data/Images/Particles/Hilda.jpg");

	//// Motion Settings
	//gpu.m_motion.m_velocityMin.x = -150.0f;
	//gpu.m_motion.m_velocityMin.y = -150.0f;
	//gpu.m_motion.m_velocityMax.x = 150.0f;
	//gpu.m_motion.m_velocityMax.y = 150.0f;
	////fireDef->SetLinearForce(Vec2(0.f, -200.f));
	//gpu.m_motion.m_drag = 0.5f;

	//// Point Force Test - 中心吸引
	////fireDef->SetPointForce(Vec2(400.f, 250.f), 300.f, 500.f, 1.0f, true);
	////fireDef->EnablePointForce(true);

	//// Vortex Force Test - 中心旋转
	//fireDef->SetVortexForce(Vec2(400.f, 250.f), 30.f, 200.f);
	//fireDef->EnableVortex(true);

	// Motion Settings - 环绕旋转
	gpu.m_motion.m_velocityMin.x = -10.f;
	gpu.m_motion.m_velocityMin.y = -10.f;
	gpu.m_motion.m_velocityMax.x = 10.f;
	gpu.m_motion.m_velocityMax.y = 10.f;
	gpu.m_motion.m_drag = 0.0f;

	// 用Point Force把粒子拉向中心（向心力），Vortex提供切向速度
	fireDef->SetPointForce(500.f, 100.f, 1.0f, true);
	fireDef->EnablePointForce(true);
	//fireDef->SetVortexForce(500.f, 400.f);
	//fireDef->EnableVortex(true);

	// ===== Color Over Lifetime: Orange -> Red -> Transparent =====
	gpu.m_appearance.m_numColorKeyframes = 3;

	// Keyframe 0: Birth - Bright Orange (t=0.0)
	Rgba8 orangeColor(255, 153, 51, 255);  // Bright orange
	orangeColor.GetAsFloats(gpu.m_appearance.m_colorKeyframes[0].m_colorPacked);
	gpu.m_appearance.m_colorKeyframes[0].m_time = 0.0f;

	// Keyframe 1: Middle - Deep Red (t=0.5)
	Rgba8 redColor(230, 25, 0, 204);  // Deep red, slightly transparent
	Rgba8::CYAN.GetAsFloats(gpu.m_appearance.m_colorKeyframes[1].m_colorPacked);
	gpu.m_appearance.m_colorKeyframes[1].m_time = 0.5f;

	// Keyframe 2: Death - Transparent Black (t=1.0)
	Rgba8 transparentColor(51, 0, 0, 0);  // Dark red, fully transparent
	transparentColor.GetAsFloats(gpu.m_appearance.m_colorKeyframes[2].m_colorPacked);
	gpu.m_appearance.m_colorKeyframes[2].m_time = 1.0f;
	gpu.m_appearance.m_emissionStrength = 0.7f;

	// ===== Size Over Lifetime: Small -> Large -> Small =====
	gpu.m_numCurves = 2;
	gpu.m_curves[0].m_type = 0;  // SIZE curve
	gpu.m_curves[0].m_numKeyframes = 3;

	// Size Keyframe 0: Birth - Small (3.0)
	gpu.m_curves[0].m_keyframes[0].m_value = 1.0f;
	gpu.m_curves[0].m_keyframes[0].m_time = 0.0f;

	// Size Keyframe 1: Middle - Large (10.0)
	gpu.m_curves[0].m_keyframes[1].m_value = 2.0f;
	gpu.m_curves[0].m_keyframes[1].m_time = 0.5f;

	// Size Keyframe 2: Death - Small (2.0)
	gpu.m_curves[0].m_keyframes[2].m_value = 1.0f;
	gpu.m_curves[0].m_keyframes[2].m_time = 1.0f;

	//// ✅ 添加：Curve 1: Curl Noise
	//gpu.m_curves[1].m_type = 5;  // CURL_NOISE = 5
	//gpu.m_curves[1].m_numKeyframes = 1;
	//gpu.m_curves[1].m_keyframes[0].m_value = 0.0f;  // Curl strength (试试不同值：100-1000)
	//gpu.m_curves[1].m_keyframes[0].m_time = 0.0f;

	//// Existing curves setup
	//gpu.m_curves[1].m_type = 5;  // CURL_NOISE = 5
	//gpu.m_curves[1].m_numKeyframes = 1;
	//gpu.m_curves[1].m_keyframes[0].m_value = 0.0f;
	//gpu.m_curves[1].m_keyframes[0].m_time = 0.0f;

	//// ===== Collision Setup =====
	//gpu.m_collision.m_enableCollision = 1;  // Enable collision
	//gpu.m_collision.m_numRules = 1;  // 3 rules

	//// Rule 0: Black (terrain) -> BOUNCE
	//gpu.m_collision.m_rules[0].m_targetColor[0] = 1.0f;  // R
	//gpu.m_collision.m_rules[0].m_targetColor[1] = 0.0f;  // G
	//gpu.m_collision.m_rules[0].m_targetColor[2] = 0.0f;  // B
	//gpu.m_collision.m_rules[0].m_targetColor[3] = 0.0f;  // A
	//gpu.m_collision.m_rules[0].m_response = 1;  // BOUNCE
	//gpu.m_collision.m_rules[0].m_bounceDamping = 1.f;  // Keep 70% velocity
	//gpu.m_collision.m_rules[0].m_slowFactor = 1.0f;  // Not used for bounce
	//gpu.m_collision.m_rules[0].m_maxBounces = 3;

	gpu.m_motion.m_orientToVelocity = 1;

	// Register Definition
	uint32_t fireDefID = g_nova2D->CreateDefinition(fireDef);

	// Create Instance 1 (Left)
	uint32_t inst1ID = g_nova2D->CreateInstance(fireDefID, 400.0f, 250.0f);
	Nova2DEmitterInstance* inst1 = g_nova2D->GetInstance(inst1ID);
	inst1->Play();

	// Create Instance 2 (Right)
	//uint32_t inst2ID = g_nova2D->CreateInstance(fireDefID, 500.0f, 150.0f);
	//Nova2DEmitterInstance* inst2 = g_nova2D->GetInstance(inst2ID);
	//inst2->Play();

	//SetupFireEffect(g_nova2D);

	//DebuggerPrintf("Fire Effect: Definition ID=%u, Instances: %u, %u\n",
	//	fireDefID, inst1ID, inst2ID);

	//CreateNebulaburstEffect();
}

Nova2DEmitterDefinition* Nova2DTestMap::CreateFireDefinition_5x5()
{
	Nova2DEmitterDefinition* fireDef = new Nova2DEmitterDefinition();

	fireDef->SetTexturePath("Data/Images/Particles/566.png");

	//======================================================================
	// 1. Emission Module（发射模块）
	//======================================================================
	fireDef->GetGPUData().m_emission.m_emissionRate = 50.0f;          // 每秒50个粒子
	fireDef->GetGPUData().m_emission.m_emissionMode = 0;              // CONTINUOUS模式
	fireDef->GetGPUData().m_emission.m_lifetimeMin = 1.5f;            // 最小生命1.5秒
	fireDef->GetGPUData().m_emission.m_lifetimeMax = 2.5f;            // 最大生命2.5秒

	// 发射形状：圆形
	fireDef->GetGPUData().m_emission.m_emissionType = 1;              // 1 = Circle
	fireDef->GetGPUData().m_emission.m_emissionRadius = 10.0f;        // 半径10单位

	//======================================================================
	// 2. Motion Module（运动模块）
	//======================================================================
	fireDef->GetGPUData().m_motion.m_velocityMin.x = -20.0f;
	fireDef->GetGPUData().m_motion.m_velocityMin.y = 50.0f;            // 向上
	fireDef->GetGPUData().m_motion.m_velocityMax.x = 20.0f;
	fireDef->GetGPUData().m_motion.m_velocityMax.y = 100.0f;

	fireDef->GetGPUData().m_motion.m_velocityMode = 0;                // 0 = Random
	//fireDef->GetGPUData().m_motion.m_gravityScale = 0.0f;             // 火焰不受重力

	//======================================================================
	// 3. Appearance Module（外观模块 - 关键部分）
	//======================================================================
	// ★ 5×5 Sprite Sheet配置
	//fireDef->GetGPUData().m_appearance.m_spriteSheetDimensionsX = 5;  // 5列
	//fireDef->GetGPUData().m_appearance.m_spriteSheetDimensionsY = 5;  // 5行
	//fireDef->GetGPUData().m_appearance.m_spriteStartIndex = 0;        // 从第0帧开始
	//fireDef->GetGPUData().m_appearance.m_spriteEndIndex = 24;         // 到第24帧（共25帧）

	fireDef->SetSpriteSheet(13, 9, 0, 12,5);

	// Color Over Lifetime（火焰颜色渐变）
	fireDef->GetGPUData().m_appearance.m_numColorKeyframes = 3;

	// 关键帧0：t=0.0，明亮黄色
	Rgba8(255, 230, 77, 255).
		GetAsFloats(fireDef->GetGPUData().m_appearance.m_colorKeyframes[0].m_colorPacked);  // 亮黄色
	fireDef->GetGPUData().m_appearance.m_colorKeyframes[0].m_time = 0.0f;

	// 关键帧1：t=0.5，橙红色
	Rgba8(255, 102, 26, 255).
		GetAsFloats(fireDef->GetGPUData().m_appearance.m_colorKeyframes[1].m_colorPacked);  // 橙红色
	fireDef->GetGPUData().m_appearance.m_colorKeyframes[1].m_time = 0.5f;

	// 关键帧2：t=1.0，暗红色透明
	Rgba8(153, 26, 0, 255).
		 GetAsFloats(fireDef->GetGPUData().m_appearance.m_colorKeyframes[2].m_colorPacked);  // 暗红色+完全透明
	fireDef->GetGPUData().m_appearance.m_colorKeyframes[2].m_time = 1.0f;

	//======================================================================
	// 4. Size Over Lifetime（尺寸变化）
	//======================================================================
	fireDef->GetGPUData().m_numCurves = 1;

	// Size Curve（从小到大再缩小）
	fireDef->GetGPUData().m_curves[0].m_type = 0;                     // SIZE = 0
	fireDef->GetGPUData().m_curves[0].m_numKeyframes = 3;

	// 关键帧0：t=0.0，size=5
	fireDef->GetGPUData().m_curves[0].m_keyframes[0].m_value = 5.0f;
	fireDef->GetGPUData().m_curves[0].m_keyframes[0].m_time = 0.0f;

	// 关键帧1：t=0.5，size=15（最大）
	fireDef->GetGPUData().m_curves[0].m_keyframes[1].m_value = 15.0f;
	fireDef->GetGPUData().m_curves[0].m_keyframes[1].m_time = 0.5f;

	// 关键帧2：t=1.0，size=8
	fireDef->GetGPUData().m_curves[0].m_keyframes[2].m_value = 8.0f;
	fireDef->GetGPUData().m_curves[0].m_keyframes[2].m_time = 1.0f;

	//======================================================================
	// 5. Emitter Properties
	//======================================================================
	fireDef->GetGPUData().m_properties.m_lifetime = -1.0f;            // 无限发射
	fireDef->GetGPUData().m_properties.m_worldSimulation = 1;         // 世界空间

	return fireDef;
}

void Nova2DTestMap::SetupFireEffect(Nova2DSystem* particleSystem)
{
	// 1. 创建Definition
	Nova2DEmitterDefinition* fireDef = CreateFireDefinition_5x5();
	uint32_t defID = particleSystem->CreateDefinition(fireDef);
	fireDef->AddFloatCurve(N2D_FloatCurveType::ROTATION_SPEED, 6.28318f);
	// 2. 创建Instance
	uint32_t instanceID = particleSystem->CreateInstance(
		defID,      // Definition ID
		400.0f,     // 位置X
		100.0f      // 位置Y
	);

	// 3. 播放
	Nova2DEmitterInstance* instance = particleSystem->GetInstance(instanceID);
	instance->Play();

	DebuggerPrintf("Fire effect created: Def=%u, Instance=%u\n", defID, instanceID);
}

void Nova2DTestMap::CreateNewEmitter() 
{
	//Nova2DEmitter* newEmitter = new Nova2DEmitter(m_editingConfig);
	//newEmitter->SetPosition(Vec2(400, 300));
	//newEmitter->SetSpriteDef();

	//m_emitters.push_back(newEmitter);
	//g_nova2D->RegisterEmitter(newEmitter);

	//m_selectedEmitterIndex = (int)m_emitters.size() - 1;
}

void Nova2DTestMap::DeleteCurrentEmitter()
{
	if (m_emitters.size() <= 1) {
		return;  // 至少保留一个
	}

	Nova2DEmitter* emitter = GetCurrentEmitter();
	if (emitter) 
	{
		g_nova2D->UnregisterEmitter(emitter);
		delete emitter;

		m_emitters.erase(m_emitters.begin() + m_selectedEmitterIndex);
		m_selectedEmitterIndex = GetClamped(m_selectedEmitterIndex, 0, (int)m_emitters.size() - 1);
	}
}

//==========================================================================
// ImGui UI 实现（下一部分）
//==========================================================================
void Nova2DTestMap::RenderUI() const 
{
	ImGui::Begin("Nova2D Emitter Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	RenderEmitterSelector();
	ImGui::Separator();

	RenderEmitterPropertiesUI();
	ImGui::Separator();

	RenderEmissionUI();
	ImGui::Separator();

	RenderMotionUI();
	ImGui::Separator();

	RenderAppearanceUI();
	ImGui::Separator();

	RenderControlPanel();

	ImGui::End();
}

void Nova2DTestMap::RenderEmitterSelector() const 
{
	ImGui::Text("Emitter Selection");

	// 发射器列表（单选框）
	for (int i = 0; i < (int)m_emitters.size(); ++i) 
	{
		char label[64];
		sprintf_s(label, "Emitter %d", i + 1);

		if (ImGui::RadioButton(label, m_selectedEmitterIndex == i)) 
		{
			m_selectedEmitterIndex = i;
			m_editingConfig = m_emitters[i]->GetConfig();
		}
	}

	// 新建/删除按钮
	//if (ImGui::Button("+ New Emitter")) 
	//{
	//	CreateNewEmitter();
	//}
	//ImGui::SameLine();
	//if (ImGui::Button("- Delete")) 
	//{
	//	DeleteCurrentEmitter();
	//}
}

void Nova2DTestMap::RenderEmitterPropertiesUI() const 
{
	ImGui::Text("Emitter Properties");

	bool changed = false;

	// Has Lifetime
	changed |= ImGui::Checkbox("Has Lifetime", &m_editingConfig.m_propertiesConfig.m_hasLifetime);
	if (m_editingConfig.m_propertiesConfig.m_hasLifetime)
	{
		ImGui::Indent();
		changed |= ImGui::SliderFloat("Lifetime", &m_editingConfig.m_propertiesConfig.m_emitterLifetime, 0.1f, 10.0f);
		ImGui::Unindent();
	}

	// Start Delay
	changed |= ImGui::SliderFloat("Start Delay", &m_editingConfig.m_propertiesConfig.m_startDelay, 0.0f, 5.0f);

	// Simulation Space
	const char* spaceItems[] = { "World", "Local" };
	int currentSpace = (int)m_editingConfig.m_propertiesConfig.m_simulationSpace;
	if (ImGui::Combo("Simulation Space", &currentSpace, spaceItems, 2)) 
	{
		m_editingConfig.m_propertiesConfig.m_simulationSpace = (n2d_SimulationSpace)currentSpace;
		changed = true;
	}

	if (changed) 
	{
		m_configDirty = true;
	}
}

void Nova2DTestMap::RenderEmissionUI() const 
{
	ImGui::Text("Emission");

	bool changed = false;

	// Emission Mode
	const char* modeItems[] = { "Continuous", "Burst" };
	int currentMode = (int)m_editingConfig.m_emissionConfig.m_mode;
	if (ImGui::Combo("Mode", &currentMode, modeItems, 2)) {
		m_editingConfig.m_emissionConfig.m_mode = (n2d_EmissionMode)currentMode;
		changed = true;
	}

	// 根据模式显示不同参数
	if (m_editingConfig.m_emissionConfig.m_mode == n2d_EmissionMode::CONTINUOUS) {
		changed |= ImGui::SliderFloat("Emission Rate", &m_editingConfig.m_emissionConfig.m_emissionRate, 1.0f, 200.0f);
	}
	else {
		changed |= ImGui::SliderInt("Burst Count", &m_editingConfig.m_emissionConfig.m_burstCount, 10, 500);
	}

	// Lifetime Range
	changed |= ImGui::SliderFloat("Lifetime Min", &m_editingConfig.m_emissionConfig.m_lifetimeMin, 0.1f, 5.0f);
	changed |= ImGui::SliderFloat("Lifetime Max", &m_editingConfig.m_emissionConfig.m_lifetimeMax, 0.1f, 5.0f);

	// Emission Shape
	const char* shapeItems[] = { "Point", "Circle", "Box" };
	int currentShape = (int)m_editingConfig.m_emissionConfig.m_shape;
	if (ImGui::Combo("Shape", &currentShape, shapeItems, 3)) {
		m_editingConfig.m_emissionConfig.m_shape = (n2d_EmitterShape)currentShape;
		changed = true;
	}

	// 根据形状显示参数
	ImGui::Indent();
	if (m_editingConfig.m_emissionConfig.m_shape == n2d_EmitterShape::CIRCLE) {
		changed |= ImGui::SliderFloat("Radius", &m_editingConfig.m_emissionConfig.m_shapeRadius, 0.0f, 100.0f);
	}
	else if (m_editingConfig.m_emissionConfig.m_shape == n2d_EmitterShape::BOX) {
		changed |= ImGui::SliderFloat2("Box Size", &m_editingConfig.m_emissionConfig.m_shapeBoxSize.x, 0.0f, 100.0f);
	}
	ImGui::Unindent();

	if (changed) {
		m_configDirty = true;
	}
}

void Nova2DTestMap::RenderMotionUI() const {
	ImGui::Text("Motion");

	bool changed = false;

	// Start Velocity
	changed |= ImGui::SliderFloat2("Velocity Min", &m_editingConfig.m_motionConfig.m_startVelocityMin.x, -500.0f, 500.0f);
	changed |= ImGui::SliderFloat2("Velocity Max", &m_editingConfig.m_motionConfig.m_startVelocityMax.x, -500.0f, 500.0f);

	// Max Speed
	changed |= ImGui::SliderFloat("Max Speed", &m_editingConfig.m_motionConfig.m_maxSpeed, -1.0f, 1000.0f);
	ImGui::SameLine();
	if (ImGui::Button("No Limit")) {
		m_editingConfig.m_motionConfig.m_maxSpeed = -1.0f;
		changed = true;
	}

	// Gravity
	changed |= ImGui::Checkbox("Enable Gravity", &m_editingConfig.m_enableGravity);
	if (m_editingConfig.m_enableGravity) {
		ImGui::Indent();
		changed |= ImGui::SliderFloat("Gravity Scale", &m_editingConfig.m_gravityScale, 0.0f, 2.0f);
		ImGui::Unindent();
	}

	if (changed) {
		m_configDirty = true;
	}
}

void Nova2DTestMap::RenderAppearanceUI() const {
	ImGui::Text("Appearance");

	bool changed = false;

	// Size
	changed |= ImGui::SliderFloat("Size Min", &m_editingConfig.m_appearanceConfig.m_sizeMin, 1.0f, 50.0f);
	changed |= ImGui::SliderFloat("Size Max", &m_editingConfig.m_appearanceConfig.m_sizeMax, 1.0f, 50.0f);

	// Color
	float color[4] = {
		m_editingConfig.m_appearanceConfig.m_colorStart.r / 255.0f,
		m_editingConfig.m_appearanceConfig.m_colorStart.g / 255.0f,
		m_editingConfig.m_appearanceConfig.m_colorStart.b / 255.0f,
		m_editingConfig.m_appearanceConfig.m_colorStart.a / 255.0f
	};
	if (ImGui::ColorEdit4("Color", color)) {
		m_editingConfig.m_appearanceConfig.m_colorStart = Rgba8(
			(unsigned char)(color[0] * 255),
			(unsigned char)(color[1] * 255),
			(unsigned char)(color[2] * 255),
			(unsigned char)(color[3] * 255)
		);
		changed = true;
	}

	if (changed) {
		m_configDirty = true;
	}
}

void Nova2DTestMap::RenderControlPanel() const {
	ImGui::Text("Control");

	Nova2DEmitter* emitter = GetCurrentEmitter();
	if (!emitter) return;

	// 播放控制按钮
	if (ImGui::Button("Play")) {
		emitter->Play();
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause")) {
		emitter->Pause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		emitter->Stop();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
		emitter->Reset();
		emitter->Play();
	}

	// 状态显示
	ImGui::Text("Status: %s", emitter->IsPlaying() ? (emitter->IsPaused() ? "Paused" : "Playing") : "Stopped");
	ImGui::Text("Alive Particles: %d", g_nova2D->GetAliveParticleCount());
}

//==========================================================================
// 🌌 "星云爆发" 效果 - 三发射器组合
// 
// 效果描述：
// 1. 核心：高速旋转的能量核心（Point Force 向心力 + Vortex 旋转）
// 2. 能量环：脉冲式扩散的彩色光环（Color 渐变 + Size 脉动）
// 3. 碎片：爆炸飞溅的粒子碎片（随机速度 + 重力下落 + Sprite 动画）
//
// 视觉层次：内核 → 能量环 → 外围碎片
//==========================================================================

void Nova2DTestMap::CreateNebulaburstEffect()
{
	Vec2 centerPos(350.0f, 150.0f);  // 效果中心点

	//======================================================================
	// 🔥 Emitter 1: 能量核心 (Energy Core)
	// 特点：高速旋转 + 青色→紫色渐变 + 向心力吸引
	//======================================================================
	auto* coreDef = new Nova2DEmitterDefinition();
	Nova2DEmitterDefinitionGPU& coreGPU = coreDef->GetGPUData();

	// === Emission ===
	coreGPU.m_emission.m_emissionRate = 3000.0f;       // 高密度粒子
	coreGPU.m_emission.m_lifetimeMin = 1.5f;
	coreGPU.m_emission.m_lifetimeMax = 4.0f;
	coreGPU.m_emission.m_emissionType = 1;           // Circle
	coreGPU.m_emission.m_emissionRadius = 15.0f;     // 小范围发射
	coreGPU.m_emission.m_emissionMode = 0;           // CONTINUOUS

	//coreDef->SetTexturePath("Data/Images/Particles/Hilda.jpg");

	// === Motion: 强力向心 + 高速旋转 ===
	coreGPU.m_motion.m_velocityMin.x = 0.0f;
	coreGPU.m_motion.m_velocityMin.y = 0.0f;
	coreGPU.m_motion.m_velocityMax.x = 0.0f;
	coreGPU.m_motion.m_velocityMax.y = 0.0f;
	coreGPU.m_motion.m_drag = 0.2f;                  // 轻微阻力

	// 核心吸引力（向心力）
	coreDef->SetPointForce(200,200.f, 100.0f, true);  // 强力向心
	coreDef->EnablePointForce(true);

	// 高速旋转（涡流）
	coreDef->SetVortexForce(100.f, 100.0f);       // 高速切向力
	coreDef->EnableVortex(true);

	// === Appearance: 青色 → 紫色 → 透明 ===
	coreGPU.m_appearance.m_numColorKeyframes = 4;

	// t=0.0: 亮青色
	Rgba8(0, 255, 255, 255).GetAsFloats(coreGPU.m_appearance.m_colorKeyframes[0].m_colorPacked);
	coreGPU.m_appearance.m_colorKeyframes[0].m_time = 0.0f;

	// t=0.3: 蓝紫色
	Rgba8(100, 100, 255, 255).GetAsFloats(coreGPU.m_appearance.m_colorKeyframes[1].m_colorPacked);
	coreGPU.m_appearance.m_colorKeyframes[1].m_time = 0.3f;

	// t=0.7: 深紫色
	Rgba8(150, 80, 255, 200).GetAsFloats(coreGPU.m_appearance.m_colorKeyframes[2].m_colorPacked);
	coreGPU.m_appearance.m_colorKeyframes[2].m_time = 0.7f;

	// t=1.0: 透明
	Rgba8(100, 0, 150, 0).GetAsFloats(coreGPU.m_appearance.m_colorKeyframes[3].m_colorPacked);
	coreGPU.m_appearance.m_colorKeyframes[3].m_time = 1.0f;

	coreGPU.m_appearance.m_emissionStrength = 0.5f;  // 高发光强度

	// === Size: 脉动效果 ===
	coreGPU.m_numCurves = 1;
	coreGPU.m_curves[0].m_type = 0;  // SIZE
	coreGPU.m_curves[0].m_numKeyframes = 5;

	// 脉动：小 → 大 → 小 → 大 → 消失
	coreGPU.m_curves[0].m_keyframes[0].m_value = 1.0f;   // t=0.0
	coreGPU.m_curves[0].m_keyframes[0].m_time = 0.0f;

	coreGPU.m_curves[0].m_keyframes[1].m_value = 2.0f;   // t=0.25
	coreGPU.m_curves[0].m_keyframes[1].m_time = 0.25f;

	coreGPU.m_curves[0].m_keyframes[2].m_value = 2.0f;   // t=0.5
	coreGPU.m_curves[0].m_keyframes[2].m_time = 0.5f;

	coreGPU.m_curves[0].m_keyframes[3].m_value = 2.0f;   // t=0.75
	coreGPU.m_curves[0].m_keyframes[3].m_time = 0.75f;

	coreGPU.m_curves[0].m_keyframes[4].m_value = 1.0f;   // t=1.0
	coreGPU.m_curves[0].m_keyframes[4].m_time = 1.0f;

	coreGPU.m_motion.m_orientToVelocity = 1;  // 朝向运动方向

	uint32_t coreDefID = g_nova2D->CreateDefinition(coreDef);
	uint32_t coreInstID = g_nova2D->CreateInstance(coreDefID, centerPos.x, centerPos.y);
	g_nova2D->GetInstance(coreInstID)->Play();

	//======================================================================
	// 💫 Emitter 2: 能量环 (Energy Ring)
	// 特点：脉冲式扩散 + 彩虹渐变 + 快速消失
	//======================================================================
	auto* ringDef = new Nova2DEmitterDefinition();
	Nova2DEmitterDefinitionGPU& ringGPU = ringDef->GetGPUData();

	// === Emission: Burst 模式，周期性脉冲 ===
	ringGPU.m_emission.m_emissionMode = 1;           // BURST
	ringGPU.m_emission.m_lifetimeMin = 1.2f;
	ringGPU.m_emission.m_lifetimeMax = 1.8f;
	ringGPU.m_emission.m_emissionType = 1;           // Circle
	ringGPU.m_emission.m_emissionRadius =300.0f;      // 从中心发射

	ringDef->SetBurstConfig(50, 0.8f, -1);           // 每0.8秒爆发50个粒子
	//ringDef->SetTexturePath("Data/Images/Particles/566.png");

	// === Motion: 高速向外扩散 ===
	ringGPU.m_motion.m_velocityMin.x = -250.0f;
	ringGPU.m_motion.m_velocityMin.y = -250.0f;
	ringGPU.m_motion.m_velocityMax.x = 250.0f;
	ringGPU.m_motion.m_velocityMax.y = 250.0f;
	ringGPU.m_motion.m_velocityMode = 0;             // Random
	ringGPU.m_motion.m_drag = 1.5f;                  // 强阻力，快速减速

	// === Appearance: 彩虹渐变 ===
	ringGPU.m_appearance.m_numColorKeyframes = 5;

	// 彩虹序列：红 → 黄 → 绿 → 青 → 透明
	Rgba8(255, 0, 100, 255).GetAsFloats(ringGPU.m_appearance.m_colorKeyframes[0].m_colorPacked);  // 红
	ringGPU.m_appearance.m_colorKeyframes[0].m_time = 0.0f;

	Rgba8(255, 200, 0, 255).GetAsFloats(ringGPU.m_appearance.m_colorKeyframes[1].m_colorPacked);  // 黄
	ringGPU.m_appearance.m_colorKeyframes[1].m_time = 0.25f;

	Rgba8(0, 255, 100, 255).GetAsFloats(ringGPU.m_appearance.m_colorKeyframes[2].m_colorPacked);  // 绿
	ringGPU.m_appearance.m_colorKeyframes[2].m_time = 0.5f;

	Rgba8(0, 200, 255, 200).GetAsFloats(ringGPU.m_appearance.m_colorKeyframes[3].m_colorPacked);  // 青
	ringGPU.m_appearance.m_colorKeyframes[3].m_time = 0.75f;

	Rgba8(100, 100, 255, 0).GetAsFloats(ringGPU.m_appearance.m_colorKeyframes[4].m_colorPacked);  // 透明
	ringGPU.m_appearance.m_colorKeyframes[4].m_time = 1.0f;

	ringGPU.m_appearance.m_emissionStrength = 0.8f;

	// === Size: 快速扩大后消失 ===
	ringGPU.m_numCurves = 1;
	ringGPU.m_curves[0].m_type = 0;  // SIZE
	ringGPU.m_curves[0].m_numKeyframes = 3;

	ringGPU.m_curves[0].m_keyframes[0].m_value = 1.0f;   // t=0.0 小
	ringGPU.m_curves[0].m_keyframes[0].m_time = 0.0f;

	ringGPU.m_curves[0].m_keyframes[1].m_value = 3.0f;  // t=0.4 突然变大
	ringGPU.m_curves[0].m_keyframes[1].m_time = 0.4f;

	ringGPU.m_curves[0].m_keyframes[2].m_value = 1.0f;   // t=1.0 缩小消失
	ringGPU.m_curves[0].m_keyframes[2].m_time = 1.0f;

	//ringDef->SetSpriteSheet(13, 9, 0, 12, 30);  // 快速播放动画

	uint32_t ringDefID = g_nova2D->CreateDefinition(ringDef);
	uint32_t ringInstID = g_nova2D->CreateInstance(ringDefID, centerPos.x, centerPos.y);
	g_nova2D->GetInstance(ringInstID)->Play();

	//======================================================================
	// ✨ Emitter 3: 碎片喷溅 (Debris Spray)
	// 特点：爆炸式飞溅 + 重力下落 + 旋转 + 弹跳
	//======================================================================
	auto* debrisDef = new Nova2DEmitterDefinition();
	Nova2DEmitterDefinitionGPU& debrisGPU = debrisDef->GetGPUData();

	// === Emission: 持续发射 ===
	debrisGPU.m_emission.m_emissionRate = 180.0f;
	debrisGPU.m_emission.m_lifetimeMin = 2.5f;
	debrisGPU.m_emission.m_lifetimeMax = 4.0f;
	debrisGPU.m_emission.m_emissionType = 1;         // Circle
	debrisGPU.m_emission.m_emissionRadius = 20.0f;
	debrisGPU.m_emission.m_emissionMode = 0;         // CONTINUOUS

	//debrisDef->SetTexturePath("Data/Images/Particles/Hilda.jpg");

	// === Motion: 爆炸式飞溅 + 重力 ===
	debrisGPU.m_motion.m_velocityMin.x = -300.0f;
	debrisGPU.m_motion.m_velocityMin.y = -100.0f;    // 向上偏多
	debrisGPU.m_motion.m_velocityMax.x = 300.0f;
	debrisGPU.m_motion.m_velocityMax.y = 350.0f;     // 强力向上
	debrisGPU.m_motion.m_velocityMode = 0;           // Random
	debrisGPU.m_motion.m_drag = 0.3f;                // 轻微空气阻力

	// 添加重力（通过 Linear Force）
	debrisDef->SetLinearForce(Vec2(0.0f, -400.0f));  // 向下重力

	// === Appearance: 白热 → 橙 → 红 → 黑 ===
	debrisGPU.m_appearance.m_numColorKeyframes = 4;

	Rgba8(255, 255, 200, 255).GetAsFloats(debrisGPU.m_appearance.m_colorKeyframes[0].m_colorPacked);  // 白热
	debrisGPU.m_appearance.m_colorKeyframes[0].m_time = 0.0f;

	Rgba8(255, 150, 50, 255).GetAsFloats(debrisGPU.m_appearance.m_colorKeyframes[1].m_colorPacked);   // 橙
	debrisGPU.m_appearance.m_colorKeyframes[1].m_time = 0.3f;

	Rgba8(200, 50, 0, 200).GetAsFloats(debrisGPU.m_appearance.m_colorKeyframes[2].m_colorPacked);     // 深红
	debrisGPU.m_appearance.m_colorKeyframes[2].m_time = 0.7f;

	Rgba8(50, 0, 0, 0).GetAsFloats(debrisGPU.m_appearance.m_colorKeyframes[3].m_colorPacked);         // 黑灰透明
	debrisGPU.m_appearance.m_colorKeyframes[3].m_time = 1.0f;

	debrisGPU.m_appearance.m_emissionStrength = 0.4f;

	// === Size: 持续缩小 ===
	debrisGPU.m_numCurves = 1;
	debrisGPU.m_curves[0].m_type = 0;  // SIZE
	debrisGPU.m_curves[0].m_numKeyframes = 2;

	debrisGPU.m_curves[0].m_keyframes[0].m_value = 3.0f;   // t=0.0
	debrisGPU.m_curves[0].m_keyframes[0].m_time = 0.0f;

	debrisGPU.m_curves[0].m_keyframes[1].m_value = 1.0f;   // t=1.0 缩小
	debrisGPU.m_curves[0].m_keyframes[1].m_time = 1.0f;

	debrisGPU.m_motion.m_orientToVelocity = 1;  // 朝向运动方向（拖尾效果）

	// 可选：启用碰撞反弹
	debrisGPU.m_collision.m_enableCollision = 1;
	debrisGPU.m_collision.m_numRules = 1;

	// 与红色地形碰撞时反弹
	debrisGPU.m_collision.m_rules[0].m_targetColor[0] = 1.0f;  // R
	debrisGPU.m_collision.m_rules[0].m_targetColor[1] = 0.0f;  // G
	debrisGPU.m_collision.m_rules[0].m_targetColor[2] = 0.0f;  // B
	debrisGPU.m_collision.m_rules[0].m_targetColor[3] = 0.0f;  // A
	debrisGPU.m_collision.m_rules[0].m_response = 1;           // BOUNCE
	debrisGPU.m_collision.m_rules[0].m_bounceDamping = 0.6f;   // 损失40%能量
	debrisGPU.m_collision.m_rules[0].m_maxBounces = 2;

	uint32_t debrisDefID = g_nova2D->CreateDefinition(debrisDef);
	uint32_t debrisInstID = g_nova2D->CreateInstance(debrisDefID, centerPos.x, centerPos.y);
	g_nova2D->GetInstance(debrisInstID)->Play();

	//======================================================================
	// 🎉 效果完成
	//======================================================================
	DebuggerPrintf("✨ Nebula Burst Effect Created!\n");
	DebuggerPrintf("   Core (Vortex): Def=%u, Inst=%u\n", coreDefID, coreInstID);
	DebuggerPrintf("   Ring (Pulse):  Def=%u, Inst=%u\n", ringDefID, ringInstID);
	DebuggerPrintf("   Debris (Spray): Def=%u, Inst=%u\n", debrisDefID, debrisInstID);
}