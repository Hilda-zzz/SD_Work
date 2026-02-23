// Nova2DGPUBuffers.hpp
// GPU Buffer 管理器 - 封装所有GPU资源
#pragma once
#include "Nova2DGPUParticle.hpp"

class Renderer;
class StructuredBuffer;
class ConstantBuffer;
class IndirectArgsBuffer;

//==========================================================================
// GPU粒子Buffer管理器
// 职责：
//   - 创建和管理所有GPU Buffers
//   - 提供统一的Buffer访问接口
//   - 处理Buffer的生命周期
//==========================================================================

struct UpdateConstants
{
	float deltaTime;        // 0: 4 bytes
	float gravity;          // 4: 4 bytes
	float currentTime;      // 8: 4 bytes
	uint32_t maxParticles;  // 12: 4 bytes ← 移到这里

	Vec2 viewportMin;       // 16: 8 bytes ← 16字节对齐
	Vec2 viewportMax;       // 24: 8 bytes
	Vec2 textureSize;       // 32: 8 bytes
	Vec2 padding;           // 40: 8 bytes

	// Total: 48 bytes
};

class Nova2DGPUBuffers
{
public:
	Nova2DGPUBuffers(Renderer* renderer, int maxParticles);
	~Nova2DGPUBuffers();

	// ===== 生命周期 =====
	void Startup();
	void Shutdown();

	// ===== 初始化死亡列表（所有粒子初始都是"死亡"状态）=====
	void InitializeDeadList();

	// ===== Buffer访问 =====
	StructuredBuffer* GetParticleBuffer() const { return m_particleBuffer; }
	StructuredBuffer* GetDeadListBuffer() const { return m_deadListBuffer; }
	StructuredBuffer* GetCounterBuffer() const { return m_counterBuffer; }
	ConstantBuffer* GetEmitParamsBuffer() const { return m_emitParamsBuffer; }
	ConstantBuffer* GetUpdateConstantsBuffer() const { return m_updateConstantsBuffer; }

	// ← 新增：AliveList ping-pong 访问
	StructuredBuffer* GetAliveList1() const { return m_aliveList1; }
	StructuredBuffer* GetAliveList2() const { return m_aliveList2; }

	// ===== 调试：回读GPU数据到CPU =====
	Nova2DGPUParticleCounters ReadCounters() const;
	void ReadParticles(Nova2DGPUParticle* outParticles, int count) const;

	// ===== 新增：Definition/Instance访问 =====
	StructuredBuffer* GetDefinitionBuffer() const { return m_definitionBuffer; }
	StructuredBuffer* GetInstanceBuffer() const { return m_instanceBuffer; }

	IndirectArgsBuffer* GetIndirectArgsBuffer() const { return m_indirectArgsBuffer; }

	StructuredBuffer* GetEmitOffsetsBuffer() const { return m_emitOffsetsBuffer; }

private:
	Renderer* m_renderer = nullptr;
	int m_maxParticles = 0;

	// ===== GPU Buffers =====
	StructuredBuffer* m_particleBuffer = nullptr;       // 粒子数据 (RW)
	StructuredBuffer* m_deadListBuffer = nullptr;       // 死亡粒子索引列表 (RW)
	StructuredBuffer* m_counterBuffer = nullptr;        // 计数器 (aliveCount, deadCount, etc.)

	// Ping Pang AliveList
	StructuredBuffer* m_aliveList1 = nullptr;           // AliveList A (RW)
	StructuredBuffer* m_aliveList2 = nullptr;           // AliveList B (RW)

	ConstantBuffer* m_emitParamsBuffer = nullptr;       // 发射参数
	ConstantBuffer* m_updateConstantsBuffer = nullptr;  // 更新参数（deltaTime, gravity, etc.）

	// ===== def and instance =====
	int m_maxDefinitions = 64;
	int m_maxInstances = 256;

	StructuredBuffer* m_definitionBuffer = nullptr;
	StructuredBuffer* m_instanceBuffer = nullptr;

	// ===== Indirect Draw Buffer =====
	IndirectArgsBuffer* m_indirectArgsBuffer = nullptr;

	// ===== Emit Offsets Buffer =====
	StructuredBuffer* m_emitOffsetsBuffer = nullptr;  // 存储每个Instance的发射起始索引
};