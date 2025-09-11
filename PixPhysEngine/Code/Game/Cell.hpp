#pragma once
#include "CellMatDef.hpp"
#include "Engine/Core/Rgba8.hpp"

struct Cell {
	CellMatType m_type = CellMatType::MAT_EMPTY;

	// 简化的速度系统（整数，不涉及dt）
	int m_velocityY = 0;           // 垂直速度（负数向下）
	int m_velocityX = 0;           // 水平速度

	// 移动概率系统（核心创新）
	float m_moveProbability = 0.0f; // 0.0-1.0，本帧移动的概率

	// 方向偏好
	bool m_hasSlideDirection = false;
	int m_preferredSlideDir = 0;   // -1左，1右，0无偏好

	// 防重复更新
	bool m_updatedThisFrame = false;

	bool IsEmpty() const { return m_type == CellMatType::MAT_EMPTY; }
	void SetEmpty() {
		m_type = CellMatType::MAT_EMPTY;
		m_velocityY = 0;
		m_velocityX = 0;
		m_moveProbability = 0.0f;
		m_hasSlideDirection = false;
		m_preferredSlideDir = 0;
		m_updatedThisFrame = false;
	}
};