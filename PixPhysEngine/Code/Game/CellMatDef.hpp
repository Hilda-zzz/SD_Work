#pragma once
#include <stdint.h>
#include <string>
#include "Engine/Core/Rgba8.hpp"

enum class PhyType :uint8_t
{
	PHY_STATIC_SOLID,
	PHY_MOVE_SOLID,
	PHY_LIQUID
};

enum class CellMatType : uint8_t
{
	MAT_EMPTY=0,
	MAT_SAND=1,
	MAT_WATER=2,
	MAT_SALT=3,
	MAT_STONE=4,
	MAT_WOOD=5,
	MAT_SOIL=6,
	MAT_GRAVEL=7,
	MAT_OIL = 8,
	MAT_LAVA = 9
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