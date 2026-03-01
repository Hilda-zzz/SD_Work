#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

class Texture;

// ========================================
// 视差层配置
// ========================================
struct ParallaxLayerConfig
{
    Texture* texture = nullptr;         // 纹理资源
    float scrollSpeedX = 1.0f;          // 横向滚动速度因子 (1.0 = 与相机同速)
    float scrollSpeedY = 1.0f;          // 纵向滚动速度因子 (通常设为 1.0 以完全跟随)
    float depthFactor = 1.0f;           // 深度因子(越小越远，视差效果越明显)
    Vec2 textureScale = Vec2(1.0f, 1.0f); // 纹理缩放
    Rgba8 tint = Rgba8::WHITE;          // 色调
    float alpha = 1.0f;                 // 透明度
    bool isLoopingX = true;             // 是否在X轴循环
    bool isLoopingY = false;            // 是否在Y轴循环 (洞穴场景通常不需要)
    float verticalOffsetPercent = 0.0f; // 垂直偏移百分比 (0.0 = 底部对齐, 0.5 = 居中, 1.0 = 顶部对齐)
};

// ========================================
// 单个视差层
// ========================================
class ParallaxLayer
{
public:
    ParallaxLayer(ParallaxLayerConfig const& config);
    ~ParallaxLayer() = default;

    // 更新层位置（基于相机位置）
    void UpdatePosition(Vec2 const& cameraPos, Vec2 const& prevCameraPos);

    // 获取渲染所需的参数
    Texture* GetTexture() const { return m_config.texture; }
    Vec2 GetRenderPosition() const { return m_currentPosition; }
    Vec2 GetTextureScale() const { return m_config.textureScale; }
    Rgba8 GetTint() const;
    float GetDepthFactor() const { return m_config.depthFactor; }
    bool IsLoopingX() const { return m_config.isLoopingX; }
    bool IsLoopingY() const { return m_config.isLoopingY; }

    // 配置访问
    ParallaxLayerConfig& GetConfig() { return m_config; }
    const ParallaxLayerConfig& GetConfig() const { return m_config; }

private:
    ParallaxLayerConfig m_config;
    Vec2 m_currentPosition = Vec2::ZERO;  // 当前渲染位置
    Vec2 m_basePosition = Vec2::ZERO;     // 基准位置（用于循环）
};
