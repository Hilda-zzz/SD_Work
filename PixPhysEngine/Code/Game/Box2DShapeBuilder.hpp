#pragma once
#include "Cell.hpp"
#include <unordered_set>
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

struct Cell;
class b2Body;

struct IntVec2Hash
{
	size_t operator()(const IntVec2& vec) const {
		return std::hash<int>()(vec.x) ^ (std::hash<int>()(vec.y) << 1);
	}
};

struct TriangulationOutput 
{
	std::vector<Vec2> vertices;
	std::vector<unsigned int> indices;  
};

class Box2DShapeBuilder
{
public:
	static std::vector<std::vector<CellWithCoords>> SeparateConnectedComponents(
		const std::vector<CellWithCoords>& cells);

	static bool IfDiscardComponent(const std::vector<CellWithCoords>& cells);

	static std::vector<IntVec2> FillHoles(const std::vector<CellWithCoords>& cells);

	static Vec2 CalculateCentroid(std::vector<Cell*> const& cells);

	static std::vector<Vec2> ExtractOutlineFromCells(std::vector<CellWithCoords> const& cells, std::vector<Vec2>& marchingPoints);
	static Vec2 CalculateCentroid(const std::vector<Vec2>& points);

	static TriangulationOutput EarClipping(const std::vector<Vec2>& inputVertices);
	static TriangulationOutput CDTTriangulation(std::vector<Vec2> const& points);

private:
	static int GetMarchingSquaresConfig(int gridX, int gridY,
		std::unordered_set<IntVec2,IntVec2Hash> const& occupiedCells);
	static std::vector<Vec2> ConnectAndOrderVertices(std::vector<Vec2> const& vertices);

	static std::vector<Vec2> ReorderOutlineForDouglasPeucker(const std::vector<Vec2>& closedLoop);
	static std::vector<Vec2> DouglasPeucker(const std::vector<Vec2>& points, float epsilon);
	static float PointToLineDistanceSq(const Vec2& point, const Vec2& lineStart, const Vec2& lineEnd);

	static bool IsClockwise(const std::vector<Vec2>& points);
	static float GetOrientation(const Vec2& a, const Vec2& b, const Vec2& c);
	static bool IsPointInTriangle(const Vec2& p, const Vec2& a, const Vec2& b, const Vec2& c);
};