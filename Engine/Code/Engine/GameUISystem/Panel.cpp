#include "Panel.hpp"
#include "../Core/VertexUtils.hpp"
#include "../Renderer/Renderer.hpp"
#include "../Core/EngineCommon.hpp"

Panel::Panel(const Vec2& position, Texture* normalTex, AABB2 const& bkgExtent)
	:m_texNormal(normalTex)
{
	m_position = position;

	m_bounds = bkgExtent;
	m_bounds.m_mins += position;
	m_bounds.m_maxs += position;

	m_curTex = m_texNormal;

	UpdateVertices();
}

void Panel::Update(float deltaTime)
{
	Widget::Update(deltaTime);
}

void Panel::Render(Renderer* renderer) const
{
	Widget::Render(renderer);
}

void Panel::RenderSelf(Renderer* renderer) const
{
	if (m_isRenderSelf)
	{
		renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		renderer->SetBlendMode(BlendMode::ALPHA);
		renderer->BindShader(nullptr);
		renderer->SetModelConstants();
		renderer->BindTexture(m_curTex);
		renderer->DrawVertexArray(m_bkgVerts);
	}
}

bool Panel::HandleInput(InputEvent const& event)
{
	UNUSED(event);
	return false;
}

void Panel::UpdateVertices()
{
	m_bkgVerts.clear();
	AddVertsForAABB2D(m_bkgVerts, m_bounds, Rgba8::WHITE);
}
