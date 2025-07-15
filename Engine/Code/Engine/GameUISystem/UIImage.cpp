#include "UIImage.hpp"
#include "../Core/VertexUtils.hpp"
#include "../Renderer/Renderer.hpp"

UIImage::UIImage(const Vec2& position, Texture* texture, AABB2 const& imageExtent, AABB2 const& uv)
	: m_texture(texture), m_extent(imageExtent), m_uv(uv)
{
	m_position = position;
	m_bounds = AABB2(m_extent.m_mins + m_position, m_extent.m_maxs + m_position);
	AddVertsForAABB2D(m_bkgVerts, m_bounds, Rgba8::WHITE, uv.m_mins, uv.m_maxs);
}

void UIImage::RenderSelf(Renderer* renderer) const
{
	renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	renderer->SetBlendMode(BlendMode::ALPHA);
	renderer->BindShader(nullptr);
	renderer->SetModelConstants();
	renderer->BindTexture(m_texture);
	renderer->DrawVertexArray(m_bkgVerts);
}

void UIImage::ChangeTexture(Texture* newTexture)
{
	m_texture = newTexture;
}

void UIImage::UpdateVertices(AABB2 const& newUv)
{
	m_bkgVerts.clear();
	m_bkgVerts.reserve(6);
	AddVertsForAABB2D(m_bkgVerts, m_bounds, Rgba8::WHITE, newUv.m_mins, newUv.m_maxs);
}
