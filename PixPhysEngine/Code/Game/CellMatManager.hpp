#pragma once
#include <unordered_map>
#include "Game/CellMatDef.hpp"
class CellMatManager
{
public:
	CellMatManager() {}

	static void InitializeMaterials() {
		// 空气/真空
		s_materialDefs[CellMatType::MAT_EMPTY] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_EMPTY].m_density = 0.0f;

		// 沙子 - 标准颗粒物
		s_materialDefs[CellMatType::MAT_SAND] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SAND].m_density = 1.5f;
		s_materialDefs[CellMatType::MAT_SAND].m_friction = 0.6f;
		s_materialDefs[CellMatType::MAT_SAND].m_restitution = 0.2f;
		s_materialDefs[CellMatType::MAT_SAND].m_moveSolid.m_slideAngle = 35.0f;
		s_materialDefs[CellMatType::MAT_SAND].m_moveSolid.m_collisionMomentumTransfer = 0.15f;

		// 盐 - 更细小的颗粒，更容易流动
		s_materialDefs[CellMatType::MAT_SALT] = CellMatDef(PhyType::PHY_MOVE_SOLID);
		s_materialDefs[CellMatType::MAT_SALT].m_density = 2.2f;
		s_materialDefs[CellMatType::MAT_SALT].m_friction = 0.4f;
		s_materialDefs[CellMatType::MAT_SALT].m_restitution = 0.1f;
		s_materialDefs[CellMatType::MAT_SALT].m_moveSolid.m_slideAngle = 25.0f; // 更小的滑动角
		s_materialDefs[CellMatType::MAT_SALT].m_moveSolid.m_neighborActivationChance = 0.8f; // 更容易激活邻居
		s_materialDefs[CellMatType::MAT_SALT].m_interaction.m_isSoluble = true; // 可溶解

		// 水 - 液体
		s_materialDefs[CellMatType::MAT_WATER] = CellMatDef(PhyType::PHY_LIQUID);
		s_materialDefs[CellMatType::MAT_WATER].m_density = 1.0f;
		s_materialDefs[CellMatType::MAT_WATER].m_friction = 0.1f;
		s_materialDefs[CellMatType::MAT_WATER].m_viscosity = 0.8f; // 阻力系数
		s_materialDefs[CellMatType::MAT_WATER].m_interaction.m_isPermeable = true;
		s_materialDefs[CellMatType::MAT_WATER].m_interaction.m_penetrationResistance = 0.8f;

		// 石头 - 静态固体
		s_materialDefs[CellMatType::MAT_STONE] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_STONE].m_density = 3.0f;
		s_materialDefs[CellMatType::MAT_STONE].m_friction = 0.8f;
		s_materialDefs[CellMatType::MAT_STONE].m_restitution = 0.1f;

		// 木头 - 静态固体（可以考虑后续扩展为可破坏）
		s_materialDefs[CellMatType::MAT_WOOD] = CellMatDef(PhyType::PHY_STATIC_SOLID);
		s_materialDefs[CellMatType::MAT_WOOD].m_density = 0.6f;
		s_materialDefs[CellMatType::MAT_WOOD].m_friction = 0.7f;
		s_materialDefs[CellMatType::MAT_WOOD].m_restitution = 0.4f;
	}

	static const CellMatDef& GetMaterialDef(CellMatType matType) {
		return s_materialDefs[matType];
	}

	static CellMatDef& GetMutableMaterialDef(CellMatType matType) {
		return s_materialDefs[matType];
	}

private:
	static std::unordered_map<CellMatType, CellMatDef> s_materialDefs;
};