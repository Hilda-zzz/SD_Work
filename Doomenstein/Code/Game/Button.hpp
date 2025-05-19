#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class BitmapFont;

enum class ButtonState
{
	BTN_NORMAL,
	BTN_HOVER,
	BTN_CLICK
};

class Button
{
public:
	Button() {};
	~Button() {};

	//Button(const Vec2& position, const std::string& text = "");
	Button(const Vec2& position, Texture* normalTex, Texture* hoverTex, Texture* clickTex,
		AABB2 bkgExtent, AABB2 textExtent, std::string text,float textHeight,BitmapFont* font,std::string clickEventName);

	void Update(float deltaSeconds); 
	void Render() const;

	void SetOnClickCallback(std::string const& eventName);
	bool IsPointInside() const;
	bool IsPressed() const;

	void SetPosition(const Vec2& position);
	Vec2 GetPosition() const;

	void SetText(const std::string& text);
	std::string GetText() const;

	ButtonState GetState() const;
	void SetState(ButtonState state);

public:

private:
	void FireClickEvent();

	void UpdateVertices();

private:
	ButtonState curState = ButtonState::BTN_NORMAL;

	std::vector<Vertex_PCU> m_bkgVerts;
	Texture* m_texNormal=nullptr;
	Texture* m_texHover = nullptr;
	Texture* m_texClick = nullptr;
	Texture* m_curTex = nullptr;

	AABB2 m_bkgBox=AABB2();
	AABB2 m_textBox = AABB2();
	Vec2  m_position = Vec2();

	std::vector<Vertex_PCU> m_textVerts;
	std::string m_text = "";
	float m_textHeight = 1.0f;
	BitmapFont* m_font = nullptr;

	std::string m_clickEventName="DefaultBtnEvent";

	SoundID m_clickSound;
	SoundID m_hoverSound;
};