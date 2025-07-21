#include "Inventory.hpp"
#include "InventoryItem.hpp"
#include "InventoryItemDef.hpp"
#include "Engine/Core/EventSystem.hpp"

extern EventSystem* g_theEventSystem;

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

	for (InventoryItem& item : m_items)
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

	if(success)
	{
		// g_theEventSystem->FireEvent("UpdateInventoryPanels");
	}

	return success;
}

bool Inventory::RemoveItem(InventoryItemDef const* itemDef, int count)
{
	if (!itemDef)
	{
		return false;
	}

	bool success = false;

	for (InventoryItem& item : m_items)
	{
		if (item.m_itemDef == itemDef)
		{
			item.m_quantity -= count;
			if (item.m_quantity <= 0)
			{
				item.m_quantity = 0;
				// make it to none
				item.m_itemDef = nullptr;
			}
			success = true;
		}
	}
	if (!success)
	{
		if ((int)m_items.size() < m_totalSlotsCount)
		{
			InventoryItem newItem = InventoryItem(itemDef, count);
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
		// g_theEventSystem->FireEvent("UpdateInventoryPanels");
	}

	return success;
}

