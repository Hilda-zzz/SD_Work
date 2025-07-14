#pragma once
#include "Widget.hpp"
#include <string>
#include "../Core/Vertex_PCU.hpp"

class Texture;

class Panel : public Widget
{
public:
	Panel() {}
	Panel(const Vec2& position, Texture* normalTex, AABB2 const& bkgExtent);

	void Update(float deltaTime) override;
	void Render(Renderer* renderer) const override;
	void RenderSelf(Renderer* renderer) const override;
	bool HandleInput(InputEvent const& event) override;
	void SetIsrenderSelf(bool isRenderSelf) { m_isRenderSelf = isRenderSelf; }
private:
	void UpdateVertices();

protected:
	std::string m_name;
	bool m_isModal = false;

	std::vector<Vertex_PCU> m_bkgVerts;
	Texture* m_texNormal = nullptr;
	Texture* m_texHover = nullptr;
	Texture* m_texClick = nullptr;
	Texture* m_curTex = nullptr;

	AABB2 m_bkgBox = AABB2();
	bool m_isRenderSelf = false;
};