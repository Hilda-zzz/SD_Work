#pragma once
#include "Nova2DEmitterConfig.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

class Nova2DSystem;
class SpriteDefinition;
class SpriteAnimDefinition;

//==========================================================================
// Particle Emitter
// 负责根据配置生成粒子，但不存储粒子本身（存储在 Nova2DSystem 中）
//==========================================================================
class Nova2DEmitter 
{
public:
	Nova2DEmitter();
	explicit Nova2DEmitter(Nova2DEmitterConfig const& config);
	~Nova2DEmitter();

	//----------------------------------------------------------------------
	// 生命周期管理
	//----------------------------------------------------------------------
	void Update(float deltaTime, Nova2DSystem* particleSystem);
	void Play();
	void Stop();
	void Pause();
	void Resume();
	void Reset();

	//----------------------------------------------------------------------
	// 配置访问
	//----------------------------------------------------------------------
	void SetConfig(Nova2DEmitterConfig const& config);
	Nova2DEmitterConfig const& GetConfig() const { return m_config; }
	Nova2DEmitterConfig& GetConfigMutable() { return m_config; }

	//----------------------------------------------------------------------
	// Set
	//----------------------------------------------------------------------
	void SetPosition(Vec2 position);
	void SetEmissionRate(float rate);
	void SetEmissionMode(n2d_EmissionMode mode);
	void SetBurstCount(int count);
	void SetVelocityRange(Vec2 min, Vec2 max);
	void SetLifetimeRange(float min, float max);
	void SetSizeRange(float min, float max);
	void SetStartColor(Rgba8 color);
	void SetShape(n2d_EmitterShape shape, float param = 0.0f);

	//----------------------------------------------------------------------
	// 纹理/动画设置
	//----------------------------------------------------------------------
	void SetSprite(SpriteDefinition const* sprite);
	void SetAnimation(SpriteAnimDefinition const* anim);

	//----------------------------------------------------------------------
	// 查询
	//----------------------------------------------------------------------
	bool IsPlaying() const { return m_isPlaying; }
	bool IsPaused() const { return m_isPaused; }
	bool IsFinished() const;  // Burst 模式或有限生命周期的发射器
	float GetElapsedTime() const { return m_elapsedTime; }
	float GetEmissionAccumulator() const { return m_emissionAccumulator; }

	//----------------------------------------------------------------------
	// 手动发射（用于特殊情况）
	//----------------------------------------------------------------------
	void EmitBurst(int count, Nova2DSystem* particleSystem);

private:
	//----------------------------------------------------------------------
	// 内部发射逻辑
	//----------------------------------------------------------------------
	void UpdateContinuous(float deltaTime, Nova2DSystem* particleSystem);
	void UpdateBurst(float deltaTime, Nova2DSystem* particleSystem);
	void EmitSingleParticle(Nova2DSystem* particleSystem);

	//----------------------------------------------------------------------
	// 粒子属性生成器（对应论文的各种模块）
	//----------------------------------------------------------------------
	Vec2 GenerateEmitPosition() const;      // Emission Shape
	Vec2 GenerateVelocity(Vec2 const& emitPos) const;  // Motion Module
	Rgba8 GenerateColor() const;            // Appearance Module
	float GenerateLifetime() const;         // Emission Module
	float GenerateSize() const;             // Appearance Module
	float GenerateRotation() const;         // Motion Module (rotation)

private:
	Nova2DEmitterConfig m_config;

	// State
	bool m_isPlaying = false;
	bool m_isPaused = false;
	bool m_hasEmittedBurst = false;  // Burst 模式是否已发射

	// Time Track
	float m_elapsedTime = 0.0f;              // 发射器存活时间
	float m_emissionAccumulator = 0.0f;      // Continuous 模式的累积时间
	float m_delayTimer = 0.0f;               // Start Delay 倒计时

	// Anim
	float m_animTime = 0.0f;
};