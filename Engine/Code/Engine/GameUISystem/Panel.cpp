#include "Panel.hpp"
#include "../Core/VertexUtils.hpp"
#include "../Renderer/Renderer.hpp"

Panel::Panel(const Vec2& position, Texture* normalTex, AABB2 const& bkgExtent)
	:m_texNormal(normalTex),m_bkgBox(bkgExtent)
{
	m_position = position;
	m_bounds = bkgExtent;
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
	renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	renderer->SetBlendMode(BlendMode::ALPHA);
	renderer->BindShader(nullptr);
	renderer->SetModelConstants();
	renderer->BindTexture(m_curTex);
	renderer->DrawVertexArray(m_bkgVerts);
}

bool Panel::HandleInput(InputEvent const& event)
{
	return false;
}

void Panel::UpdateVertices()
{
	m_bkgVerts.clear();
	Vec2 bkgMins = m_position + m_bkgBox.m_mins;
	Vec2 bkgMaxs = m_position + m_bkgBox.m_maxs;
	AddVertsForAABB2D(m_bkgVerts, AABB2(bkgMins, bkgMaxs), Rgba8::WHITE);
}
