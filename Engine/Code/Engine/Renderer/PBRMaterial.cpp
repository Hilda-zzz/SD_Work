#include "PBRMaterial.hpp"
#include "Engine/Renderer/Renderer.hpp"

PBRMaterial::PBRMaterial()
	:m_albedoTexture(nullptr)
	, m_normalTexture(nullptr)
	, m_metallicRoughnessTexture(nullptr)
	, m_AOTexture(nullptr)
	, m_emissiveTexture(nullptr)
{
}

PBRMaterial::PBRMaterial(Texture* baseTex, Texture* normTex, Texture* mrTex, Texture* aoTex, Texture* emissiveTex)
	:m_albedoTexture(baseTex)
	,m_normalTexture(normTex)
	,m_metallicRoughnessTexture(mrTex)
	,m_AOTexture(aoTex)
	,m_emissiveTexture(emissiveTex)
{
}

void PBRMaterial::SetParameters(float metallic, float roughness, Vec3 const& albedo)
{
	m_metallic = metallic;
	m_albedo = albedo;
	m_roughness = roughness;
}

void PBRMaterial::Bind(Renderer* renderer) const
{
	renderer->ClearTextureSlots();

	renderer->SetTextureSlot(TextureSlot::SLOT_BASE_COLOR, m_albedoTexture);
	renderer->SetTextureSlot(TextureSlot::SLOT_NORMAL, m_normalTexture);
	renderer->SetTextureSlot(TextureSlot::SLOT_MATERIAL_PROPS, m_metallicRoughnessTexture);
	renderer->SetTextureSlot(TextureSlot::SLOT_AO, m_AOTexture);
	renderer->SetTextureSlot(TextureSlot::SLOT_EMISSIVE, m_emissiveTexture);
	renderer->SetPBRConstants(m_albedo,m_metallic,m_roughness,m_ao,Rgba8::WHITE,1.f);

	renderer->ApplyTextureBindings();
}

