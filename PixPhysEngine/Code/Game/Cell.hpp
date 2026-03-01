#pragma once
#include "CellMatDef.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Game.hpp"
#include "Game/CellMatManager.hpp"

struct Cell {
	//CellMatType m_type = CellMatType::MAT_EMPTY;
	std::atomic<CellMatType> m_type{ CellMatType::MAT_EMPTY };
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

	// Chemical
	int m_lifeCountDown = -1;
	int m_flameCountDown = -1;
	int m_dissolveCountDown = -1;
	int m_corrosionCountDown = -1;

	int m_extraData = 0;

	Cell() = default;

	Cell(const Cell& other)
		: m_type(other.m_type.load())
		, m_velocityY(other.m_velocityY)
		, m_velocityX(other.m_velocityX)
		, m_accumulMoveY(other.m_accumulMoveY)
		, m_accumulMoveX(other.m_accumulMoveX)
		, m_updatedThisFrame(other.m_updatedThisFrame)
		, m_color(other.m_color)
		, m_isFreeFalling(other.m_isFreeFalling)
		, m_framesWithoutMovement(other.m_framesWithoutMovement)
		, m_liquidReCollideTimes(other.m_liquidReCollideTimes)
		, m_isBelongRb(other.m_isBelongRb)
		, m_rigidBodyId(other.m_rigidBodyId)
		, m_lifeCountDown(other.m_lifeCountDown)
		, m_flameCountDown(other.m_flameCountDown)
		, m_dissolveCountDown(other.m_dissolveCountDown)
		, m_corrosionCountDown(other.m_corrosionCountDown)
		, m_extraData(other.m_extraData)
	{}

	Cell& operator=(const Cell& other) {
		if (this != &other) {
			m_type.store(other.m_type.load());
			m_velocityY = other.m_velocityY;
			m_velocityX = other.m_velocityX;
			m_accumulMoveY = other.m_accumulMoveY;
			m_accumulMoveX = other.m_accumulMoveX;
			m_updatedThisFrame = other.m_updatedThisFrame;
			m_color = other.m_color;
			m_isFreeFalling = other.m_isFreeFalling;
			m_framesWithoutMovement = other.m_framesWithoutMovement;
			m_liquidReCollideTimes = other.m_liquidReCollideTimes;
			m_isBelongRb = other.m_isBelongRb;
			m_rigidBodyId = other.m_rigidBodyId;
			m_lifeCountDown = other.m_lifeCountDown;
			m_flameCountDown = other.m_flameCountDown;
			m_dissolveCountDown = other.m_dissolveCountDown;
			m_corrosionCountDown = other.m_corrosionCountDown;
			m_extraData = other.m_extraData;
		}
		return *this;
	}

	Cell(Cell&& other) noexcept
		: m_type(other.m_type.load())
		, m_velocityY(other.m_velocityY)
		, m_velocityX(other.m_velocityX)
		, m_accumulMoveY(other.m_accumulMoveY)
		, m_accumulMoveX(other.m_accumulMoveX)
		, m_updatedThisFrame(other.m_updatedThisFrame)
		, m_color(other.m_color)
		, m_isFreeFalling(other.m_isFreeFalling)
		, m_framesWithoutMovement(other.m_framesWithoutMovement)
		, m_liquidReCollideTimes(other.m_liquidReCollideTimes)
		, m_isBelongRb(other.m_isBelongRb)
		, m_rigidBodyId(other.m_rigidBodyId)
		, m_lifeCountDown(other.m_lifeCountDown)
		, m_flameCountDown(other.m_flameCountDown)
		, m_dissolveCountDown(other.m_dissolveCountDown)
		, m_corrosionCountDown(other.m_corrosionCountDown)
		, m_extraData(other.m_extraData)
	{}

	Cell& operator=(Cell&& other) noexcept {
		if (this != &other) {
			m_type.store(other.m_type.load());
			m_velocityY = other.m_velocityY;
			m_velocityX = other.m_velocityX;
			m_accumulMoveY = other.m_accumulMoveY;
			m_accumulMoveX = other.m_accumulMoveX;
			m_updatedThisFrame = other.m_updatedThisFrame;
			m_color = other.m_color;
			m_isFreeFalling = other.m_isFreeFalling;
			m_framesWithoutMovement = other.m_framesWithoutMovement;
			m_liquidReCollideTimes = other.m_liquidReCollideTimes;
			m_isBelongRb = other.m_isBelongRb;
			m_rigidBodyId = other.m_rigidBodyId;
			m_lifeCountDown = other.m_lifeCountDown;
			m_flameCountDown = other.m_flameCountDown;
			m_dissolveCountDown = other.m_dissolveCountDown;
			m_corrosionCountDown = other.m_corrosionCountDown;
			m_extraData = other.m_extraData;
		}
		return *this;
	}

	bool IsEmpty() const { return m_type == CellMatType::MAT_EMPTY; }
	void SetEmpty() {
		m_type.store(CellMatType::MAT_EMPTY);
		m_velocityY = 0;
		m_velocityX = 0;
		m_accumulMoveX = 0;
		m_accumulMoveY = 0;
		m_updatedThisFrame = false;
		m_isFreeFalling = true;
		m_framesWithoutMovement = 0;
		m_liquidReCollideTimes = 0;

		m_rigidBodyId = -1;
		m_isBelongRb = false;

		m_color = Rgba8::WHITE;

		m_lifeCountDown = -1;
		m_flameCountDown = -1;
		m_dissolveCountDown = -1;
		m_corrosionCountDown = -1;
	}

	void SetToType(CellMatType toType)
	{
		CellMatDef cellDef = CellMatManager::GetMaterialDef(toType);
		//m_type = toType;
		m_type.store(toType);
		m_lifeCountDown = cellDef.m_lifeCountDown.GetRandomInRange(&Game::s_rng);
		m_flameCountDown = cellDef.m_flameCountDown.GetRandomInRange(&Game::s_rng);
		m_dissolveCountDown = cellDef.m_dissolveCountDowm.GetRandomInRange(&Game::s_rng);
		m_corrosionCountDown = cellDef.m_corrosionCountDown.GetRandomInRange(&Game::s_rng);
		m_color = Rgba8::GetRandomColorInRange(cellDef.m_colorMin, cellDef.m_colorMax, &Game::s_rng);
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