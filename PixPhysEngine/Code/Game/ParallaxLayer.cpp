#include "ParallaxLayer.hpp"
#include "Engine/Renderer/Texture.hpp"

ParallaxLayer::ParallaxLayer(ParallaxLayerConfig const& config)
    : m_config(config)
{
}

void ParallaxLayer::UpdatePosition(Vec2 const& cameraPos, Vec2 const& prevCameraPos)
{
    // 计算相机移动量
    Vec2 cameraDelta = cameraPos - prevCameraPos;

    // 应用视差速度因子
    float parallaxDeltaX = cameraDelta.x * m_config.scrollSpeedX * m_config.depthFactor;
    float parallaxDeltaY = cameraDelta.y * m_config.scrollSpeedY * m_config.depthFactor;

    // 更新位置
    m_currentPosition.x += parallaxDeltaX;
    m_currentPosition.y += parallaxDeltaY;

    // 处理横向循环
    if (m_config.isLoopingX && m_config.texture)
    {
        float textureWidth = static_cast<float>(m_config.texture->GetDimensions().x) * m_config.textureScale.x;
        
        // 当偏移超过纹理宽度时，重置循环
        if (m_currentPosition.x > textureWidth)
        {
            m_currentPosition.x -= textureWidth;
            m_basePosition.x -= textureWidth;
        }
        else if (m_currentPosition.x < -textureWidth)
        {
            m_currentPosition.x += textureWidth;
            m_basePosition.x += textureWidth;
        }
    }

    // 处理纵向循环（如果需要）
    if (m_config.isLoopingY && m_config.texture)
    {
        float textureHeight = static_cast<float>(m_config.texture->GetDimensions().y) * m_config.textureScale.y;
        
        if (m_currentPosition.y > textureHeight)
        {
            m_currentPosition.y -= textureHeight;
            m_basePosition.y -= textureHeight;
        }
        else if (m_currentPosition.y < -textureHeight)
        {
            m_currentPosition.y += textureHeight;
            m_basePosition.y += textureHeight;
        }
    }
}

Rgba8 ParallaxLayer::GetTint() const
{
    Rgba8 tint = m_config.tint;
    tint.a = static_cast<unsigned char>(m_config.alpha * 255.0f);
    return tint;
}
