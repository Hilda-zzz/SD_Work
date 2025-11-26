#pragma once
#include <vector>
#include "Nova2DParticle.hpp"

class Renderer;
class Camera;
class VertexBuffer;
class Shader;
class Nova2DEmitter;

struct Nova2DConfig 
{
	int maxParticles = 50000;  // MVP: 先用小数量
};

enum class Nova2DRenderMode
{
	CPU_VERTEX_BUILD,      // 旧方案：每帧构建顶点
	GPU_INSTANCED          // 新方案：实例化渲染
};

class Nova2DSystem {
public:
	Nova2DSystem(Renderer* renderer, Nova2DConfig const& config);
	~Nova2DSystem();

	// 生命周期
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Update(float deltaTime);
	void Render(Camera const& camera) const;

	// 粒子发射（MVP版本：直接发射，不用发射器）
	void EmitParticle(Vec2 pos, Vec2 vel, float lifetime, Rgba8 m_color, float size);
	void EmitBurst(Vec2 pos, int count, Vec2 baseVelocity, float spread);

	// 查询
	int GetAliveParticleCount() const;

	// ===== 新增：接受完整粒子结构 =====
	void EmitParticleStruct(Nova2DParticle const& particle);

	// ===== 新增：Emitter 管理 =====
	void RegisterEmitter(Nova2DEmitter* emitter);
	void UnregisterEmitter(Nova2DEmitter* emitter);

	// ===== 新增：获取当前游戏时间（用于动画） =====
	float GetCurrentGameTime() const { return m_totalGameTime; }

private:
	// Mode 1 Basic cpu update each particle
	void UpdateParticlesCPU(float deltaTime);
	void RenderParticlesCPU(Camera const& camera) const;
	int  FindDeadParticleSlot();

	// ------------------ Draw Instanced ---------------------------
	void RenderInstanced() const;

	// ------------------ Emitter ----------------------------------
	// Mode 2 Update emitters and then update each particle on CPU
	void UpdateEmitters(float deltaTime);

private:

	float m_totalGameTime = 0.0f;  // 累积游戏时间

	// CPU粒子数组（MVP: 先不用GPU）
	std::vector<Nova2DParticle> m_particles;

	// 配置
	Nova2DConfig m_config;
	Renderer* m_renderer = nullptr;

	// Bench Mark
	Nova2DRenderMode m_renderMode = Nova2DRenderMode::GPU_INSTANCED;

	// Draw Instanced
	VertexBuffer* m_quadVBO = nullptr;
	VertexBuffer* m_instanceVBO = nullptr;
	Shader* m_particleShader = nullptr;

	// Emitter
	std::vector<Nova2DEmitter*> m_activeEmitters;
	
};