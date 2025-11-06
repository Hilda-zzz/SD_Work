#pragma once
#include <vector>

class Curve1D
{
public:
	virtual ~Curve1D() = default;
	virtual float Evaluate(float t) const = 0;
};


class LinearCurve1D : public Curve1D
{
public:
	LinearCurve1D(float startT, float endT, float startV, float endV);
	float Evaluate(float t) const override;

	// Getters
	float GetStartT() const { return m_startT; }
	float GetEndT() const { return m_endT; }
	float GetStartV() const { return m_startV; }
	float GetEndV() const { return m_endV; }

private:
	float m_startT;  
	float m_endT;    
	float m_startV;  
	float m_endV;    
};


class PiecewiseCurve1D : public Curve1D
{
public:
	PiecewiseCurve1D();
	~PiecewiseCurve1D() override;

	// t is start point of the new sub curve
	void AddKey(float t, Curve1D* curve);

	void Clear(); // clear all keys and sub curve

	float Evaluate(float t) const override;

	size_t GetKeyCount() const { return m_keys.size(); }

	float GetKeyT(size_t index) const;
	LinearCurve1D* GetKeyLinearCurve(size_t index) const;

	void SetKeyT(size_t index, float newT);
	void SetKeyLinearCurve(size_t index, float startT, float endT, float startV, float endV);

	void RemoveKey(size_t index);

	void GetOutputRange(float& outMin, float& outMax) const;


private:
	struct Key
	{
		float t;          // start point
		Curve1D* curve;   // sub curve start from t
	};

	std::vector<Key> m_keys;
};