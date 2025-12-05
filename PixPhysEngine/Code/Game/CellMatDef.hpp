#pragma once
#include <stdint.h>
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntRange.hpp"

enum class PhyType :uint8_t
{
	PHY_STATIC_SOLID,
	PHY_MOVE_SOLID,
	PHY_LIQUID,
	PHY_CELLULAR_AUTOMATON
};

enum class CellMatType : uint8_t
{
	MAT_EMPTY,
	MAT_STATIC_FILL,
	MAT_SAND,
	MAT_WATER,
	MAT_SALT,
	MAT_STONE,
	MAT_WOOD,
	MAT_SOIL,
	MAT_GRAVEL,
	MAT_OIL,
	MAT_LAVA,
	MAT_ACID,
	MAT_STSOLID_FIRE,
	MAT_DYSOLID_FIRE,

	MAT_CA_SAND
};

class CellMatDef
{
public:

	CellMatDef() {}
	// 构造函数
	CellMatDef(PhyType type) : m_physicsType(type)
	{
		// 设置默认值
		m_density = 1.0f;
		m_friction = 0.5f;
		m_restitution = 0.3f;
		m_viscosity = 0.0f;
		m_gravityMultiplier = 1.0f;
		m_terminalVelocity = 200.0f;


		//---------------------------------------------
		m_collisionMomentumTransfer = 0.15f;
		m_activationThreshold = 0.8f;
		m_neighborActivationChance = 0.7f;
		m_canActivateNeighbors = true;

		m_airResistance = 0.95f;  //0.95
		m_collisionDamping = 0.6f;
		m_horizontalDamping = 0.1f;
		m_verticalDamping = 0.8f;

		m_momentumPreservation = 0.8f;
		m_randomDirectionChance = 0.5f;


		// 根据物理类型设置默认参数
		if (type == PhyType::PHY_MOVE_SOLID) {
			SetDefaultMoveSolidParams();
		}
		else if (type == PhyType::PHY_LIQUID) {
			SetDefaultLiquidParams();
		}
		else if (type == PhyType::PHY_CELLULAR_AUTOMATON)
		{
			SetDefaultCellularAutomatonParams();
		}

		SetDefaultInteractionParams();
	}

private:
	void SetDefaultMoveSolidParams() {
		//m_moveSolid.m_collisionMomentumTransfer = 0.15f;
		//m_moveSolid.m_activationThreshold = 0.8f;
		//m_moveSolid.m_neighborActivationChance = 0.7f;
		//m_moveSolid.m_canActivateNeighbors = true;

		//m_moveSolid.m_airResistance = 0.95f;  //0.95
		//m_moveSolid.m_collisionDamping = 0.6f;
		//m_moveSolid.m_horizontalDamping = 0.1f;
		//m_moveSolid.m_verticalDamping = 0.8f;

		//m_moveSolid.m_momentumPreservation = 0.8f; 
		//m_moveSolid.m_randomDirectionChance = 0.5f;
	}

	void SetDefaultLiquidParams() {
		m_liquid.m_flowRate = 1.0f;
		m_liquid.m_pressureInfluence = 0.5f;
		m_liquid.m_surfaceTension = 0.1f;
		m_liquid.m_canDisplace = true;
		m_liquid.m_displacementForce = 0.8f;
	}

	void SetDefaultCellularAutomatonParams() {
		m_gravityMultiplier = 0.0f;
		m_terminalVelocity = 0.0f;
		m_airResistance = 1.0f;
		m_collisionDamping = 1.0f;
		m_horizontalDamping = 1.0f;
		m_verticalDamping = 1.0f;
		//m_cellularAutomaton.m_diagonalCheckOrder = 0;
	}

	void SetDefaultInteractionParams() {
		m_interaction.m_penetrationResistance = 1.0f;
		m_interaction.m_dissolutionRate = 0.0f;
		m_interaction.m_isPermeable = false;
		m_interaction.m_isSoluble = false;
	}

public:
	PhyType m_physicsType=PhyType::PHY_MOVE_SOLID;

	std::string m_description = "";
	std::string m_name = "";

