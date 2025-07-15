#pragma once
#include "Widget.hpp"
#include <string>
#include "../Core/Vertex_PCU.hpp"

class Texture;

class UIImage : public Widget
{
public:
	UIImage() {}
	UIImage(const Vec2& position, Texture* texture, AABB2 const& imageExtent, AABB2 const& uv);

	void RenderSelf(Renderer* renderer) const override;
	void ChangeTexture(Texture* newTexture);

private:
	void UpdateVertices(AABB2 const& newUv);

protected:
	std::string m_name;

	std::vector<Vertex_PCU> m_bkgVerts;
	Texture* m_texture = nullptr;

	AABB2 m_extent = AABB2();
	AABB2 m_uv = AABB2();
};