#pragma once
#include "CellMatDef.hpp"
#include "Engine/Core/Rgba8.hpp"

struct Cell {
	bool IsEmpty() const { return m_type == CellMatType::MAT_EMPTY; }
	void SetEmpty() {
		m_type = CellMatType::MAT_EMPTY;        
		m_velocityY = 0.0f;
		m_velocityX = 0.0f;
		m_accumulatedMoveY = 0.0f;
		m_accumulatedMoveX = 0.0f;
	}

	CellMatType m_type=CellMatType::MAT_EMPTY;
	Rgba8 m_color=Rgba8::WHITE;
	float m_density=1.f;
	float m_temperature=1.f;

	bool m_updatedThisFrame = false;

	float m_velocityY = 0.0f;           
	float m_velocityX = 0.0f;          
	float m_accumulatedMoveY = 0.0f;   
	float m_accumulatedMoveX = 0.0f;
};