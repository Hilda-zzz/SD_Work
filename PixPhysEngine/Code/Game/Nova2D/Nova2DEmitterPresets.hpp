// Nova2DEmitterPresets.hpp
// 预设发射器定义（对应Matthew的XML文件）
#pragma once

#include "Nova2DEmitterDefinition.hpp"
#include "Engine/Core/Rgba8.hpp"

namespace Nova2DEmitterPresets {

//==========================================================================
// 辅助函数：Rgba8转packed uint32_t
//==========================================================================
inline uint32_t PackColor(Rgba8 const& color) {
    return (color.r << 0) | (color.g << 8) | (color.b << 16) | (color.a << 24);
}

//==========================================================================
// 火焰效果
//==========================================================================
inline Nova2DEmitterDefinition* CreateFireDefinition() {
    auto* def = new Nova2DEmitterDefinition();
    
    // Emission
    def->SetEmissionRate(50.0f);
    def->SetLifetimeRange(0.5f, 1.5f);
    def->SetEmissionShape(1, 10.0f);  // Circle, radius=10
    def->GetGPUData().m_emission.m_emissionMode = 0;  // CONTINUOUS
    
    // Motion
    def->SetVelocityRange(0.0f, 100.0f, 0.0f, 200.0f);
    def->GetGPUData().m_motion.m_gravityScale = 0.0f;  // 火焰向上，无重力
    
    // Appearance - Color Over Lifetime (黄→橙→红透明)
    def->ClearColorKeyframes();
    def->AddColorKeyframe(PackColor(Rgba8(255, 200, 50, 255)), 0.0f);   // 黄色
    def->AddColorKeyframe(PackColor(Rgba8(255, 100, 0, 255)), 0.5f);    // 橙色
    def->AddColorKeyframe(PackColor(Rgba8(200, 50, 0, 0)), 1.0f);       // 红色透明
    
    // Float Curves
    def->AddFloatCurve(N2D_FloatCurveType::SIZE, 5.0f);
    
    return def;
}

//==========================================================================
// 烟雾效果
//==========================================================================
inline Nova2DEmitterDefinition* CreateSmokeDefinition() {
    auto* def = new Nova2DEmitterDefinition();
    
    def->SetEmissionRate(20.0f);
    def->SetLifetimeRange(1.0f, 3.0f);
    def->SetEmissionShape(1, 15.0f);  // Circle, radius=15
    
    def->SetVelocityRange(0.0f, 30.0f, 0.0f, 80.0f);
    def->GetGPUData().m_motion.m_gravityScale = 0.0f;
    
    // 灰色半透明
    def->ClearColorKeyframes();
    def->AddColorKeyframe(PackColor(Rgba8(100, 100, 100, 150)), 0.0f);
    def->AddColorKeyframe(PackColor(Rgba8(80, 80, 80, 0)), 1.0f);  // 淡出
    
    def->AddFloatCurve(N2D_FloatCurveType::SIZE, 10.0f);
    
    return def;
}

//==========================================================================
// 爆炸效果（对应ExplosionCore.xml）
//==========================================================================
inline Nova2DEmitterDefinition* CreateExplosionDefinition() {
    auto* def = new Nova2DEmitterDefinition();
    
    // Emission - Burst模式
    def->GetGPUData().m_emission.m_emissionMode = 1;  // BURST
    def->GetGPUData().m_emission.m_numBursts = 200;
    def->SetLifetimeRange(0.9f, 1.0f);
    def->SetEmissionShape(2, 5.0f, 5.0f);  // Box 5x5
    
    // Motion - 全方向爆炸
    def->SetVelocityRange(-200.0f, -200.0f, 200.0f, 200.0f);
    def->GetGPUData().m_motion.m_gravityScale = 0.5f;
    
    // Color Over Lifetime (白→黄→橙→红透明)
    def->ClearColorKeyframes();
    def->AddColorKeyframe(PackColor(Rgba8(255, 255, 255, 255)), 0.0f);   // 白色
    def->AddColorKeyframe(PackColor(Rgba8(244, 217, 91, 255)), 0.2f);    // 黄色
    def->AddColorKeyframe(PackColor(Rgba8(235, 78, 23, 255)), 0.6f);     // 橙色
    def->AddColorKeyframe(PackColor(Rgba8(234, 76, 20, 0)), 1.0f);       // 红透明
    
    // Size Over Lifetime (爆炸扩张后缩小)
    def->AddFloatCurve(N2D_FloatCurveType::SIZE, 20.0f);
    def->AddFloatCurveKeyframe(0, 4.0f, 0.2f);
    def->AddFloatCurveKeyframe(0, 2.0f, 0.7f);
    def->AddFloatCurveKeyframe(0, 0.0f, 1.0f);
    
    return def;
}

//==========================================================================
// 电火花（对应ChargeSparks.xml）
//==========================================================================
inline Nova2DEmitterDefinition* CreateSparkDefinition() {
    auto* def = new Nova2DEmitterDefinition();
    
    // Emission - 高速率
    def->SetEmissionRate(300.0f);
    def->SetLifetimeRange(0.3f, 0.5f);
    def->SetEmissionShape(1, 6.0f);  // Circle, radius=6
    def->GetGPUData().m_emission.m_emissionMode = 0;  // CONTINUOUS
    
    // Motion - 朝向速度方向
    def->SetVelocityRange(-50.0f, -50.0f, 50.0f, 50.0f);
    def->GetGPUData().m_motion.m_orientToVelocity = 1;
    def->GetGPUData().m_motion.m_gravityScale = 1.0f;
    
    // Color - 蓝色电光（对应ChargeSparks的155,244,255）
    def->ClearColorKeyframes();
    def->AddColorKeyframe(PackColor(Rgba8(155, 244, 255, 255)), 0.0f);
    def->AddColorKeyframe(PackColor(Rgba8(155, 243, 255, 255)), 0.85f);
    def->AddColorKeyframe(PackColor(Rgba8(155, 243, 255, 0)), 1.0f);  // 淡出
    
    // Size
    def->AddFloatCurve(N2D_FloatCurveType::SIZE, 2.0f);
    
    return def;
}

//==========================================================================
// 血液飞溅（2D游戏常用）
//==========================================================================
inline Nova2DEmitterDefinition* CreateBloodDefinition() {
    auto* def = new Nova2DEmitterDefinition();
    
    def->GetGPUData().m_emission.m_emissionMode = 1;  // BURST
    def->GetGPUData().m_emission.m_numBursts = 50;
    def->SetLifetimeRange(0.3f, 0.8f);
    def->SetEmissionShape(1, 5.0f);
    
    def->SetVelocityRange(-150.0f, 50.0f, 150.0f, 250.0f);
    def->GetGPUData().m_motion.m_gravityScale = 1.5f;  // 重力强
    
    // 深红色
    def->ClearColorKeyframes();
    def->AddColorKeyframe(PackColor(Rgba8(200, 0, 0, 255)), 0.0f);
    def->AddColorKeyframe(PackColor(Rgba8(150, 0, 0, 0)), 1.0f);
    
    def->AddFloatCurve(N2D_FloatCurveType::SIZE, 3.0f);
    
    return def;
}

//==========================================================================
// 测试粒子（简单白色）
//==========================================================================
inline Nova2DEmitterDefinition* CreateTestDefinition() {
    auto* def = new Nova2DEmitterDefinition();
    
    def->SetEmissionRate(10.0f);
    def->SetLifetimeRange(2.0f, 3.0f);
    def->SetEmissionShape(0, 0.0f);  // Point
    
    def->SetVelocityRange(0.0f, 50.0f, 0.0f, 100.0f);
    def->GetGPUData().m_motion.m_gravityScale = 1.0f;
    
    // 纯白色
    def->ClearColorKeyframes();
    def->AddColorKeyframe(PackColor(Rgba8::WHITE), 0.0f);
    
    def->AddFloatCurve(N2D_FloatCurveType::SIZE, 5.0f);
    
    return def;
}

} // namespace Nova2DEmitterPresets
