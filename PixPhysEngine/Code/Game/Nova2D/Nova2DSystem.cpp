#include "Nova2DSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Nova2DParticleInstance.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Nova2DEmitter.hpp"
#include "Nova2DGPUBuffers.hpp"
#include <map>
#include "Nova2DEmitterDefinition.hpp"
#include "Nova2DEmitterInstance.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include <ThirdParty/tracy/tracy/Tracy.hpp>
#include <Engine/Core/Time.hpp>
#include "Engine/Renderer/Texture2DArray.hpp"
#include "Engine/Core/Image.hpp"

Nova2DSystem::Nova2DSystem(Renderer* renderer, Nova2DConfig const& config)
	:m_renderer(renderer),m_config(config)
{
}

Nova2DSystem::~Nova2DSystem() 
{

}

void Nova2DSystem::Startup() 
{
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		StartupCPU();
		break;

	case Nova2DRenderMode::GPU_INSTANCED:
		StartupGPUInstanced();
		break;

	case Nova2DRenderMode::GPU_COMPUTE:
		StartupGPUCompute();
		break;
	}

	// 测试创建Texture2DArray
	std::vector<std::string> paths = {
		"Data/Images/fire.png",
		"Data/Images/smoke.png",
		"Data/Images/spark.png"
	};
}

void Nova2DSystem::Shutdown() 
{
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		ShutdownCPU();
		break;

	case Nova2DRenderMode::GPU_INSTANCED:
		ShutdownGPUInstanced();
		break;

	case Nova2DRenderMode::GPU_COMPUTE:
		ShutdownGPUCompute();
		break;
	}
}

void Nova2DSystem::BeginFrame()
{
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		BeginFrameCPU();
		break;

	case Nova2DRenderMode::GPU_INSTANCED:
		BeginFrameGPUInstanced();
		break;

	case Nova2DRenderMode::GPU_COMPUTE:
		BeginFrameGPUCompute();
		break;
	}
}

void Nova2DSystem::EndFrame()
{
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		EndFrameCPU();
		break;

	case Nova2DRenderMode::GPU_INSTANCED:
		EndFrameGPUInstanced();
		break;

	case Nova2DRenderMode::GPU_COMPUTE:
		EndFrameGPUCompute();
		break;
	}
}

void Nova2DSystem::Update(float deltaTime, Camera const& camera, Texture* collisionTexture)
{
	//ZoneScoped;
	auto startTime = GetCurrentTimeSeconds();
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		UpdateEmittersCPU(deltaTime);
		UpdateCPU(deltaTime);
		break;

	case Nova2DRenderMode::GPU_INSTANCED:
		UpdateEmittersGPUInstanced(deltaTime);
		UpdateGPUInstanced(deltaTime);
		break;

	case Nova2DRenderMode::GPU_COMPUTE:
		ResetGPUCounters();
		UpdateEmittersGPUCompute(deltaTime);
		//UpdateEmittersCPU(deltaTime);
		UpdateGPUCompute(deltaTime,camera,collisionTexture);
		break;
	}
	auto endTime = GetCurrentTimeSeconds();
	auto duration = endTime - startTime;
	static int frameCounter = 0;
	if (++frameCounter >= 60)
	{
		DebuggerPrintf("[Nova2D] Update() Time: %.3f ms\n", duration * 1000.0f);
		frameCounter = 0;
	}

}

