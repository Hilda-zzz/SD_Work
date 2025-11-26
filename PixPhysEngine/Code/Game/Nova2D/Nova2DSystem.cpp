#include "Nova2DSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Nova2DParticleInstance.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Nova2DEmitter.hpp"
#include <map>

Nova2DSystem::Nova2DSystem(Renderer* renderer, Nova2DConfig const& config)
	:m_renderer(renderer),m_config(config)
{
}

Nova2DSystem::~Nova2DSystem() 
{

}

void Nova2DSystem::Startup() 
{
	// 1 -----------------------------------------------------------
	// 预分配粒子数组
	m_particles.resize(m_config.maxParticles);
	// 初始化为死亡状态
	for (auto& p : m_particles) 
	{
		p.m_lifetime = 0.0f;
	}

	// 2 -----------------------------------------------------------
	struct QuadVertex {
		Vec3 position;  
		Vec2 uv;
	};

	QuadVertex quadVerts[6] = {
		// 第一个三角形 (左下 → 右下 → 左上)
		{ Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f) },
		{ Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f) },
		{ Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f) },

		// 第二个三角形 (左上 → 右下 → 右上)
		{ Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f) },
		{ Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f) },
		{ Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f) },  
	};
	m_quadVBO = m_renderer->CreateVertexBuffer(6, sizeof(QuadVertex), false);
	m_renderer->CopyGameVertexBufferToGPU(quadVerts, 6, m_quadVBO);

	// 3 -----------------------------------------------------------
	m_instanceVBO = m_renderer->CreateVertexBuffer(
		m_config.maxParticles,
		sizeof(Nova2DParticleInstance),
		true  // ✅ Per-Instance
	);

	// 4 -----------------------------------------------------------
	m_particleShader = m_renderer->CreateShaderFromFile("Data/Nova2DShaders/Nova2DParticleInstanced",VertexType::NOVA2D_PARTICLE_INSTANCED);

	DebuggerPrintf("Nova2D System started with %d max particles\n", m_config.maxParticles);
}

void Nova2DSystem::Shutdown() 
{
	m_particles.clear();

	delete m_instanceVBO;
	delete m_quadVBO;

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
	// 更新总时间
	m_totalGameTime += deltaTime;

	// 更新发射器（会产生新粒子）
	UpdateEmitters(deltaTime);

	// 更新粒子物理
	UpdateParticlesCPU(deltaTime);
}

void Nova2DSystem::Render(Camera const& camera) const 
{
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		RenderParticlesCPU(camera);
		break;
	case Nova2DRenderMode::GPU_INSTANCED:
		RenderInstanced();
		break;
	default:
		break;
	}
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

void Nova2DSystem::EmitParticleStruct(Nova2DParticle const& particle)
{
	int slot = FindDeadParticleSlot();
	if (slot == -1) return;  // 粒子池满了

	m_particles[slot] = particle;
}

void Nova2DSystem::RegisterEmitter(Nova2DEmitter* emitter)
{
	if (emitter) 
	{
		m_activeEmitters.push_back(emitter);
	}
}

void Nova2DSystem::UnregisterEmitter(Nova2DEmitter* emitter)
{
	auto it = std::find(m_activeEmitters.begin(), m_activeEmitters.end(), emitter);
	if (it != m_activeEmitters.end()) 
	{
		m_activeEmitters.erase(it);
	}
}

void Nova2DSystem::UpdateParticlesCPU(float deltaTime) 
{
	constexpr float GLOBAL_GRAVITY = 200.0f;

	for (auto& p : m_particles) {
		if (!p.IsAlive()) continue;

		// 更新位置
		p.m_position += p.m_velocity * deltaTime;

		// 更新生命
		p.m_lifetime -= deltaTime;

		// ===== 根据标志应用重力 =====
		if (p.HasFlag(Nova2DParticleFlags::NOVA_FLAG_GRAVITY)) {
			p.m_velocity.y -= GLOBAL_GRAVITY * deltaTime;
		}

		//// ===== 根据标志应用旋转 =====
		//if (p.HasFlag(Nova2DParticleFlags::NOVA_FLAG_ROTATE)) {
		//	p.m_rotation += 90.0f * deltaTime;  // 每秒旋转 90 度
		//}

		//// ===== 根据标志应用淡出 =====
		//if (p.HasFlag(Nova2DParticleFlags::NOVA_FLAG_FADE_OUT)) {
		//	float alpha = p.GetLifetimeRatio();
		//	p.m_color.a = (unsigned char)(alpha * 255);
		//}
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

void Nova2DSystem::RenderInstanced() const
{
	/*std::vector<Nova2DParticleInstance> instances;
	for (auto& p : m_particles)
	{
		if (!p.IsAlive()) continue;
		instances.push_back({ p.m_position, Vec2(p.m_size, p.m_size), p.m_color, 0.0f });
	}
	m_renderer->CopyGameVertexBufferToGPU(instances.data(), instances.size(), m_instanceVBO);

	m_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	Texture* tex = m_renderer->CreateOrGetTextureFromFile("Data/Images/TestUV2.png");
	m_renderer->BindTexture(tex);
	m_renderer->BindShader(m_particleShader);

	m_renderer->DrawInstanced(m_quadVBO, m_instanceVBO, 6, instances.size());*/

	// ----------------------------------- new ---------------------------------------
	int aliveCount = GetAliveParticleCount();
	if (aliveCount == 0) return;

	// ===== 关键：按纹理分组渲染 =====
	// 避免频繁切换纹理状态
	std::map<Texture*, std::vector<Nova2DParticleInstance>> batchedParticles;

	float currentTime = m_totalGameTime;

	for (Nova2DParticle const& p : m_particles) 
	{
		if (!p.IsAlive()) continue;

		// 获取纹理和 UV
		Texture* tex = p.GetTexture();
		if (!tex) tex = nullptr;  

		AABB2 uvs = p.GetCurrentUVs(currentTime);

		// 构建实例数据
		Nova2DParticleInstance data;
		data.m_worldPosition = p.m_position;
		data.m_size = Vec2(p.m_size, p.m_size);
		data.m_color = p.m_color;
		data.m_rotation = p.m_rotation;
		data.m_uvMinX = uvs.m_mins.x;
		data.m_uvMinY = uvs.m_mins.y;
		data.m_uvMaxX = uvs.m_maxs.x;
		data.m_uvMaxY = uvs.m_maxs.y;

		// 按纹理分组
		batchedParticles[tex].push_back(data);
	}

	// ===== 按批次渲染 =====
	for (auto const& [texture, instances] : batchedParticles) 
	{
		if (instances.empty()) continue;

		// 创建临时实例 VBO
		VertexBuffer* instanceVBO = m_renderer->CreateVertexBuffer(
			instances.size(),
			sizeof(Nova2DParticleInstance),
			true  // ✅ Per-Instance
		);
		m_renderer->CopyGameVertexBufferToGPU(instances.data(), instances.size(), instanceVBO);

		// 设置渲染状态
		m_renderer->BindShader(m_particleShader);
		m_renderer->BindTexture(texture);
		m_renderer->SetBlendMode(BlendMode::ALPHA);  // 或 ADDITIVE

		// 绘制
		m_renderer->DrawInstanced(m_quadVBO, instanceVBO, 6, instances.size());

		// 清理
		delete instanceVBO;
	}
}

void Nova2DSystem::UpdateEmitters(float deltaTime)
{
	for (Nova2DEmitter* emitter : m_activeEmitters) 
	{
		if (emitter) 
		{
			emitter->Update(deltaTime, this);
		}
	}
}
