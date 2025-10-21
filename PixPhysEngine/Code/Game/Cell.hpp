#pragma once
#include "CellMatDef.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"

struct Cell {
	CellMatType m_type = CellMatType::MAT_EMPTY;
	float m_velocityY = 0.f;          
	float m_velocityX = 0.f;      
	float m_accumulMoveY = 0.f;
	float m_accumulMoveX = 0.f;
	bool m_updatedThisFrame = false;
	Rgba8 m_color;

	bool m_isFreeFalling = true;
	int m_framesWithoutMovement = 0;

	int m_liquidReCollideTimes = 0;

	bool m_isBelongRb = false;
	int m_rigidBodyId = -1;

	bool IsEmpty() const { return m_type == CellMatType::MAT_EMPTY; }
	void SetEmpty() {
		m_type = CellMatType::MAT_EMPTY;
		m_velocityY = 0;
		m_velocityX = 0;
		m_updatedThisFrame = false;
		m_rigidBodyId = -1;
		m_isBelongRb = false;
		m_color = Rgba8::WHITE;
	}
};

struct CellWithCoords
{
	Cell* m_cell = nullptr;
	IntVec2 m_worldCoords=IntVec2::ZERO;

	CellWithCoords() {}
	CellWithCoords(Cell* cell, IntVec2 coords)
		: m_cell(cell), m_worldCoords(coords) {}
};