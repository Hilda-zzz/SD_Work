#include "InventoryItem.hpp"
#include "Engine/Math/AABB2.hpp"

InventoryItem::InventoryItem(InventoryItemDef const* def, int quantity)
	:	m_itemDef(def),m_quantity(quantity)
{
	
}
