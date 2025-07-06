#pragma once
#include "StaticMeshInfo.hpp"

class Texture;
class VertexBuffer;
class IndexBuffer;
class Renderer;
class Shader;

class MeshResource
{
public:
	MeshResource(StaticMeshInfo const& info,Renderer* curRenderer);
	~MeshResource();

public:
	StaticMeshInfo m_meshInfo;
	Shader* m_shader=nullptr;
	Texture* m_diffuseTex=nullptr;
	Texture* m_normalTex = nullptr;
	Texture* m_sgeTex = nullptr;
	Vec3 m_fwd;
	Vec3 m_left;
	Vec3 m_up;
	VertexBuffer* m_vertexBuffer=nullptr;    
	IndexBuffer* m_indexBuffer = nullptr;
};