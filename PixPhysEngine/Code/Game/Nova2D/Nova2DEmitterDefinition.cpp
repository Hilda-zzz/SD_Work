// Nova2DEmitterDefinition.cpp
#include "Nova2DEmitterDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cstring>

//==========================================================================
// Constructor
//==========================================================================
Nova2DEmitterDefinition::Nova2DEmitterDefinition() {
    // Zero-initialize GPU data
    memset(&m_gpuData, 0, sizeof(Nova2DEmitterDefinitionGPU));
    
    // ===== 设置合理默认值 =====
    
    // Emission
    m_gpuData.m_emission.m_emissionRate = 10.0f;
    m_gpuData.m_emission.m_numBursts = 1;
    m_gpuData.m_emission.m_lifetimeMin = 1.0f;
    m_gpuData.m_emission.m_lifetimeMax = 2.0f;
    m_gpuData.m_emission.m_emissionType = 0;  // Point
    m_gpuData.m_emission.m_emissionRadius = 5.0f;
    m_gpuData.m_emission.m_boxDimensionX = 5.0f;
    m_gpuData.m_emission.m_boxDimensionY = 5.0f;
    m_gpuData.m_emission.m_emissionMode = 0;  // CONTINUOUS
	m_gpuData.m_emission.m_burstInterval = 1.0f;
	m_gpuData.m_emission.m_burstCycles = 1;
    
    // Motion
    m_gpuData.m_motion.m_velocityMin.x = 0.0f;
    m_gpuData.m_motion.m_velocityMin.y = 50.0f;
    m_gpuData.m_motion.m_velocityMax.x = 0.0f;
    m_gpuData.m_motion.m_velocityMax.y = 150.0f;
    m_gpuData.m_motion.m_velocityMode = 0;
    m_gpuData.m_motion.m_orientToVelocity = 0;
    //m_gpuData.m_motion.m_gravityScale = 1.0f;
    
    // Appearance
    m_gpuData.m_appearance.m_spriteSheetDimensionsX = 1;
    m_gpuData.m_appearance.m_spriteSheetDimensionsY = 1;
    m_gpuData.m_appearance.m_spriteStartIndex = 0;
    m_gpuData.m_appearance.m_spriteEndIndex = 0;
    m_gpuData.m_appearance.m_animationFPS = 12.0f;
    m_gpuData.m_appearance.m_numColorKeyframes = 1;
    
    // Default: 白色恒定
    Rgba8::WHITE.GetAsFloats(m_gpuData.m_appearance.m_colorKeyframes[0].m_colorPacked);
    m_gpuData.m_appearance.m_colorKeyframes[0].m_time = 0.0f;
    
    // Properties
    m_gpuData.m_properties.m_lifetime = -1.0f;  // 无限
    m_gpuData.m_properties.m_startDelay = 0.0f;
    m_gpuData.m_properties.m_worldSimulation = 1;  // World space
    
    // Curves
    m_gpuData.m_numCurves = 0;
}

//==========================================================================
// Destructor
//==========================================================================
Nova2DEmitterDefinition::~Nova2DEmitterDefinition() {
}

//==========================================================================
// Emission API
//==========================================================================
void Nova2DEmitterDefinition::SetEmissionRate(float rate) {
    m_gpuData.m_emission.m_emissionRate = rate;
    MarkDirty();
}

void Nova2DEmitterDefinition::SetLifetimeRange(float min, float max) {
    m_gpuData.m_emission.m_lifetimeMin = min;
    m_gpuData.m_emission.m_lifetimeMax = max;
    MarkDirty();
}

void Nova2DEmitterDefinition::SetEmissionShape(uint32_t type, float param1, float param2) {
    m_gpuData.m_emission.m_emissionType = type;
    
    if (type == 1) {  // Circle
        m_gpuData.m_emission.m_emissionRadius = param1;
    } else if (type == 2) {  // Box
        m_gpuData.m_emission.m_boxDimensionX = param1;
        m_gpuData.m_emission.m_boxDimensionY = param2;
    }
    
    MarkDirty();
}

void Nova2DEmitterDefinition::SetLinearForce(Vec2 const force)
{
    m_gpuData.m_motion.m_linearForce = force;
    MarkDirty();
}

//==========================================================================
// Motion API
//==========================================================================
void Nova2DEmitterDefinition::SetVelocityRange(float minX, float minY, float maxX, float maxY) {
    m_gpuData.m_motion.m_velocityMin.x = minX;
    m_gpuData.m_motion.m_velocityMin.y = minY;
    m_gpuData.m_motion.m_velocityMax.x = maxX;
    m_gpuData.m_motion.m_velocityMax.y = maxY;
    MarkDirty();
}

//==========================================================================
// Color Over Lifetime API
//==========================================================================
void Nova2DEmitterDefinition::ClearColorKeyframes() {
    m_gpuData.m_appearance.m_numColorKeyframes = 0;
    MarkDirty();
}

void Nova2DEmitterDefinition::AddColorKeyframe(Rgba8 color, float time) {
    uint32_t& count = m_gpuData.m_appearance.m_numColorKeyframes;
    
    GUARANTEE_OR_DIE(count < 8, "Cannot add more than 8 color keyframes");
    
    color.GetAsFloats(m_gpuData.m_appearance.m_colorKeyframes[count].m_colorPacked);
    m_gpuData.m_appearance.m_colorKeyframes[count].m_time = time;
    
    count++;
    MarkDirty();
}

