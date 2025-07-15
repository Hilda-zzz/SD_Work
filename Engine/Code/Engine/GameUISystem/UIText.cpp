#include "UIText.hpp"
#include "../Renderer/Renderer.hpp"

UIText::UIText(const std::string& name, const std::string& content, BitmapFont* font,
	const AABB2& bounds, float cellHeight,Rgba8 const& tint)
	: m_name(name)
	, m_content(content)
	, m_font(font)
	, m_extent(bounds) 
	, m_cellHeight(cellHeight)
	, m_tint(tint)
	, m_cellAspectScale(1.0f)
	, m_alignment(Vec2(0.5f, 0.5f)) 
	, m_textMode(TextBoxMode::SHRINK_TO_FIT)
{
	m_bounds = bounds;
	UpdateVertices();
}

void UIText::RenderSelf(Renderer* renderer) const
{
	renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	renderer->SetBlendMode(BlendMode::ALPHA);
	renderer->BindShader(nullptr);
	renderer->SetModelConstants();
	renderer->BindTexture(&m_font->GetTexture());
	renderer->DrawVertexArray(m_textVerts);
}

void UIText::ChangeText(std::string const& newContent)
{
	m_content = newContent;
}

void UIText::UpdateVertices()
{
	m_textVerts.clear();
	m_font->AddVertsForTextInBox2D(m_textVerts, m_content, m_bounds, m_cellHeight,
		m_tint, m_cellAspectScale, m_alignment, m_textMode);
}
