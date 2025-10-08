#pragma once
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Texture;

class BlinnPhongMaterial : public Material 
{
public:
	Texture* diffuseTexture = nullptr;
	Texture* normalTexture = nullptr;
	Texture* specularGlossinessEmissive = nullptr;
	Rgba8 tint = Rgba8::WHITE;

	MaterialType GetType() const override { return MaterialType::BLINN_PHONG; }

	void Bind(Renderer* renderer) override 
	{
		 // renderer->BindShader(renderer->GetBlinnPhongShader());
		renderer->BindTexture(diffuseTexture, normalTexture, specularGlossinessEmissive);
	}

	Shader* GetShader() const override 
	{
		// return g_theRenderer->GetBlinnPhongShader();
	}
};