void Nova2DSystem::Render(Camera const& camera) const 
{
	ZoneScoped;
	switch (m_renderMode)
	{
	case Nova2DRenderMode::CPU_VERTEX_BUILD:
		RenderCPU(camera);
		break;

	case Nova2DRenderMode::GPU_INSTANCED:
		RenderGPUInstanced();
		break;

	case Nova2DRenderMode::GPU_COMPUTE:
		RenderGPUCompute(camera);
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

void Nova2DSystem::StartupCPU()
{
	// 预分配粒子数组
	m_particles.resize(m_config.maxParticles);
	for (auto& p : m_particles)
	{
		p.m_lifetime = 0.0f;
	}

	DebuggerPrintf("Nova2D System started with CPU_VERTEX_BUILD mode\n");
}

void Nova2DSystem::ShutdownCPU()
{
	m_particles.clear();
	DebuggerPrintf("Nova2D CPU mode shutdown\n");
}

void Nova2DSystem::BeginFrameCPU()
{
}

void Nova2DSystem::EndFrameCPU()
{
}

void Nova2DSystem::UpdateEmittersCPU(float deltaTime)
{
	for (Nova2DEmitter* emitter : m_activeEmitters)
	{
		if (!emitter) continue;

		// 1. CPU计算发射数量
		int particlesToEmit = CalculateEmitCount(emitter, deltaTime);

		// 2. 如果有粒子要发射
		if (particlesToEmit > 0)
		{
			// 每个发射器单独GPU发射（简单但不是最优）
			EmitParticlesGPU(emitter->GetConfig(), particlesToEmit);
		}
	}
}

void Nova2DSystem::UpdateCPU(float deltaTime)
{
	UpdateParticlesCPU(deltaTime);
}

void Nova2DSystem::RenderCPU(Camera const& camera) const
{
	RenderParticlesCPU(camera);
}

void Nova2DSystem::EmitParticleStruct(Nova2DParticle const& particle)
{
	int slot = FindDeadParticleSlot();
	if (slot == -1) return;  // 粒子池满了

	m_particles[slot] = particle;
}

void Nova2DSystem::EmitParticlesGPU(Nova2DEmitterConfig const& config, int count)
{
	if (!m_gpuBuffers || !m_emitShader || count <= 0)
		return;

	// ===== 1. 准备发射参数 =====
	Nova2DGPUEmitParams params;

	// 位置
	params.m_emitterPosition = config.m_position;

	// 速度范围
	params.m_velocityMin = config.m_motionConfig.m_startVelocityMin;
	params.m_velocityMax = config.m_motionConfig.m_startVelocityMax;

	// 生命周期范围
	params.m_lifetimeMin = config.m_emissionConfig.m_lifetimeMin;
	params.m_lifetimeMax = config.m_emissionConfig.m_lifetimeMax;

	// 尺寸范围
	params.m_sizeMin = config.m_appearanceConfig.m_sizeMin;
	params.m_sizeMax = config.m_appearanceConfig.m_sizeMax;

	// 发射形状
	params.m_shapeType = (uint32_t)config.m_emissionConfig.m_shape;
	params.m_shapeParam1 = config.m_emissionConfig.m_shapeRadius;
	params.m_shapeParam2 = config.m_emissionConfig.m_shapeBoxSize.x;

	params.m_color = config.m_appearanceConfig.m_colorStart;

	// 精灵索引（MVP阶段固定为0）
	params.m_spriteIndex = 0;

	// 发射数量
	params.m_emitCount = count;

	// 标志位
	uint32_t flags = 0;
	if (config.m_enableGravity)
	{
		flags |= 0x1;  // PARTICLE_FLAG_GRAVITY
	}
	params.m_flags = flags;

	// 随机种子（每次发射不同）
	params.m_randomSeed = (uint32_t)(rand() % INT_MAX);

	// 最大粒子数
	params.m_maxParticles = m_config.maxParticles;

	// 填充对齐
	params.m_padding[0] = 0;

	// ===== 2. 上传参数到GPU =====
	m_renderer->CopyConstantBufferToGPU(
		&params,
		sizeof(Nova2DGPUEmitParams),
		m_gpuBuffers->GetEmitParamsBuffer()
	);

	// ===== 3. 绑定资源 =====
	// dealing with ping pong buffer

	StructuredBuffer* currentAliveList = (m_pingPongIndex == 0)
		? m_gpuBuffers->GetAliveList1()   // pingPong=0 → In=List1
		: m_gpuBuffers->GetAliveList2();

	m_renderer->BindComputeShader(m_emitShader);
	m_renderer->BindStructuredBufferUAV(0, m_gpuBuffers->GetParticleBuffer());    // u0
	m_renderer->BindStructuredBufferUAV(1, m_gpuBuffers->GetDeadListBuffer());    // u1
	m_renderer->BindStructuredBufferUAV(2, m_gpuBuffers->GetCounterBuffer());     // u2
	m_renderer->BindStructuredBufferUAV(3, currentAliveList);     // u3
	m_renderer->BindConstantBufferCS(0, m_gpuBuffers->GetEmitParamsBuffer());       // b0

	// ===== 4. 调度GPU发射 =====
	// 计算需要的线程组数：每组256线程
	int threadGroups = (count + 255) / 256;
	m_renderer->Dispatch(threadGroups, 1, 1);

	Nova2DGPUParticleCounters counters = m_gpuBuffers->ReadCounters();
	DebuggerPrintf("  After emit: aliveCount=%d, deadCount=%d\n",
		counters.m_aliveCount, counters.m_deadCount);

	// ===== 5. 解绑资源 =====
	m_renderer->UnbindStructuredBuffers();
	m_renderer->UnbindComputeShader();
}

void Nova2DSystem::EmitParticlesGPU_V2()
{
	if (!m_gpuBuffers || !m_emitShader_v2 || m_emitterInstances.empty())
		return;

	// ===== 1. 准备EmitParams (b0) =====
	struct EmitParamsV2
	{
		uint32_t instanceCount;
		uint32_t randomSeed;
		uint32_t maxParticles;
		uint32_t padding;
	};

	EmitParamsV2 params;
	params.instanceCount = (uint32_t)m_emitterInstances.size();
	params.randomSeed = (uint32_t)(rand() % INT_MAX);
	params.maxParticles = m_config.maxParticles;
	params.padding = 0;

	// 上传到EmitParams Buffer（复用旧的Buffer）
	m_renderer->CopyConstantBufferToGPU(
		&params,
		sizeof(EmitParamsV2),
		m_gpuBuffers->GetEmitParamsBuffer()
	);

	// ===== 2. 绑定Shader和Buffers =====
	StructuredBuffer* currentAliveList = (m_pingPongIndex == 0)
		? m_gpuBuffers->GetAliveList1()
		: m_gpuBuffers->GetAliveList2();

	m_renderer->BindComputeShader(m_emitShader_v2);

	// UAV Buffers
	m_renderer->BindStructuredBufferUAV(0, m_gpuBuffers->GetParticleBuffer());  // u0
	m_renderer->BindStructuredBufferUAV(1, m_gpuBuffers->GetDeadListBuffer());  // u1
	m_renderer->BindStructuredBufferUAV(2, m_gpuBuffers->GetCounterBuffer());   // u2
	m_renderer->BindStructuredBufferUAV(3, currentAliveList);                    // u3

	// SRV Buffers（新增）
	m_renderer->BindStructuredBufferSRV(5, m_gpuBuffers->GetDefinitionBuffer()); // t5
	m_renderer->BindStructuredBufferSRV(6, m_gpuBuffers->GetInstanceBuffer());   // t6

	// Constant Buffer
	m_renderer->BindConstantBufferCS(0, m_gpuBuffers->GetEmitParamsBuffer());    // b0

	// ===== 3. Dispatch =====
	// 每个线程处理一个Instance
	int threadGroups = ((int)m_emitterInstances.size() + 255) / 256;
	m_renderer->Dispatch(threadGroups, 1, 1);

	// ===== 4. Unbind =====
	m_renderer->UnbindStructuredBuffers();
	m_renderer->UnbindComputeShader();

}

void Nova2DSystem::EmitParticlesGPU_V3()
{
	if (!m_gpuBuffers || !m_emitShader_v3 || m_emitterInstances.empty())
		return;

	std::vector<uint32_t> emitOffsets;
	emitOffsets.reserve(m_emitterInstances.size());

	uint32_t totalEmitCount = 0;

	for (Nova2DEmitterInstance* inst : m_emitterInstances)
	{
		emitOffsets.push_back(totalEmitCount);
		int toEmit = (int)inst->GetGPUDataReadOnly().m_emissionAccumulator;
		totalEmitCount += toEmit;
	}

	if (totalEmitCount == 0)
	{
		return;
	}

	m_renderer->CopyDataToStructuredBuffer(m_gpuBuffers->GetEmitOffsetsBuffer(),
		emitOffsets.data(),
		emitOffsets.size());

	struct EmitParamsV3
	{
		uint32_t totalEmitCount;    
		uint32_t instanceCount;   
		uint32_t randomSeed;
		uint32_t maxParticles;
	};

	EmitParamsV3 params;
	params.totalEmitCount = totalEmitCount;
	params.instanceCount = (uint32_t)m_emitterInstances.size();
	params.randomSeed = (uint32_t)(rand() % INT_MAX);
	params.maxParticles = m_config.maxParticles;

	m_renderer->CopyConstantBufferToGPU(
		&params,
		sizeof(EmitParamsV3),
		m_gpuBuffers->GetEmitParamsBuffer()
	);

	StructuredBuffer* currentAliveList = (m_pingPongIndex == 0)
		? m_gpuBuffers->GetAliveList1()
		: m_gpuBuffers->GetAliveList2();

	m_renderer->BindComputeShader(m_emitShader_v3);

	// UAV Buffers
	m_renderer->BindStructuredBufferUAV(0, m_gpuBuffers->GetParticleBuffer());  // u0
	m_renderer->BindStructuredBufferUAV(1, m_gpuBuffers->GetDeadListBuffer());  // u1
	m_renderer->BindStructuredBufferUAV(2, m_gpuBuffers->GetCounterBuffer());   // u2
	m_renderer->BindStructuredBufferUAV(3, currentAliveList);                    // u3

	// SRV Buffers
	m_renderer->BindStructuredBufferSRV(5, m_gpuBuffers->GetDefinitionBuffer()); // t5
	m_renderer->BindStructuredBufferSRV(6, m_gpuBuffers->GetInstanceBuffer());   // t6
	m_renderer->BindStructuredBufferSRV(7, m_gpuBuffers->GetEmitOffsetsBuffer()); // t7 ← 新增

	// Constant Buffer
	m_renderer->BindConstantBufferCS(0, m_gpuBuffers->GetEmitParamsBuffer());    // b0

	// ===== 5. Dispatch =====
	// 关键变化：按粒子数Dispatch，不再按Instance数
	int threadGroups = (totalEmitCount + 63) / 64;  // 每组64个线程
	m_renderer->Dispatch(threadGroups, 1, 1);

	// ===== 6. Unbind =====
	m_renderer->UnbindStructuredBuffers();
	m_renderer->UnbindComputeShader();
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

void Nova2DSystem::StartupGPUInstanced()
{
	// 1. CPU粒子数组
	m_particles.resize(m_config.maxParticles);
	for (auto& p : m_particles)
	{
		p.m_lifetime = 0.0f;
	}

	// 2. 渲染资源
	struct QuadVertex {
		Vec3 position;
		Vec2 uv;
	};

	QuadVertex quadVerts[6] = {
		{ Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f) },
		{ Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f) },
		{ Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f) },
		{ Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f) },
		{ Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f) },
		{ Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f) },
	};
	m_quadVBO = m_renderer->CreateVertexBuffer(6, sizeof(QuadVertex), false);
	m_renderer->CopyGameVertexBufferToGPU(quadVerts, 6, m_quadVBO);

	m_instanceVBO = m_renderer->CreateVertexBuffer(
		m_config.maxParticles,
		sizeof(Nova2DParticleInstance),
		true
	);

	m_particleShader = m_renderer->CreateShaderFromFile(
		"Data/Shaders/Nova2DShaders/Nova2DParticleInstanced",
		VertexType::NOVA2D_PARTICLE_INSTANCED
	);

	DebuggerPrintf("Nova2D System started with GPU_INSTANCED mode\n");
}

