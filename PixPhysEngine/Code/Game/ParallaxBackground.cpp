#include "ParallaxBackground.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <algorithm>
#include <Engine/Core/VertexUtils.hpp>
#include "GameCommon.hpp"

ParallaxBackground::ParallaxBackground(Renderer* renderer)
    : m_renderer(renderer)
{
}

ParallaxBackground::~ParallaxBackground()
{
    Shutdown();
}

// ========================================
// 生命周期
// ========================================

void ParallaxBackground::Initialize()
{
    // 不需要创建VertexBuffer，直接使用DrawVertexArray
}

void ParallaxBackground::Shutdown()
{
    m_layers.clear();
}

void ParallaxBackground::Update(Camera const& camera)
{
    if (!m_isEnabled)
        return;

    Vec2 cameraPos = Vec2(camera.GetOrthoBottomLeft());
    
    // 更新每一层
    for (auto& layer : m_layers)
    {
        layer.UpdatePosition(cameraPos, m_previousCameraPos);
    }
    
    m_previousCameraPos = cameraPos;
}

void ParallaxBackground::Render(Camera const& camera) const
{
	if (!m_isEnabled || m_layers.empty())
		return;

	// 提前计算相机视野范围（所有层共用）
	Vec2 cameraBottomLeft = camera.GetOrthoBottomLeft();
	Vec2 cameraTopRight = camera.GetOrthoTopRight();

	// 从后往前渲染每一层
	for (const auto& layer : m_layers)
	{
		RenderLayer(layer, cameraBottomLeft, cameraTopRight);
	}
}

// ========================================
// 层管理
// ========================================

void ParallaxBackground::AddLayer(ParallaxLayerConfig const& config)
{
    m_layers.emplace_back(config);
}

void ParallaxBackground::SetLayers(std::vector<ParallaxLayerConfig> const& layers)
{
    m_layers.clear();
    m_layers.reserve(layers.size());
    
    for (const auto& config : layers)
    {
        m_layers.emplace_back(config);
    }
}

void ParallaxBackground::ClearLayers()
{
    m_layers.clear();
}

ParallaxLayer* ParallaxBackground::GetLayer(size_t index)
{
    if (index >= m_layers.size())
        return nullptr;
    return &m_layers[index];
}

const ParallaxLayer* ParallaxBackground::GetLayer(size_t index) const
{
    if (index >= m_layers.size())
        return nullptr;
    return &m_layers[index];
}

// ========================================
// 便捷方法：快速设置洞穴背景
// ========================================

void ParallaxBackground::SetupCaveBackground(
    std::vector<Texture*> const& textures,
    std::vector<float> const& depthFactors)
{
    ClearLayers();
    
    if (textures.empty())
        return;
    
	// 如果没有提供深度因子，自动生成
	std::vector<float> depths = depthFactors;
	if (depths.empty())
	{
		// 自动生成深度因子：从1.0（最远）到0.2（最近）
		// 远景 depth 大 → 紧跟相机 → 移动慢
		// 近景 depth 小 → 远离相机 → 移动快
		float depthStep = 0.8f / static_cast<float>(textures.size());
		for (size_t i = 0; i < textures.size(); ++i)
		{
			depths.push_back(1.0f - depthStep * static_cast<float>(i));
		}
	}
    
    // 创建每一层
    for (size_t i = 0; i < textures.size(); ++i)
    {
        if (!textures[i])
            continue;
            
        ParallaxLayerConfig config;
        config.texture = textures[i];
        config.depthFactor = depths[i];
        
        // 洞穴场景配置
        config.scrollSpeedX = 1.0f;  // 横向完全跟随
        config.scrollSpeedY = 1.0f;  // 纵向完全跟随
        config.isLoopingX = true;    // 横向循环
        config.isLoopingY = false;   // 纵向不循环
        
        // 根据纹理尺寸计算合适的缩放
        IntVec2 texSize = textures[i]->GetDimensions();
        config.textureScale = Vec2(
            static_cast<float>(texSize.x) / 1920.0f,  // 假设参考分辨率1920x1080
            static_cast<float>(texSize.y) / 1080.0f
        );
        
        AddLayer(config);
    }
}

