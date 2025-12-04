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
#include "Engine/Renderer/Material.hpp"

class Shader;
class Window;
class Texture;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Image;
class TextureCube;
class Material;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilView;
struct ID3D11DepthStencilState;
struct D3D11_VIEWPORT;

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
	SOLID_CULL_FRONT,
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
	VERTEX_PCUTBN,
	NOVA2D_PARTICLE_INSTANCED
};

struct RendererConfig
{
	Window* m_window = nullptr;
	bool m_enableShadow = false;

	bool m_imguiInitialized = false;
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
	
	//-----------------------Texture2D/ Image------------------------------------------------------------
	Image*		CreateImageFromFile(char const* imagePath);
	Texture*	CreateOrGetTextureFromFile(char const* imageFilePath, unsigned int mipLevels=1);
	Texture*	GetTextureFromFileName(char const* imageFilePath);
	Texture*	CreateTextureFromImage(const Image& image, unsigned int mipLevels = 1);
	void		BindTexture(Texture* texture, Texture* normalTexture=nullptr,Texture* sgeTexture=nullptr);
	//void		BindTextureWithNormal(Texture* diffuseTexture,Texture* normalTexture);

	//----------------Cube Sky Box/ Cube Texture---------------------------------------------------
	TextureCube* CreateOrGetCubeTextureFromFiles(const std::string filePaths[6]);
	TextureCube* GetTextureCubeFromFileName(char const* firstFilePath);
	TextureCube* CreateCubeTextureFromFiles(const std::string filePaths[6]);
	void		 BindTextureCube(TextureCube* textureCube,int slot=0);
	
	//-----------------------BitmapFont------------------------------------------------------------
	BitmapFont* CreateOrGetBitmapFont(char const* imageFilePath);
	BitmapFont* GetBitmapFontFromFileName(char const* imageFilePath);
	BitmapFont* CreateBitmapFont(char const* imageFilePath);
	
	//-----------------------Set render mode------------------------------------------------------------
	void		SetBlendMode(BlendMode blendMode);
	void		SetSamplerMode(SamplerMode samplerMode, 
		SamplerMode normalSamplerMode=SamplerMode::BILINEAR_WRAP);
	void        SetDepthMode(DepthMode depthMode);
	void        SetRasterizerMode(RasterizerMode rasterizerMode);

	//-----------------------Set Constants------------------------------------------------------------
	void SetModelConstants(const Mat44& modelToWorldTransform = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);
	void SetLightConstants(Vec3 const& sunDirection= Vec3(2.f, 1.f, -1.f), float sunIntensity=0.85f, float ambientIntensity=0.35f);
	void SetPointLightsConstants(const std::vector<PointLight>& lights);
	void SetSpotLightsConstants(const std::vector<SpotLight>& lights);
	void SetShadowConstants(Mat44 const& lightViewProjectionMat);
	void SetPerFrameConstants(float time,int debugInt,float debugFloat);
	void SetPBRConstants(Vec3 const& albedo, float metallic, float roughness, float ao,
		Rgba8 emissiveColor, float emissiveStrength);

	//-----------------------Shader-------------------------------------------------------------------
	Shader* CreateShaderFromFile(char const* shaderName, VertexType vertexType = VertexType::VERTEX_PCU);
	void			BindShader(Shader* shader);

	//-----------------------Buffer-------------------------------------------------------------------
	VertexBuffer* CreateVertexBuffer(const unsigned int verticeCount, unsigned int stride, bool isPerInstance = false);
	IndexBuffer* CreateIndexBuffer(unsigned int size);

	void CopyGameVertexBufferToGPU(const void* data, unsigned int verticeCount, VertexBuffer* vbo);
	void CopyGameIndexBufferToGPU(const void* data, unsigned int count, IndexBuffer* ibo);
	void DrawGameIndexedVertexBuffer(VertexBuffer* vbo, IndexBuffer* ibo);
	void DrawGameVertexBuffer(VertexBuffer* vbo);

