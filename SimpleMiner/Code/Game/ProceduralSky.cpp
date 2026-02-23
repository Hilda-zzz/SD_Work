#include "Game/ProceduralSky.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <vector>

ProceduralSky::ProceduralSky(Renderer* renderer)
	: m_renderer(renderer)
{
	// 初始化天空球网格
	InitializeSkyMesh();
	//InitializeStarMesh();
	
	// 加载着色器
	m_skyShader = m_renderer->CreateShaderFromFile("Data/Shaders/ProceduralSky");
	//m_starShader = m_renderer->CreateShaderFromFile("Data/Shaders/Stars");
	
	std::string moonTexPath = "Data/Images/MoonColorMap.jpg";
	m_moonTexture = m_renderer->CreateOrGetTextureFromFile("Data/Images/MoonColorMap.jpg");

	// 创建常量缓冲区
	m_skyConstantBuffer = m_renderer->CreateConstantBuffer(sizeof(SkyConstants));
	
	// 初始化天体位置
	//UpdateCelestialPositions();
}

ProceduralSky::~ProceduralSky()
{
	delete m_skyVertexBuffer;
	delete m_starVertexBuffer;
	delete m_skyConstantBuffer;
}

void ProceduralSky::Update(float worldTimeInDays)
{
	m_worldTimeInDays = worldTimeInDays;
	CalculateSkyColorsForCurrentTime();
	// 更新天体位置
	//UpdateCelestialPositions();
	
	// 更新星星亮度（根据太阳高度）
	//float sunHeight = m_sunDirection.z;
	//if (sunHeight < -0.1f)
	//{
	//	// 太阳在地平线下，星星逐渐显示
	//	m_starBrightness = GetClamped((-sunHeight - 0.1f) / 0.2f, 0.0f, 1.0f);
	//}
	//else
	//{
	//	m_starBrightness = 0.0f;
	//}
}

void ProceduralSky::Render()
{
	// 更新常量缓冲区
	SetSkyConstantsToGPU();
	
	// 设置渲染状态
	m_renderer->SetModelConstants();
	m_renderer->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
	m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	m_renderer->SetBlendMode(BlendMode::OPAQUE);
	m_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	m_renderer->BindTexture(m_moonTexture);
	
	// 渲染天空球
	m_renderer->BindShader(m_skyShader);
	m_renderer->DrawGameVertexBuffer(m_skyVertexBuffer);
	
	// 渲染星星（如果是夜晚）
	//if (m_starBrightness > 0.01f)
	//{
	//	m_renderer->SetBlendMode(BlendMode::ADDITIVE);
	//	m_renderer->BindShader(m_starShader);
	//	m_renderer->DrawGameVertexBuffer(m_starVertexBuffer, m_starVertexCount);
	//}

	// direct x invert z buffer
}

void ProceduralSky::SetSunDirection(const Vec3& direction)
{
	m_sunDirection = direction;
	m_sunDirection.Normalized();
}

void ProceduralSky::SetMoonDirection(const Vec3& direction)
{
	m_moonDirection = direction;
	m_moonDirection.Normalized();
}

void ProceduralSky::SetSunIntensity(float intensity)
{
	m_atmosphereParams.sunIntensity = intensity;
}

void ProceduralSky::SetMoonIntensity(float intensity)
{
	m_atmosphereParams.moonIntensity = intensity;
}

Rgba8 ProceduralSky::GetAmbientColor() const
{
	float sunHeight = m_sunDirection.z;
	
	// 日出/日落时的颜色
	if (sunHeight < 0.2f && sunHeight > -0.2f)
	{
		float t = (sunHeight + 0.2f) / 0.4f; // 0-1
		// 从深蓝到浅蓝
		return Rgba8(
			(unsigned char)(30 + 100 * t),
			(unsigned char)(40 + 100 * t),
			(unsigned char)(60 + 120 * t),
			255
		);
	}
	// 白天
	else if (sunHeight > 0.2f)
	{
		return Rgba8(140, 160, 200, 255);
	}
	// 夜晚
	else
	{
		return Rgba8(20, 25, 40, 255);
	}
}

Rgba8 ProceduralSky::GetSunColor() const
{
	float sunHeight = m_sunDirection.z;
	
	if (sunHeight > 0.0f)
	{
		// 白天：从橙色（日出）到白色（正午）
		float t = GetClamped(sunHeight * 2.0f, 0.0f, 1.0f);
		return Rgba8(
			255,
			(unsigned char)(200 + 55 * t),
			(unsigned char)(150 + 105 * t),
			255
		);
	}
	else
	{
		// 太阳在地平线下，返回月光颜色
		return Rgba8(100, 100, 150, 255);
	}
}