void Nova2DSystem::ShutdownGPUInstanced()
{
	m_particles.clear();
	delete m_instanceVBO;
	delete m_quadVBO;
	m_instanceVBO = nullptr;
	m_quadVBO = nullptr;

	DebuggerPrintf("Nova2D GPU Instanced mode shutdown\n");
}

void Nova2DSystem::BeginFrameGPUInstanced()
{
}

void Nova2DSystem::EndFrameGPUInstanced()
{
}

void Nova2DSystem::UpdateEmittersGPUInstanced(float deltaTime)
{
	for (Nova2DEmitter* emitter : m_activeEmitters)
	{
		if (!emitter) continue;

		// 1. CPU计算发射数量
		int particlesToEmit = CalculateEmitCount(emitter, deltaTime);

		// 2. 如果有粒子要发射
		if (particlesToEmit > 0)
		{
			// 每个发射器单独GPU发射（简单但不是最优）
			EmitParticlesGPU(emitter->GetConfig(), particlesToEmit);
		}
	}
}

void Nova2DSystem::UpdateGPUInstanced(float deltaTime)
{
	UpdateParticlesCPU(deltaTime);
}

void Nova2DSystem::RenderGPUInstanced() const
{
	int aliveCount = GetAliveParticleCount();
	if (aliveCount == 0) return;

	// 按纹理分组（完整实现见之前代码）
	std::map<Texture*, std::vector<Nova2DParticleInstance>> batchedParticles;

	for (Nova2DParticle const& p : m_particles)
	{
		if (!p.IsAlive()) continue;

		Texture* tex = p.GetTexture();
		float particleLifeTime = p.GetPassedTime();
		AABB2 uvs = p.GetCurrentUVs(particleLifeTime);

		Nova2DParticleInstance data;
		data.m_worldPosition = p.m_position;
		data.m_size = Vec2(p.m_size, p.m_size);

		data.m_color = p.m_color;

		data.m_rotation = p.m_rotation;
		data.m_uvMinX = uvs.m_mins.x;
		data.m_uvMinY = uvs.m_mins.y;
		data.m_uvMaxX = uvs.m_maxs.x;
		data.m_uvMaxY = uvs.m_maxs.y;

		batchedParticles[tex].push_back(data);
	}

	// 批次渲染
	for (auto const& [texture, instances] : batchedParticles)
	{
		if (instances.empty()) continue;

		VertexBuffer* instanceVBO = m_renderer->CreateVertexBuffer(
			instances.size(),
			sizeof(Nova2DParticleInstance),
			true
		);
		m_renderer->CopyGameVertexBufferToGPU(instances.data(), instances.size(), instanceVBO);

		m_renderer->BindShader(m_particleShader);
		m_renderer->BindTexture(texture);
		m_renderer->SetBlendMode(BlendMode::ALPHA);
		m_renderer->SetModelConstants();

		m_renderer->DrawInstanced(m_quadVBO, instanceVBO, 6, instances.size());

		delete instanceVBO;
	}
}

