#include "Nova2DEmitter.hpp"
#include "Nova2DSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

//==========================================================================
// Constructor
//==========================================================================
Nova2DEmitter::Nova2DEmitter() {
	static int s_emitterCount = 0;
	int emitterID = s_emitterCount++;

	m_noiseSeed = 123456789u + emitterID * 987654u;
	m_noiseIndex = emitterID * 1000;
}

Nova2DEmitter::Nova2DEmitter(Nova2DEmitterConfig const& config)
	: m_config(config) {
	static int s_emitterCount = 0;
	static RandomNumberGenerator s_seedGenerator;

	m_noiseSeed = s_seedGenerator.RollRandomIntInRange(1, INT_MAX);
	m_noiseIndex = s_emitterCount++ * 1000;
}

Nova2DEmitter::~Nova2DEmitter() {
}

//==========================================================================
// 生命周期管理
//==========================================================================
void Nova2DEmitter::Update(float deltaTime, Nova2DSystem* particleSystem) 
{
	if (!m_isPlaying || m_isPaused || !particleSystem) return;

	// 更新总时间
	m_elapsedTime += deltaTime;

	// 处理 Start Delay
	if (m_config.m_propertiesConfig.m_startDelay > 0.0f)
	{
		m_delayTimer += deltaTime;
		if (m_delayTimer < m_config.m_propertiesConfig.m_startDelay) 
		{
			return;  // 还在延迟中，不发射
		}
	}

	// 检查发射器生命周期
	if (m_config.m_propertiesConfig.m_hasLifetime) 
	{
		if (m_elapsedTime > m_config.m_propertiesConfig.m_emitterLifetime) 
		{
			Stop();
			return;
		}
	}

	// 更新动画时间
	if (m_config.m_animation) 
	{
		m_animTime += deltaTime;
	}

	// 根据发射模式更新
	switch (m_config.m_emissionConfig.m_mode) 
	{
	case n2d_EmissionMode::CONTINUOUS:
		UpdateContinuous(deltaTime, particleSystem);
		break;

	case n2d_EmissionMode::BURST:
		UpdateBurst(deltaTime, particleSystem);
		break;
	}
}

void Nova2DEmitter::UpdateContinuous(float deltaTime, Nova2DSystem* particleSystem) 
{
	// 累积发射时间
	m_emissionAccumulator += deltaTime * m_config.m_emissionConfig.m_emissionRate;

	// 计算本帧应发射的粒子数
	int particlesToEmit = (int)m_emissionAccumulator;

	if (particlesToEmit > 0) 
	{
		m_emissionAccumulator -= (float)particlesToEmit;

		// 发射粒子
		for (int i = 0; i < particlesToEmit; ++i) 
		{
			EmitSingleParticle(particleSystem);
		}
	}
}

void Nova2DEmitter::UpdateBurst(float deltaTime, Nova2DSystem* particleSystem) 
{
	if (!m_hasEmittedBurst) 
	{
		// 一次性发射所有粒子
		EmitBurst(m_config.m_emissionConfig.m_burstCount, particleSystem);
		m_hasEmittedBurst = true;

		// Burst 模式发射后立即停止（除非有重复设置）
		if (!m_config.m_propertiesConfig.m_hasLifetime) 
		{
			Stop();
		}
	}
}

