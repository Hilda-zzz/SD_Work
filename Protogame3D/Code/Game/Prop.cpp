#include "Prop.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
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
	
	std::vector<Vertex_PCU> temp_verts;
	for (int i = 0; i < static_cast<int>(m_vertexs.size()); i++)
	{
		temp_verts.push_back(m_vertexs[i]);
	}

	//TransformVertexArrayXY3D(static_cast<int>(m_vertexs.size()), temp_verts, 1.f, m_orientation, m_position);
	g_theRenderer->SetModelConstants(GetModelToWorldTransform(),m_color);
	
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(temp_verts);
}

Prop::~Prop()
{
}
