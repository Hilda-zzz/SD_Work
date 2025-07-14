#pragma once

class InventoryItemDef;

class InventoryItem 
{
public:
	InventoryItem(InventoryItemDef const* def, int quantity);



public:
	InventoryItemDef const* m_itemDef;
	int m_quantity=0;                     
	int m_durability=0;                   

// 	bool CanStackWith(const InventoryItem& other) const;
// 	bool IsEmpty() const { return m_itemDef == nullptr || m_quantity <= 0; }
};