	//---------------------- Constants Buffer -----------------------------------
	ConstantBuffer* CreateConstantBuffer(const unsigned int size);
	void			CopyConstantBufferToGPU(const void* data, unsigned int size, ConstantBuffer* cbo);
	void			BindConstantBuffer(int slot, ConstantBuffer* cbo);
	//-----------------------Shadow-------------------------------------------------------------------
	void InitializeShadowMapping();
	void BeginShadowMapRender(Mat44 const& lightViewProjection);
	void EndShadowMapRender();
	Mat44 GetDirectLightProjectionMat(Vec3 const& sunDirection, Vec3 const& sceneCenter, float sceneRadius);
	void BindShadowTexture();
	void SetShadowSampleState();

	//----------------------Imgui--------------------------------------------
	void InitializeImgui();
	void ShutdownImgui();
	void BeginImguiFrame();
	void RenderImguiFrame();
	bool IsImGuiEnabled() { return m_enableImgui; };

	//---------------------New Material System-------------------------------
	// Use this
	void BindMaterial(Material* material);
	// bind material calls these funcs
	void SetTextureSlot(TextureSlot slot=TextureSlot::SLOT_BASE_COLOR, Texture* texture=nullptr); // call by each child material
	void ApplyTextureBindings();
	void ClearTextureSlots();

	// ---------------------Draw Instanced -----------------------------------
	void DrawInstanced(VertexBuffer* vertexVBO,     // 顶点数据（四边形）
		VertexBuffer* instanceVBO,					// 实例数据（粒子）
		unsigned int vertexCount,					// 4（四边形顶点数）
		unsigned int instanceCount);				// 粒子数量

	// ========== 水体渲染相关方法 ==========

	// 创建可作为RenderTarget的纹理
	Texture* CreateOrGetRenderTargetTexture(IntVec2 dimensions, char const* name);
	Texture* CreateOrGetDepthTexture(IntVec2 dimensions, char const* name);

	// 拷贝当前帧缓冲到纹理
	void CopyFramebufferToTexture(Texture* destTexture);

	// 拷贝深度缓冲到纹理
	void CopyDepthToTexture(Texture* destTexture);

	// 绑定纹理到特定槽位（扩展现有BindTexture）
	void BindTextureToSlot(int slot, Texture const* texture);

	// 获取屏幕尺寸
	IntVec2 GetScreenDimensions() const;


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
	ConstantBuffer*			m_shadowCBO = nullptr;
	ConstantBuffer*			m_perFrameCBO = nullptr;
	ConstantBuffer*			m_pbrCBO = nullptr;

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
	void			CopyCPUToGPU(const void* data, unsigned int size, ConstantBuffer* cbo);
	
	//----------------------------------------------------------------
	
	//----------------------------------------------------------------
	void				SetStateIfChanged();
	ID3D11BlendState*	m_blendState = nullptr;
	BlendMode			m_desiredBlendMode = BlendMode::ALPHA;
	ID3D11BlendState*	m_blendStates[(int)(BlendMode::COUNT)] = {};
	//----------------------------------------------------------------
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11SamplerState* m_normalSamplerState = nullptr;
	SamplerMode			m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	SamplerMode			m_desiredNormalSamplerMode = SamplerMode::BILINEAR_WRAP;
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
	//------------------------shadow map------------------------------
	ID3D11Texture2D* m_shadowMapTexture = nullptr;
	ID3D11DepthStencilView* m_shadowDepthView = nullptr;
	ID3D11ShaderResourceView* m_shadowResourceView = nullptr;
	ID3D11SamplerState* m_comparisonSampler_point=nullptr;
// 	ID3D11RasterizerState* m_shadowGenerateRasterizerStates= m_rasterizerStates[(int)(RasterizerMode::SOLID_CULL_BACK)];
// 	ID3D11RasterizerState* m_shadowDrawRasterizerStates= m_rasterizerStates[(int)(RasterizerMode::SOLID_CULL_NONE)];
	D3D11_VIEWPORT* m_shadowViewport=nullptr;

	float m_shadowDimensionX = 2048;
	float m_shadowDimensionY = 2048;
	int m_shadowSamplerSlotIndex = 2;
	int m_shadowTexSlotIndex = 3;

	bool m_enableImgui = false;

	// -------------New Material System-----------------------
	static constexpr int MAX_TEXTURE_SLOTS = 16;
	ID3D11ShaderResourceView* m_boundTextures[16] = { nullptr }; // track bound textures

private:
	void CreateInputLayout_PCU(Shader* shader);
	void CreateInputLayout_PCUTBN(Shader* shader);
	void CreateInputLayout_ParticleInstanced(Shader* shader);
};