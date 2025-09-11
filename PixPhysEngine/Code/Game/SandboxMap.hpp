#pragma once
#include "Cell.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <utility>
class SandboxPlayer;

constexpr float GRAVITY = -98.f;           
constexpr float TERMINAL_VELOCITY = -200.0f; 
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
	void HandlePlayerInteraction();
	void renderGrid();

	void UpdatePhysics();
	void ResetUpdateFlags();
	bool TryMoveParticle(int fromX, int fromY, int toX, int toY);
	bool TryDiagonalSlide(int x, int y);
	void HandleSpeedTransfer(int x1, int y1, int x2, int y2);
	
	// update diff mats
	void UpdateSandParticle(int x, int y);


	bool ProcessVerticalMovement(int startX, int startY, int& finalX, int& finalY);
	bool HandleSandCollision(int currentX, int currentY, int finalX, int targetY);
	void TransferMomentum(Cell& faster, Cell& slower);
	void ProcessDiagonalSlide(int startX, int startY, int& finalX, int& finalY);
	void ApplyPositionChange(int startX, int startY, int finalX, int finalY);
	void UpdateWaterParticle(int x, int y);
	void HandleWaterFlow(int x, int y);
	void TryWaterHozrizontalFlow(int x, int y);
	int  GetEffectiveHeight(int x, int baseY);

	bool TryVerticalFlow(int x, int y);
	bool TryDiagonalFlow(int x, int y);
	void TryHorizontalFlow(int x, int y);
	bool IsPathClear(int fromX, int fromY, int toX, int toY);
	void HandleHorizontalSpeedTransfer(int x1, int y1, int x2, int y2);

	bool TryVelocityMove(int x, int y);
	bool TryPushParticle(int fromX, int fromY, int toX, int toY);
	void SwapParticles(int x1, int y1, int x2, int y2);
	void MoveParticle(int fromX, int fromY, int toX, int toY);
	bool TrySimpleFall(int x, int y);
	void HandleStuckParticle(int x, int y);
	//bool TryDiagonalSlide(int x, int y);
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
