#pragma once
#include "Game/Entity.hpp"
#include <vector>
#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
class Prop :public Entity
{
public:
	Prop(Game* owner);
	void Update(float deltaSeconds) override;
	void Render() const override;
	~Prop();
public:
	std::vector<Vertex_PCU> m_vertexs;
	Rgba8 m_color = Rgba8::WHITE;
	Texture* m_texture= nullptr;
};