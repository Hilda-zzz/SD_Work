#pragma once
#include <vector>
#include "Nova2DParticle.hpp"

class Renderer;
class Camera;
class VertexBuffer;
class Shader;

struct Nova2DConfig 
{
	int maxParticles = 10000;  // MVP: 先用小数量
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

private:
	void UpdateParticlesCPU(float deltaTime);
	void RenderParticlesCPU(Camera const& camera) const;
	int  FindDeadParticleSlot();

	// ------------------ Draw Instanced ---------------------------
	void RenderInstanced() const;

private:
	// CPU粒子数组（MVP: 先不用GPU）
	std::vector<Nova2DParticle> m_particles;

	// 配置
	Nova2DConfig m_config;
	Renderer* m_renderer = nullptr;

	// Draw Instanced
	VertexBuffer* m_quadVBO = nullptr;
	VertexBuffer* m_instanceVBO = nullptr;
	Shader* m_particleShader = nullptr;
};