void Nova2DSystem::StartupGPUCompute()
{
	// 1. 渲染资源
	struct QuadVertex {
		Vec3 position;
		Vec2 uv;
	};

	QuadVertex quadVerts[6] = {
		{ Vec3(-0.5f, -0.5f, 0.0f), Vec2(0.0f, 0.0f) },
		{ Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f) },
		{ Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f) },
		{ Vec3(-0.5f, 0.5f, 0.0f), Vec2(0.0f, 1.0f) },
		{ Vec3(0.5f, -0.5f, 0.0f), Vec2(1.0f, 0.0f) },
		{ Vec3(0.5f, 0.5f, 0.0f), Vec2(1.0f, 1.0f) },
	};
	m_quadVBO = m_renderer->CreateVertexBuffer(6, sizeof(QuadVertex), false);
	m_renderer->CopyGameVertexBufferToGPU(quadVerts, 6, m_quadVBO);

	//m_instanceVBO = m_renderer->CreateVertexBuffer(
	//	m_config.maxParticles,
	//	sizeof(Nova2DParticleInstance),
	//	true
	//);

	m_populatedVBO = m_renderer->CreateVertexBuffer(
		m_config.maxParticles,              // 最大实例数
		32,
		false,true                               // ← enableUAV = true ✅
	);

	//m_particleShader = m_renderer->CreateShaderFromFile(
	//	"Data/Nova2DShaders/Nova2DParticleInstanced",
	//	VertexType::NOVA2D_PARTICLE_INSTANCED
	//);

	// 2. GPU资源
	m_gpuBuffers = new Nova2DGPUBuffers(m_renderer, m_config.maxParticles);
	m_gpuBuffers->Startup();

	m_bindlessRenderShader = m_renderer->CreateShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DBindlessRender",VertexType::VERTEX_PCU_TEXINDEX);

	// 3. Compute Shaders
	m_resetCountersShader = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DResetCounters");
	//m_emitShader = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DEmit");
	//m_emitShader_v2 = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DEmit_v2");
	m_emitShader_v3 = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DEmit_v3");
	m_updateShader = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DUpdate");
	m_populateVBOShader = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DPopulateVBO");
	m_writeIndirectArgsShader = m_renderer->CreateComputeShaderFromFile("Data/Shaders/Nova2DShaders/Nova2DWriteIndirectArgs");
	m_populateParamsCBO = m_renderer->CreateConstantBuffer(16);

	m_pingPongIndex = 0;

	//--------------------------------
	m_populatedIBO = m_renderer->CreateIndexBuffer(m_config.maxParticles * 6);
	std::vector<unsigned int> indices;
	indices.reserve(m_config.maxParticles * 6);

	for (int i = 0; i < m_config.maxParticles; ++i)
	{
		unsigned int baseVertex = i * 4;

		// 第一个三角形（逆时针）
		indices.push_back(baseVertex + 0);  // 左下
		indices.push_back(baseVertex + 1);  // 右下
		indices.push_back(baseVertex + 2);  // 右上

		// 第二个三角形（逆时针）
		indices.push_back(baseVertex + 0);  // 左下
		indices.push_back(baseVertex + 2);  // 右上
		indices.push_back(baseVertex + 3);  // 左上
	}
	m_renderer->CopyGameIndexBufferToGPU(indices.data(), indices.size(), m_populatedIBO);

	InitializeDefaultTextureArray();

	//-----------------------------------------------
	DebuggerPrintf("Nova2D System started with GPU_COMPUTE mode\n");
}

