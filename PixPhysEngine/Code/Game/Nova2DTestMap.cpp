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

extern Nova2DSystem* g_nova2D;
extern Window* g_theWindow;

//==========================================================================
// 构造与析构
//==========================================================================
Nova2DTestMap::Nova2DTestMap(SandboxPlayer* player)
	:BaseMap(IntVec2(WANG_MAP_CHUNKS_X* WANG_CHUNK_SIZE, WANG_MAP_CHUNKS_Y* WANG_CHUNK_SIZE), WANG_CHUNK_SIZE)
{
	m_player = player;
	Initialize();
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

	// 清理纹理资源（根据你的资源管理策略）
	delete m_sparkSheet;
	delete m_circleSheet;
	delete m_explosionSheet;
	delete m_explosionAnim;
}

//==========================================================================
// 生命周期
//==========================================================================
void Nova2DTestMap::Initialize() 
{
	LoadTextureResources();
	CreatePresetEmitters();
}

void Nova2DTestMap::LoadTextureResources() 
{
	// 加载火花纹理
	//Texture* sparkTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Spark.png");
	//if (sparkTex) {
	//	m_sparkSheet = new SpriteSheet(*sparkTex, IntVec2(1, 1));
	//}

	//// 加载圆形纹理
	//Texture* circleTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Circle.png");
	//if (circleTex) {
	//	m_circleSheet = new SpriteSheet(*circleTex, IntVec2(1, 1));
	//}

	//// 加载爆炸动画（假设是 4x4 的 SpriteSheet）
	//Texture* explosionTex = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Explosion.png");
	//if (explosionTex) {
	//	SpriteSheet* m_explosionSheet = new SpriteSheet(*explosionTex, IntVec2(4, 4));
	//	m_explosionAnim = new SpriteAnimDefinition(
	//		*m_explosionSheet,
	//		0, 15,  // 16 帧动画
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
	fireEmitter->SetSprite(m_sparkSheet);
	m_emitters.push_back(fireEmitter);
	g_nova2D->RegisterEmitter(fireEmitter);

	// 预设 2：爆炸发射器
	Nova2DEmitter* explosionEmitter = new Nova2DEmitter(EmitterPresets::CreateExplosionConfig());
	explosionEmitter->SetPosition(Vec2(600, 300));
	if (m_explosionAnim) {
		explosionEmitter->SetAnimation(m_explosionAnim);
	}
	else {
		explosionEmitter->SetSprite(m_sparkSheet);
	}
	m_emitters.push_back(explosionEmitter);
	g_nova2D->RegisterEmitter(explosionEmitter);

	// 预设 3：烟雾发射器
	Nova2DEmitter* smokeEmitter = new Nova2DEmitter(EmitterPresets::CreateSmokeConfig());
	smokeEmitter->SetPosition(Vec2(800, 300));
	smokeEmitter->SetSprite(m_circleSheet);
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
	g_nova2D->Update(deltaTime);

	// 鼠标点击发射（测试用）
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE)) 
	{
		Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
		Vec2 mousePosInWorld = AABB2(m_player->m_camera.GetOrthoBottomLeft(), m_player->m_camera.GetOrthoTopRight()).GetPointAtUV(mouseUV);

		Nova2DEmitter* currentEmitter = GetCurrentEmitter();
		if (currentEmitter) 
		{
			currentEmitter->SetPosition(mousePosInWorld);
			currentEmitter->Reset();
			currentEmitter->Play();
		}
	}

	// 如果配置有修改，同步到 Emitter
	if (m_configDirty) 
	{
		ApplyConfigToEmitter();
		m_configDirty = false;
	}
}

void Nova2DTestMap::Render() const 
{
	g_theRenderer->BeginCamera(m_player->m_camera);
	// 渲染粒子系统
	g_nova2D->Render(m_player->GetCamera());
	g_theRenderer->EndCamera(m_player->m_camera);

	// 渲染 UI
	RenderUI();

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

void Nova2DTestMap::CreateNewEmitter() 
{
	Nova2DEmitter* newEmitter = new Nova2DEmitter(m_editingConfig);
	newEmitter->SetPosition(Vec2(400, 300));
	newEmitter->SetSprite(m_sparkSheet);

	m_emitters.push_back(newEmitter);
	g_nova2D->RegisterEmitter(newEmitter);

	m_selectedEmitterIndex = (int)m_emitters.size() - 1;
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