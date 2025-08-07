#pragma once
#include "Engine/GameUISystem/Button.hpp"

class InventoryItem;
class GameUISystem;

class InventorySlotButton : public Button
{
public:
	InventorySlotButton() {}

	void UpdateFromInventoryItem(InventoryItem* item);

	InventorySlotButton(GameUISystem* uiSystem, const Vec2& position, 
		Texture* normalTex, Texture* hoverTex, Texture* clickTex, Texture* iconTexture, Texture* borderTexture,
		AABB2 bkgExtent, AABB2 textExtent, std::string text, float textHeight, BitmapFont* font, std::string clickEventName);

	void UpdateVertices() override;
	void FireClickEvent() override;

	void Render(Renderer* renderer) const override;
	void SetSlotIndex(int index) { m_slotIndex = index; }
	void SetSelectedState(bool isSelected) { m_isSelected = isSelected; }

	InventoryItem* GetItem() { return m_item; }

	void UseInventoryItem(int count);

	void SetIconTexture(Texture* iconTex);

private:
	InventoryItem* m_item;
	int m_slotIndex = 0;
	Texture* m_iconTexture = nullptr;
	Texture* m_borderTexture = nullptr;

	std::vector<Vertex_PCU> m_iconVerts;
	std::vector<Vertex_PCU> m_boarderVerts;

	bool m_isSelected = false;
};