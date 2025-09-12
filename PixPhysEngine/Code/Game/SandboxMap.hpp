#pragma once
#include "Cell.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <utility>
class SandboxPlayer;

constexpr float GRAVITY = -98.f;           
constexpr float TERMINAL_SPEED = 200.0f; 
constexpr float AIR_RESISTANCE = 0.98f;    
constexpr float SAND_FRICTION = 0.7f;  

constexpr float WATER_HORIZONTAL_SPEED = 50.0f;  
constexpr float WATER_FLOW_DAMPING = 0.8f;      

static const float SPEED_THRESHOLD = 5.0f;

class SandboxMap {
public:
	SandboxMap(SandboxPlayer* playerPtr,IntVec2 const& size=IntVec2(1920,1080));

	void Update(float deltaTime);
	void Render() const;

	void Initialize();

	void PlaceMaterial(int x, int y, CellMatType type, int brushSize = 1);
	void ClearArea(int x, int y, int radius);

	Cell& GetCell(int x, int y);
	bool IsValidPosition(int x, int y) const;

private:
	float GetFrameTime();

	void UpdateCell(int x, int y);

	void UpdatePhysics();
	void ResetUpdateFlags();
	bool IsInBounds(int x, int y);
	
	// update diff mats
	void UpdateSandParticle(int x, int y);
	void UpdateSandY(int x, int y);


public:
	

private:
	std::vector<std::vector<Cell>> m_grid;
	SandboxPlayer* m_player;
	IntVec2 m_mapSize;
	AABB2 m_mapBound;

	std::vector<Vertex_PCU> m_boundVerts;

	float m_curDeltaTime = 0.f;

	//------------
	std::vector<std::pair<int, int>> m_updateOrder;
	int m_frameCount = 0;
};
