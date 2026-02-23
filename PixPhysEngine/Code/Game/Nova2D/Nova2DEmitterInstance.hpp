// Nova2DEmitterInstance.hpp
// GPU粒子系统 - Emitter Instance（发射器实例）
// 对应Matthew论文的Instance架构
#pragma once
#include "Engine/Math/Vec2.hpp"
#include <cstdint>

class Nova2DEmitterDefinition;
//==========================================================================
// GPU Instance Data（64 bytes，16字节对齐）
//==========================================================================
struct Nova2DEmitterInstanceGPU {
    // ===== Transform (16 bytes) =====
    float m_positionX = 0.0f;
    float m_positionY = 0.0f;
    float m_rotation = 0.0f;              // 弧度（2D只需1个角度）
    float m_scale = 1.0f;                 // 统一缩放
    
    // ===== Particle Ownership (16 bytes) =====
    uint32_t m_definitionIndex = 0;       // 指向哪个Definition
    uint32_t m_particleStartIndex = 0;    // 在粒子池中的起始索引
    uint32_t m_particleCount = 0;         // 拥有的粒子数量
    uint32_t m_maxParticles = 0;          // 最大可拥有粒子数
    
    // ===== Emission State (16 bytes) =====
    float m_emissionAccumulator = 0.0f;   // CONTINUOUS模式累积器
    float m_elapsedTime = 0.0f;           // 发射器存活时间
    uint32_t m_isActive = 0;              // 是否激活（1=active, 0=inactive）
    uint32_t m_killFlag = 0;              // 销毁标记（1=kill all particles）

	// ===== BURST State (8 bytes) =====
	float m_burstTimer = 0.0f;            // 爆发计时器
	int32_t m_burstCycleCount = 0;        // 已执行的爆发次数
    
	Vec2 m_pointForceOffset = Vec2(0.f, 0.f);
	Vec2 m_vortexOffset = Vec2(0.f, 0.f);
    // ===== Padding (16 bytes) =====
    uint32_t m_padding[2];
    
    // Total: 64 bytes
};
static_assert(sizeof(Nova2DEmitterInstanceGPU) == 80, "InstanceGPU must be 64 bytes");
static_assert(sizeof(Nova2DEmitterInstanceGPU) % 16 == 0, "Must be 16-byte aligned");

//==========================================================================
// CPU Instance Manager（CPU端封装类）
//==========================================================================
class Nova2DEmitterInstance {
public:
    explicit Nova2DEmitterInstance(uint32_t definitionID);
    ~Nova2DEmitterInstance();
    
    // ===== ID管理 =====
    uint32_t GetInstanceID() const { return m_instanceID; }
    void SetInstanceID(uint32_t id) { m_instanceID = id; }
    
    uint32_t GetDefinitionID() const { return m_definitionID; }
    
    // ===== Transform =====
    void SetPosition(float x, float y);
    void SetRotation(float radians);
    void SetScale(float scale);
    
    // ===== Particle Ownership =====
    void SetParticleRange(uint32_t startIndex, uint32_t maxParticles);
    uint32_t GetParticleStartIndex() const { return m_gpuData.m_particleStartIndex; }
    uint32_t GetMaxParticles() const { return m_gpuData.m_maxParticles; }

	void SetDefinition(Nova2DEmitterDefinition const* definition) {
		m_definition = definition;
	}
    
    // ===== Playback Control =====
    void Play();
    void Stop();
    void Pause();
    void Resume();
    void Kill();  // 设置killFlag，销毁所有粒子
    
    bool IsPlaying() const { return m_isPlaying; }
    bool IsPaused() const { return m_isPaused; }
    bool IsActive() const { return m_gpuData.m_isActive != 0; }
    
    // ===== GPU Data Access =====
    Nova2DEmitterInstanceGPU& GetGPUData() { return m_gpuData; }
    Nova2DEmitterInstanceGPU const& GetGPUDataReadOnly() const { return m_gpuData; }
    
    // ===== CPU Update（每帧调用，更新状态）=====
    void UpdateCPU(float deltaTime);

	void SetPointForceOffset(Vec2 offset);
	void SetVortexOffset(Vec2 offset);
    
private:
    uint32_t m_instanceID = 0;
    uint32_t m_definitionID = 0;
    Nova2DEmitterDefinition const* m_definition = nullptr;

    // CPU状态（不上传GPU）
    bool m_isPlaying = false;
    bool m_isPaused = false;
    
    // GPU数据
    Nova2DEmitterInstanceGPU m_gpuData;



};
