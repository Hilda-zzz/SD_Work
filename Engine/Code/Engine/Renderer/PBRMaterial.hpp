#pragma once
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"

class Renderer;
class Texture;

class PBRMaterial : public Material {
public:
	PBRMaterial(Texture* baseTex, Texture* normTex, Texture* mrTex,Texture* aoTex,Texture* emissiveTex);
	Texture* m_albedoTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_metallicRoughnessTexture = nullptr;
	Texture* m_AOTexture = nullptr;
	Texture* m_emissiveTexture = nullptr;

	Vec3 m_albedo = Vec3(1.0f, 1.0f, 1.0f);
	float m_metallic = 0.0f;
	float m_roughness = 0.5f;

	MaterialType GetType() const override { return MaterialType::PBR_METALLIC_ROUGHNESS; }

	void Bind(Renderer* renderer) override;


	Shader* GetShader() const override;

};