#include "Curve1D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <algorithm>

// ====================================================================
// LinearCurve1D Implementation
// ====================================================================

LinearCurve1D::LinearCurve1D(float startT, float endT, float startV, float endV)
	: m_startT(startT)
	, m_endT(endT)
	, m_startV(startV)
	, m_endV(endV)
{
}

float LinearCurve1D::Evaluate(float t) const
{
	if (t < m_startT)
	{
		return m_startV;
	}

	if (t >= m_endT)
	{
		return m_endV;
	}

	float fraction = (t - m_startT) / (m_endT - m_startT);
	return Interpolate(m_startV, m_endV, fraction);
}


// ====================================================================
// PiecewiseCurve1D Implementation
// ====================================================================

PiecewiseCurve1D::PiecewiseCurve1D()
{
}

PiecewiseCurve1D::~PiecewiseCurve1D()
{
	Clear();
}

void PiecewiseCurve1D::AddKey(float t, Curve1D* curve)
{
	if (!curve)
		return;

	Key key;
	key.t = t;
	key.curve = curve;
	m_keys.push_back(key);

	// sort by t
	std::sort(m_keys.begin(), m_keys.end(),
		[](const Key& a, const Key& b) {
			return a.t < b.t;
		});
}

void PiecewiseCurve1D::Clear()
{
	for (Key& key : m_keys)
	{
		delete key.curve;
		key.curve = nullptr;
	}
	m_keys.clear();
}

float PiecewiseCurve1D::Evaluate(float t) const
{
	if (m_keys.empty())
		return 0.0f;

	if (m_keys.size() == 1)
	{
		return m_keys[0].curve->Evaluate(t);  // 直接传 t
	}

	// 找到包含 t 的段
	for (size_t i = 0; i < m_keys.size() - 1; ++i)
	{
		float t0 = m_keys[i].t;
		float t1 = m_keys[i + 1].t;

		if (t >= t0 && t < t1)
		{
			// ✅ 直接传入 t，不要归一化
			return m_keys[i].curve->Evaluate(t);
		}
	}

	// 处理超出范围的情况
	if (t >= m_keys.back().t)
	{
		return m_keys.back().curve->Evaluate(t);  // 直接传 t
	}

	return m_keys[0].curve->Evaluate(t);  // 直接传 t
}

float PiecewiseCurve1D::GetKeyT(size_t index) const
{
	if (index >= m_keys.size())
		return 0.0f;
	return m_keys[index].t;
}

LinearCurve1D* PiecewiseCurve1D::GetKeyLinearCurve(size_t index) const
{
	if (index >= m_keys.size())
		return nullptr;
	return dynamic_cast<LinearCurve1D*>(m_keys[index].curve);
}

void PiecewiseCurve1D::SetKeyT(size_t index, float newT)
{
	if (index >= m_keys.size())
		return;

	m_keys[index].t = newT;

	// Re-sort keys
	std::sort(m_keys.begin(), m_keys.end(),
		[](const Key& a, const Key& b) { return a.t < b.t; });
}

void PiecewiseCurve1D::SetKeyLinearCurve(size_t index, float startT, float endT, float startV, float endV)
{
	if (index >= m_keys.size())
		return;

	// Get the existing curve as LinearCurve1D
	LinearCurve1D* linearCurve = dynamic_cast<LinearCurve1D*>(m_keys[index].curve);
	if (linearCurve)
	{
		// Delete old curve and create new one
		delete m_keys[index].curve;
		m_keys[index].curve = new LinearCurve1D(startT, endT, startV, endV);
	}
}

void PiecewiseCurve1D::RemoveKey(size_t index)
{
	if (index >= m_keys.size())
		return;

	// Delete the curve
	delete m_keys[index].curve;
	m_keys[index].curve = nullptr;

	// Remove from vector
	m_keys.erase(m_keys.begin() + index);
}

void PiecewiseCurve1D::GetOutputRange(float& outMin, float& outMax) const
{
	if (m_keys.empty())
	{
		outMin = 0.0f;
		outMax = 0.0f;
		return;
	}

	outMin = 999999.0f;
	outMax = -999999.0f;

	// Check all linear curves
	for (const Key& key : m_keys)
	{
		LinearCurve1D* linearCurve = dynamic_cast<LinearCurve1D*>(key.curve);
		if (linearCurve)
		{
			float startV = linearCurve->GetStartV();
			float endV = linearCurve->GetEndV();

			outMin = std::min(outMin, std::min(startV, endV));
			outMax = std::max(outMax, std::max(startV, endV));
		}
	}
}