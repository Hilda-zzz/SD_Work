#include "StaticMeshComponent.hpp"
#include "../Renderer/Renderer.hpp"
#include "Engine/ResourceManager/MeshResource.hpp"

StaticMeshComponent::StaticMeshComponent(MeshResource* meshResource):m_meshResource(meshResource)
{
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::Render(Renderer* curRenderer, Mat44 const& modelToWorldMat) const
{
	Mat44 modelMat = modelToWorldMat;
	Mat44 rotateMat = m_relativeOrientation.GetAsMatrix_IFwd_JLeft_KUp();
	Mat44 relativeMat = Mat44::MakeTranslation3D(m_relativePos);
	relativeMat.Append(rotateMat);
	modelMat.Append(relativeMat);

	curRenderer->SetModelConstants(modelMat);
	curRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	curRenderer->SetBlendMode(BlendMode::ALPHA);
	curRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	curRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	curRenderer->BindShader(m_meshResource->m_shader);
	curRenderer->BindTexture(m_meshResource->m_diffuseTex, m_meshResource->m_normalTex, m_meshResource->m_sgeTex);

	if (m_meshResource->m_meshInfo.m_useIndex)
	{
		curRenderer->DrawGameIndexedVertexBuffer(m_meshResource->m_vertexBuffer, m_meshResource->m_indexBuffer);
	}
	else
	{
		curRenderer->DrawGameVertexBuffer(m_meshResource->m_vertexBuffer);
	}
	
}

void StaticMeshComponent::SetRelativePosition(Vec3 const& relativePos, EulerAngles const& relativeOrientation)
{
	m_relativePos = relativePos;
	m_relativeOrientation = relativeOrientation;
}
