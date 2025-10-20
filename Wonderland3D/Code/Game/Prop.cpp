#include "Prop.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
Prop::Prop(Game* owner):Entity(owner)
{
	m_vertexs.reserve(40);
}

void Prop::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Prop::Render() const
{
	g_theRenderer->SetModelConstants(GetModelToWorldTransform(),m_color);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(m_game->m_pbrShader);

	m_pbrMat.Bind(g_theRenderer);
	g_theRenderer->DrawVertexArray_WithTBN(m_vertexs,m_indices);
}

Prop::~Prop()
{
}
