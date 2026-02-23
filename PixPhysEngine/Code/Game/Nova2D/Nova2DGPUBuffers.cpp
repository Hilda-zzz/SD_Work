// Nova2DGPUBuffers.cpp
#include "Nova2DGPUBuffers.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include "Nova2DEmitterDefinition.hpp"
#include "Nova2DEmitterInstance.hpp"
#include "Engine/Renderer/IndirectArgsBuffer.hpp"

Nova2DGPUBuffers::Nova2DGPUBuffers(Renderer* renderer, int maxParticles)
	: m_renderer(renderer)
	, m_maxParticles(maxParticles)
{
}

Nova2DGPUBuffers::~Nova2DGPUBuffers()
{
	Shutdown();
}

void Nova2DGPUBuffers::Startup()
{
	// ===== 1. 创建粒子Buffer (Read/Write) =====
	// 大小：maxParticles * sizeof(GPUParticle2D)
	// 用途：存储所有粒子数据，Compute Shader 会读写
	m_particleBuffer = m_renderer->CreateStructuredBuffer(
		m_maxParticles,
		sizeof(Nova2DGPUParticle),
		true  // isReadWrite = true (UAV)
	);

	// ===== 2. 创建死亡列表Buffer (Read/Write) =====
	// 大小：maxParticles * sizeof(uint32_t)
	// 用途：存储所有"死亡"粒子的索引，用于快速分配新粒子
	// 初始时，所有索引都在死亡列表中（0, 1, 2, ..., maxParticles-1）
	m_deadListBuffer = m_renderer->CreateStructuredBuffer(
		m_maxParticles,
		sizeof(uint32_t),
		true  // isReadWrite = true (UAV)
	);

	// ===== 3. 创建计数器Buffer (Read/Write) =====
	// 大小：4 * sizeof(uint32_t) = 16 bytes
	// 用途：存储全局计数器（aliveCount, deadCount, emitCount, aliveCountAfterSim ） // 新增
	m_counterBuffer = m_renderer->CreateStructuredBuffer(
		4,  // 4个uint32_t
		sizeof(uint32_t),
		true  // isReadWrite = true (UAV)
	);

	// ===== 4. 创建 AliveList 双缓冲 =====  ← 新增
	m_aliveList1 = m_renderer->CreateStructuredBuffer(
		m_maxParticles,
		sizeof(uint32_t),
		true  // isReadWrite = true (UAV)
	);

	m_aliveList2 = m_renderer->CreateStructuredBuffer(
		m_maxParticles,
		sizeof(uint32_t),
		true  // isReadWrite = true (UAV)
	);

	// ===== 4. 创建发射参数常量缓冲区 (Read Only) =====
	// 用途：CPU传递发射参数给GPU发射Shader
	m_emitParamsBuffer = m_renderer->CreateConstantBuffer(sizeof(Nova2DGPUEmitParams));

	// ===== 5. 创建更新常量缓冲区 (Read Only) =====
	// 用途：CPU传递每帧更新参数（deltaTime, gravity, etc.）给GPU更新Shader
	m_updateConstantsBuffer = m_renderer->CreateConstantBuffer(sizeof(UpdateConstants));

	// ===== 6. 初始化死亡列表 =====
	InitializeDeadList();

	DebuggerPrintf("Nova2D GPU Buffers created: %d max particles\n", m_maxParticles);

	// ===== 新增：创建Definition Buffer =====
	m_definitionBuffer = m_renderer->CreateStructuredBuffer(
		m_maxDefinitions,
		sizeof(Nova2DEmitterDefinitionGPU),
		false  // 只读
	);

	// ===== 新增：创建Instance Buffer =====
	m_instanceBuffer = m_renderer->CreateStructuredBuffer(
		m_maxInstances,
		sizeof(Nova2DEmitterInstanceGPU),
		false  // 只读
	);

	DebuggerPrintf("Created Definition Buffer: %d definitions\n", m_maxDefinitions);
	DebuggerPrintf("Created Instance Buffer: %d instances\n", m_maxInstances);

	// DrawIndexedInstancedIndirect
	m_indirectArgsBuffer = m_renderer->CreateIndirectArgsBuffer(5 * sizeof(uint32_t));

	uint32_t initialArgs[5] = { 0, 1, 0, 0, 0 };
	m_renderer->InitializeIndirectArgsBuffer(
		m_indirectArgsBuffer,
		initialArgs,
		sizeof(initialArgs)
	);

	DebuggerPrintf("Created IndirectArgs Buffer\n");

	// ===== 创建Emit Offsets Buffer =====
	m_emitOffsetsBuffer = m_renderer->CreateStructuredBuffer(
		m_maxInstances,
		sizeof(uint32_t),
		false  // 只读（CPU写，GPU读）
	);

	DebuggerPrintf("Created EmitOffsets Buffer: %d instances\n", m_maxInstances);
}

