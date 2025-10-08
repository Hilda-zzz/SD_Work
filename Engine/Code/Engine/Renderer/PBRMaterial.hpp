#pragma once
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Texture;

class PBRMaterial : public Material {
public:
	Texture* m_albedoTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_metallicRoughnessAO = nullptr;

	Vec3 m_albedo = Vec3(1.0f, 1.0f, 1.0f);
	float m_metallic = 0.0f;
	float m_roughness = 0.5f;

	MaterialType GetType() const override { return MaterialType::PBR_METALLIC_ROUGHNESS; }

	void Bind(Renderer* renderer) override 
	{
		//renderer->BindShader(renderer->GetPBRShader());
		renderer->SetTextureSlot(TextureSlot::SLOT_BASE_COLOR, m_albedoTexture);
		renderer->SetTextureSlot(TextureSlot::SLOT_NORMAL, m_normalTexture);
		renderer->SetTextureSlot(TextureSlot::SLOT_MATERIAL_PROPS, m_metallicRoughnessAO);
		//renderer->SetPBRMaterialConstants(*this);
	}

	Shader* GetShader() const override 
	{
		// return g_theRenderer->GetPBRShader();
	}
};