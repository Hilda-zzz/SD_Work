#include "Game/TentacleDebugPanel.hpp"
#include "Game/Tentacle.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "ThirdParty/ImGUI/implot.h"
#include <vector>

TentacleEditPanel::TentacleEditPanel()
    : ImguiUIPanel("Tentacle Debug")
    , m_tentacle(nullptr)
{
}

TentacleEditPanel::~TentacleEditPanel()
{
    // Don't delete m_tentacle - we don't own it
}

void TentacleEditPanel::Render()
{
	if (!m_isVisible)
		return;

	if (!m_tentacle)
	{
		ImGui::Begin(m_panelName.c_str(), &m_isVisible);
		ImGui::Text("No tentacle set for debugging");
		ImGui::End();
		return;
	}

	ImGui::Begin(m_panelName.c_str(), &m_isVisible, ImGuiWindowFlags_AlwaysAutoResize);

	// =========================================================================
	// Section 3: Basic Configuration
	// =========================================================================
	ImGui::SeparatorText("Configuration");

	ImGui::Text("Segment Count: %d", m_tentacle->GetSegmentCount());
	ImGui::Text("Total Length: %.1f", m_tentacle->GetTotalLength());

	bool rootFixed = m_tentacle->IsRootFixed();
	if (ImGui::Checkbox("Root Fixed", &rootFixed))
	{
		m_tentacle->SetRootFixed(rootFixed);
	}

	float angleConstraint = m_tentacle->GetAngleConstraint();
	if (ImGui::SliderFloat("Angle Constraint", &angleConstraint, 0.0f, 360.0f, "%.1f deg"))
	{
		m_tentacle->SetAngleConstraint(angleConstraint);
	}

	// =========================================================================
	// Section 4: Wave Animation
	// =========================================================================
	ImGui::SeparatorText("Wave Animation");

	bool waveEnabled = m_tentacle->IsWaveEnabled();
	if (ImGui::Checkbox("Enable Wave", &waveEnabled))
	{
		m_tentacle->SetWaveEnabled(waveEnabled);
	}

	if (waveEnabled)
	{
		float waveFrequency = m_tentacle->GetWaveFrequency();
		if (ImGui::SliderFloat("Wave Frequency", &waveFrequency, 0.1f, 10.0f))
		{
			m_tentacle->SetWaveFrequency(waveFrequency);
		}

		float waveAmplitude = m_tentacle->GetWaveAmplitude();
		if (ImGui::SliderFloat("Wave Amplitude", &waveAmplitude, 0.5f, 20.0f))
		{
			m_tentacle->SetWaveAmplitude(waveAmplitude);
		}

		float waveSpeed = m_tentacle->GetWaveSpeed();
		if (ImGui::SliderFloat("Wave Speed", &waveSpeed, 0.1f, 10.0f))
		{
			m_tentacle->SetWaveSpeed(waveSpeed);
		}
	}

	// =========================================================================
	// Section 5: Radius Curve Editor
	// =========================================================================
	ImGui::SeparatorText("Radius Curve Editor");

	std::vector<Vec2>& controlPoints = m_tentacle->GetRadiusControlPointsReference();
	std::vector<float>& radiusCurve = m_tentacle->GetRadiusCurveReference();

	if (controlPoints.size() > 0 && radiusCurve.size() > 0)
	{
		if (ImPlot::BeginPlot("Radius Profile", ImVec2(-1, 300)))
		{
			ImPlot::SetupAxes("Position (0=Root, 1=Tip)", "Radius");
			ImPlot::SetupAxisLimits(ImAxis_X1, -0.05, 1.05, ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0.5, 15.0, ImGuiCond_Always);

			// Draw interpolated curve
			std::vector<double> curveX, curveY;
			for (int i = 0; i < (int)radiusCurve.size(); ++i)
			{
				double t = (double)i / (double)(radiusCurve.size() - 1);
				curveX.push_back(t);
				curveY.push_back((double)radiusCurve[i]);
			}
			ImPlot::PlotLine("Interpolated", curveX.data(), curveY.data(), (int)curveX.size());

			// Draw control points
			std::vector<double> cpX, cpY;
			for (int i = 0; i < (int)controlPoints.size(); ++i)
			{
				cpX.push_back((double)controlPoints[i].x);
				cpY.push_back((double)controlPoints[i].y);
			}

			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 8.0f);
			ImPlot::PlotScatter("Control Points", cpX.data(), cpY.data(), (int)cpX.size());

			// Drag control points
			bool pointsChanged = false;
			for (int i = 0; i < (int)controlPoints.size(); ++i)
			{
				double x = (double)controlPoints[i].x;
				double y = (double)controlPoints[i].y;

				if (i == 0 || i == (int)controlPoints.size() - 1)
				{
					// First and last - only Y draggable
					if (ImPlot::DragPoint(i, &x, &y, ImVec4(1, 0, 0, 1), 8.0f))
					{
						controlPoints[i].y = GetClamped((float)y, 0.5f, 15.0f);
						pointsChanged = true;
					}
				}
				else
				{
					// Middle points - X and Y draggable
					if (ImPlot::DragPoint(i, &x, &y, ImVec4(1, 0.5f, 0, 1), 8.0f))
					{
						float minX = controlPoints[i - 1].x + 0.01f;
						float maxX = controlPoints[i + 1].x - 0.01f;
						controlPoints[i].x = GetClamped((float)x, minX, maxX);
						controlPoints[i].y = GetClamped((float)y, 0.5f, 15.0f);
						pointsChanged = true;
					}
				}
			}

			if (pointsChanged)
			{
				m_tentacle->UpdateRadiusCurveFromControlPoints();
			}

			ImPlot::EndPlot();
		}

		if (ImGui::Button("Reset to Default"))
		{
			controlPoints.clear();
			controlPoints.push_back(Vec2(0.0f, 10.0f));
			controlPoints.push_back(Vec2(0.25f, 8.0f));
			controlPoints.push_back(Vec2(0.5f, 5.0f));
			controlPoints.push_back(Vec2(0.75f, 3.0f));
			controlPoints.push_back(Vec2(1.0f, 1.5f));
			m_tentacle->UpdateRadiusCurveFromControlPoints();
		}
	}

	// IMPORTANT: Only ONE ImGui::End() at the very end!
	ImGui::End();
}