	// === Basic Phys ===
	float m_density;              // 密度 (影响重力和碰撞)
	float m_friction;             // 摩擦系数 (0.0-1.0)
	float m_restitution;          // 弹性系数 (0.0-1.0, 碰撞后保留的能量比例)
	float m_viscosity;            // 粘度 (仅液体有效, 影响阻力)
	float m_gravityMultiplier;			// 重力倍数 (1.0为标准重力)
	float m_terminalVelocity;			// 终端速度

	//-------------------------------------------
	float m_collisionMomentumTransfer;	// 碰撞动量转移率 (0.0-1.0)
	float m_activationThreshold;		// 激活邻居的速度阈值  =》这个值目前被当做自身“被外界激活的概率使用”
	float m_neighborActivationChance;	// 激活邻居的概率
	bool m_canActivateNeighbors;		// 是否能激活邻居颗粒

	// 运动衰减参数
	float m_airResistance;            // 空气阻力 (速度衰减率)
	float m_collisionDamping;         // 碰撞阻尼 (碰撞后速度保留率)
	float m_horizontalDamping;        // 水平移动阻尼
	float m_verticalDamping;          // 垂直碰撞阻尼

	// 方向性参数
	float m_momentumPreservation;     // 动量保持度 (影响移动连续性) =》用作碰撞后保留多少比例的momentum给接收碰撞的cell
	float m_randomDirectionChance;    // 随机方向概率 (无水平速度时)

	Rgba8 m_color=Rgba8::HILDA;

	// Chemical Params
	bool m_isHighTemp = false;
	bool m_isPersist = true;
	IntRange m_lifeCountDown = IntRange(-1,-1);
	CellMatType m_lifeEndMatType = CellMatType::MAT_EMPTY;

	bool m_isFlammable = false;
	IntRange m_flameCountDown = IntRange(-1, -1);
	CellMatType m_flammableType = CellMatType::MAT_EMPTY;

	bool m_isDissolve = false;
	IntRange m_dissolveCountDowm = IntRange(-1, -1);
	CellMatType m_dissolveType = CellMatType::MAT_EMPTY;

	bool m_isAcid = false;
	bool m_isCorroded = false;
	IntRange m_corrosionCountDown = IntRange(-1, -1);
	CellMatType m_corrodeType = CellMatType::MAT_EMPTY;


	// === 移动固体专用参数 ===
	struct MoveSolidParams {
		// float m_slideAngle;					// 滑动角度阈值 (角度制)
		//float m_collisionMomentumTransfer;	// 碰撞动量转移率 (0.0-1.0)
		//float m_activationThreshold;		// 激活邻居的速度阈值  =》这个值目前被当做自身“被外界激活的概率使用”
		//float m_neighborActivationChance;	// 激活邻居的概率
		//bool m_canActivateNeighbors;		// 是否能激活邻居颗粒

		//// 运动衰减参数
		//float m_airResistance;            // 空气阻力 (速度衰减率)
		//float m_collisionDamping;         // 碰撞阻尼 (碰撞后速度保留率)
		//float m_horizontalDamping;        // 水平移动阻尼
		//float m_verticalDamping;          // 垂直碰撞阻尼

		//// 方向性参数
		//float m_momentumPreservation;     // 动量保持度 (影响移动连续性) =》用作碰撞后保留多少比例的momentum给接收碰撞的cell
		//float m_randomDirectionChance;    // 随机方向概率 (无水平速度时)
	} m_moveSolid;

	// === 液体专用参数 ===
	struct LiquidParams {
		float m_flowRate;                 // 流动速率
		float m_pressureInfluence;        // 压力影响因子
		float m_surfaceTension;           // 表面张力
		bool m_canDisplace;               // 是否能置换其他物质
		float m_displacementForce;        // 置换力度
	} m_liquid;

	// === 交互参数 ===
	struct InteractionParams {
		float m_penetrationResistance;    // 被穿透阻力 (其他物质穿过时的阻力)
		float m_dissolutionRate;          // 溶解速率 (在某些液体中)
		bool m_isPermeable;               // 是否可穿透
		bool m_isSoluble;                 // 是否可溶解
	} m_interaction;

};