void Nova2DEmitter::EmitSingleParticle(Nova2DSystem* particleSystem) 
{
	// ✅ 关键：每发射一个粒子，递增索引
		// 这样每个粒子的所有随机属性都基于唯一的索引
	int particleNoiseIndex = m_noiseIndex++;

	// 临时修改 m_noiseIndex 用于生成属性
	int savedIndex = m_noiseIndex;
	m_noiseIndex = particleNoiseIndex;

	// 生成粒子属性
	Vec2 pos = m_config.m_position + GenerateEmitPosition();
	Vec2 vel = GenerateVelocity(pos);
	Rgba8 color = GenerateColor();
	float lifetime = GenerateLifetime();
	float size = GenerateSize();
	float rotation = GenerateRotation();

	// 恢复索引
	m_noiseIndex = savedIndex;

	// 创建粒子结构
	Nova2DParticle particle;
	particle.m_position = pos;
	particle.m_velocity = vel;
	particle.m_color = color;
	particle.m_lifetime = lifetime;
	particle.m_maxLifetime = lifetime;
	particle.m_size = size;
	particle.m_rotation = rotation;

	// ===== 设置标志位 =====
	particle.m_flags = 0;
	if (m_config.m_enableGravity) {
		particle.SetFlag(Nova2DParticleFlags::NOVA_FLAG_GRAVITY,true);
	}

	// ===== 用户类型标识（可选）=====
	//particle.m_userInt = (uint32_t)GameParticleType::SPARK;  // 游戏层定义

	// 设置纹理/动画
	if (m_config.m_animation) 
	{
		particle.m_anim = m_config.m_animation;
		particle.m_animStartTime = m_animTime;
	}
	else if (m_config.m_sprite) 
	{
		particle.m_sprite = m_config.m_sprite;
	}

	// 发射粒子
	particleSystem->EmitParticleStruct(particle);
}

void Nova2DEmitter::EmitBurst(int count, Nova2DSystem* particleSystem) 
{
	for (int i = 0; i < count; ++i) 
	{
		EmitSingleParticle(particleSystem);
	}
}

//==========================================================================
// 粒子属性生成器
//==========================================================================
Vec2 Nova2DEmitter::GenerateEmitPosition() const 
{
	// 注意：需要使用可变索引，所以参数改为引用
   // 但为了保持接口简洁，我们用局部变量
	int localIndex = m_noiseIndex;

	switch (m_config.m_emissionConfig.m_shape) {
	case n2d_EmitterShape::POINT:
		return Vec2::ZERO;

	case n2d_EmitterShape::CIRCLE: {
		float angle = GetRandomAngleDegrees(localIndex++, m_noiseSeed);
		float radius = GetRandomInRange(localIndex++, m_noiseSeed,
			0.0f, m_config.m_emissionConfig.m_shapeRadius);
		return Vec2::MakeFromPolarDegrees(angle, radius);
	}

	case n2d_EmitterShape::BOX: {
		float halfWidth = m_config.m_emissionConfig.m_shapeBoxSize.x * 0.5f;
		float halfHeight = m_config.m_emissionConfig.m_shapeBoxSize.y * 0.5f;
		float x = GetRandomInRange(localIndex++, m_noiseSeed, -halfWidth, halfWidth);
		float y = GetRandomInRange(localIndex++, m_noiseSeed, -halfHeight, halfHeight);
		return Vec2(x, y);
	}

	default:
		return Vec2::ZERO;
	}
}

Vec2 Nova2DEmitter::GenerateVelocity(Vec2 const& emitPos) const 
{
	int localIndex = m_noiseIndex + 10;  // 偏移避免重复

	Vec2 velocity = GetRandomVec2InRange(
		localIndex,  // 注意：这里会递增 2 次
		m_noiseSeed,
		m_config.m_motionConfig.m_startVelocityMin,
		m_config.m_motionConfig.m_startVelocityMax
	);

	// 限制最大速度
	if (m_config.m_motionConfig.m_maxSpeed > 0.0f) 
	{
		float speed = velocity.GetLength();
		if (speed > m_config.m_motionConfig.m_maxSpeed) 
		{
			velocity = velocity.GetNormalized() * m_config.m_motionConfig.m_maxSpeed;
		}
	}

	return velocity;
}

Rgba8 Nova2DEmitter::GenerateColor() const
{
	// MVP: 直接返回配置颜色
	return m_config.m_appearanceConfig.m_colorStart;

	// [进阶] 颜色随机化示例：
	// int localIndex = m_noiseIndex + 20;
	// if (m_config.appearance.randomizeHue) {
	//     float hueShift = GetRandomInRange(localIndex, m_noiseSeed, 
	//                                       -30.0f, 30.0f);
	//     return ShiftHue(m_config.appearance.colorStart, hueShift);
	// }
}

float Nova2DEmitter::GenerateLifetime() const 
{
	int localIndex = m_noiseIndex + 30;
	return GetRandomInRange(localIndex, m_noiseSeed,
		m_config.m_emissionConfig.m_lifetimeMin,
		m_config.m_emissionConfig.m_lifetimeMax);
}

