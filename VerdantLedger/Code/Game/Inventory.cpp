#include "Inventory.hpp"
#include "InventoryItem.hpp"

Inventory::Inventory(Player* curPlayer):m_curPlayer(curPlayer)
{
}

Inventory::~Inventory()
{
}

bool Inventory::AddItem(InventoryItemDef const* itemDef, int count)
{
	if (!itemDef)
	{
		return false;
	}

	bool success = false;

	for (InventoryItem item : m_items)
	{
		if (item.m_itemDef == itemDef)
		{
			item.m_quantity += count;
			success = true;
		}
	}
	if (!success)
	{
		if ((int)m_items.size() < m_totalSlotsCount)
		{
			InventoryItem newItem = InventoryItem(itemDef,count);
			m_items.push_back(newItem);
			success = true;
		}
		else
		{
			success = false;
		}
	}
	if (success)
	{
		// update 2 panels
	}

	return success;
}
