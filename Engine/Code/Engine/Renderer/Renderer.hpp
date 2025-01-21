#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include "Engine/Renderer/BitmapFont.hpp"


class Window;
class Texture;

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
};

struct RendererConfig
{
	Window* m_window = nullptr;
};

class Renderer
{
public:
	Renderer(RendererConfig rendererConfig);
	~Renderer();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(const Rgba8& clearColor);
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawVertexArray(int numVertexs, const Vertex_PCU* vertexs);
	void DrawVertexArray(const std::vector<Vertex_PCU>& vertexs);
	Texture* CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData);
	void BindTexture(Texture* texture);
	Texture* CreateOrGetTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromFile(char const* imageFilePath);
	BitmapFont* CreateOrGetBitmapFont(char const* imageFilePath);
	BitmapFont* CreateBitmapFont(char const* imageFilePath);
	//another vector type func .data 
	void CreateRenderingContext();
	Texture* GetTextureForFileName(char const* imageFilePath);
	BitmapFont* GetBitmapFontFromFileName(char const* imageFilePath);
	void SetBlendMode(BlendMode blendMode);
public:
	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont* > m_loadedFonts;
private:
	RendererConfig m_config;
};