void Nova2DSystem::ShutdownGPUCompute()
{
	if (m_gpuBuffers)
	{
		m_gpuBuffers->Shutdown();
		delete m_gpuBuffers;
		m_gpuBuffers = nullptr;
	}

	delete m_populatedVBO;
	delete m_instanceVBO;
	delete m_quadVBO;
	delete m_populatedIBO;

	m_populatedVBO = nullptr;
	m_quadVBO = nullptr;
	m_instanceVBO = nullptr;
	m_populatedIBO = nullptr;

	delete m_populateParamsCBO;  // ← 新增
	m_populateParamsCBO = nullptr;

	m_particleTextureArray = nullptr; 
	m_registeredTextures.clear();
	m_texturePathToIndex.clear();
	m_textureUVScales.clear();

	DebuggerPrintf("Nova2D GPU Compute mode shutdown\n");
}

void Nova2DSystem::BeginFrameGPUCompute()
{
}

void Nova2DSystem::EndFrameGPUCompute()
{
}

void Nova2DSystem::UpdateEmittersGPUCompute(float deltaTime)
{
	ZoneScoped;
	// ===== 1. CPU更新Instance状态 =====
	for (auto* instance : m_emitterInstances) {
		if (!instance) continue;
		instance->UpdateCPU(deltaTime);
	}

	// ===== 2. 上传Definition/Instance到GPU =====
	UploadDefinitionsToGPU();
	UploadInstancesToGPU();

	// ===== 3. GPU发射粒子（新架构）=====
	//EmitParticlesGPU_V2();
	EmitParticlesGPU_V3();
}

void Nova2DSystem::UpdateGPUCompute(float deltaTime, Camera const& camera, Texture* collisionTexture)
{
	ZoneScoped;
	UpdateParticlesGPU(deltaTime,camera,collisionTexture);
}

void Nova2DSystem::RenderGPUCompute(Camera const& camera) const
{
	//uint32_t args[5];
	//m_renderer->ReadIndirectArgsBuffer(m_gpuBuffers->GetIndirectArgsBuffer(), args, 5 * sizeof(uint32_t));

	//Nova2DGPUParticleCounters counters = m_gpuBuffers->ReadCounters();

	//DebuggerPrintf("=== FRAME RENDER DEBUG ===\n");
	//DebuggerPrintf("IndirectArgs:    [%u, %u, %u, %u, %u]\n",
	//	args[0], args[1], args[2], args[3], args[4]);
	//DebuggerPrintf("Counters[0] (aliveCount):         %u\n", counters.m_aliveCount);
	//DebuggerPrintf("Counters[3] (aliveCountAfterSim): %u\n", counters.m_aliveCountAfterSim);
	//DebuggerPrintf("PingPongIndex: %d\n", m_pingPongIndex);
	//DebuggerPrintf("Expected IndirectArgs[1] should equal Counters[3]\n");
	//DebuggerPrintf("========================\n");
	//uint32_t args[5];
	//m_renderer->ReadIndirectArgsBuffer(m_gpuBuffers->GetIndirectArgsBuffer(), args, 5);
	//DebuggerPrintf("IndirectArgs: [%u, %u, %u, %u, %u]\n",
	//	args[0], args[1], args[2], args[3], args[4]);
	////// ===== 1. 读取存活粒子数 =====
	//Nova2DGPUParticleCounters counters = m_gpuBuffers->ReadCounters();
	//int aliveCount = counters.m_aliveCountAfterSim;  // ← 使用Update后的数量

	//DebuggerPrintf("Active Particles: %d / %d (%.1f%%)\n",
	//	aliveCount,
	//	m_config.maxParticles,
	//	(float)aliveCount / m_config.maxParticles * 100.0f);

	//if (aliveCount <= 0)
	//return;

	m_renderer->BeginCamera(camera);
	// ===== 2. 确定当前AliveList（Update的输出）=====
	// Update 写入 aliveListOut，所以PopulateVBO读取 aliveListOut
	StructuredBuffer* currentAliveList = (m_pingPongIndex == 0)
	? m_gpuBuffers->GetAliveList2()   // Update刚写入List2
		: m_gpuBuffers->GetAliveList1();  // Update刚写入List1

	// ===== 3. PopulateVBO（GPU填充顶点）=====
	{
		m_renderer->BindComputeShader(m_populateVBOShader);

		// 输入（SRV）
		m_renderer->BindStructuredBufferSRV(0, currentAliveList);              // t0
		m_renderer->BindStructuredBufferSRV(1, m_gpuBuffers->GetParticleBuffer()); // t1
		m_renderer->BindStructuredBufferSRV(2, m_gpuBuffers->GetCounterBuffer());  // t2
		m_renderer->BindStructuredBufferSRV(5, m_gpuBuffers->GetDefinitionBuffer()); // t5
		m_renderer->BindStructuredBufferSRV(6, m_gpuBuffers->GetInstanceBuffer());   // t6
		// 输出（UAV）
		m_renderer->BindVertexBufferUAV(0, m_populatedVBO);  // u0 ← ByteAddressBuffer

		// Dispatch（每256个粒子一组）
		int threadGroups = (m_config.maxParticles + 255) / 256;
		m_renderer->Dispatch(threadGroups, 1, 1);

		// 解绑
		m_renderer->UnbindStructuredBuffers();
		m_renderer->UnbindComputeShader();
		m_renderer->BindVertexBufferUAV(0, nullptr);
	}

	// ===== 4. 渲染（直接使用GPU填充的VBO）=====
	{
		if (m_particleTextureArray)
		{
			m_renderer->BindShader(m_bindlessRenderShader);
			m_renderer->BindTexture2DArray(m_particleTextureArray, 10);  // slot 10
		}
		else
		{
			m_renderer->BindShader(nullptr);
			m_renderer->BindTexture(nullptr);
		}

		//m_renderer->BindTexture(nullptr);
		m_renderer->SetBlendMode(BlendMode::ALPHA);  
		m_renderer->SetModelConstants();

		//m_renderer->DrawGameIndexedVertexBuffer(m_populatedVBO,m_populatedIBO, counters.m_aliveCountAfterSim*6);

		m_renderer->DrawIndexedIndirect(
			m_populatedVBO,
			m_populatedIBO,
			m_gpuBuffers->GetIndirectArgsBuffer(),
			0  // byteOffset
		);
	}
	m_renderer->EndCamera(camera);
}