// ========================================
// 内部渲染方法
// ========================================
void ParallaxBackground::RenderLayer(
	ParallaxLayer const& layer,
	Vec2 const& cameraBottomLeft,
	Vec2 const& cameraTopRight) const
{
	Texture* texture = layer.GetTexture();
	if (!texture)
		return;

	// ========================================
	// 1. 获取纹理原始尺寸和相机信息
	// ========================================
	IntVec2 texSize = texture->GetDimensions();
	float texWidth = static_cast<float>(texSize.x);
	float texHeight = static_cast<float>(texSize.y);
	float viewHeight = 400.f;

	// ========================================
	// 2. 计算缩放后的尺寸
	// ========================================
	float scaleToFitHeight = viewHeight / texHeight;
	float scaledWidth = texWidth * scaleToFitHeight;
	float scaledHeight = texHeight * scaleToFitHeight;

	// ========================================
	// 3. 计算相机中心
	// ========================================
	Vec2 cameraCenter(
		(cameraBottomLeft.x + cameraTopRight.x) * 0.5f,
		(cameraBottomLeft.y + cameraTopRight.y) * 0.5f
	);

	// ========================================
	// 4. 计算横向位置（视差效果）
	// ========================================
	float depthFactor = layer.GetDepthFactor();
	float parallaxOffsetX = cameraCenter.x * (1.0f - depthFactor);
	float textureCenterX = cameraCenter.x - parallaxOffsetX;
	float centerStartX = textureCenterX - scaledWidth * 0.5f;

	// ========================================
	// 5. 计算纵向位置（居中对齐）
	// ========================================
	float textureCenterY = cameraCenter.y;
	float startY = textureCenterY - scaledHeight * 0.5f;

	// ========================================
	// 6. 生成多个四边形（中间1个 + 左3个 + 右3个 = 7个）
	// ========================================
	std::vector<Vertex_PCU> verts;
	Vec2 uvMins(0.0f, 0.0f);
	Vec2 uvMaxs(1.0f, 1.0f);

	// 循环渲染：左3个 + 中间1个 + 右3个
	for (int i = -2; i <= 2; ++i)
	{
		float startX = centerStartX + i * scaledWidth;

		Vec2 bottomLeft(startX, startY);
		Vec2 topRight(startX + scaledWidth, startY + scaledHeight);

		AABB2 quad(bottomLeft, topRight);
		AddVertsForAABB2D(verts, quad, Rgba8(100,100,100), uvMins, uvMaxs);
	}

	// ========================================
	// 7. 渲染
	// ========================================
	if (!verts.empty())
	{
		m_renderer->BindTexture(texture);
		m_renderer->SetModelConstants();
		m_renderer->DrawVertexArray(verts);
	}
}

void ParallaxBackground::CalculateRenderBounds(
	ParallaxLayer const& layer,
	Vec2 const& cameraBottomLeft,
	Vec2 const& cameraTopRight,
	Vec2& outBottomLeft,
	Vec2& outTopRight,
	int& outRepeatCountX,
	int& outRepeatCountY) const
{
	// ========================================
	// 1. 获取纹理和缩放信息
	// ========================================
	Texture* texture = layer.GetTexture();
	IntVec2 texSize = texture->GetDimensions();
	Vec2 texScale = layer.GetTextureScale() * m_renderScale;
	float texWidth = static_cast<float>(texSize.x) * texScale.x;
	float texHeight = static_cast<float>(texSize.y) * texScale.y;

	float viewWidth = cameraTopRight.x - cameraBottomLeft.x;
	float viewHeight = cameraTopRight.y - cameraBottomLeft.y;

	// ========================================
	// 2. 计算视差偏移后的层位置
	// ========================================
	float depthFactor = layer.GetDepthFactor();
	Vec2 layerOffset = layer.GetRenderPosition();

	// 视差效果：深度越小（越远），移动越慢
	// depthFactor = 0.0 → 完全不动（无限远）
	// depthFactor = 1.0 → 完全跟随相机（相机平面）
	Vec2 parallaxShift = Vec2(
		layerOffset.x * depthFactor,
		layerOffset.y * depthFactor
	);

	// ========================================
	// 3. 横向处理（X轴）
	// ========================================
	if (layer.IsLoopingX())
	{
		// 计算需要多少个纹理重复才能填满视野（+2用于边界缓冲）
		outRepeatCountX = static_cast<int>(std::ceil(viewWidth / texWidth)) + 2;

		// 计算起始X坐标，确保无缝循环
		// 将层的偏移映射到[0, texWidth)范围内
		float wrappedOffsetX = std::fmod(parallaxShift.x, texWidth);
		if (wrappedOffsetX > 0.0f)
			wrappedOffsetX -= texWidth;  // 确保从视野左侧之前开始

		outBottomLeft.x = cameraBottomLeft.x + wrappedOffsetX;
	}
	else
	{
		// 不循环，只渲染一次
		outRepeatCountX = 1;
		outBottomLeft.x = cameraBottomLeft.x + parallaxShift.x;
	}

	// ========================================
	// 4. 纵向处理（Y轴）
	// ========================================
	if (layer.IsLoopingY())
	{
		// 纵向也循环（较少使用）
		outRepeatCountY = static_cast<int>(std::ceil(viewHeight / texHeight)) + 2;

		float wrappedOffsetY = std::fmod(parallaxShift.y, texHeight);
		if (wrappedOffsetY > 0.0f)
			wrappedOffsetY -= texHeight;

		outBottomLeft.y = cameraBottomLeft.y + wrappedOffsetY;
	}
	else
	{
		// 不循环，根据对齐方式定位（洞穴场景常用）
		outRepeatCountY = 1;

		float verticalAlign = layer.GetConfig().verticalOffsetPercent;
		// verticalAlign = 0.0 → 底部对齐
		// verticalAlign = 0.5 → 居中
		// verticalAlign = 1.0 → 顶部对齐

		float yOffset = (viewHeight - texHeight) * verticalAlign + parallaxShift.y;
		outBottomLeft.y = cameraBottomLeft.y + yOffset;
	}

	// ========================================
	// 5. 计算渲染区域的右上角
	// ========================================
	outTopRight = outBottomLeft + Vec2(
		texWidth * static_cast<float>(outRepeatCountX),
		texHeight * static_cast<float>(outRepeatCountY)
	);
}


