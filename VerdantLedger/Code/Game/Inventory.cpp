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

	// 计算实际占用的槽位数
	int occupiedSlots = 0;
	for (const InventoryItem& item : m_items)
	{
		if (item.m_itemDef != nullptr)
		{
			occupiedSlots++;
		}
	}

	bool success = false;

	// 先尝试叠加到现有物品
	for (InventoryItem& item : m_items)
	{
		if (item.m_itemDef == itemDef)
		{
			item.m_quantity += count;
			success = true;
			break; // 找到就退出
		}
	}

	if (!success)
	{
		// 检查是否还有空间添加新物品
		if (occupiedSlots >= m_totalSlotsCount)
		{
			// 背包真的满了
			return false;
		}

		// 尝试使用空槽位
		for (InventoryItem& item : m_items)
		{
			if (item.m_itemDef == nullptr)
			{
				item.m_itemDef = itemDef;
				item.m_quantity = count;
				success = true;
				break;
			}
		}

		// 如果没有空槽位但容器还没达到最大大小，添加新项
		if (!success && (int)m_items.size() < m_totalSlotsCount)
		{
			InventoryItem newItem = InventoryItem(itemDef, count);
			m_items.push_back(newItem);
			success = true;
		}
	}

	if (success)
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

