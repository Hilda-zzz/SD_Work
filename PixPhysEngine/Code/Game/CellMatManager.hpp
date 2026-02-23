#pragma once
#include <unordered_map>
#include "Game/CellMatDef.hpp"
#include "CellMatBrush.hpp"

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
		s_materialDefs[CellMatType::MAT_STATIC_FILL].m_color = Rgba8::HILDA;

		// 沙子 - 标准颗粒物
		s_materialDefs[CellMatType::MAT_SAND] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SAND].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_SAND].m_friction = 0.6f;
		s_materialDefs[CellMatType::MAT_SAND].m_restitution = 0.4f;
		s_materialDefs[CellMatType::MAT_SAND].m_collisionMomentumTransfer = 0.0f;
		s_materialDefs[CellMatType::MAT_SAND].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_SAND].m_color = Rgba8(245, 164, 96);
		s_materialDefs[CellMatType::MAT_SAND].m_name = "Sand";
		s_materialDefs[CellMatType::MAT_SAND].m_description = "Standard granular material";

		// 盐 - 更细小的颗粒，更容易流动
		s_materialDefs[CellMatType::MAT_SALT] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SALT].m_density = 2.2f;
		s_materialDefs[CellMatType::MAT_SALT].m_friction = 0.4f;
		s_materialDefs[CellMatType::MAT_SALT].m_restitution = 0.1f;
		s_materialDefs[CellMatType::MAT_SALT].m_neighborActivationChance = 0.6f;
		s_materialDefs[CellMatType::MAT_SALT].m_interaction.m_isSoluble = true;
		s_materialDefs[CellMatType::MAT_SALT].m_color = Rgba8(248, 248, 255);
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
		s_materialDefs[CellMatType::MAT_SOIL].m_color = Rgba8(101, 67, 33);
		s_materialDefs[CellMatType::MAT_SOIL].m_name = "Soil";
		s_materialDefs[CellMatType::MAT_SOIL].m_description = "Rich earth material - sticky";

		// 碎石 - 小石子，密度大，流动性好
		s_materialDefs[CellMatType::MAT_GRAVEL] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_GRAVEL].m_density = 2.8f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_friction = 0.5f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_restitution = 0.6f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_collisionMomentumTransfer = 0.2f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_neighborActivationChance = 0.8f;
		s_materialDefs[CellMatType::MAT_GRAVEL].m_color = Rgba8(128, 128, 128);
		s_materialDefs[CellMatType::MAT_GRAVEL].m_name = "Gravel";
		s_materialDefs[CellMatType::MAT_GRAVEL].m_description = "Small stones - bouncy";

		// 颗粒火
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_friction = 0.5f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_restitution = 0.6f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_collisionMomentumTransfer = 0.2f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_neighborActivationChance = 0.8f;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_color = Rgba8(180, 20, 20);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_name = "MAT_DYSOLID_FIRE";
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_description = "Fire transited from dynamic solid material";

		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_isHighTemp = true;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_lifeCountDown =IntRange(120,360);
		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_lifeEndMatType = CellMatType::MAT_EMPTY;

		s_materialDefs[CellMatType::MAT_DYSOLID_FIRE].m_emissionValue = 30;

		// 水 - 液体
		s_materialDefs[CellMatType::MAT_WATER] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_WATER].m_density = 1.0f;
		s_materialDefs[CellMatType::MAT_WATER].m_friction = 0.1f;
		s_materialDefs[CellMatType::MAT_WATER].m_viscosity = 0.8f;
		s_materialDefs[CellMatType::MAT_WATER].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_WATER].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_WATER].m_color = Rgba8(30, 144, 255,50);
		s_materialDefs[CellMatType::MAT_WATER].m_name = "Water";
		s_materialDefs[CellMatType::MAT_WATER].m_description = "Fluid material - liquid";

		s_materialDefs[CellMatType::MAT_WATER].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_WATER].m_collisionMomentumTransfer = 0.1f;
		s_materialDefs[CellMatType::MAT_WATER].m_horizontalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_WATER].m_verticalDamping = 0.99f;
		s_materialDefs[CellMatType::MAT_WATER].m_collisionDamping = 0.9f;

		s_materialDefs[CellMatType::MAT_WATER].m_emissionValue = 10;

		// oil - 液体
		s_materialDefs[CellMatType::MAT_OIL] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_OIL].m_density = 0.8f;
		s_materialDefs[CellMatType::MAT_OIL].m_friction = 0.1f;
		s_materialDefs[CellMatType::MAT_OIL].m_viscosity = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_OIL].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_OIL].m_color = Rgba8(144, 144, 10);
		s_materialDefs[CellMatType::MAT_OIL].m_name = "Oil";
		s_materialDefs[CellMatType::MAT_OIL].m_description = "Fluid material - liquid";

		s_materialDefs[CellMatType::MAT_OIL].m_neighborActivationChance = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_collisionMomentumTransfer = 0.1f;
		s_materialDefs[CellMatType::MAT_OIL].m_horizontalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_verticalDamping = 0.9f;
		s_materialDefs[CellMatType::MAT_OIL].m_collisionDamping = 0.5f;

		// lava 液体
		s_materialDefs[CellMatType::MAT_LAVA] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_LAVA].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_LAVA].m_friction = 0.3f;
		s_materialDefs[CellMatType::MAT_LAVA].m_viscosity = 0.6f;
		s_materialDefs[CellMatType::MAT_LAVA].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_LAVA].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_LAVA].m_color = Rgba8(255, 100, 30);
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
		s_materialDefs[CellMatType::MAT_LAVA].m_lifeEndMatType = CellMatType::MAT_SOIL;

		s_materialDefs[CellMatType::MAT_LAVA].m_emissionValue = 30;

		// Acid
		s_materialDefs[CellMatType::MAT_ACID] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_ACID].m_density = 1.2f;
		s_materialDefs[CellMatType::MAT_ACID].m_friction = 0.3f;
		s_materialDefs[CellMatType::MAT_ACID].m_viscosity = 0.6f;
		s_materialDefs[CellMatType::MAT_ACID].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_ACID].m_interaction.m_penetrationResistance = 0.8f;
		s_materialDefs[CellMatType::MAT_ACID].m_color = Rgba8(30, 200, 30);
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
		s_materialDefs[CellMatType::MAT_STONE].m_color = Rgba8(105, 105, 105);
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
		s_materialDefs[CellMatType::MAT_WOOD].m_color = Rgba8(139, 69, 19);
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
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_color = Rgba8::MAGNETA;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_name = "STATIC_SOLID_FIRE";
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_description = "Fire transited from static solid material";

		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_isHighTemp = true;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_lifeCountDown = IntRange(120, 240);
		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_lifeEndMatType = CellMatType::MAT_EMPTY;

		s_materialDefs[CellMatType::MAT_STSOLID_FIRE].m_emissionValue = 30;


		// CA Sand
		s_materialDefs[CellMatType::MAT_CA_SAND] = CellMatDef(PhyType::PHY_CELLULAR_AUTOMATON);
		s_materialDefs[CellMatType::MAT_CA_SAND].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_CA_SAND].m_color = Rgba8(146,205,255);
		s_materialDefs[CellMatType::MAT_CA_SAND].m_name = "CA Sand";
		s_materialDefs[CellMatType::MAT_CA_SAND].m_description = "Cellular automaton sand";
		s_materialDefs[CellMatType::MAT_CA_SAND].m_lifeCountDown = IntRange(240, 360);
		s_materialDefs[CellMatType::MAT_CA_SAND].m_isPersist = false;
		s_materialDefs[CellMatType::MAT_CA_SAND].m_lifeEndMatType = CellMatType::MAT_WATER;
		//s_materialDefs[CellMatType::MAT_CA_SAND].m_cellularAutomaton.m_diagonalCheckOrder = 0;

		s_materialDefs[CellMatType::MAT_CA_SAND].m_emissionValue = 10;
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