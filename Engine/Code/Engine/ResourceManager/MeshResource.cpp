#include "MeshResource.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "../Renderer/Renderer.hpp"
#include "../Core/ObjLoader.hpp"

MeshResource::MeshResource(StaticMeshInfo const& info, Renderer* curRenderer):m_meshInfo(info)
{
	m_vertexBuffer= curRenderer->CreateVertexBuffer(200000, sizeof(Vertex_PCUTBN));
	m_indexBuffer = curRenderer->CreateIndexBuffer(200000);

	m_shader = curRenderer->CreateShaderFromFile(m_meshInfo.m_shaderName.c_str(), VertexType::VERTEX_PCUTBN);
	m_diffuseTex = curRenderer->CreateOrGetTextureFromFile(m_meshInfo.m_diffuseTexPath.c_str());
	m_normalTex = curRenderer->CreateOrGetTextureFromFile(m_meshInfo.m_normalTexPath.c_str());
	m_sgeTex = curRenderer->CreateOrGetTextureFromFile(m_meshInfo.m_sgeTexPath.c_str());

	m_fwd = m_meshInfo.m_fwd;
	m_left = m_meshInfo.m_left;
	m_up = m_meshInfo.m_up;

	std::vector<Vertex_PCUTBN> meshVerts;
	std::vector<unsigned int> meshIndices;
	Vec3 test = Vec3(0.f, 1.f, 0.f);
	Mat44 modelToEngine = Mat44(m_fwd, m_left, m_up, Vec3(0.f, 0.f, 0.f));
	modelToEngine.Transpose();
	test = modelToEngine.TransformPosition3D(test);
	if (!m_meshInfo.m_useIndex)
	{
		ObjLoader::LoadObjFromFile_WithTBN(m_meshInfo.m_objFilePath, meshVerts, 
			1.f / info.m_unitsPerMeter, modelToEngine);
		curRenderer->CopyGameVertexBufferToGPU(meshVerts.data(), (int)meshVerts.size(), m_vertexBuffer);
	}
	else
	{
		ObjLoader::LoadObjFromFile_WithTBN_WithIndex(m_meshInfo.m_objFilePath, meshVerts, meshIndices,
			1.f / info.m_unitsPerMeter, modelToEngine);
		curRenderer->CopyGameVertexBufferToGPU(meshVerts.data(), (int)meshVerts.size(), m_vertexBuffer);
		curRenderer->CopyGameIndexBufferToGPU(meshIndices.data(), (int)meshIndices.size(), m_indexBuffer);
	}
}

MeshResource::~MeshResource()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;
}

