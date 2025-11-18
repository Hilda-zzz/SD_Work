#include "Nova2DSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Nova2DSystem::Nova2DSystem(Renderer* renderer, Nova2DConfig const& config)
	:m_renderer(renderer),m_config(config)
{
}

Nova2DSystem::~Nova2DSystem() {
}

void Nova2DSystem::Startup() 
{
	// 预分配粒子数组
	m_particles.resize(m_config.maxParticles);

	// 初始化为死亡状态
	for (auto& p : m_particles) 
	{
		p.m_lifetime = 0.0f;
	}

	DebuggerPrintf("Nova2D System started with %d max particles\n", m_config.maxParticles);
}

void Nova2DSystem::Shutdown() 
{
	m_particles.clear();
	DebuggerPrintf("Nova2D System shutdown\n");
}

void Nova2DSystem::BeginFrame()
{
}

void Nova2DSystem::EndFrame()
{
}

void Nova2DSystem::Update(float deltaTime) 
{
	UpdateParticlesCPU(deltaTime);
}

void Nova2DSystem::Render(Camera const& camera) const 
{
	RenderParticlesCPU(camera);
}

void Nova2DSystem::EmitParticle(Vec2 pos, Vec2 vel, float lifetime, Rgba8 m_color, float size) 
{
	int slot = FindDeadParticleSlot();
	if (slot == -1) return;  // 粒子池满了

	Nova2DParticle& p = m_particles[slot];
	p.m_position = pos;
	p.m_velocity = vel;
	p.m_lifetime = lifetime;
	p.m_maxLifetime = lifetime;
	p.m_size = size;
	p.m_color = m_color;
}

void Nova2DSystem::EmitBurst(Vec2 pos, int count, Vec2 baseVelocity, float spread) 
{
	RandomNumberGenerator rng;

	for (int i = 0; i < count; i++) 
	{
		// 随机角度
		float angle = rng.RollRandomFloatInRange(0.0f, 360.0f);
		float speed = rng.RollRandomFloatInRange(50.0f, 150.0f);

		Vec2 velocity = Vec2::MakeFromPolarDegrees(angle, speed);
		velocity += baseVelocity;

		// 随机颜色（黄到红的火花）
		Rgba8 m_color(
			255,
			rng.RollRandomFloatInRange(100, 200),
			rng.RollRandomFloatInRange(0, 50),
			255
		);

		float lifetime = rng.RollRandomFloatInRange(0.5f, 1.5f);
		float size = rng.RollRandomFloatInRange(2.0f, 5.0f);

		EmitParticle(pos, velocity, lifetime, m_color, size);
	}
}

int Nova2DSystem::GetAliveParticleCount() const 
{
	int count = 0;
	for (auto const& p : m_particles) 
	{
		if (p.IsAlive()) count++;
	}
	return count;
}

void Nova2DSystem::UpdateParticlesCPU(float deltaTime) 
{
	for (auto& p : m_particles) {
		if (!p.IsAlive()) continue;

		// 更新位置
		p.m_position += p.m_velocity * deltaTime;

		// 更新生命
		p.m_lifetime -= deltaTime;

		// 简单重力
		p.m_velocity.y -= 200.0f * deltaTime;  // 向下加速
	}
}

void Nova2DSystem::RenderParticlesCPU(Camera const& camera) const 
{
	std::vector<Vertex_PCU> verts;
	verts.reserve(GetAliveParticleCount() * 6);  // 每个粒子6个顶点（2三角形）

	for (auto const& p : m_particles) 
	{
		if (!p.IsAlive()) continue;

		Vec2 pixelPos(
			floorf(p.m_position.x),  // 向下取整到整数像素
			floorf(p.m_position.y)
		);

		float pixelSize = floorf(p.m_size);  // 取整
		if (pixelSize < 1.0f) pixelSize = 1.0f;  // 最小1像素

		// 创建方形粒子
		Vec2 mins = pixelPos;
		Vec2 maxs = pixelPos + Vec2(pixelSize, pixelSize);

		AddVertsForAABB2D(verts, AABB2(mins, maxs), p.m_color);
	}

	if (!verts.empty()) 
	{
		m_renderer->BindTexture(nullptr);  // 纯色，不用纹理
		m_renderer->SetModelConstants();
		m_renderer->DrawVertexArray(verts);
	}
}

int Nova2DSystem::FindDeadParticleSlot() 
{
	for (int i = 0; i < (int)m_particles.size(); i++) 
	{
		if (!m_particles[i].IsAlive()) 
		{
			return i;
		}
	}
	return -1;  // 没有空位
}