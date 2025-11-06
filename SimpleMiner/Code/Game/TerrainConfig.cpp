#include "TerrainConfig.hpp"
#include "Curve1D.hpp"

const TerrainConfig::ContinentParams TerrainConfig::DEFAULT_CONTINENT = { 256.0f, 16, true };
const TerrainConfig::ErosionParams TerrainConfig::DEFAULT_EROSION = { 256.0f, 8, true };
const TerrainConfig::PVParams TerrainConfig::DEFAULT_PV = { 128.0f, 8, 15.0f, true };
const TerrainConfig::TemperatureParams TerrainConfig::DEFAULT_TEMPERATURE = { 256.0f, 1, true };
const TerrainConfig::HumidityParams TerrainConfig::DEFAULT_HUMIDITY = { 256.0f, 4, true };
const TerrainConfig::TreeParams TerrainConfig::DEFAULT_TREE = { 256.0f, 4, true };

TerrainConfig& TerrainConfig::GetInstance()
{
	static TerrainConfig instance;
	return instance;
}

TerrainConfig::TerrainConfig()
{
	m_curves.InitializeDefaults();
}

TerrainConfig::~TerrainConfig()
{
	m_curves.Cleanup();
}

void TerrainCurves::InitializeDefaults()
{
	Cleanup();

	// ========================================
	// Continent Height Offset Curve
	// 控制点: (-1.0, -1.0), (-0.45, -0.85), (-0.42, -0.38), (-0.4, -0.35), 
	//         (-0.38, -0.33), (-0.27, -0.32), (-0.25, -0.3), (-0.23, -0.27), 
	//         (-0.22, -0.05), (-0.2, -0.02), (-0.15, 0.0), (-0.10, 0.02), 
	//         (0.05, 0.1), (0.20, 0.3), (0.35, 0.4), (0.65, 0.5), 
	//         (0.85, 0.85), (1.00, 1.0)
	// ========================================
	m_continentOffset = new PiecewiseCurve1D();
	m_continentOffset->AddKey(-1.0f, new LinearCurve1D(-1.0f, -0.45f, -1.0f, -0.85f));
	m_continentOffset->AddKey(-0.45f, new LinearCurve1D(-0.45f, -0.42f, -0.85f, -0.38f));
	m_continentOffset->AddKey(-0.42f, new LinearCurve1D(-0.42f, -0.4f, -0.38f, -0.35f));
	m_continentOffset->AddKey(-0.4f, new LinearCurve1D(-0.4f, -0.38f, -0.35f, -0.33f));
	m_continentOffset->AddKey(-0.38f, new LinearCurve1D(-0.38f, -0.27f, -0.33f, -0.32f));
	m_continentOffset->AddKey(-0.27f, new LinearCurve1D(-0.27f, -0.25f, -0.32f, -0.3f));
	m_continentOffset->AddKey(-0.25f, new LinearCurve1D(-0.25f, -0.23f, -0.3f, -0.27f));
	m_continentOffset->AddKey(-0.23f, new LinearCurve1D(-0.23f, -0.22f, -0.27f, -0.05f));
	m_continentOffset->AddKey(-0.22f, new LinearCurve1D(-0.22f, -0.2f, -0.05f, -0.02f));
	m_continentOffset->AddKey(-0.2f, new LinearCurve1D(-0.2f, -0.15f, -0.02f, 0.0f));
	m_continentOffset->AddKey(-0.15f, new LinearCurve1D(-0.15f, -0.10f, 0.0f, 0.02f));
	m_continentOffset->AddKey(-0.10f, new LinearCurve1D(-0.10f, 0.05f, 0.02f, 0.1f));
	m_continentOffset->AddKey(0.05f, new LinearCurve1D(0.05f, 0.20f, 0.1f, 0.3f));
	m_continentOffset->AddKey(0.20f, new LinearCurve1D(0.20f, 0.35f, 0.3f, 0.4f));
	m_continentOffset->AddKey(0.35f, new LinearCurve1D(0.35f, 0.65f, 0.4f, 0.5f));
	m_continentOffset->AddKey(0.65f, new LinearCurve1D(0.65f, 0.85f, 0.5f, 0.85f));
	m_continentOffset->AddKey(0.85f, new LinearCurve1D(0.85f, 1.0f, 0.85f, 1.0f));

	// ========================================
	// Continent Squashing Curve
	// 控制点: (-1.0, 0.05), (-0.45, 0.02), (-0.2, 0.0), (-0.05, 0.02), 
	//         (0.05, 0.05), (0.20, 0.05), (0.35, 0.05), (0.45, 0.02), (1.0, 0.0)
	// ========================================
	m_continentAmplitude = new PiecewiseCurve1D();
	m_continentAmplitude->AddKey(-1.0f, new LinearCurve1D(-1.0f, -0.45f, 0.05f, 0.02f));
	m_continentAmplitude->AddKey(-0.45f, new LinearCurve1D(-0.45f, -0.2f, 0.02f, 0.0f));
	m_continentAmplitude->AddKey(-0.2f, new LinearCurve1D(-0.2f, -0.05f, 0.0f, 0.02f));
	m_continentAmplitude->AddKey(-0.05f, new LinearCurve1D(-0.05f, 0.05f, 0.02f, 0.05f));
	m_continentAmplitude->AddKey(0.05f, new LinearCurve1D(0.05f, 0.20f, 0.05f, 0.05f));
	m_continentAmplitude->AddKey(0.20f, new LinearCurve1D(0.20f, 0.35f, 0.05f, 0.05f));
	m_continentAmplitude->AddKey(0.35f, new LinearCurve1D(0.35f, 0.45f, 0.05f, 0.02f));
	m_continentAmplitude->AddKey(0.45f, new LinearCurve1D(0.45f, 1.0f, 0.02f, 0.0f));

	// ========================================
	// Erosion Height Offset Curve
	// 控制点: (-1.0, 0.0), (-0.05, 0.0), (0.05, -0.05), 
	//         (0.35, -0.05), (0.45, 0.0), (1.0, 0.0)
	// ========================================
	m_erosionOffset = new PiecewiseCurve1D();
	m_erosionOffset->AddKey(-1.0f, new LinearCurve1D(-1.0f, -0.05f, 0.0f, 0.0f));
	m_erosionOffset->AddKey(-0.05f, new LinearCurve1D(-0.05f, 0.05f, 0.0f, -0.05f));
	m_erosionOffset->AddKey(0.05f, new LinearCurve1D(0.05f, 0.35f, -0.05f, -0.05f));
	m_erosionOffset->AddKey(0.35f, new LinearCurve1D(0.35f, 0.45f, -0.05f, 0.0f));
	m_erosionOffset->AddKey(0.45f, new LinearCurve1D(0.45f, 1.0f, 0.0f, 0.0f));

	// ========================================
	// Erosion Squashing Curve
	// 控制点: (-1.0, 0.0), (-0.05, 0.0), (0.05, 0.2), 
	//         (0.35, 0.2), (0.45, 0.0), (1.0, 0.0)
	// ========================================
	m_erosionAmplitude = new PiecewiseCurve1D();
	m_erosionAmplitude->AddKey(-1.0f, new LinearCurve1D(-1.0f, -0.05f, 0.0f, 0.0f));
	m_erosionAmplitude->AddKey(-0.05f, new LinearCurve1D(-0.05f, 0.05f, 0.0f, 0.2f));
	m_erosionAmplitude->AddKey(0.05f, new LinearCurve1D(0.05f, 0.35f, 0.2f, 0.2f));
	m_erosionAmplitude->AddKey(0.35f, new LinearCurve1D(0.35f, 0.45f, 0.2f, 0.0f));
	m_erosionAmplitude->AddKey(0.45f, new LinearCurve1D(0.45f, 1.0f, 0.0f, 0.0f));

	// ========================================
	// Peaks And Valleys Height Offset Curve
	// 控制点: (-1.0, 0.0), (0.35, 0.0), (0.4, 0.15), 
	//         (0.6, 0.2), (0.8, 0.25), (1.0, 0.3)
	// ========================================
	m_pvOffset = new PiecewiseCurve1D();
	m_pvOffset->AddKey(-1.0f, new LinearCurve1D(-1.0f, 0.35f, 0.0f, 0.0f));
	m_pvOffset->AddKey(0.35f, new LinearCurve1D(0.35f, 0.4f, 0.0f, 0.15f));
	m_pvOffset->AddKey(0.4f, new LinearCurve1D(0.4f, 0.6f, 0.15f, 0.2f));
	m_pvOffset->AddKey(0.6f, new LinearCurve1D(0.6f, 0.8f, 0.2f, 0.25f));
	m_pvOffset->AddKey(0.8f, new LinearCurve1D(0.8f, 1.0f, 0.25f, 0.3f));

	// ========================================
	// Peaks And Valleys Squashing Curve (新增)
	// 控制点: (-1.0, 0.0), (0.35, 0.0), (0.4, -0.05), 
	//         (0.6, -0.1), (0.8, -0.15), (1.0, -0.3)
	// ========================================
	m_pvAmplitude = new PiecewiseCurve1D();
	m_pvAmplitude->AddKey(-1.0f, new LinearCurve1D(-1.0f, 0.35f, 0.0f, 0.0f));
	m_pvAmplitude->AddKey(0.35f, new LinearCurve1D(0.35f, 0.4f, 0.0f, -0.05f));
	m_pvAmplitude->AddKey(0.4f, new LinearCurve1D(0.4f, 0.6f, -0.05f, -0.1f));
	m_pvAmplitude->AddKey(0.6f, new LinearCurve1D(0.6f, 0.8f, -0.1f, -0.15f));
	m_pvAmplitude->AddKey(0.8f, new LinearCurve1D(0.8f, 1.0f, -0.15f, -0.3f));

	// ========================================
	// Height Squeeze Curve (保持默认)
	// ========================================
	m_heightSqueeze = new PiecewiseCurve1D();
	m_heightSqueeze->AddKey(0.0f, new LinearCurve1D(0.0f, 128.0f, 1.0f, 1.0f));
}

void TerrainCurves::Cleanup()
{
	if (m_continentOffset) delete m_continentOffset;
	if (m_erosionOffset) delete m_erosionOffset;
	if (m_pvOffset) delete m_pvOffset;
	if (m_continentAmplitude) delete m_continentAmplitude;
	if (m_erosionAmplitude) delete m_erosionAmplitude;
	if (m_pvAmplitude) delete m_pvAmplitude;  
	if (m_heightSqueeze) delete m_heightSqueeze;

	m_continentOffset = nullptr;
	m_erosionOffset = nullptr;
	m_pvOffset = nullptr;
	m_continentAmplitude = nullptr;
	m_erosionAmplitude = nullptr;
	m_pvAmplitude = nullptr; 
	m_heightSqueeze = nullptr;
}
