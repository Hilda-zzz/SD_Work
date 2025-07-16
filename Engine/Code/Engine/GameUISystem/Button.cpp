#include "Button.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/GameUISystem/GameUISystem.hpp"

extern Window* g_theWindow;
extern EventSystem* g_theEventSystem;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;

// Button::Button( Vec2& position,
// 	Texture* normalTex, Texture* hoverTex, Texture* clickTex, 
// 	AABB2 bkgExtent, AABB2 textExtent, std::string text, float textHeight, 
// 	BitmapFont* font, std::string clickEventName)
// 	: m_texNormal(normalTex)
// 	, m_texHover(hoverTex ? hoverTex : normalTex)  
// 	, m_texClick(clickTex ? clickTex : normalTex)  
// 	, m_curTex(normalTex)                          
// 	, m_bkgBox(bkgExtent)
// 	, m_textBox(textExtent)
// 	, m_text(text)
// 	, m_textHeight(textHeight)
// 	, m_font(font)
// 	, m_clickEventName(clickEventName)
// {
// 	m_position = position;
// 	m_bounds = m_bkgBox;
// 
// 	UpdateVertices();
// 
// 	m_clickSound = g_theAudio->CreateOrGetSound("Data/Audio/DefaultBtnClick.mp3");
// 	m_hoverSound = g_theAudio->CreateOrGetSound("Data/Audio/DefaultBtnHover.mp3");
// }

Button::Button(GameUISystem* uiSystem, const Vec2& position, 
	Texture* normalTex, Texture* hoverTex, Texture* clickTex, 
	AABB2 bkgExtent, AABB2 textExtent, std::string text, 
	float textHeight, BitmapFont* font, std::string clickEventName)
	: m_texNormal(normalTex)
	, m_texHover(hoverTex ? hoverTex : normalTex)
	, m_texClick(clickTex ? clickTex : normalTex)
	, m_curTex(normalTex)
	, m_bkgBox(bkgExtent)
	, m_textBox(textExtent)
	, m_text(text)
	, m_textHeight(textHeight)
	, m_font(font)
	, m_clickEventName(clickEventName)
{
	m_curUISystem = uiSystem;
	m_position = position;
	m_bounds = m_bkgBox;

	UpdateVertices();

	m_clickSound = g_theAudio->CreateOrGetSound("Data/Audio/DefaultBtnClick.mp3");
	m_hoverSound = g_theAudio->CreateOrGetSound("Data/Audio/DefaultBtnHover.mp3");
}

void Button::Update(float deltaTime)
{
	UNUSED(deltaTime);
	UpdatePressState();
	UpdateHoverState();

	if (m_curState != m_prevState)
	{
		OnStateChange();
	}
	m_prevState = m_curState;
// 	Vec2 mousePos = g_theWindow->GetMousePixelPos();
// 
// 	bool isInside = IsPointInside();
// 
// 	bool isPressed = IsPressed();
// 
// 	bool handled = false;
// 	ButtonState prevState = m_curState;
// 	if (isInside)
// 	{
// 		if (isPressed)
// 		{
// 			m_curState = ButtonState::BTN_CLICK;
// 			if (prevState != ButtonState::BTN_CLICK)
// 			{
// 				// Mouse button just pressed
// 				m_curTex = m_texClick;
// 			}
// 		}
// 		else
// 		{
// 			if (prevState == ButtonState::BTN_CLICK)
// 			{
// 				// Mouse button released while over button - trigger click event
// 				FireClickEvent();
// 			}
// 
// 			m_curState = ButtonState::BTN_HOVER;
// 			if (prevState != ButtonState::BTN_HOVER)
// 			{
// 				m_curTex = m_texHover;
// 				g_theAudio->StartSound(m_hoverSound);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		m_curState = ButtonState::BTN_NORMAL;
// 		if (prevState != ButtonState::BTN_NORMAL)
// 		{
// 			m_curTex = m_texNormal;
// 		}
// 	}
}

void Button::Render(Renderer* renderer) const
{
	renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	renderer->SetBlendMode(BlendMode::ALPHA);
	renderer->BindShader(nullptr);
	renderer->SetModelConstants();
	renderer->BindTexture(m_curTex);
	renderer->DrawVertexArray(m_bkgVerts);
	renderer->BindTexture(&m_font->GetTexture());
	renderer->DrawVertexArray(m_textVerts);
}