void Nova2DGPUBuffers::Shutdown()
{
	delete m_particleBuffer;
	delete m_deadListBuffer;
	delete m_counterBuffer;
	delete m_aliveList1;          
	delete m_aliveList2;          
	delete m_emitParamsBuffer;
	delete m_updateConstantsBuffer;

	m_particleBuffer = nullptr;
	m_deadListBuffer = nullptr;
	m_counterBuffer = nullptr;
	m_aliveList1 = nullptr;
	m_aliveList2 = nullptr;
	m_emitParamsBuffer = nullptr;
	m_updateConstantsBuffer = nullptr;

	delete m_definitionBuffer;
	delete m_instanceBuffer;

	m_definitionBuffer = nullptr;
	m_instanceBuffer = nullptr;

	delete m_indirectArgsBuffer;
	m_indirectArgsBuffer = nullptr;

	delete m_emitOffsetsBuffer;
	m_emitOffsetsBuffer = nullptr;

	DebuggerPrintf("Nova2D GPU Buffers destroyed\n");
}

void Nova2DGPUBuffers::InitializeDeadList()
{
	// ===== 初始化：所有粒子索引都在死亡列表中 =====
	std::vector<uint32_t> deadIndices(m_maxParticles);
	for (int i = 0; i < m_maxParticles; ++i) {
		deadIndices[i] = i;
	}

	// 上传到GPU
	m_renderer->CopyDataToStructuredBuffer(m_deadListBuffer, deadIndices.data(), m_maxParticles);

	// ===== 初始化计数器 =====
	Nova2DGPUParticleCounters counters;
	counters.m_aliveCount = 0;               // 初始无存活粒子
	counters.m_deadCount = m_maxParticles;   // 所有粒子都在死亡列表
	counters.m_emitCount = 0;
	counters.m_aliveCountAfterSim = 0;

	DebuggerPrintf("  Setting counters: alive=%d, dead=%d\n",
		counters.m_aliveCount, counters.m_deadCount);

	uint32_t counterArray[4] = { counters.m_aliveCount ,counters.m_deadCount ,counters.m_emitCount,counters.m_aliveCountAfterSim };
	uint32_t counterData[4] = { 0, m_maxParticles, 0, 0 };
	m_renderer->CopyDataToStructuredBuffer(m_counterBuffer, counterArray, 4);

	// ===== 3. 验证 =====
	Nova2DGPUParticleCounters verify = ReadCounters();
	DebuggerPrintf("  Verification read: alive=%d, dead=%d\n",
		verify.m_aliveCount, verify.m_deadCount);

	GUARANTEE_OR_DIE(verify.m_deadCount == m_maxParticles,
		"Counter initialization failed!");

	DebuggerPrintf("=== InitializeDeadList COMPLETE ===\n");
}

Nova2DGPUParticleCounters Nova2DGPUBuffers::ReadCounters() const
{
	Nova2DGPUParticleCounters counters;
	uint32_t counterData[4];
	m_renderer->ReadStructuredBuffer(m_counterBuffer, counterData, 4);
	counters.m_aliveCount = counterData[0];
	counters.m_deadCount = counterData[1];
	counters.m_emitCount = counterData[2];
	counters.m_aliveCountAfterSim = counterData[3];
	return counters;
}

void Nova2DGPUBuffers::ReadParticles(Nova2DGPUParticle* outParticles, int count) const
{
	m_renderer->ReadStructuredBuffer(m_particleBuffer, outParticles, count);
}