float Nova2DEmitter::GenerateSize() const 
{
	int localIndex = m_noiseIndex + 40;
	return GetRandomInRange(localIndex, m_noiseSeed,
		m_config.m_appearanceConfig.m_sizeMin,
		m_config.m_appearanceConfig.m_sizeMax);
}

float Nova2DEmitter::GenerateRotation() const 
{
	// MVP: 返回固定值
	return 0.0f;

	// [进阶] 如果启用旋转
	// if (m_config.motion.enableRotation) {
	//     return m_rng.RollRandomFloatInRange(0.0f, 360.0f);
	// }
}

//==========================================================================
// 播放控制
//==========================================================================
void Nova2DEmitter::Play() 
{
	m_isPlaying = true;
	m_isPaused = false;
}

void Nova2DEmitter::Stop() 
{
	m_isPlaying = false;
	m_isPaused = false;
	m_hasEmittedBurst = false;
}

void Nova2DEmitter::Pause() 
{
	m_isPaused = true;
}

void Nova2DEmitter::Resume() 
{
	m_isPaused = false;
}

void Nova2DEmitter::Reset() 
{
	m_elapsedTime = 0.0f;
	m_emissionAccumulator = 0.0f;
	m_delayTimer = 0.0f;
	m_animTime = 0.0f;
	m_hasEmittedBurst = false;
}

bool Nova2DEmitter::IsFinished() const 
{
	// Burst 模式且已发射
	if (m_config.m_emissionConfig.m_mode == n2d_EmissionMode::BURST && m_hasEmittedBurst) {
		return true;
	}

	// 有生命周期且已过期
	if (m_config.m_propertiesConfig.m_hasLifetime &&
		m_elapsedTime >= m_config.m_propertiesConfig.m_emitterLifetime) {
		return true;
	}

	return false;
}

//==========================================================================
// 配置接口
//==========================================================================
void Nova2DEmitter::SetConfig(Nova2DEmitterConfig const& config) {
	m_config = config;
}

void Nova2DEmitter::SetPosition(Vec2 position) {
	m_config.m_position = position;
}

void Nova2DEmitter::SetEmissionRate(float rate) {
	m_config.m_emissionConfig.m_emissionRate = rate;
}

void Nova2DEmitter::SetEmissionMode(n2d_EmissionMode mode) {
	m_config.m_emissionConfig.m_mode = mode;
}

void Nova2DEmitter::SetBurstCount(int count) {
	m_config.m_emissionConfig.m_burstCount = count;
}

void Nova2DEmitter::SetVelocityRange(Vec2 min, Vec2 max) {
	m_config.m_motionConfig.m_startVelocityMin = min;
	m_config.m_motionConfig.m_startVelocityMax = max;
}

void Nova2DEmitter::SetLifetimeRange(float min, float max) {
	m_config.m_emissionConfig.m_lifetimeMin = min;
	m_config.m_emissionConfig.m_lifetimeMax = max;
}

void Nova2DEmitter::SetSizeRange(float min, float max) {
	m_config.m_appearanceConfig.m_sizeMin = min;
	m_config.m_appearanceConfig.m_sizeMax = max;
}

void Nova2DEmitter::SetStartColor(Rgba8 color) {
	m_config.m_appearanceConfig.m_colorStart = color;
}

void Nova2DEmitter::SetShape(n2d_EmitterShape shape, float param) {
	m_config.m_emissionConfig.m_shape = shape;

	switch (shape) {
	case n2d_EmitterShape::CIRCLE:
		m_config.m_emissionConfig.m_shapeRadius = param;
		break;

	case n2d_EmitterShape::BOX:
		m_config.m_emissionConfig.m_shapeBoxSize = Vec2(param, param);
		break;

	default:
		break;
	}
}

//==========================================================================
// 纹理/动画设置
//==========================================================================
void Nova2DEmitter::SetSprite(SpriteSheet const* sprite) {
	m_config.m_sprite = sprite;
	m_config.m_animation = nullptr;
}

void Nova2DEmitter::SetAnimation(SpriteAnimDefinition const* anim) {
	m_config.m_animation = anim;
	m_config.m_sprite = nullptr;
	m_animTime = 0.0f;
}