#pragma once
#include "Game/Entity.hpp"
#include <vector>
#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/PBRMaterial.hpp"
class Prop :public Entity
{
public:
	Prop(Game* owner);
	void Update(float deltaSeconds) override;
	void Render() const override;
	~Prop();
public:
	std::vector<Vertex_PCUTBN> m_vertexs;
	std::vector<unsigned int> m_indices;
	Rgba8 m_color = Rgba8::WHITE;
	Texture* m_texture= nullptr;
	PBRMaterial m_pbrMat;
};