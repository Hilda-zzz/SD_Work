#pragma once
#include "Widget.hpp"
#include "../Math/Vec2.hpp"
#include <string>
#include "../Core/Vertex_PCU.hpp"
#include "../Renderer/BitmapFont.hpp"

class Texture;
class BitmapFont;

class UIText : public Widget
{
public:
	UIText() {}
	UIText(const std::string& name, const std::string& content, BitmapFont* font,
		const AABB2& bounds, float cellHeight, Rgba8 const& tint);

	void RenderSelf(Renderer* renderer) const override;
	void ChangeText(std::string const& newContent);

private:
	void UpdateVertices();

protected:
	std::string m_name;
	std::string m_content;
	std::vector<Vertex_PCU> m_textVerts;
	BitmapFont* m_font;

	AABB2 m_extent = AABB2();

	float m_cellHeight;
	Rgba8 m_tint = Rgba8::WHITE;
	float m_cellAspectScale = 1.f;
	Vec2 m_alignment = Vec2(.5f, .5f);
	TextBoxMode m_textMode = TextBoxMode::SHRINK_TO_FIT;
};