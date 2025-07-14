#pragma once
#include <vector>
#include "InventoryItem.hpp"

class Player;

class Inventory
{
public:
	Inventory(Player* curPlayer);
	~Inventory();

	bool AddItem(InventoryItemDef const* itemDef, int count);
	bool RemoveItem(InventoryItemDef const* itemDef, int count);

public:
	Player* m_curPlayer = nullptr;
	std::vector<InventoryItem> m_items;

	int m_totalSlotsCount = 36;
};