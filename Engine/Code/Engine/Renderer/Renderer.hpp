#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include <vector>
#include <Engine/Core/Vertex_PCUTBN.hpp>
#include "Engine/Renderer/PointLight.hpp"
#include "SpotLight.hpp"

class Shader;
class Window;
class Texture;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Image;
class TextureCube;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilView;
struct ID3D11DepthStencilState;

#if defined(OPAQUE)
#undef OPAQUE
#endif

enum class BlendMode
{
	OPAQUE,
	ALPHA,
	ADDITIVE,
	COUNT
};

enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	COUNT
};

enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	READ_ONLY_ALWYAS,
	READ_ONLY_LESS_EQUAL,
	READ_WRITE_LESS_EQUAL,
	COUNT
};

enum class VertexType
{
	VERTEX_PCU,
	VERTEX_PCUTBN
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
	void		Startup();
	void		BeginFrame();
	void		EndFrame();
	void		Shutdown();

	void		ClearScreen(const Rgba8& clearColor);
	void		BeginCamera(const Camera& camera);
	void		EndCamera(const Camera& camera);
	void		DrawVertexArray(int numVertexs, const Vertex_PCU* vertexs);
	void		DrawVertexArray(const std::vector<Vertex_PCU>& vertexs);
	void		DrawVertexArray_WithTBN(std::vector<Vertex_PCUTBN> const& vertexs);
	void		DrawVertexArray(std::vector<Vertex_PCU> const& vertexs, std::vector<unsigned int> const& indexes);
	void		DrawVertexArray_WithTBN(std::vector<Vertex_PCUTBN> const& vertexs, std::vector<unsigned int> const& indexes);
	void		DrawVertexArray_WithTBN(std::vector<Vertex_PCUTBN> const& vertexs, std::vector<unsigned int> const& indexes,
					VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer);
	
	//void		DrawVertexArrayTest(const std::vector<Vertex_PCU>& vertexs,Mat44 const& localToWorld);

	Image*		CreateImageFromFile(char const* imagePath);
	Texture*	CreateOrGetTextureFromFile(char const* imageFilePath);
	Texture*	GetTextureFromFileName(char const* imageFilePath);
	Texture*	CreateTextureFromImage(const Image& image);
	void		BindTexture(Texture* texture);

	TextureCube* CreateOrGetCubeTextureFromFiles(const std::string filePaths[6]);
	TextureCube* GetTextureCubeFromFileName(char const* firstFilePath);
	TextureCube* CreateCubeTextureFromFiles(const std::string filePaths[6]);
	void		 BindTextureCube(TextureCube* textureCube);
	
	BitmapFont* CreateOrGetBitmapFont(char const* imageFilePath);
	BitmapFont* GetBitmapFontFromFileName(char const* imageFilePath);
	BitmapFont* CreateBitmapFont(char const* imageFilePath);
	
	void		SetBlendMode(BlendMode blendMode);
	void		SetSamplerMode(SamplerMode samplerMode);
	void        SetDepthMode(DepthMode depthMode);
	void        SetRasterizerMode(RasterizerMode rasterizerMode);


	void SetModelConstants(const Mat44& modelToWorldTransform = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);
	void SetLightConstants(Vec3 const& sunDirection= Vec3(2.f, 1.f, -1.f), float sunIntensity=0.85f, float ambientIntensity=0.35f);
	void SetPointLightsConstants(const std::vector<PointLight>& lights);
	void SetSpotLightsConstants(const std::vector<SpotLight>& lights);

	Shader* CreateShaderFromFile(char const* shaderName, VertexType vertexType = VertexType::VERTEX_PCU);
	void			BindShader(Shader* shader);

	VertexBuffer* CreateVertexBuffer(const unsigned int verticeCount, unsigned int stride);
	IndexBuffer* CreateIndexBuffer(unsigned int size);
	
public:
	std::vector<Texture*>		m_loadedTextures;
	std::vector<TextureCube*>       m_loadedCubeTextures;
	std::vector<BitmapFont*>	m_loadedFonts;

private:
	RendererConfig			m_config;

protected:
	ID3D11Device*			m_device = nullptr;
	ID3D11DeviceContext*	m_deviceContext = nullptr;
	IDXGISwapChain*			m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11RasterizerState*	m_rasterizerState = nullptr;

// 	std::vector<uint8_t>	m_vertexShaderByteCode;
// 	std::vector<uint8_t>	m_pixelShaderByteCode;

	std::vector<Shader*>	m_loadedShaders;
	Shader*					m_currentShader = nullptr;
	Shader*					m_defaultShader = nullptr;
	VertexBuffer*			m_immediateVBO = nullptr;
	VertexBuffer*			m_immediateVBO_WithTBN = nullptr;
	IndexBuffer*			m_immediateIBO = nullptr;
	ConstantBuffer*			m_cameraCBO = nullptr;
	ConstantBuffer*         m_modelCBO = nullptr;
	ConstantBuffer*			m_lightCBO = nullptr;
	ConstantBuffer*			m_pointLightCBO = nullptr;
	ConstantBuffer*			m_spotLightCBO = nullptr;
	const Texture*			m_defaultTexture = nullptr;
protected:
	Shader*			CreateShaderFromSource(char const* shaderName, char const* shaderSource, VertexType vertexType = VertexType::VERTEX_PCU);
	
	bool			CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name,
					char const* source, char const* enetryPoint, char const* target);
	
	//----------------------------------------------------------------

	void			CopyCPUToGPU(const void* data, unsigned int verticeCount, VertexBuffer* vbo);
	void			BindVertextBuffer(VertexBuffer* vbo);
	void			BindVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo);
	void			DrawVertexBuffer(VertexBuffer* vbo, unsigned int vertexCount);

	void			DrawIndexedVertexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, unsigned int indexCount);
	void			CopyCPUToGPU(const void* data, unsigned int count, IndexBuffer* ibo);
	//----------------------------------------------------------------
	ConstantBuffer* CreateConstantBuffer(const unsigned int size);
	void			CopyCPUToGPU(const void* data, unsigned int size, ConstantBuffer* cbo);
	void			BindConstantBuffer(int slot, ConstantBuffer* cbo);
	//----------------------------------------------------------------
	
	//----------------------------------------------------------------
	void				SetStateIfChanged();
	ID3D11BlendState*	m_blendState = nullptr;
	BlendMode			m_desiredBlendMode = BlendMode::ALPHA;
	ID3D11BlendState*	m_blendStates[(int)(BlendMode::COUNT)] = {};
	//----------------------------------------------------------------
	ID3D11SamplerState* m_samplerState = nullptr;
	SamplerMode			m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};
	//----------------------------------------------------------------
	RasterizerMode  m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};
	//----------------------------------------------------------------
	ID3D11Texture2D* m_depthStencilTexture = nullptr;
	ID3D11DepthStencilView* m_depthStencilDSV = nullptr;
	ID3D11DepthStencilState* m_depthStencilStates[(int)(DepthMode::COUNT)] = {};
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	DepthMode m_desiredDepthMode = DepthMode::DISABLED;
};