//==========================================================================
// Float Curve API
//==========================================================================
void Nova2DEmitterDefinition::AddFloatCurve(N2D_FloatCurveType type, float constantValue) {
    uint32_t& count = m_gpuData.m_numCurves;
    
    GUARANTEE_OR_DIE(count < 4, "Cannot add more than 4 float curves");
    
    N2D_FloatCurve& curve = m_gpuData.m_curves[count];
    curve.m_type = (uint32_t)type;
    curve.m_numKeyframes = 1;
    curve.m_keyframes[0].m_value = constantValue;
    curve.m_keyframes[0].m_time = 0.0f;
    curve.m_keyframes[0].m_easingFunction = 0;  // Linear
    
    count++;
    MarkDirty();
}

void Nova2DEmitterDefinition::AddFloatCurveKeyframe(uint32_t curveIndex, float value, float time) {
    GUARANTEE_OR_DIE(curveIndex < m_gpuData.m_numCurves, "Invalid curve index");
    
    N2D_FloatCurve& curve = m_gpuData.m_curves[curveIndex];
    uint32_t& count = curve.m_numKeyframes;
    
    GUARANTEE_OR_DIE(count < 8, "Cannot add more than 8 keyframes to curve");
    
    curve.m_keyframes[count].m_value = value;
    curve.m_keyframes[count].m_time = time;
    curve.m_keyframes[count].m_easingFunction = 0;
    
    count++;
    MarkDirty();
}

//==========================================================================
// BURST Config API
//==========================================================================
void Nova2DEmitterDefinition::SetBurstConfig(uint32_t numBursts, float interval, int32_t cycles) {
	m_gpuData.m_emission.m_emissionMode = 1;  // 设置为BURST模式
	m_gpuData.m_emission.m_numBursts = numBursts;
	m_gpuData.m_emission.m_burstInterval = interval;
	m_gpuData.m_emission.m_burstCycles = cycles;
	MarkDirty();
}

//==========================================================================
// Sprite Sheet Config API
//==========================================================================
// 静态纹理（1×1，无动画）
void Nova2DEmitterDefinition::SetStaticSprite() {
	m_gpuData.m_appearance.m_spriteSheetDimensionsX = 1;
	m_gpuData.m_appearance.m_spriteSheetDimensionsY = 1;
	m_gpuData.m_appearance.m_spriteStartIndex = 0;
	m_gpuData.m_appearance.m_spriteEndIndex = 0;
	MarkDirty();
}

// Sprite Sheet动画
void Nova2DEmitterDefinition::SetSpriteSheet(uint32_t dimX, uint32_t dimY, 
    uint32_t startIdx, uint32_t endIdx, float fps) {
	m_gpuData.m_appearance.m_spriteSheetDimensionsX = dimX;
	m_gpuData.m_appearance.m_spriteSheetDimensionsY = dimY;
	m_gpuData.m_appearance.m_spriteStartIndex = startIdx;
	m_gpuData.m_appearance.m_spriteEndIndex = endIdx;
    m_gpuData.m_appearance.m_animationFPS = fps;
	MarkDirty();
}

//==========================================================================
// Texture Config
//==========================================================================
void Nova2DEmitterDefinition::SetTexturePath(const char* path)
{
	m_texturePath = path;
	MarkDirty();
}

void Nova2DEmitterDefinition::EnableCollision(bool enable)
{
	m_gpuData.m_collision.m_enableCollision = enable ? 1 : 0;
	MarkDirty();
}

void Nova2DEmitterDefinition::AddCollisionRule(Rgba8 targetColor,
	N2D_CollisionResponse response,
	float bounceDamping,
	float slowFactor,uint32_t maxBounces)
{
	if (m_gpuData.m_collision.m_numRules >= 8) return;  // 已满

	uint32_t idx = m_gpuData.m_collision.m_numRules;
	N2D_CollisionRule& rule = m_gpuData.m_collision.m_rules[idx];

	// 转换为归一化颜色
	rule.m_targetColor[0] = targetColor.r / 255.0f;
	rule.m_targetColor[1] = targetColor.g / 255.0f;
	rule.m_targetColor[2] = targetColor.b / 255.0f;
	rule.m_targetColor[3] = targetColor.a / 255.0f;

	rule.m_response = static_cast<uint32_t>(response);
	rule.m_bounceDamping = bounceDamping;
	rule.m_slowFactor = slowFactor;
    rule.m_maxBounces = maxBounces;

	m_gpuData.m_collision.m_numRules++;
	MarkDirty();
}

void Nova2DEmitterDefinition::ClearCollisionRules()
{
	m_gpuData.m_collision.m_numRules = 0;
	MarkDirty();
}

void Nova2DEmitterDefinition::SetOrientToVelocity(bool enable)
{
	m_gpuData.m_motion.m_orientToVelocity = enable ? 1 : 0;
	MarkDirty();
}

void Nova2DEmitterDefinition::SetPointForce(float radius, float strength, float falloff, bool attract)
{
	m_gpuData.m_motion.m_pointForceRadius = radius;
	m_gpuData.m_motion.m_pointForceStrength = strength;
	m_gpuData.m_motion.m_pointForceFalloff = falloff;
	m_gpuData.m_motion.m_pointForceAttract = attract ? 1 : 0;
	MarkDirty();
}

void Nova2DEmitterDefinition::SetVortexForce(float radius, float strength)
{
	m_gpuData.m_motion.m_vortexRadius = radius;
	m_gpuData.m_motion.m_vortexStrength = strength;
	MarkDirty();
}

void Nova2DEmitterDefinition::EnablePointForce(bool enable)
{
	m_gpuData.m_motion.m_enablePointForce = enable ? 1 : 0;
	MarkDirty();
}

void Nova2DEmitterDefinition::EnableVortex(bool enable)
{
	m_gpuData.m_motion.m_enableVortex = enable ? 1 : 0;
	MarkDirty();
}
