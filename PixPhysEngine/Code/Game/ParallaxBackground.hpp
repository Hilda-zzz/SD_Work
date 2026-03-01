#pragma once
#include "ParallaxLayer.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"

class Camera;
class Renderer;
class Texture;

// ========================================
// 视差背景系统
// ========================================
class ParallaxBackground
{
public:
    ParallaxBackground(Renderer* renderer);
    ~ParallaxBackground();

    // ========================================
    // 生命周期
    // ========================================
    void Initialize();
    void Shutdown();
    void Update(Camera const& camera);
    void Render(Camera const& camera) const;

    // ========================================
    // 层管理
    // ========================================
    
    // 添加视差层（从后到前，索引越大越靠前）
    void AddLayer(ParallaxLayerConfig const& config);
    
    // 批量设置层（推荐使用此方法）
    void SetLayers(std::vector<ParallaxLayerConfig> const& layers);
    
    // 清除所有层
    void ClearLayers();
    
    // 获取层数量
    size_t GetLayerCount() const { return m_layers.size(); }
    
    // 获取指定层的配置（可修改）
    ParallaxLayer* GetLayer(size_t index);
    const ParallaxLayer* GetLayer(size_t index) const;

    // ========================================
    // 全局配置
    // ========================================
    
    // 设置全局视差强度（影响所有层）
    void SetGlobalParallaxStrength(float strength) { m_globalParallaxStrength = strength; }
    float GetGlobalParallaxStrength() const { return m_globalParallaxStrength; }
    
    // 启用/禁用渲染
    void SetEnabled(bool enabled) { m_isEnabled = enabled; }
    bool IsEnabled() const { return m_isEnabled; }
    
    // 设置渲染区域缩放（用于适配不同分辨率）
    void SetRenderScale(Vec2 const& scale) { m_renderScale = scale; }
    Vec2 GetRenderScale() const { return m_renderScale; }

    // ========================================
    // 便捷方法：快速设置洞穴背景
    // ========================================
    
    // 自动创建多层洞穴背景
    // textures: 从最远到最近的纹理数组
    // depthFactors: 每层的深度因子 (可选，默认自动计算)
    void SetupCaveBackground(
        std::vector<Texture*> const& textures,
        std::vector<float> const& depthFactors = {}
    );

private:
    // ========================================
    // 内部渲染方法
    // ========================================
    
    // 渲染单个层
	void RenderLayer(ParallaxLayer const& layer, 
        Vec2 const& cameraBottomLeft,
		Vec2 const& cameraTopRight) const;
    
    // 计算层的渲染范围（考虑循环）
    void CalculateRenderBounds(
        ParallaxLayer const& layer,
        Vec2 const& cameraBottomLeft,
        Vec2 const& cameraTopRight,
        Vec2& outBottomLeft,
        Vec2& outTopRight,
        int& outRepeatCountX,
        int& outRepeatCountY) const;

    // 生成层的渲染四边形
    void GenerateLayerQuad(
        Vec2 const& bottomLeft,
        Vec2 const& topRight,
        Rgba8 const& tint,
        std::vector<Vertex_PCU>& outVerts
    ) const;

private:
    Renderer* m_renderer = nullptr;
    std::vector<ParallaxLayer> m_layers;       // 从后到前排序
    
    Vec2 m_previousCameraPos = Vec2::ZERO;     // 用于计算相机移动量
    
    float m_globalParallaxStrength = 1.0f;     // 全局视差强度
    Vec2 m_renderScale = Vec2(1.0f, 1.0f);     // 渲染缩放
    bool m_isEnabled = true;                   // 是否启用
};
