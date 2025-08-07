#pragma once
#include "TileTypesInGame.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
class Texture;
class SpriteSheet;
class CropDefinitions;

class CropObject
{
public:
	CropObject(CropDefinitions* curCropDef, IntVec2 gridPos);
	~CropObject();

	void Update(float deltaSeconds);
	void Render() const;

	void BeWatering() { m_hasWater = true; }
	IntVec2 GetGridPos() const { return m_gridPos; }

	void SettleDailyState();
	bool CanHarvest() { return m_canHarvest; }
	CropDefinitions* GetCropDef() { return m_cropDef; }

private:
	CropDefinitions* m_cropDef = nullptr;
	std::vector<Vertex_PCU> m_cropVerts;
	std::vector<Vertex_PCU> m_harvestIconVerts;
	IntVec2 m_gridPos;

	SpriteSheet* m_spriteSheet = nullptr;
	IntVec2 m_spriteGridPos = IntVec2(0, 0);
	int m_spriteIndex = 0;
	int m_harvestCount = 10;

	int m_curState = 0;
	bool m_canHarvest = false;
	bool m_hasWater = false;
};