void Nova2DSystem::ResetGPUCounters()
{
	ZoneScoped;
	if (m_gpuBuffers && m_resetCountersShader)
	{
		m_renderer->BindComputeShader(m_resetCountersShader);
		m_renderer->BindStructuredBufferUAV(0, m_gpuBuffers->GetCounterBuffer());
		m_renderer->Dispatch(1, 1, 1);
		m_renderer->UnbindStructuredBuffers();
		m_renderer->UnbindComputeShader();
	}
}

void Nova2DSystem::UpdateParticlesGPU(float deltaTime, Camera const& camera, Texture* collisionTexture)
{
	if (!m_gpuBuffers || !m_updateShader || !m_resetCountersShader) return;

	//Nova2DGPUParticleCounters countersBefore = m_gpuBuffers->ReadCounters();
	//DebuggerPrintf("Before Update: Counters[0]=%u, Counters[3]=%u\n",
	//	countersBefore.m_aliveCount, countersBefore.m_aliveCountAfterSim);

	//// ===== 使用缓存的aliveCount，避免ReadCounters =====
	//static int cachedAliveCount = 0;  // 临时方案：缓存上一帧的值
	//int aliveCount = cachedAliveCount;

	//int aliveCount = 0;
	//{
	//	ZoneScopedN("alive count");
	//	Nova2DGPUParticleCounters counters = m_gpuBuffers->ReadCounters();
	//	aliveCount = counters.m_aliveCount;
	//	
	//}

	//if (aliveCount > 0)
	//{


		UpdateConstants params;
		params.deltaTime = deltaTime;
		params.gravity = 200.0f;
		params.currentTime = 0.0f;
		params.maxParticles = m_config.maxParticles;

		// Get viewport from camera
		params.viewportMin = camera.GetOrthoBottomLeft();
		params.viewportMax = camera.GetOrthoTopRight();

		if(collisionTexture)
			params.textureSize = collisionTexture->GetDimensions();

		m_renderer->CopyConstantBufferToGPU(&params, sizeof(UpdateConstants),
			m_gpuBuffers->GetUpdateConstantsBuffer());

		// Ping Pong Buffer In-> read Out-> write
		StructuredBuffer* aliveListIn = (m_pingPongIndex == 0)
			? m_gpuBuffers->GetAliveList1()
			: m_gpuBuffers->GetAliveList2();
		StructuredBuffer* aliveListOut = (m_pingPongIndex == 0)
			? m_gpuBuffers->GetAliveList2()
			: m_gpuBuffers->GetAliveList1();

		m_renderer->BindComputeShader(m_updateShader);
		m_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		m_renderer->BindStructuredBufferUAV(0, m_gpuBuffers->GetParticleBuffer());
		m_renderer->BindStructuredBufferUAV(1, m_gpuBuffers->GetDeadListBuffer());
		m_renderer->BindStructuredBufferUAV(2, m_gpuBuffers->GetCounterBuffer());
		m_renderer->BindStructuredBufferUAV(3, aliveListIn);                         // u3: AliveListIn  ← 新增
		m_renderer->BindStructuredBufferUAV(4, aliveListOut);
		//m_renderer->BindIndirectArgsBufferUAV(5, m_gpuBuffers->GetIndirectArgsBuffer());

		m_renderer->BindStructuredBufferSRV(5, m_gpuBuffers->GetDefinitionBuffer());
		m_renderer->BindStructuredBufferSRV(6, m_gpuBuffers->GetInstanceBuffer());

		m_renderer->BindConstantBufferCS(0, m_gpuBuffers->GetUpdateConstantsBuffer());
		m_renderer->BindTextureToComputeShader(10,collisionTexture);
		int threadGroups = (m_config.maxParticles + 255) / 256;
		m_renderer->Dispatch(threadGroups, 1, 1);

		m_renderer->UnbindStructuredBuffers();
		m_renderer->UnbindComputeShader();

		// ===== 写入IndirectArgs（独立Shader，在Update之后执行）=====
	{
		m_renderer->BindComputeShader(m_writeIndirectArgsShader);
		m_renderer->BindStructuredBufferUAV(0, m_gpuBuffers->GetCounterBuffer());
		m_renderer->BindIndirectArgsBufferUAV(1, m_gpuBuffers->GetIndirectArgsBuffer());

		m_renderer->Dispatch(1, 1, 1);  // 只需要1个线程组（1个线程）

		m_renderer->UnbindStructuredBuffers();
		m_renderer->BindIndirectArgsBufferUAV(1, nullptr);
		m_renderer->UnbindComputeShader();
		m_renderer->BindTextureToComputeShader(10, nullptr);
	}
	//}
	m_pingPongIndex = 1 - m_pingPongIndex;
}

