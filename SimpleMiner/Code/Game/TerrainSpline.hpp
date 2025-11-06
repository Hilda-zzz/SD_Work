#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/CubicHermiteSpline2D.hpp"
#include <vector>

class TerrainSpline
{
public:
	TerrainSpline();
	TerrainSpline(std::vector<Vec2> const& points, float inputMin = -1.0f, float inputMax = 1.0f);
	
	float Evaluate(float input) const;
	
	// x: input, y: output
	std::vector<Vec2> const& GetControlPoints() const { return m_controlPoints; }
	
	// rebuild hermite spline
	void SetControlPoints(std::vector<Vec2> const& points);
	
	// sort by x
	void AddControlPoint(Vec2 const& point);
	
	void RemoveControlPoint(size_t index);
	
	void SetControlPoint(size_t index, Vec2 const& newPoint);
	
	float GetInputMin() const { return m_inputMin; }
	float GetInputMax() const { return m_inputMax; }
	void SetInputRange(float minVal, float maxVal);
	
	void GetOutputRange(float& outMin, float& outMax) const;
	
private:
	void RebuildSpline();
	
	// sort by x
	void SortControlPoints();

private:
	std::vector<Vec2> m_controlPoints;      //  x: input, y: output
	CubicHermiteSpline2D m_hermiteSpline; 
	float m_inputMin;                        
	float m_inputMax;                        
	bool m_needsRebuild;                    
};
