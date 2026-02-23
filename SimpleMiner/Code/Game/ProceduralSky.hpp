#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Camera.hpp"

class Renderer;
class Shader;
class VertexBuffer;
class ConstantBuffer;
class Texture;

// 大气散射参数
struct AtmosphereParams
{
	// 散射系数（无量纲，适配游戏尺度）
	Vec3 rayleighScattering = Vec3(0.02f, 0.05f, 0.1f);
	Vec3 mieScattering = Vec3(0.5f, 0.5f, 0.5f);

	// 消光系数（通常是 mieScattering 的 1.1 倍）
	float mieExtinction = 0.1f;  // mieScattering * 1.1

	// 半径（米）
	float atmosphereRadius = 6471000.0f;  // 6471 km = 6,471,000 米
	float planetRadius = 6371000.0f;      // 6371 km = 6,371,000 米

	// 高度（米）
	float rayleighScaleHeight = 8000.0f;  // 8 km = 8,000 米
	float mieScaleHeight = 1200.0f;       // 1.2 km = 1,200 米

	// 相位函数参数（无量纲）
	float mieG = 0.76f;

	// 光源强度（无量纲）
	float sunIntensity = 100.0f;
	float moonIntensity = 2.0f;

	// 相机位置（米，通常接近地表）
	Vec3 cameraPosition = Vec3(0.0f, 0.0f, 0.0f);  // 假设在地表
};

// 天空渲染常量缓冲区
struct SkyConstants
{
	Vec3 sunDirection;
	float timeOfDay;        // 0-24小时
	
	Vec3 moonDirection;
	float sunIntensity;
	
	Vec3 rayleighScattering;
	float atmosphereRadius;
	
	Vec3 mieScattering;
	float planetRadius;
	
	float rayleighScaleHeight;
	float mieScaleHeight;
	float mieG;
	float moonIntensity;
	
	Vec3 cameraPosition;
	float starBrightness;

	// sky color
	float SkyHorizonColor[3];    
	float Padding2;

	float SkyZenithColor[3];
	float Padding3;

	float UnderHorizonColor[3];
	float Padding4;

	float UnderNadirColor[3];
	float Padding5;
};

class ProceduralSky
{
public:
	ProceduralSky(Renderer* renderer);
	~ProceduralSky();
	
	// 更新天空状态
	void Update(float worldTimeInDays);
	
	// 渲染天空球
	void Render();

	void SetSunDirection(const Vec3& direction);
	void SetMoonDirection(const Vec3& direction);
	void SetSunIntensity(float intensity);
	void SetMoonIntensity(float intensity);
	
	// 获取天空光照信息（用于场景光照）
	Vec3 GetSunDirection() const { return m_sunDirection; }
	Vec3 GetMoonDirection() const { return m_moonDirection; }
	Rgba8 GetAmbientColor() const;
	Rgba8 GetSunColor() const;
	
	// 调试和配置
	void SetAtmosphereParams(const AtmosphereParams& params) { m_atmosphereParams = params; }
	AtmosphereParams& GetAtmosphereParams() { return m_atmosphereParams; }
	
private:
	void InitializeSkyMesh();
	void SetSkyConstantsToGPU();

	void CalculateSkyColorsForCurrentTime();
	
private:
	Renderer* m_renderer = nullptr;
	
	// Texture
	Texture* m_moonTexture = nullptr;


	// 天空球网格
	VertexBuffer* m_skyVertexBuffer = nullptr;
	VertexBuffer* m_starVertexBuffer = nullptr;
	int m_skyVertexCount = 0;
	int m_starVertexCount = 0;
	
	// 着色器
	Shader* m_skyShader = nullptr;
	Shader* m_starShader = nullptr;
	
	// 常量缓冲区
	ConstantBuffer* m_skyConstantBuffer = nullptr;
	
	// 时间系统
	float m_worldTimeInDays = 0.f;
	
	// 天体位置
	Vec3 m_sunDirection;
	Vec3 m_moonDirection;
	
	// 大气参数
	AtmosphereParams m_atmosphereParams;
	
	// 星星亮度（夜晚时显示）
	float m_starBrightness = 0.0f;

	// sky color
	// 地平线颜色（4个关键帧）
	Rgba8 m_skyMidnightHorizon = Rgba8(30, 40, 70);       // 深蓝
	Rgba8 m_skyDawnHorizon = Rgba8(255, 180, 120);        // 橙粉朝霞 ⭐
	Rgba8 m_skyNoonHorizon = Rgba8(220, 240, 255);        // 浅蓝
	Rgba8 m_skyDuskHorizon = Rgba8(255, 160, 80);         // 金橙晚霞 ⭐

	// 天顶颜色（4个关键帧）
	Rgba8 m_skyMidnightZenith = Rgba8(10, 15, 30);        // 黑蓝
	Rgba8 m_skyDawnZenith = Rgba8(100, 150, 220);         // 冷蓝天空 ⭐
	Rgba8 m_skyNoonZenith = Rgba8(60, 120, 200);          // 深蓝
	Rgba8 m_skyDuskZenith = Rgba8(180, 100, 140);         // 紫红天空 ⭐

	// 计算好的当前颜色（每帧更新）
	Rgba8 m_currentSkyHorizonColor;
	Rgba8 m_currentSkyZenithColor;
	Rgba8 m_currentUnderHorizonColor;
	Rgba8 m_currentUnderNadirColor;

};