void ProceduralSky::InitializeSkyMesh()
{
	std::vector<Vertex_PCU> vertices;
	
	AddVertsForSkySphere3D(vertices, Vec3(0.f, 0.f, 0.f), 10.f, Rgba8::HILDA, AABB2::ZERO_TO_ONE,32,16);
	
	m_skyVertexCount = (int)vertices.size();
	m_skyVertexBuffer = m_renderer->CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_renderer->CopyGameVertexBufferToGPU(vertices.data(), (unsigned int)vertices.size(), m_skyVertexBuffer);
}

void ProceduralSky::SetSkyConstantsToGPU()
{
	if (!m_skyConstantBuffer)
		return;

	// ========== 组装常量数据 ==========
	SkyConstants constants = {};

	// 太阳和月亮方向
	constants.sunDirection = m_sunDirection;
	constants.timeOfDay = m_worldTimeInDays;

	constants.moonDirection = m_moonDirection;
	constants.sunIntensity = m_atmosphereParams.sunIntensity;

	// Rayleigh 散射参数（波长依赖的大气散射）
	constants.rayleighScattering = m_atmosphereParams.rayleighScattering;
	constants.atmosphereRadius = m_atmosphereParams.atmosphereRadius;

	// Mie 散射参数（雾霾和光晕效果）
	constants.mieScattering = m_atmosphereParams.mieScattering;
	constants.planetRadius = m_atmosphereParams.planetRadius;

	// 散射高度和相位参数
	constants.rayleighScaleHeight = m_atmosphereParams.rayleighScaleHeight;
	constants.mieScaleHeight = m_atmosphereParams.mieScaleHeight;
	constants.mieG = m_atmosphereParams.mieG;
	constants.moonIntensity = 0.3f;

	// 相机位置设为原点（因为模型矩阵已经让天空球跟随相机）
	// 在着色器中计算 viewDir = normalize(worldPos - cameraPos)
	// 由于 worldPos 是相对于相机的，cameraPos 为原点即可
	constants.cameraPosition = Vec3(0.0f, 0.0f, 0.0f);
	constants.starBrightness = m_starBrightness;

	// ==== = 传递预计算的颜色 ==== =
	m_currentSkyHorizonColor.GetAsFloats(constants.SkyHorizonColor);
	m_currentSkyZenithColor.GetAsFloats(constants.SkyZenithColor);
	m_currentUnderHorizonColor.GetAsFloats(constants.UnderHorizonColor);
	m_currentUnderNadirColor.GetAsFloats(constants.UnderNadirColor);

	// ========== 使用 Renderer 的通用接口上传和绑定 ==========
	m_renderer->CopyConstantBufferToGPU(&constants, sizeof(SkyConstants), m_skyConstantBuffer);
	m_renderer->BindConstantBuffer(4, m_skyConstantBuffer);
}

void ProceduralSky::CalculateSkyColorsForCurrentTime()
{
	// 计算时间分数（0.0 到 1.0）
	float fractionalTime = m_worldTimeInDays;

	// ===== 根据时间插值天空颜色 =====
	Rgba8 horizonColor;
	Rgba8 zenithColor;

	if (fractionalTime >= 0.0f && fractionalTime < 0.25f)
	{
		// 午夜 → 黎明
		float t = fractionalTime / 0.25f;
		horizonColor = Interpolate(m_skyMidnightHorizon, m_skyDawnHorizon, t);
		zenithColor = Interpolate(m_skyMidnightZenith, m_skyDawnZenith, t);
	}
	else if (fractionalTime >= 0.25f && fractionalTime < 0.5f)
	{
		// 黎明 → 正午
		float t = (fractionalTime - 0.25f) / 0.25f;
		horizonColor = Interpolate(m_skyDawnHorizon, m_skyNoonHorizon, t);
		zenithColor = Interpolate(m_skyDawnZenith, m_skyNoonZenith, t);
	}
	else if (fractionalTime >= 0.5f && fractionalTime < 0.75f)
	{
		// 正午 → 黄昏
		float t = (fractionalTime - 0.5f) / 0.25f;
		horizonColor = Interpolate(m_skyNoonHorizon, m_skyDuskHorizon, t);
		zenithColor = Interpolate(m_skyNoonZenith, m_skyDuskZenith, t);
	}
	else
	{
		// 黄昏 → 午夜
		float t = (fractionalTime - 0.75f) / 0.25f;
		horizonColor = Interpolate(m_skyDuskHorizon,m_skyMidnightHorizon, t);
		zenithColor = Interpolate(m_skyDuskZenith,m_skyMidnightZenith, t);
	}

	// 计算地下颜色（更暗的版本）
	Rgba8 underHorizonColor = Rgba8(
		horizonColor.r / 3,
		horizonColor.g / 3,
		horizonColor.b / 3
	);

	Rgba8 underNadirColor = Rgba8(
		horizonColor.r / 6,
		horizonColor.g / 6,
		horizonColor.b / 6
	);

	// 保存计算结果
	m_currentSkyHorizonColor = horizonColor;
	m_currentSkyZenithColor = zenithColor;
	m_currentUnderHorizonColor = underHorizonColor;
	m_currentUnderNadirColor = underNadirColor;
}

