#pragma once
#include <stdint.h>
#include "Engine/Core/GameTimer.hpp"

class CropDefinitions;

enum class TerrainType : uint32_t   //may change to uint8
{
	NONE = 0,
	NORMAL_RODE= 1 << 0, // 0x01
	SOLID = 1 << 1,   // 0x02 
	FARMABLE = 1 << 2,     
	WATER = 1 << 3,      
	// 	INTERACTABLE = 1 << 3, // 0x08
	// 	BUILDABLE = 1 << 4,    // 0x10
	// 	NPC_PATH = 1 << 5,     // 0x20
	// 	TELEPORT = 1 << 6,     // 0x40
	// 	SHOP = 1 << 7,         // 0x80
		// can be expanded to 32
};

enum class FarmState : uint8_t {
	UNPLOWED = 0,   
	PLOWED = 1,
// 	WATER=2,
// 	PLANTED = 3,    
// 	GROWN = 4       
};

enum class ObstacleType : uint8_t {
	NONE = 0,
	ROCK = 1,       
	LOG = 2,        
	WEED = 3,       
	TREE = 4        
};

struct CropInfo
{
	int cropType = 0;
	int growthStage = 0;
	float growthProgress = 0.0f;
	int waterLevel = 50;
	bool hasWater = false;
};

 struct DynamicTileData
 {
 	ObstacleType m_obstacleType=ObstacleType::NONE;
 	int m_curObstacleDurability = 0; // if m_obstacleType!=none, meaningful
 	FarmState m_farmState = FarmState::UNPLOWED;
	bool m_isWater = false;
	bool m_isPlanted = false;
	CropDefinitions* m_curCrop = nullptr; // if m_farmState==PLANTED/ GROWN, meaningful

	int m_spriteIndex = 0;
 };

