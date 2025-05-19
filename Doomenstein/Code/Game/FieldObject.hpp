#pragma once
#include <vector>
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Mat44.hpp"

class Texture;
class Timer;
class Map;
class PlayerController;

enum class PlantState
{
	PLANT_SEED,
	PLANT_SPROUT,
	PLANT_GROWING,
	PLANT_HARVESTABLE
};

class FieldObject
{
public:
	FieldObject(Vec3 const& position, Map* curMap);
	~FieldObject();
	void Update(float deltaSeconds);
	void Render(PlayerController* curPlayer) const;
	void RenderShadowTexture(PlayerController* curPlayer) const;

	void PourWater();
	void Harvest(PlayerController* curPlayer);
	bool IsReadyToHarvest() const;

	PlantState GetCurrentState() const { return m_curPlantState; }
	Vec3 GetPosition() const { return m_position; }
	AABB3 GetBoundingBox() const { return m_boundingBox; }

	bool RaycastAgainst(Vec3 const& rayStart, Vec3 const& rayDirection, float& outDistance) const;

private:
	PlantState GetNextState(PlantState currentState);
	Mat44 GetFinalModelMat(PlayerController* curPlayer) const;

public:
	bool m_isDie = false;

private:
	Map* m_curMap = nullptr;

	std::vector<Vertex_PCUTBN> m_fieldVerts;
	std::vector<Vertex_PCUTBN> m_plantVerts;
	std::vector<Vertex_PCUTBN> m_tipIconVerts;
	Texture* m_fieldTex = nullptr;
	Texture* m_plantTex = nullptr;
	Texture* m_waterIconTex = nullptr;
	Texture* m_basketIconTex = nullptr;
	SpriteSheet* m_plantSheet;

	PlantState m_curPlantState = PlantState::PLANT_SEED;
	AABB3 m_boundingBox;
	Vec3 m_position;

	float m_seedToSproutTime = 5.0f;      
	float m_sproutToGrowingTime = 10.0f;  
	float m_growingToMatureTime = 15.0f;  
	Timer* m_growthTimer = nullptr; 
	

	bool m_needWater = false;
	bool m_canHarvest = false;

	int m_harvestCoinCount = 10;
};