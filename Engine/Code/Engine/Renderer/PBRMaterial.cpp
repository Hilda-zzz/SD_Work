#include "PBRMaterial.hpp"
#include "Engine/Renderer/Renderer.hpp"

PBRMaterial::PBRMaterial(Texture* baseTex, Texture* normTex, Texture* mrTex, Texture* aoTex, Texture* emissiveTex)
	:m_albedoTexture(baseTex)
	,m_normalTexture(normTex)
	,m_metallicRoughnessTexture(mrTex)
	,m_AOTexture(aoTex)
	,m_emissiveTexture(emissiveTex)
{
}

void PBRMaterial::Bind(Renderer* renderer)
{
	//renderer->BindShader(renderer->GetPBRShader());
	renderer->SetTextureSlot(TextureSlot::SLOT_BASE_COLOR, m_albedoTexture);
	renderer->SetTextureSlot(TextureSlot::SLOT_NORMAL, m_normalTexture);
	renderer->SetTextureSlot(TextureSlot::SLOT_MATERIAL_PROPS, m_metallicRoughnessTexture);
	//renderer->SetPBRMaterialConstants(*this);
}

Shader* PBRMaterial::GetShader() const
{
	// return g_theRenderer->GetPBRShader();
	return nullptr;
}
