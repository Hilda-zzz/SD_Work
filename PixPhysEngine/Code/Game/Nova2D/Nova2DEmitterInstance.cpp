// Nova2DEmitterInstance.cpp
#include "Nova2DEmitterInstance.hpp"
#include <cstring>
#include "Nova2DEmitterDefinition.hpp"

//==========================================================================
// Constructor
//==========================================================================
Nova2DEmitterInstance::Nova2DEmitterInstance(uint32_t definitionID)
    : m_definitionID(definitionID)
{
    // Zero-initialize GPU data
    memset(&m_gpuData, 0, sizeof(Nova2DEmitterInstanceGPU));
    
    // 设置默认值
    m_gpuData.m_definitionIndex = definitionID;
    m_gpuData.m_positionX = 0.0f;
    m_gpuData.m_positionY = 0.0f;
    m_gpuData.m_rotation = 0.0f;
    m_gpuData.m_scale = 1.0f;
    
    m_gpuData.m_particleStartIndex = 0;
    m_gpuData.m_particleCount = 0;
    m_gpuData.m_maxParticles = 0;
    
    m_gpuData.m_emissionAccumulator = 0.0f;
    m_gpuData.m_elapsedTime = 0.0f;
    m_gpuData.m_isActive = 0;  // 默认未激活
    m_gpuData.m_killFlag = 0;
    
    // CPU状态
    m_isPlaying = false;
    m_isPaused = false;
}

//==========================================================================
// Destructor
//==========================================================================
Nova2DEmitterInstance::~Nova2DEmitterInstance()
{
}

//==========================================================================
// Transform API
//==========================================================================
void Nova2DEmitterInstance::SetPosition(float x, float y)
{
    m_gpuData.m_positionX = x;
    m_gpuData.m_positionY = y;
}

void Nova2DEmitterInstance::SetRotation(float radians)
{
    m_gpuData.m_rotation = radians;
}

void Nova2DEmitterInstance::SetScale(float scale)
{
    m_gpuData.m_scale = scale;
}

//==========================================================================
// Particle Ownership API
//==========================================================================
void Nova2DEmitterInstance::SetParticleRange(uint32_t startIndex, uint32_t maxParticles)
{
    m_gpuData.m_particleStartIndex = startIndex;
    m_gpuData.m_maxParticles = maxParticles;
    m_gpuData.m_particleCount = 0;  // 初始没有粒子
}

//==========================================================================
// Playback Control
//==========================================================================
void Nova2DEmitterInstance::Play()
{
    m_isPlaying = true;
    m_isPaused = false;
    m_gpuData.m_isActive = 1;

	// 重置BURST状态
	m_gpuData.m_burstTimer = 0.0f;
	m_gpuData.m_burstCycleCount = 0;
}

void Nova2DEmitterInstance::Stop()
{
    m_isPlaying = false;
    m_isPaused = false;
    m_gpuData.m_isActive = 0;
    
    // 重置状态
    m_gpuData.m_emissionAccumulator = 0.0f;
    m_gpuData.m_elapsedTime = 0.0f;
}

void Nova2DEmitterInstance::Pause()
{
    m_isPaused = true;
}

void Nova2DEmitterInstance::Resume()
{
    if (m_isPlaying) {
        m_isPaused = false;
    }
}

void Nova2DEmitterInstance::Kill()
{
    m_gpuData.m_killFlag = 1;  // GPU会读取这个标志并销毁所有粒子
    Stop();
}

//==========================================================================
// CPU Update（每帧调用）
//==========================================================================
void Nova2DEmitterInstance::UpdateCPU(float deltaTime)
{
	// 如果未播放或暂停，不更新
	if (!m_isPlaying || m_isPaused) {
		return;
	}

	// ===== 检查Definition =====
	if (!m_definition) {
		return;  // 没有Definition，无法更新
	}

	// ===== 1. 清零上一帧整数部分 =====
	float& acc = m_gpuData.m_emissionAccumulator;
	int emitted = (int)acc;
	acc -= (float)emitted;

	// ===== 1. 更新时间 =====
	m_gpuData.m_elapsedTime += deltaTime;

	// ===== 2. 读取Definition的最新参数 =====
	Nova2DEmitterDefinitionGPU const& def = m_definition->GetGPUDataReadOnly();

	// ===== 3. 累积 emissionAccumulator =====
	if (def.m_emission.m_emissionMode == 0)  // CONTINUOUS 模式
	{
		// 每次都读取最新的emissionRate ← 动态响应Definition变化
		m_gpuData.m_emissionAccumulator += deltaTime * def.m_emission.m_emissionRate;
	}
	else if (def.m_emission.m_emissionMode == 1)  // BURST 模式
	{
		// 检查是否还有剩余爆发次数（-1=无限）
		bool hasRemainingCycles = (def.m_emission.m_burstCycles == -1) ||
			(m_gpuData.m_burstCycleCount < def.m_emission.m_burstCycles);

		if (hasRemainingCycles)
		{
			m_gpuData.m_burstTimer += deltaTime;

			// 到达爆发间隔时间
			if (m_gpuData.m_burstTimer >= def.m_emission.m_burstInterval)
			{
				// 触发爆发
				m_gpuData.m_emissionAccumulator = (float)def.m_emission.m_numBursts;

				// 重置计时器（支持连续爆发）
				m_gpuData.m_burstTimer -= def.m_emission.m_burstInterval;

				// 增加已执行次数
				m_gpuData.m_burstCycleCount++;

				// 如果达到指定次数，停止（无限循环除外）
				if (def.m_emission.m_burstCycles != -1 &&
					m_gpuData.m_burstCycleCount >= def.m_emission.m_burstCycles)
				{
					// 完成所有爆发，可选择自动停止
					// Stop();  // 取消注释以自动停止
				}
			}
		}
	}
}

void Nova2DEmitterInstance::SetPointForceOffset(Vec2 offset)
{
	m_gpuData.m_pointForceOffset = offset;
}

void Nova2DEmitterInstance::SetVortexOffset(Vec2 offset)
{
	m_gpuData.m_vortexOffset = offset;
}
