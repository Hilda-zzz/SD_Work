#pragma once
#include "ParallaxBackground.hpp"
#include <ThirdParty/imgui/imgui.h>
#include "ParallaxLayer.hpp"
#include "Engine/Renderer/Texture.hpp"

// ========================================
// 视差背景ImGui调试面板
// ========================================
class ParallaxBackgroundDebugPanel
{
public:
    static void RenderImGui(ParallaxBackground* parallaxBg, const char* windowName = "Parallax Background Debug");
    
private:
    static void RenderGlobalControls(ParallaxBackground* parallaxBg);
    static void RenderLayerControls(ParallaxBackground* parallaxBg);
    static void RenderPresets(ParallaxBackground* parallaxBg);
};

// 实现（可以放在 .cpp 中，这里为了方便直接在头文件中实现）

inline void ParallaxBackgroundDebugPanel::RenderImGui(ParallaxBackground* parallaxBg, const char* windowName)
{
    if (!parallaxBg)
        return;
        
    if (ImGui::Begin(windowName))
    {
        RenderGlobalControls(parallaxBg);
        ImGui::Separator();
        RenderLayerControls(parallaxBg);
        ImGui::Separator();
        RenderPresets(parallaxBg);
    }
    ImGui::End();
}

inline void ParallaxBackgroundDebugPanel::RenderGlobalControls(ParallaxBackground* parallaxBg)
{
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Global Controls");
    
    // 启用/禁用
    bool enabled = parallaxBg->IsEnabled();
    if (ImGui::Checkbox("Enable Background", &enabled))
    {
        parallaxBg->SetEnabled(enabled);
    }
    
    // 全局强度
    float strength = parallaxBg->GetGlobalParallaxStrength();
    if (ImGui::SliderFloat("Parallax Strength", &strength, 0.0f, 3.0f))
    {
        parallaxBg->SetGlobalParallaxStrength(strength);
    }
    
    // 渲染缩放
    Vec2 scale = parallaxBg->GetRenderScale();
    float scaleArr[2] = { scale.x, scale.y };
    if (ImGui::DragFloat2("Render Scale", scaleArr, 0.01f, 0.1f, 5.0f))
    {
        parallaxBg->SetRenderScale(Vec2(scaleArr[0], scaleArr[1]));
    }
    
    // 统计信息
    ImGui::Text("Layer Count: %zu", parallaxBg->GetLayerCount());
}

inline void ParallaxBackgroundDebugPanel::RenderLayerControls(ParallaxBackground* parallaxBg)
{
    ImGui::TextColored(ImVec4(0.2f, 0.5f, 1.0f, 1.0f), "Layer Controls");
    
    for (size_t i = 0; i < parallaxBg->GetLayerCount(); ++i)
    {
        ParallaxLayer* layer = parallaxBg->GetLayer(i);
        if (!layer)
            continue;
            
        char label[64];
        sprintf_s(label, "Layer %zu", i);
        
        if (ImGui::TreeNode((void*)(intptr_t)i, "%s", label))
        {
            ParallaxLayerConfig& config = layer->GetConfig();
            
            // 深度因子
            ImGui::SliderFloat("Depth Factor", &config.depthFactor, 0.0f, 1.0f);
            ImGui::SameLine();
            if (ImGui::Button("?##depth"))
            {
                ImGui::SetTooltip("0.0 = Farthest (slowest), 1.0 = Nearest (camera speed)");
            }
            
            // 滚动速度
            ImGui::DragFloat("Scroll Speed X", &config.scrollSpeedX, 0.01f, -2.0f, 2.0f);
            ImGui::DragFloat("Scroll Speed Y", &config.scrollSpeedY, 0.01f, -2.0f, 2.0f);
            
            // 透明度
            ImGui::SliderFloat("Alpha", &config.alpha, 0.0f, 1.0f);
            
            // 色调
            float tintArr[4] = {
                config.tint.r / 255.0f,
                config.tint.g / 255.0f,
                config.tint.b / 255.0f,
                config.tint.a / 255.0f
            };
            if (ImGui::ColorEdit4("Tint", tintArr))
            {
                config.tint = Rgba8(
                    static_cast<unsigned char>(tintArr[0] * 255),
                    static_cast<unsigned char>(tintArr[1] * 255),
                    static_cast<unsigned char>(tintArr[2] * 255),
                    static_cast<unsigned char>(tintArr[3] * 255)
                );
            }
            
            // 纹理缩放
            float scaleArr[2] = { config.textureScale.x, config.textureScale.y };
            if (ImGui::DragFloat2("Texture Scale", scaleArr, 0.01f, 0.1f, 5.0f))
            {
                config.textureScale = Vec2(scaleArr[0], scaleArr[1]);
            }
            
            // 循环选项
            ImGui::Checkbox("Loop X", &config.isLoopingX);
            ImGui::SameLine();
            ImGui::Checkbox("Loop Y", &config.isLoopingY);
            
            // 垂直对齐
            ImGui::SliderFloat("Vertical Offset %", &config.verticalOffsetPercent, 0.0f, 1.0f);
            ImGui::SameLine();
            if (ImGui::Button("?##valign"))
            {
                ImGui::SetTooltip("0.0 = Bottom align, 0.5 = Center, 1.0 = Top align");
            }
            
            // 纹理信息
            if (config.texture)
            {
                IntVec2 texSize = config.texture->GetDimensions();
                ImGui::Text("Texture: %dx%d", texSize.x, texSize.y);
            }
            
            ImGui::TreePop();
        }
    }
}

inline void ParallaxBackgroundDebugPanel::RenderPresets(ParallaxBackground* parallaxBg)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Quick Presets");
    
    if (ImGui::Button("Subtle Parallax"))
    {
        for (size_t i = 0; i < parallaxBg->GetLayerCount(); ++i)
        {
            ParallaxLayer* layer = parallaxBg->GetLayer(i);
            if (layer)
            {
                float t = static_cast<float>(i) / static_cast<float>(parallaxBg->GetLayerCount() - 1);
                layer->GetConfig().depthFactor = 0.7f + t * 0.3f;  // 0.7 to 1.0
            }
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Dramatic Parallax"))
    {
        for (size_t i = 0; i < parallaxBg->GetLayerCount(); ++i)
        {
            ParallaxLayer* layer = parallaxBg->GetLayer(i);
            if (layer)
            {
                float t = static_cast<float>(i) / static_cast<float>(parallaxBg->GetLayerCount() - 1);
                layer->GetConfig().depthFactor = 0.1f + t * 0.9f;  // 0.1 to 1.0
            }
        }
    }
    
    if (ImGui::Button("Reset All Alphas to 1.0"))
    {
        for (size_t i = 0; i < parallaxBg->GetLayerCount(); ++i)
        {
            ParallaxLayer* layer = parallaxBg->GetLayer(i);
            if (layer)
            {
                layer->GetConfig().alpha = 1.0f;
            }
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Fade Far Layers"))
    {
        for (size_t i = 0; i < parallaxBg->GetLayerCount(); ++i)
        {
            ParallaxLayer* layer = parallaxBg->GetLayer(i);
            if (layer)
            {
                float t = static_cast<float>(i) / static_cast<float>(parallaxBg->GetLayerCount() - 1);
                layer->GetConfig().alpha = 0.4f + t * 0.6f;  // 0.4 to 1.0
            }
        }
    }
}