//void Nova2DSystem::UpdateEmitters(float deltaTime)
//{
//	for (Nova2DEmitter* emitter : m_activeEmitters)
//	{
//		if (!emitter) continue;
//
//		// ===== GPU Compute模式 =====
//		if (m_renderMode == Nova2DRenderMode::GPU_COMPUTE)
//		{
//			// 1. CPU计算发射数量
//			int particlesToEmit = CalculateEmitCount(emitter, deltaTime);
//
//			// 2. 如果有粒子要发射
//			if (particlesToEmit > 0)
//			{
//				// 每个发射器单独GPU发射（简单但不是最优）
//				EmitParticlesGPU(emitter->GetConfig(), particlesToEmit);
//			}
//
//			// 3. 更新发射器状态（不实际发射粒子）
//			//UpdateEmitterStateOnly(emitter, deltaTime);
//		}
//		// ===== CPU模式 =====
//		else
//		{
//			emitter->Update(deltaTime, this);
//		}
//	}
//}

int Nova2DSystem::CalculateEmitCount(Nova2DEmitter* emitter, float deltaTime)
{
	if (!emitter->IsPlaying() || emitter->IsPaused())
		return 0;

	Nova2DEmitterConfig const& config = emitter->GetConfig();

	// Continuous 模式
	if (config.m_emissionConfig.m_mode == n2d_EmissionMode::CONTINUOUS)
	{
		float accumulator = emitter->GetEmissionAccumulator() +
			deltaTime * config.m_emissionConfig.m_emissionRate;
		return (int)accumulator;
	}

	// Burst 模式
	if (config.m_emissionConfig.m_mode == n2d_EmissionMode::BURST)
	{
		return config.m_emissionConfig.m_burstCount;
		//if (!emitter->HasEmittedBurst())
		//{
		//	return config.m_emissionConfig.m_burstCount;
		//}
	}

	return 0;
}

void Nova2DSystem::InitializeDefaultTextureArray()
{
	// ===== 预先在索引0注册默认纹理 =====
	m_registeredTextures.clear();
	m_texturePathToIndex.clear();

	m_registeredTextures.push_back("Data/Images/Particles/ENGINE_DEFAULT_WHITE.png");  // 索引0 = 默认白色纹理
	m_texturePathToIndex["Data/Images/Particles/ENGINE_DEFAULT_WHITE.png"] = 0;
	m_textureUVScales.push_back(Vec2(1.0f, 1.0f));

	// ===== 创建初始Array =====
	RebuildTextureArray();

	DebuggerPrintf("Nova2D: Initialized default texture at index 0\n");
}

uint32_t Nova2DSystem::RegisterTexture(const std::string& path)
{
	// ===== 空路径：返回索引0（默认纹理）=====
	if (path.empty())
	{
		return 0;
	}

	// ===== 检查是否已注册 =====
	auto it = m_texturePathToIndex.find(path);
	if (it != m_texturePathToIndex.end())
	{
		return it->second;
	}

	// ===== 注册新纹理 =====
	uint32_t newIndex = (uint32_t)m_registeredTextures.size();  // ← 直接用size
	m_registeredTextures.push_back(path);
	m_texturePathToIndex[path] = newIndex;

	DebuggerPrintf("Nova2D: Registered texture [%u] %s\n", newIndex, path.c_str());

	// ===== 重建Array =====
	RebuildTextureArray();

	return newIndex;
}

void Nova2DSystem::RebuildTextureArray()
{
	// ===== 1. 加载所有Image并找到最大尺寸 =====
	std::vector<Image*> images;
	IntVec2 maxDimensions(0, 0);

	for (const auto& path : m_registeredTextures)
	{
		Image* image = nullptr;

		if (path.empty())
		{
			image = new Image(IntVec2(1, 1), Rgba8::WHITE);
		}
		else
		{
			image = m_renderer->CreateImageFromFile(path.c_str());
		}

		if (image)
		{
			IntVec2 dims = image->GetDimensions();
			if (dims.x > maxDimensions.x) maxDimensions.x = dims.x;
			if (dims.y > maxDimensions.y) maxDimensions.y = dims.y;
			images.push_back(image);
		}
	}

	// ===== 2. 计算POT尺寸 =====
	int potSize = 1;
	while (potSize < maxDimensions.x || potSize < maxDimensions.y)
	{
		potSize *= 2;
	}
	if (potSize > 2048) potSize = 2048;

	// ===== 3. 计算每张纹理的UV缩放 =====
	m_textureUVScales.clear();
	m_textureUVScales.reserve(images.size());

	for (size_t i = 0; i < images.size(); i++)
	{
		IntVec2 dims = images[i]->GetDimensions();
		Vec2 uvScale(
			(float)dims.x / (float)potSize,
			(float)dims.y / (float)potSize
		);
		m_textureUVScales.push_back(uvScale);

		DebuggerPrintf("  [%d] UV Scale: (%.3f, %.3f) - %dx%d in %dx%d POT\n",
			(int)i, uvScale.x, uvScale.y, dims.x, dims.y, potSize, potSize);
	}

	// ===== 4. 清理Image =====
	for (Image* img : images)
	{
		delete img;
	}

	// ===== 5. 创建Texture2DArray =====
	m_particleTextureArray = m_renderer->CreateTexture2DArrayFromFiles(m_registeredTextures);

	DebuggerPrintf("Nova2D: Texture2DArray rebuilt (%d textures, POT=%d)\n",
		(int)m_registeredTextures.size(), potSize);

	UpdateAllDefinitionsUVScale();
}

