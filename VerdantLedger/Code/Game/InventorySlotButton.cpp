#include "InventorySlotButton.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "InventoryItem.hpp"
#include "InventoryItemDef.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"

extern AudioSystem* g_theAudio;
extern EventSystem* g_theEventSystem;

void InventorySlotButton::UpdateFromInventoryItem(InventoryItem* item)
{
	if (item)
	{
		m_item = item;
		m_iconTexture = item->m_itemDef->m_iconTexture;
		m_text = std::to_string(20);//m_item->m_quantity
	}
	UpdateVertices();
}

InventorySlotButton::InventorySlotButton(GameUISystem* uiSystem, const Vec2& position,
	Texture* normalTex, Texture* hoverTex, Texture* clickTex, 
	Texture* iconTexture, Texture* borderTexture, 
	AABB2 bkgExtent, AABB2 textExtent, std::string text, float textHeight, 
	BitmapFont* font, std::string clickEventName)
	: Button(uiSystem, position, normalTex, hoverTex, clickTex, bkgExtent, textExtent, text, textHeight, font, clickEventName)
{
	//UpdateVertices();

	m_iconTexture = iconTexture;
	m_borderTexture = borderTexture;
	m_slotIndex = -1;  
// 	m_currentItem = nullptr;
// 	m_itemCount = 0;
// 	m_isSelected = false;
// 	m_isHotbarSlot = false;
}

void InventorySlotButton::UpdateVertices()
{
	m_bkgVerts.clear();
	Vec2 bkgMins = m_position + m_bkgBox.m_mins;
	Vec2 bkgMaxs = m_position + m_bkgBox.m_maxs;
	AddVertsForAABB2D(m_bkgVerts, AABB2(bkgMins, bkgMaxs), Rgba8::WHITE);

	m_iconVerts.clear();
	if (m_item)
	{
		AABB2 iconUV = m_item->m_itemDef->GetIconUV();
		AddVertsForAABB2D(m_iconVerts, AABB2(bkgMins, bkgMaxs), Rgba8::WHITE, iconUV.m_mins, iconUV.m_maxs);
	}

	m_boarderVerts.clear();
	AddVertsForAABB2D(m_boarderVerts, AABB2(bkgMins, bkgMaxs), Rgba8::WHITE);

	m_textVerts.clear();
	Vec2 textMins = m_position + m_textBox.m_mins;
	Vec2 textMaxs = m_position + m_textBox.m_maxs;
	m_font->AddVertsForTextInBox2D(m_textVerts, m_text, AABB2(textMins, textMaxs), m_textHeight, Rgba8::WHITE);
}

void InventorySlotButton::FireClickEvent()
{
	g_theAudio->StartSound(m_clickSound);
	EventArgs args;
	args.SetValue("slotIndex", std::to_string(m_slotIndex));
	g_theEventSystem->FireEvent(m_clickEventName,args);
}

void InventorySlotButton::Render(Renderer* renderer) const
{
 	renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
 	renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
 	renderer->SetBlendMode(BlendMode::ALPHA);
 	renderer->BindShader(nullptr);
 	renderer->SetModelConstants();
 
 	renderer->BindTexture(m_curTex);
 	renderer->DrawVertexArray(m_bkgVerts);
 
 	// icon
 	renderer->BindTexture(m_iconTexture);
 	renderer->DrawVertexArray(m_iconVerts);
 
  	if (m_isSelected)
  	{
  		renderer->BindTexture(m_borderTexture);
  		renderer->DrawVertexArray(m_boarderVerts);
  	}

	renderer->BindTexture(&m_font->GetTexture());
	renderer->DrawVertexArray(m_textVerts);
}