bool Button::HandleInput(InputEvent const& event)
{
	if (!m_canReceiveInput || !m_isVisible)
	{
		return false;
	}

	if (event.keyCode == KEYCODE_LEFT_MOUSE)
	{
		if (m_prevState==ButtonState::BTN_HOVER&&event.type==InputEventType::KEY_PRESS)
		{
			m_curState = ButtonState::BTN_PRESS;
			return true;
		}
	}
	return false;
// 	switch (event.keyCode)
// 	{
// 	case KEYCODE_LEFT_MOUSE:
// 		return HandleKeyPress(event.keyCode);
// 
// 	default:
// 		return false;
// 	}
// 
// // Vec2 mousePos = g_theWindow->GetMousePixelPos();
// 
// 	bool isInside = IsPointInside();
// 
// 	bool isPressed = IsPressed();
// 
// 	ButtonState prevState = m_curState;
// 
// 	if (isInside)
// 	{
// 		if (isPressed)
// 		{
// 			m_curState = ButtonState::BTN_CLICK;
// 			if (prevState != ButtonState::BTN_CLICK)
// 			{
// 				// Mouse button just pressed
// 				m_curTex = m_texClick;
// 			}
// 		}
// 		else
// 		{
// 			if (prevState == ButtonState::BTN_CLICK)
// 			{
// 				// Mouse button released while over button - trigger click event
// 				FireClickEvent();
// 			}
// 
// 			m_curState = ButtonState::BTN_HOVER;
// 			if (prevState != ButtonState::BTN_HOVER)
// 			{
// 				m_curTex = m_texHover;
// 				g_theAudio->StartSound(m_hoverSound);
// 			}
// 		}
// 
// 		return true;
// 	}
// 	else
// 	{
// 		m_curState = ButtonState::BTN_NORMAL;
// 		if (prevState != ButtonState::BTN_NORMAL)
// 		{
// 			m_curTex = m_texNormal;
// 		}
// 	}
}

void Button::UpdateHoverState()
{
	bool isHovered = IsPointInside();

	if (isHovered && !m_wasHover)
	{
		 OnMouseEnter();
	}
	else if(!isHovered&&m_wasHover)
	{
		 OnMouseLeave();
	}

	if (isHovered && m_curState == ButtonState::BTN_NORMAL)
	{
		m_curState = ButtonState::BTN_HOVER;
	}
	else if (!isHovered && m_curState == ButtonState::BTN_HOVER)
	{
		m_curState = ButtonState::BTN_NORMAL;
	}
	m_wasHover = isHovered;
}

void Button::UpdatePressState()
{
	bool isInside = IsPointInside();
	if (m_curState==ButtonState::BTN_PRESS&&
		!m_curUISystem->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		if (isInside)
		{
			OnMouseClick();
			m_curState = ButtonState::BTN_HOVER;
		}
		else
		{
			m_curState = ButtonState::BTN_NORMAL;
		}
	}
}

void Button::SetOnClickCallback(std::string const& eventName)
{
	m_clickEventName = eventName;
}

bool Button::IsPointInside() const
{
	Vec2 mouseUV = g_theWindow->GetNormalizedMouseUV();
	Vec2 mousePosition = AABB2(Vec2(0.f,0.f), Vec2(1600.f,800.f)).GetPointAtUV(mouseUV);

	Vec2 localPoint = mousePosition - m_position;
	return m_bkgBox.IsPointInside(localPoint);
}

bool Button::IsPressed() const
{
	return g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE);
}

void Button::SetPosition(const Vec2& position)
{
	if (m_position != position)
	{
		m_position = position;
		UpdateVertices();
	}
}

Vec2 Button::GetPosition() const
{
	return m_position;
}

void Button::SetText(const std::string& text)
{
	if (m_text != text)
	{
		m_text = text;
		UpdateVertices();
	}
}

std::string Button::GetText() const
{
	return m_text;
}

ButtonState Button::GetState() const
{
	return m_curState;
}

void Button::SetState(ButtonState state)
{
	if (m_curState != state)
	{
		m_curState = state;

		switch (m_curState)
		{
		case ButtonState::BTN_NORMAL:
			m_curTex = m_texNormal;
			break;
		case ButtonState::BTN_HOVER:
			m_curTex = m_texHover;
			break;
		case ButtonState::BTN_CLICK:
			m_curTex = m_texClick;
			break;
		}
	}
}

void Button::OnStateChange()
{
	switch (m_curState)
	{
	case ButtonState::BTN_NORMAL:
		m_curTex = m_texNormal;
		break;
	case ButtonState::BTN_HOVER:
		m_curTex = m_texHover;
		break;
	case ButtonState::BTN_PRESS:
		m_curTex = m_texClick;
		break;
	}
}

void Button::OnMouseEnter()
{
	g_theAudio->StartSound(m_hoverSound);
}

void Button::OnMouseLeave()
{
}

void Button::OnMouseClick()
{
	FireClickEvent();
}

void Button::FireClickEvent()
{
	g_theAudio->StartSound(m_clickSound);
	g_theEventSystem->FireEvent(m_clickEventName);
}

void Button::UpdateVertices()
{
	m_bkgVerts.clear();
	Vec2 bkgMins = m_position + m_bkgBox.m_mins;
	Vec2 bkgMaxs = m_position + m_bkgBox.m_maxs;
	AddVertsForAABB2D(m_bkgVerts, AABB2(bkgMins, bkgMaxs),Rgba8::WHITE);

	m_textVerts.clear();
	Vec2 textMins = m_position + m_textBox.m_mins;
	Vec2 textMaxs = m_position + m_textBox.m_maxs;
	m_font->AddVertsForTextInBox2D(m_textVerts, m_text, AABB2(textMins, textMaxs), m_textHeight, Rgba8::WHITE);
}