void Nova2DSystem::UpdateAllDefinitionsUVScale()
{
	for (Nova2DEmitterDefinition* def : m_emitterDefs)
	{
		uint32_t texIndex = def->GetGPUDataReadOnly().m_appearance.m_textureIndex;

		if (texIndex < m_textureUVScales.size())
		{
			Vec2 uvScale = m_textureUVScales[texIndex];
			def->GetGPUData().m_appearance.m_textureUVScaleX = uvScale.x;
			def->GetGPUData().m_appearance.m_textureUVScaleY = uvScale.y;

			DebuggerPrintf("  Updated Definition ID=%u: texIndex=%u, uvScale=(%.3f, %.3f)\n",
				def->GetID(), texIndex, uvScale.x, uvScale.y);
		}
	}
}


//==========================================================================
// Definition 管理
//==========================================================================
uint32_t Nova2DSystem::CreateDefinition(Nova2DEmitterDefinition* definition) 
{
	definition->SetID(m_nextEmitterDefID);
	m_emitterDefs.push_back(definition);

	// 注册纹理
	uint32_t texIndex = RegisterTexture(definition->GetTexturePath());
	definition->GetGPUData().m_appearance.m_textureIndex = texIndex;

	// 设置UV缩放
	if (texIndex < m_textureUVScales.size())
	{
		Vec2 uvScale = m_textureUVScales[texIndex];
		definition->GetGPUData().m_appearance.m_textureUVScaleX = uvScale.x;
		definition->GetGPUData().m_appearance.m_textureUVScaleY = uvScale.y;
	}

	DebuggerPrintf("Created Definition ID=%u (texIndex=%u)\n",
		m_nextEmitterDefID, texIndex);

	return m_nextEmitterDefID++;
}

Nova2DEmitterDefinition* Nova2DSystem::GetDefinition(uint32_t defID) {
	for (auto* def : m_emitterDefs) {
		if (def->GetID() == defID) {
			return def;
		}
	}
	return nullptr;
}

//==========================================================================
// Instance 管理
//==========================================================================
uint32_t Nova2DSystem::CreateInstance(uint32_t definitionID, float posX, float posY) {
	// 验证Definition存在
	Nova2DEmitterDefinition* def = GetDefinition(definitionID);
	if (!def) {
		ERROR_AND_DIE(Stringf("CreateInstance: Invalid definition ID %u", definitionID));
	}

	// 创建Instance
	auto* instance = new Nova2DEmitterInstance(definitionID);
	instance->SetInstanceID(m_nextEmitterInstanceID);
	instance->SetPosition(posX, posY);
	instance->SetDefinition(def);

	// 分配粒子池范围（简单策略：每个Instance平均分配）
	uint32_t particlesPerInstance = m_config.maxParticles / 256;  // 假设最多256个Instance
	instance->SetParticleRange(m_nextParticlePoolIndex, particlesPerInstance);
	m_nextParticlePoolIndex += particlesPerInstance;

	m_emitterInstances.push_back(instance);

	DebuggerPrintf("Created Instance ID=%u (Def=%u) at (%.1f, %.1f)\n",
		m_nextEmitterInstanceID, definitionID, posX, posY);

	return m_nextEmitterInstanceID++;
}

Nova2DEmitterInstance* Nova2DSystem::GetInstance(uint32_t instanceID) {
	for (auto* inst : m_emitterInstances) {
		if (inst->GetInstanceID() == instanceID) {
			return inst;
		}
	}
	return nullptr;
}

void Nova2DSystem::DestroyInstance(uint32_t instanceID) {
	for (auto it = m_emitterInstances.begin(); it != m_emitterInstances.end(); ++it) {
		if ((*it)->GetInstanceID() == instanceID) {
			delete* it;
			m_emitterInstances.erase(it);
			DebuggerPrintf("Destroyed Instance ID=%u\n", instanceID);
			return;
		}
	}
}

//==========================================================================
// GPU上传
//==========================================================================
void Nova2DSystem::UploadDefinitionsToGPU() {
	if (!m_gpuBuffers) return;

	// 只上传dirty的Definitions
	std::vector<Nova2DEmitterDefinitionGPU> dirtyDefs;
	std::vector<uint32_t> dirtyIndices;

	for (auto* def : m_emitterDefs) {
		if (def->IsDirty()) {
			dirtyDefs.push_back(def->GetGPUDataReadOnly());
			dirtyIndices.push_back(def->GetID());
			def->ClearDirty();
		}
	}

	if (dirtyDefs.empty()) return;

	// 上传（简化版本：每次上传整个数组）
	// TODO: 优化为只上传dirty的元素
	std::vector<Nova2DEmitterDefinitionGPU> allDefs(m_emitterDefs.size());
	for (size_t i = 0; i < m_emitterDefs.size(); ++i) {
		allDefs[i] = m_emitterDefs[i]->GetGPUDataReadOnly();
	}

	m_renderer->CopyDataToStructuredBuffer(
		m_gpuBuffers->GetDefinitionBuffer(),
		allDefs.data(),
		allDefs.size()
	);
}

void Nova2DSystem::UploadInstancesToGPU() {
	if (!m_gpuBuffers || m_emitterInstances.empty()) return;

	// 收集所有Instance的GPU数据
	std::vector<Nova2DEmitterInstanceGPU> instanceData;
	instanceData.reserve(m_emitterInstances.size());

	for (auto* inst : m_emitterInstances) {
		instanceData.push_back(inst->GetGPUDataReadOnly());
	}

	// 上传到GPU
	m_renderer->CopyDataToStructuredBuffer(
		m_gpuBuffers->GetInstanceBuffer(),
		instanceData.data(),
		instanceData.size()
	);
}
