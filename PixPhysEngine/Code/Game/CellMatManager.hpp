#pragma once
#include <unordered_map>
#include "Game/CellMatDef.hpp"
#include "CellMatBrush.hpp"
#include "CABehaviors.hpp"


class CellMatManager
{
public:
	CellMatManager();

	static void InitializeMaterials() {
		// 空气/真空
		s_materialDefs[CellMatType::MAT_EMPTY] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_EMPTY].m_density = 0.0f;
		s_materialDefs[CellMatType::MAT_EMPTY].m_name = "Eraser";
		s_materialDefs[CellMatType::MAT_EMPTY].m_description = "Tool - Remove material";

		// 静态刚体填充
		s_materialDefs[CellMatType::MAT_STATIC_FILL] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_STATIC_FILL].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_STATIC_FILL].m_name = "MAT_STATIC_FILL";
		s_materialDefs[CellMatType::MAT_STATIC_FILL].m_description = "Tool - Remove material";
		s_materialDefs[CellMatType::MAT_STATIC_FILL].m_colorMin = Rgba8::HILDA;
		s_materialDefs[CellMatType::MAT_STATIC_FILL].m_colorMax = Rgba8::HILDA;

		// 沙子 - 标准颗粒物
		s_materialDefs[CellMatType::MAT_SAND] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SAND].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_SAND].m_friction = 0.6f;
		s_materialDefs[CellMatType::MAT_SAND].m_restitution = 0.4f;
		s_materialDefs[CellMatType::MAT_SAND].m_collisionMomentumTransfer = 0.0f;
		s_materialDefs[CellMatType::MAT_SAND].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_SAND].m_colorMin = Rgba8(220, 140, 70);
		s_materialDefs[CellMatType::MAT_SAND].m_colorMax = Rgba8(255, 190, 120);
		s_materialDefs[CellMatType::MAT_SAND].m_name = "Sand";
		s_materialDefs[CellMatType::MAT_SAND].m_description = "Standard granular material";

		// 盐 - 更细小的颗粒，更容易流动
		s_materialDefs[CellMatType::MAT_SALT] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SALT].m_density = 2.2f;
		s_materialDefs[CellMatType::MAT_SALT].m_friction = 0.4f;
		s_materialDefs[CellMatType::MAT_SALT].m_restitution = 0.1f;
		s_materialDefs[CellMatType::MAT_SALT].m_neighborActivationChance = 0.6f;
		s_materialDefs[CellMatType::MAT_SALT].m_interaction.m_isSoluble = true;
		s_materialDefs[CellMatType::MAT_SALT].m_colorMin = Rgba8(220, 225, 235);
		s_materialDefs[CellMatType::MAT_SALT].m_colorMax = Rgba8(255, 255, 255);
		s_materialDefs[CellMatType::MAT_SALT].m_name = "Salt";
		s_materialDefs[CellMatType::MAT_SALT].m_description = "Fine granular material - flows easily";

		s_materialDefs[CellMatType::MAT_SALT].m_isDissolve = true;
		s_materialDefs[CellMatType::MAT_SALT].m_dissolveCountDowm = IntRange(30, 180);
		s_materialDefs[CellMatType::MAT_SALT].m_dissolveType = CellMatType::MAT_EMPTY;

		// 土壤 - 比沙子更粘稠，含有有机物
		s_materialDefs[CellMatType::MAT_SOIL] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SOIL].m_density = 1.9f;
		s_materialDefs[CellMatType::MAT_SOIL].m_friction = 0.8f;
		s_materialDefs[CellMatType::MAT_SOIL].m_restitution = 0.1f;
		s_materialDefs[CellMatType::MAT_SOIL].m_collisionMomentumTransfer = 0.05f;
		s_materialDefs[CellMatType::MAT_SOIL].m_neighborActivationChance = 0.4f;
		s_materialDefs[CellMatType::MAT_SOIL].m_airResistance = 0.85f;
		s_materialDefs[CellMatType::MAT_SOIL].m_colorMin = Rgba8(70, 40, 15);
		s_materialDefs[CellMatType::MAT_SOIL].m_colorMax = Rgba8(130, 85, 45);
		s_materialDefs[CellMatType::MAT_SOIL].m_name = "Soil";
		s_materialDefs[CellMatType::MAT_SOIL].m_description = "Rich earth material - sticky";

		// 碎石 - 小石子，密度大，流动性好
		s_materialDefs[CellMatType::MAT_GRAVEL] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_GRAVEL].m_density = 2.8f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_friction = 0.5f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_restitution = 0.6f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_collisionMomentumTransfer = 0.2f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_neighborActivationChance = 0.8f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_colorMin = Rgba8(90, 90, 90);
		s_materialDefs[CellMatType::MAT_GRAVEL].m_colorMax = Rgba8(170, 170, 165);
		s_materialDefs[CellMatType::MAT_GRAVEL].m_name = "Gravel";
		s_materialDefs[CellMatType::MAT_GRAVEL].m_description = "Small stones - bouncy";

		// 黑曜石 - 岩浆冷却后的移动固体，重、不弹、几乎不流动
		s_materialDefs[CellMatType::MAT_OBSIDIAN] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_density = 3.5f;
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_friction = 0.9f;
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_restitution = 0.05f;          // 极低弹性，和gravel(0.6)形成对比
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_collisionMomentumTransfer = 0.05f;
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_neighborActivationChance = 0.2f; // 很难激活邻居，几乎堆死
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_airResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_collisionDamping = 0.2f;
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_colorMin = Rgba8(20, 10, 25);
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_colorMax = Rgba8(60, 40, 70); // 深紫黑，带一点岩浆余温的紫调
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_name = "Obsidian";
		s_materialDefs[CellMatType::MAT_OBSIDIAN].m_description = "Cooled lava - dense and inert";

		// 颗粒火
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_friction = 0.5f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_restitution = 0.6f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_collisionMomentumTransfer = 0.2f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_neighborActivationChance = 0.8f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_colorMin = Rgba8(180, 20, 0);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_colorMax = Rgba8(255, 120, 20);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_name = "MAT_DYSOLID_FIRE";
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_description = "Fire transited from dynamic solid material";

		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_isHighTemp = true;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_lifeCountDown =IntRange(120,360);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_lifeEndMatType = CellMatType::MAT_CA_SMOKE;

		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_emissionValue = 30;

		// 水 - 液体
		s_materialDefs[CellMatType::MAT_WATER] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_WATER].m_density = 1.0f;
		s_materialDefs[CellMatType::MAT_WATER].m_friction = 0.1f;
		s_materialDefs[CellMatType::MAT_WATER].m_viscosity = 0.8f;
		s_materialDefs[CellMatType::MAT_WATER].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_WATER].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_WATER].m_colorMin = Rgba8(20, 100, 200);
		s_materialDefs[CellMatType::MAT_WATER].m_colorMax = Rgba8(20, 100, 200);
		s_materialDefs[CellMatType::MAT_WATER].m_name = "Water";
		s_materialDefs[CellMatType::MAT_WATER].m_description = "Fluid material - liquid";

		s_materialDefs[CellMatType::MAT_WATER].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_WATER].m_collisionMomentumTransfer = 0.1f;
		s_materialDefs[CellMatType::MAT_WATER].m_horizontalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_WATER].m_verticalDamping = 0.99f;
		s_materialDefs[CellMatType::MAT_WATER].m_collisionDamping = 0.9f;

		s_materialDefs[CellMatType::MAT_WATER].m_emissionValue = 10;

		s_materialDefs[CellMatType::MAT_WATER].m_isFlammable = true;
		s_materialDefs[CellMatType::MAT_WATER].m_flameCountDown = IntRange(1, 5);
		s_materialDefs[CellMatType::MAT_WATER].m_flammableType = CellMatType::MAT_CA_STEAM;

		// oil - 液体
		s_materialDefs[CellMatType::MAT_OIL] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_OIL].m_density = 0.8f;
		s_materialDefs[CellMatType::MAT_OIL].m_friction = 0.1f;
		s_materialDefs[CellMatType::MAT_OIL].m_viscosity = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_OIL].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_OIL].m_colorMin = Rgba8(100, 100, 5);
		s_materialDefs[CellMatType::MAT_OIL].m_colorMax = Rgba8(100, 100, 5);
		s_materialDefs[CellMatType::MAT_OIL].m_name = "Oil";
		s_materialDefs[CellMatType::MAT_OIL].m_description = "Fluid material - liquid";

		s_materialDefs[CellMatType::MAT_OIL].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_collisionMomentumTransfer = 0.1f;
		s_materialDefs[CellMatType::MAT_OIL].m_horizontalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_verticalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_collisionDamping = 0.5f;

		s_materialDefs[CellMatType::MAT_OIL].m_isFlammable = true;
		s_materialDefs[CellMatType::MAT_OIL].m_flameCountDown = IntRange(1, 10);
		s_materialDefs[CellMatType::MAT_OIL].m_flammableType = CellMatType::MAT_CA_FIRE;

		// lava 液体
		s_materialDefs[CellMatType::MAT_LAVA] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_LAVA].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_LAVA].m_friction = 0.3f;
		s_materialDefs[CellMatType::MAT_LAVA].m_viscosity = 0.6f;
		s_materialDefs[CellMatType::MAT_LAVA].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_LAVA].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_LAVA].m_colorMin = Rgba8(200, 50, 10);
		s_materialDefs[CellMatType::MAT_LAVA].m_colorMax = Rgba8(255, 150, 40);
		s_materialDefs[CellMatType::MAT_LAVA].m_name = "Lava";
		s_materialDefs[CellMatType::MAT_LAVA].m_description = "Fluid material - liquid";

		s_materialDefs[CellMatType::MAT_LAVA].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_LAVA].m_collisionMomentumTransfer = 0.1f;
		s_materialDefs[CellMatType::MAT_LAVA].m_horizontalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_LAVA].m_verticalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_LAVA].m_collisionDamping = 0.5f;

		s_materialDefs[CellMatType::MAT_LAVA].m_isHighTemp = true;
		s_materialDefs[CellMatType::MAT_LAVA].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_LAVA].m_lifeCountDown = IntRange(360, 720);
		s_materialDefs[CellMatType::MAT_LAVA].m_lifeEndMatType = CellMatType::MAT_OBSIDIAN;

		s_materialDefs[CellMatType::MAT_LAVA].m_emissionValue = 30;

		// Acid
		s_materialDefs[CellMatType::MAT_ACID] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_ACID].m_density = 1.2f;
		s_materialDefs[CellMatType::MAT_ACID].m_friction = 0.3f;
		s_materialDefs[CellMatType::MAT_ACID].m_viscosity = 0.6f;
		s_materialDefs[CellMatType::MAT_ACID].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_ACID].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_ACID].m_colorMin = Rgba8(80, 160, 20);
		s_materialDefs[CellMatType::MAT_ACID].m_colorMax = Rgba8(80, 160, 20);
		s_materialDefs[CellMatType::MAT_ACID].m_name = "Acid";
		s_materialDefs[CellMatType::MAT_ACID].m_description = "Fluid material - liquid";
										
		s_materialDefs[CellMatType::MAT_ACID].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_ACID].m_collisionMomentumTransfer = 0.1f;
		s_materialDefs[CellMatType::MAT_ACID].m_horizontalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_ACID].m_verticalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_ACID].m_collisionDamping = 0.5f;

		s_materialDefs[CellMatType::MAT_ACID].m_isAcid = true;

		s_materialDefs[CellMatType::MAT_ACID].m_emissionValue = 20;


		// 石头 - 静态固体
		s_materialDefs[CellMatType::MAT_STONE] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_STONE].m_density = 3.0f;
		s_materialDefs[CellMatType::MAT_STONE].m_friction = 0.8f;
		s_materialDefs[CellMatType::MAT_STONE].m_restitution = 0.1f;
		s_materialDefs[CellMatType::MAT_STONE].m_colorMin = Rgba8(75, 75, 75);
		s_materialDefs[CellMatType::MAT_STONE].m_colorMax = Rgba8(140, 140, 135);
		s_materialDefs[CellMatType::MAT_STONE].m_name = "Stone";
		s_materialDefs[CellMatType::MAT_STONE].m_description = "Static obstacle - hard solid";

		s_materialDefs[CellMatType::MAT_STONE].m_isCorroded = true;
		s_materialDefs[CellMatType::MAT_STONE].m_corrosionCountDown = IntRange(60, 80);
		s_materialDefs[CellMatType::MAT_STONE].m_corrodeType = CellMatType::MAT_EMPTY;

		// 木头 - 静态固体
		s_materialDefs[CellMatType::MAT_WOOD] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_WOOD].m_density = 1.8f;
		s_materialDefs[CellMatType::MAT_WOOD].m_friction = 0.7f;
		s_materialDefs[CellMatType::MAT_WOOD].m_restitution = 0.4f;
		s_materialDefs[CellMatType::MAT_WOOD].m_colorMin = Rgba8(100, 45, 10);
		s_materialDefs[CellMatType::MAT_WOOD].m_colorMax = Rgba8(175, 100, 35);
		s_materialDefs[CellMatType::MAT_WOOD].m_name = "Wood";
		s_materialDefs[CellMatType::MAT_WOOD].m_description = "Building material - light solid";
		
		s_materialDefs[CellMatType::MAT_WOOD].m_isFlammable = true;
		s_materialDefs[CellMatType::MAT_WOOD].m_flameCountDown = IntRange(20, 120);
		s_materialDefs[CellMatType::MAT_WOOD].m_flammableType = CellMatType::MAT_STSOLID_FIRE;

		s_materialDefs[CellMatType::MAT_WOOD].m_isCorroded = true;
		s_materialDefs[CellMatType::MAT_WOOD].m_corrosionCountDown = IntRange(20, 80);
		s_materialDefs[CellMatType::MAT_WOOD].m_corrodeType = CellMatType::MAT_EMPTY;

		// 静态固体 - 火焰
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_density = 1.8f;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_friction = 0.1f;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_restitution = 0.1f;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_colorMin = Rgba8(180, 0, 10);
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_colorMax = Rgba8(255, 80, 20);
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_name = "STATIC_SOLID_FIRE";
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_description = "Fire transited from static solid material";

		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_isHighTemp = true;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_lifeCountDown = IntRange(120, 240);
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_lifeEndMatType = CellMatType::MAT_CA_SMOKE;

		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_emissionValue = 30;


		// CA Snow
		s_materialDefs[CellMatType::MAT_CA_SNOW] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_density = 1.2f;
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_colorMin = Rgba8(180, 200, 240, 220);
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_colorMax = Rgba8(230, 240, 255, 255);
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_name = "CA Snow";
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_description = "Cellular automaton snow";
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_lifeCountDown = IntRange(240, 360);
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_lifeEndMatType = CellMatType::MAT_WATER;
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_caUpdateFunc = &CABehaviors::Update_Snow;
		s_materialDefs[CellMatType::MAT_CA_SNOW].m_emissionValue = 10;

		// CA Cloud
		s_materialDefs[CellMatType::MAT_CA_CLOUD] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_density = 0.3f;
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_colorMin = Rgba8(220, 220, 220, 210);
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_colorMax = Rgba8(150, 190, 240, 220);
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_name = "CA Cloud";
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_description = "Cellular automaton cloud";
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_lifeCountDown = IntRange(600, 1200);
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_lifeEndMatType = CellMatType::MAT_WATER;
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_caUpdateFunc = &CABehaviors::Update_Cloud;
		s_materialDefs[CellMatType::MAT_CA_CLOUD].m_emissionValue = 5;

		// CA Steam
		s_materialDefs[CellMatType::MAT_CA_STEAM] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_density = 0.1f;
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_colorMin = Rgba8(160, 220, 210, 100);
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_colorMax = Rgba8(200, 245, 240, 150);
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_name = "CA Steam";
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_description = "Cellular automaton steam";
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_lifeCountDown = IntRange(60, 100);
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_lifeEndMatType = CellMatType::MAT_EMPTY;
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_caUpdateFunc = &CABehaviors::Update_Steam;
		s_materialDefs[CellMatType::MAT_CA_STEAM].m_emissionValue = 3;

		// CA Plant
		s_materialDefs[CellMatType::MAT_CA_PLANT] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_density = 2.f;
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_colorMin = Rgba8(30, 160, 120, 255);  // 青绿
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_colorMax = Rgba8(100, 210, 80, 255);  // 嫩绿
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_name = "CA Plant";
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_description = "Grows in water, spreads upward";
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_isPersist = true;
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_isFlammable = true;
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_flameCountDown = IntRange(80, 160);
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_flammableType = CellMatType::MAT_STSOLID_FIRE;
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_caUpdateFunc = &CABehaviors::Update_Plant;
		s_materialDefs[CellMatType::MAT_CA_PLANT].m_emissionValue = 0;

		// CA Flower
		s_materialDefs[CellMatType::MAT_CA_FLOWER] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_density = 2.0f;
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_colorMin = Rgba8(220, 40, 60, 255);  // 注册颜色不重要
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_colorMax = Rgba8(240, 80, 160, 255);  // 运行时动态赋色
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_name = "CA Flower";
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_description = "Blooms from plant, fades outward";
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_lifeCountDown = IntRange(300, 600);
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_lifeEndMatType = CellMatType::MAT_CA_PLANT;
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_isFlammable = true;
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_flameCountDown = IntRange(40, 80);
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_flammableType = CellMatType::MAT_STSOLID_FIRE;
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_caUpdateFunc = &CABehaviors::Update_Flower;
		s_materialDefs[CellMatType::MAT_CA_FLOWER].m_emissionValue = 5;

		// CA Smoke
		s_materialDefs[CellMatType::MAT_CA_SMOKE] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_density = 0.2f;
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_colorMin = Rgba8(60, 60, 60, 180);
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_colorMax = Rgba8(110, 110, 110, 255);
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_name = "CA Smoke";
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_description = "Drifts upward, disperses slowly";
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_lifeCountDown = IntRange(100, 150);
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_lifeEndMatType = CellMatType::MAT_EMPTY;
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_caUpdateFunc = &CABehaviors::Update_Smoke;
		s_materialDefs[CellMatType::MAT_CA_SMOKE].m_emissionValue = 2;

		// CA Vine
		s_materialDefs[CellMatType::MAT_CA_VINE] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_VINE].m_density = 2.f;
		s_materialDefs[CellMatType::MAT_CA_VINE].m_colorMin = Rgba8(30, 90, 50, 255);  // 深绿主体
		s_materialDefs[CellMatType::MAT_CA_VINE].m_colorMax = Rgba8(50, 120, 70, 255);
		s_materialDefs[CellMatType::MAT_CA_VINE].m_name = "CA Vine";
		s_materialDefs[CellMatType::MAT_CA_VINE].m_description = "Grows tendrils downward";
		s_materialDefs[CellMatType::MAT_CA_VINE].m_isPersist = true;
		s_materialDefs[CellMatType::MAT_CA_VINE].m_isFlammable = true;
		s_materialDefs[CellMatType::MAT_CA_VINE].m_flameCountDown = IntRange(60, 120);
		s_materialDefs[CellMatType::MAT_CA_VINE].m_flammableType = CellMatType::MAT_STSOLID_FIRE;
		s_materialDefs[CellMatType::MAT_CA_VINE].m_caUpdateFunc = &CABehaviors::Update_Vine;
		s_materialDefs[CellMatType::MAT_CA_VINE].m_emissionValue = 0;

		// CA Seed
		s_materialDefs[CellMatType::MAT_CA_SEED] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_SEED].m_density = 2.f;
		s_materialDefs[CellMatType::MAT_CA_SEED].m_colorMin = Rgba8(120, 80, 40, 255);  // 深棕
		s_materialDefs[CellMatType::MAT_CA_SEED].m_colorMax = Rgba8(160, 110, 60, 255);  // 浅棕
		s_materialDefs[CellMatType::MAT_CA_SEED].m_name = "CA Seed";
		s_materialDefs[CellMatType::MAT_CA_SEED].m_description = "Falls like sand, sprouts into plant";
		s_materialDefs[CellMatType::MAT_CA_SEED].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_SEED].m_lifeCountDown = IntRange(-1, -1);  // 不自然消亡，靠发芽转化
		s_materialDefs[CellMatType::MAT_CA_SEED].m_caUpdateFunc = &CABehaviors::Update_Seed;
		s_materialDefs[CellMatType::MAT_CA_SEED].m_emissionValue = 0;

		// CA Fire
		s_materialDefs[CellMatType::MAT_CA_FIRE] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_density = 0.1f;
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_colorMin = Rgba8(255, 80, 10, 200);  // 深橙红
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_colorMax = Rgba8(255, 200, 50, 240);  // 亮黄
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_name = "CA Fire";
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_description = "Swaying fire, rises like fluid";
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_lifeCountDown = IntRange(60, 150);
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_lifeEndMatType = CellMatType::MAT_CA_SMOKE;
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_isHighTemp = true;
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_caUpdateFunc = &CABehaviors::Update_FireCA;
		s_materialDefs[CellMatType::MAT_CA_FIRE].m_emissionValue = 25;
	}
	static void InitializeMaterialUIInfo();

	static const CellMatDef& GetMaterialDef(CellMatType matType) {
		return s_materialDefs[matType];
	}

	static CellMatDef& GetMutableMaterialDef(CellMatType matType) {
		return s_materialDefs[matType];
	}

	// 新增：获取所有材质定义的方法，用于UI自动化
	static const std::unordered_map<CellMatType, CellMatDef>& GetAllMaterialDefs();

private:
	
	static const char* GetPhysicsTypeName(PhyType physType);
public:
	static std::unordered_map<CellMatType, CellMatUIInfo> s_materialUIInfo;

private:
	static std::unordered_map<CellMatType, CellMatDef> s_materialDefs;

};