#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Camera.hpp"
//-------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
//-------------------------------------------------------------
#if defined(_DEBUG)
#define ENGINE_DEBUG_RENDER
#endif

#if defined(ENGINE_DEBUG_RENDER)
void* m_dxgiDebug = nullptr;
void* m_dxgiDebugModule = nullptr;
#endif

#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma  comment(lib,"dxguid.lib")
#pragma comment(lib,"dxguid.lib")
#endif
#include "TextureCube.hpp"
#include "PointLight.hpp"
#include "../Math/MathUtils.hpp"
//-------------------------------------------------------------
constexpr int MAX_POINT_LIGHTS= 10;
constexpr int MAX_SPOT_LIGHTS = 10;
//-------------------------------------------------------------
#define DX_SAFE_RELEASE(dxObject)			\
{											\
	if ((dxObject) != nullptr)				\
	{										\
		(dxObject)->Release();				\
		(dxObject)=nullptr;					\
	}										\
}
//-------------------------------------------------------------
#if defined(OPAQUE)
#undef OPAQUE
#endif
//-------------------------------------------------------------
struct CameraConstants
{
	Mat44 WorldToCameraTransform;
	Mat44 CameraToRenderTransform;
	Mat44 RenderToClipTransform;
};
static const int k_cameraConstantsSlot = 2;

struct ModelConstants
{
	Mat44 ModelToWorldTransform;
	float ModelColor[4];
};
static const int k_modelConstantsSlot = 3;

struct LightConstants
{
	Vec3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float Padding[3];
};
static const int k_lightConstantsSlot = 1;

struct PointLightConstants
{
	PointLight PointLights[MAX_POINT_LIGHTS];
	uint32_t ActivePointLightCount;
	float Padding[3];
};
static const int k_pointLightConstantsSlot = 4;

struct SpotLightConstants
{
	SpotLight SpotLights[MAX_SPOT_LIGHTS];
	uint32_t ActiveSpotLightCount;
	float Padding[3];
};
static const int k_spotLightConstantsSlot = 5;

struct ShadowConstants
{
	Mat44 LightViewProjection; 
};
static const int k_shadowConstantsSlot = 6;

//-------------------------------------------------------------
BitmapFont* g_testFont = nullptr;

Renderer::Renderer(RendererConfig rendererConfig):m_config(rendererConfig)
{
	
}

Renderer::~Renderer()
{
	
}

void Renderer::Startup()
{
	//create debug module
#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*)::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("could not lead load dxgidebug.dll");
	}

	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))
		(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module")
	}
#endif

	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	m_desiredBlendMode = BlendMode::ALPHA;
	m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	m_desiredDepthMode = DepthMode::READ_WRITE_LESS_EQUAL;

	//create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = g_theWindow->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = g_theWindow->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)g_theWindow->GetHwnd();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//---------------------------------------------------------
	//Call D3D11CreateDeviceAndSwapChain.
	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
		&m_swapChain, &m_device, nullptr, &m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Couldn't create D3D 11 device and swap chain.");
	}
	//---------------------------------------------------------
	//save back buffer view
	//get back buffer texture
	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swap chain buffer.");
	}

	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could create render target view for swap chain buffer.");
	}

	backBuffer->Release();

	//--------------------------------------------------------------------------------
	//m_currentShader =CreateShader("Default", g_shaderSource);
	m_defaultShader = CreateShaderFromSource("Default", g_shaderSource);
	BindShader(m_defaultShader);
	//--------------------------------------------------------------------------------
	m_immediateVBO = CreateVertexBuffer(24, sizeof(Vertex_PCU));
	m_immediateIBO = CreateIndexBuffer(24);
	m_immediateVBO_WithTBN=CreateVertexBuffer(24, sizeof(Vertex_PCUTBN));
	//---------------------------------------------------------
	//Set rasterizer state

	//SOLID_CULL_NONE
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::SOLID_CULL_NONE failed");
	}

	//SOLID_CULL_BACK
	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::SOLID_CULL_BACK failed");
	}

	//SOLID_CULL_FRONT
	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_FRONT]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::SOLID_CULL_FRONT failed");
	}
	

	//WIREFRAME_CULL_NONE
	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::WIREFRAME_CULL_NONE failed");
	}

	//WIREFRAME_CULL_BACK
	rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::WIREFRAME_CULL_BACK failed");
	}


	//m_shadowDrawRasterizerStates = m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE];
	//m_deviceContext->RSSetState(m_rasterizerState);

	//---------------------------------------------------------

	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));
	m_pointLightCBO = CreateConstantBuffer(sizeof(PointLightConstants));
	m_spotLightCBO= CreateConstantBuffer(sizeof(SpotLightConstants));
	m_shadowCBO = CreateConstantBuffer(sizeof(ShadowConstants));
	//---------------------------------------------------------
	// OPAQUE
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
	blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)(BlendMode::OPAQUE)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::OPAQUE failed.");
	}

	// ALPHA
	blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ALPHA]);
	if (FAILED(hr)) { ERROR_AND_DIE("ALPHA blend state creation failed"); }

	// ADDITIVE
	blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ADDITIVE]);
	if (FAILED(hr)) { ERROR_AND_DIE("ADDITIVE blend state creation failed"); }

	//---------------------------------------------------------
	//Sample
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::POINT_CLAMP]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for SamplerMode::Point_Clamp failed.");
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for SamplerMode::BILINEAR_WRAP failed.");
	}

	m_samplerState = m_samplerStates[(int)m_desiredSamplerMode];
	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	//---------------------------------------------------------
	//Create depth stencil texture and view
	D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	depthTextureDesc.Width = m_config.m_window->GetClientDimensions().x;
	depthTextureDesc.Height = m_config.m_window->GetClientDimensions().y;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.SampleDesc.Count = 1;

	hr = m_device->CreateTexture2D(&depthTextureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create texture for depth stencil.");
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilDSV);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil view.");
	}

	//---------------------------------------------------------
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::DISABLE failed.");
	}

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::READ_ONLY_ALWYAS]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::READ_ONLY_ALWYAS failed.");
	}

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::READ_ONLY_LESS_EQUAL]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::READ_ONLY_LESS_EQUAL failed.");
	}

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::READ_WRITE_LESS_EQUAL]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::READ_WRITE_LESS_EQUAL failed.");
	}

	m_depthStencilState = m_depthStencilStates[(int)m_desiredDepthMode];
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	//---------------------------------------------------------
	Image defaultImage = Image(IntVec2(2, 2), Rgba8::WHITE);
	m_defaultTexture = CreateTextureFromImage(defaultImage);
	BindTexture(nullptr);
	//g_testFont =CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	//---------------------------------------------------------------------
	if (m_config.m_enableShadow)
	{
		InitializeShadowMapping();
	}
}

void Renderer::BeginFrame()
{
	//Set render target
	//m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilDSV);
}

void Renderer::EndFrame()
{
	if (m_config.m_window)
	{
		//present
		HRESULT hr;
		hr = m_swapChain->Present(0, 0);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			ERROR_AND_DIE("Device has been lost, spplication will now terminate.");
		}
	}
}

void Renderer::Shutdown()
{
	//DX_SAFE_RELEASE(m_rasterizerState);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);

	//---------------------------------------------------------
	for (Shader* shader : m_loadedShaders)
	{
		delete shader;
	}

	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	delete m_immediateVBO_WithTBN;
	m_immediateVBO_WithTBN = nullptr;

	delete m_immediateIBO;
	m_immediateIBO = nullptr;

	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	delete m_modelCBO;
	m_modelCBO = nullptr;

	delete m_lightCBO;
	m_lightCBO = nullptr;

	delete m_pointLightCBO;
	m_pointLightCBO = nullptr;

	delete m_spotLightCBO;
	m_spotLightCBO = nullptr;

	delete m_shadowCBO;
	m_shadowCBO = nullptr;

	for (int i = 0; i < (int)BlendMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_blendStates[i]);
	}
	//DX_SAFE_RELEASE(m_blendState);

	for (int i = 0; i < (int)SamplerMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}

	for (int i = 0; i < (int)RasterizerMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[i]);
	}

	for (int i = 0; i < (int)DepthMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_depthStencilStates[i]);
	}
	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilDSV);

	for (BitmapFont* bitFont : m_loadedFonts)
	{
		delete bitFont;
		bitFont = nullptr;
	}
	for (Texture* texture : m_loadedTextures)
	{
		delete texture;
	}
	for (TextureCube* textureCube : m_loadedCubeTextures)
	{
		delete textureCube;
	}

	if (m_shadowMapTexture)
	{
		DX_SAFE_RELEASE( m_shadowMapTexture);
	}
	
	if (m_shadowDepthView)
	{
		DX_SAFE_RELEASE(m_shadowDepthView);
	}

	if (m_shadowResourceView)
	{
		DX_SAFE_RELEASE(m_shadowResourceView);
	}
	if (m_comparisonSampler_point)
	{
		DX_SAFE_RELEASE(m_comparisonSampler_point);
	}
	if (m_shadowViewport)
	{
		delete m_shadowViewport;
		m_shadowViewport=nullptr;
	}

#if defined(ENGINE_DEBUG_RENDER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(
		DXGI_DEBUG_ALL,
		(DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
	);

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif

}
//-------------------------------------------------------------

void Renderer::ClearScreen(const Rgba8& clearColor)
{
	float colorAsFloats[4];
	clearColor.GetAsFloats(colorAsFloats);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorAsFloats);
	m_deviceContext->ClearDepthStencilView(m_depthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	if (m_shadowDepthView)
	{
		m_deviceContext->ClearDepthStencilView(m_shadowDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	
}

void Renderer::BeginCamera(const Camera& camera)
{
	//Set viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = camera.GetViewport().m_mins.x;
	viewport.TopLeftY = (float)g_theWindow->GetClientDimensions().y - camera.GetViewport().m_maxs.y;
	viewport.Width = camera.GetViewport().GetDimensions().x;
	viewport.Height = camera.GetViewport().GetDimensions().y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	m_deviceContext->RSSetViewports(1, &viewport);

	CameraConstants camConstants;
	camConstants.WorldToCameraTransform = camera.GetWorldToCameraTransform();
	camConstants.CameraToRenderTransform = camera.GetCameraToRenderTransorm();
	camConstants.RenderToClipTransform = camera.GetRenderToClipTransform();
	CopyCPUToGPU(&camConstants, sizeof(CameraConstants), m_cameraCBO);
	BindConstantBuffer(k_cameraConstantsSlot, m_cameraCBO);

	ModelConstants modelConstants;
	modelConstants.ModelColor[0]=255.f;
	modelConstants.ModelColor[1] = 255.f;
	modelConstants.ModelColor[2] = 255.f;
	modelConstants.ModelColor[3] = 255.f;
	modelConstants.ModelToWorldTransform = Mat44();
	CopyCPUToGPU(&modelConstants, sizeof(modelConstants), m_modelCBO);
	BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
}

void Renderer::EndCamera(const Camera& camera)
{
	UNUSED(camera);
}

void Renderer::DrawVertexArray(int numVertexs, const Vertex_PCU* vertexs)
{
	CopyCPUToGPU(vertexs, numVertexs, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexs);
}

void Renderer::DrawVertexArray(const std::vector<Vertex_PCU>& vertexs)
{
	CopyCPUToGPU(vertexs.data(), (int)vertexs.size(), m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, (int)vertexs.size());
}
void Renderer::DrawVertexArray_WithTBN(std::vector<Vertex_PCUTBN> const& vertexs)
{
	CopyCPUToGPU(vertexs.data(), (int)vertexs.size(), m_immediateVBO_WithTBN);
	DrawVertexBuffer(m_immediateVBO_WithTBN, (int)vertexs.size());
}
void Renderer::DrawVertexArray(const std::vector<Vertex_PCU>& vertexs, std::vector<unsigned int> const& indexes)
{
	CopyCPUToGPU(vertexs.data(), (int)vertexs.size(), m_immediateVBO);
	CopyCPUToGPU(indexes.data(), (int)indexes.size(), m_immediateIBO);
	DrawIndexedVertexBuffer(m_immediateVBO, m_immediateIBO,(int)indexes.size());
}
void Renderer::DrawVertexArray_WithTBN(std::vector<Vertex_PCUTBN> const& vertexs, std::vector<unsigned int> const& indexes)
{
	CopyCPUToGPU(vertexs.data(), (int)vertexs.size(), m_immediateVBO_WithTBN);
	CopyCPUToGPU(indexes.data(), (int)indexes.size(), m_immediateIBO);
	DrawIndexedVertexBuffer(m_immediateVBO_WithTBN, m_immediateIBO, (int)indexes.size());
}
void Renderer::DrawVertexArray_WithTBN(std::vector<Vertex_PCUTBN> const& vertexs, std::vector<unsigned int> const& indexes, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer)
{
// 	CopyCPUToGPU(vertexs.data(), (int)vertexs.size(), m_immediateVBO_WithTBN);
// 	CopyCPUToGPU(indexes.data(), (int)indexes.size(), m_immediateIBO);
// 	DrawIndexedVertexBuffer(m_immediateVBO_WithTBN, m_immediateIBO, (int)indexes.size());

 	CopyCPUToGPU(vertexs.data(), (int)vertexs.size(), vertexBuffer);
 	CopyCPUToGPU(indexes.data(), (int)indexes.size(), indexBuffer);
 	DrawIndexedVertexBuffer(vertexBuffer, indexBuffer, (int)indexes.size());
}
//void Renderer::DrawVertexArrayTest(const std::vector<Vertex_PCU>& vertexs, Mat44 const& localToWorld)
//{
////	ModelConstants modelConstants;
////	modelConstants.ModelToWorldTransform = localToWorld;
////	CopyCPUToGPU(&modelConstants, sizeof(modelConstants), m_modelCBO);
////	BindConstantBuffer(k_modeConstantsSlot, m_modelCBO);
//}
//-------------------------------------------------------------

Image* Renderer::CreateImageFromFile(char const* imagePath)
{
	Image* newImage = new Image(imagePath);
	return newImage;
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureFromFileName(imageFilePath); // You need to write this
	if (existingTexture)
	{
		return existingTexture;
	}

	Image curTexImg = Image(imageFilePath);
	// Never seen this texture before!  Let's load it.
	//Texture* newTexture = CreateTextureFromFile(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(imageFilePath);
	return newTexture;
}

Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(image.GetRawData(), Stringf("CreateTextureFromImage failed for \"%s\" - texelData was null!", image.GetImageFilePath().c_str()));
	//GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(image.GetDimensions().x > 0 && image.GetDimensions().y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", image.GetImageFilePath().c_str(), image.GetDimensions().x, image.GetDimensions().y));

	Texture* newTexture = new Texture();
	newTexture->m_name = image.GetImageFilePath();
	newTexture->m_dimensions = image.GetDimensions();

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = image.GetDimensions().x;
	textureDesc.Height = image.GetDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = image.GetRawData();
	textureData.SysMemPitch = 4 * image.GetDimensions().x;

	HRESULT hr;
	hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".", image.GetImageFilePath().c_str()));
	}

	hr = m_device->CreateShaderResourceView(newTexture->m_texture, NULL, &newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView failed for image file \"%s\".", image.GetImageFilePath().c_str()));
	}

	m_loadedTextures.push_back(newTexture);
	return newTexture;
}

Texture* Renderer::GetTextureFromFileName(char const* imageFilePath)
{
	for (int i = 0; i < static_cast<int>(m_loadedTextures.size()); i++)
	{
		if (m_loadedTextures[i]->GetImageFilePath() == imageFilePath)
		{
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}

void Renderer::BindTexture(Texture* texture)
{
	if (texture)
	{
		m_deviceContext->PSSetShaderResources(0, 1, &texture->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
	}
}

TextureCube* Renderer::CreateOrGetCubeTextureFromFiles(const std::string filePaths[6])
{
	// See if we already have this texture previously loaded
	TextureCube* existingTexture = GetTextureCubeFromFileName(filePaths[0].c_str()); // You need to write this
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	TextureCube* newTexture = CreateCubeTextureFromFiles(filePaths);
	return newTexture;
}

TextureCube* Renderer::GetTextureCubeFromFileName(char const* firstFilePath)
{
	for (int i = 0; i < static_cast<int>(m_loadedCubeTextures.size()); i++)
	{
		if (m_loadedCubeTextures[i]->m_firstPath== firstFilePath)
		{
			return m_loadedCubeTextures[i];
		}
	}
	return nullptr;
}

TextureCube* Renderer::CreateCubeTextureFromFiles(const std::string filePaths[6])
{
	TextureCube* newCubeTex = new TextureCube();
	newCubeTex->m_firstPath = filePaths[0];
// 	newCubeTex->m_filePaths.resize(6);
// 	newCubeTex->m_filePaths[TextureCube::POSITIVE_X] = filePaths[0];
// 	newCubeTex->m_filePaths[TextureCube::NEGATIVE_X] = filePaths[1];
// 	newCubeTex->m_filePaths[TextureCube::POSITIVE_Y] = filePaths[2];
// 	newCubeTex->m_filePaths[TextureCube::NEGATIVE_Y] = filePaths[3];
// 	newCubeTex->m_filePaths[TextureCube::POSITIVE_Z] = filePaths[4];
// 	newCubeTex->m_filePaths[TextureCube::NEGATIVE_Z] = filePaths[5];

	// load 6 temp tex2D
	ID3D11Texture2D* tempTextures[6] = {};
	D3D11_TEXTURE2D_DESC tempDesc = {};

	for (int i = 0; i < 6; i++) 
	{
		Image* image = CreateImageFromFile(filePaths[i].c_str());
		if (!image) 
		{
			// deal with error：release temp texture2D
			for (int j = 0; j < i; j++) 
{
				tempTextures[j]->Release();
			}
			delete newCubeTex;
			ERROR_AND_DIE("Can not find image path when create TextureCube");
		}

		// set the first face as the cube texture's size standard
		if (i == 0) 
		{
			newCubeTex->m_dimensions = IntVec2(image->GetDimensions().x, image->GetDimensions().y);
		}
		else if (newCubeTex->m_dimensions.x != image->GetDimensions().x ||
			newCubeTex->m_dimensions.y != image->GetDimensions().y)
		{
			ERROR_AND_DIE("Cubemap faces must have the same dimensions");
		}

		// temp 2d texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = image->GetDimensions().x;
		desc.Height = image->GetDimensions().y;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = image->GetRawData();
		initData.SysMemPitch = image->GetDimensions().x * 4; // assume that RGBA has 32 bits

		HRESULT hr = m_device->CreateTexture2D(&desc, &initData, &tempTextures[i]);
		if (FAILED(hr)) 
		{
			// error
			for (int j = 0; j < i; j++) 
			{
				tempTextures[j]->Release();
			}
			delete image;
			delete newCubeTex;
			ERROR_AND_DIE("Failed to create texture2D for cube texture.");
		}

		// save the first texture using for create cube texture 
		if (i == 0) 
		{
			tempDesc = desc;
		}

		delete image; 
	}

	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.Width = tempDesc.Width;
	cubeDesc.Height = tempDesc.Height;
	cubeDesc.MipLevels = 1;
	cubeDesc.ArraySize = 6; // 6 faces
	cubeDesc.Format = tempDesc.Format;
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;
	cubeDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cubeDesc.CPUAccessFlags = 0;
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // mark as texture cube

	HRESULT hr = m_device->CreateTexture2D(&cubeDesc, nullptr, &newCubeTex->m_texture);
	if (FAILED(hr)) 
	{
		for (int i = 0; i < 6; i++) 
		{
			tempTextures[i]->Release();
		}
		delete newCubeTex;
		return nullptr;
	}

	// copy all faces to cube texture
	for (int i = 0; i < 6; i++) 
	{
		m_deviceContext->CopySubresourceRegion(
			newCubeTex->m_texture,               // aim resource
			D3D11CalcSubresource(0, i, 1),       // index
			0, 0, 0,                             
			tempTextures[i],                     // source
			0,                                   
			nullptr                              // nullptr for the all texture
		);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;

	hr = m_device->CreateShaderResourceView(
		newCubeTex->m_texture,
		&srvDesc,
		&newCubeTex->m_shaderResourceView);

	if (FAILED(hr)) 
	{
		newCubeTex->m_texture->Release();
		for (int i = 0; i < 6; i++) 
		{
			tempTextures[i]->Release();
		}
		delete newCubeTex;
		return nullptr;
	}

	for (int i = 0; i < 6; i++) 
	{
		tempTextures[i]->Release();
	}

	m_loadedCubeTextures.push_back(newCubeTex);
	return newCubeTex;
}

void Renderer::BindTextureCube(TextureCube* textureCube)
{
	if (textureCube)
	{
		m_deviceContext->PSSetShaderResources(0, 1, &textureCube->m_shaderResourceView);
	}
	else
	{
		ERROR_AND_DIE("cannot bind texture cube");
		//m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
	}
}

BitmapFont* Renderer::CreateOrGetBitmapFont(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	BitmapFont* existingFont = GetBitmapFontFromFileName(imageFilePath);
	if (existingFont)
	{
		return existingFont;
	}

	// Never seen this texture before!  Let's load it.
	BitmapFont* newFont = CreateBitmapFont(imageFilePath);
	m_loadedFonts.push_back(newFont);
	return newFont;
}

BitmapFont* Renderer::CreateBitmapFont(char const* imageFilePath)
{
	std::string pathWithExtension = imageFilePath;
	pathWithExtension += ".png";
	//Texture* texture=CreateOrGetTextureFromFile(pathWithExtension.c_str());

	//Image img(pathWithExtension.c_str());
	Texture* texture = CreateOrGetTextureFromFile(pathWithExtension.c_str());
	BitmapFont* newFont = new BitmapFont(imageFilePath, *texture, IntVec2(16, 16));
	return newFont;
}

BitmapFont* Renderer::GetBitmapFontFromFileName(char const* imageFilePath)
{
	for (int i = 0; i < static_cast<int>(m_loadedFonts.size()); i++)
	{
		if (m_loadedFonts[i]->m_fontFilePathNameWithNoExtension == imageFilePath)
		{
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}
//-------------------------------------------------------------

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}
void Renderer::SetDepthMode(DepthMode depthMode)
{
	m_desiredDepthMode = depthMode;
}
void Renderer::SetRasterizerMode(RasterizerMode rasterizerMode)
{
	m_desiredRasterizerMode = rasterizerMode;
}
//-------------------------------------------------------------

Shader* Renderer::CreateShaderFromSource(char const* shaderName, char const* shaderSource,VertexType vertexType)
{
	ShaderConfig config =ShaderConfig(shaderName);
	m_currentShader = new Shader(config);
	CompileShaderToByteCode(m_currentShader->m_vertexShaderByteCode,shaderName,shaderSource, "VertexMain", "vs_5_0");
	HRESULT hr;
	hr = m_device->CreateVertexShader(
		m_currentShader->m_vertexShaderByteCode.data(),
		m_currentShader->m_vertexShaderByteCode.size(),
		NULL, &m_currentShader->m_vertexShader
	);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex shader."));
	}

	CompileShaderToByteCode(m_currentShader->m_pixelShaderByteCode, shaderName, shaderSource, "PixelMain", "ps_5_0");

	hr = m_device->CreatePixelShader(
		m_currentShader->m_pixelShaderByteCode.data(),
		m_currentShader->m_pixelShaderByteCode.size(),
		NULL, &m_currentShader->m_pixelShader
	);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create pixel shader."));
	}

	if (vertexType == VertexType::VERTEX_PCU)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,
			0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R8G8B8A8_UNORM,
			0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,
			0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};
		UINT numElements = ARRAYSIZE(inputElementDesc);
		hr = m_device->CreateInputLayout(
			inputElementDesc, numElements,
			m_currentShader->m_vertexShaderByteCode.data(),
			m_currentShader->m_vertexShaderByteCode.size(),
			&m_currentShader->m_inputLayout
		);
	}
	else
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM,
				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,
 				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,
 				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
 			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
 				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE(inputElementDesc);
		hr = m_device->CreateInputLayout(
			inputElementDesc, numElements,
			m_currentShader->m_vertexShaderByteCode.data(),
			m_currentShader->m_vertexShaderByteCode.size(),
			&m_currentShader->m_inputLayout
		);
	}
	
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex layout"));
	}
	m_loadedShaders.push_back(m_currentShader);
	return m_currentShader;
}

Shader* Renderer::CreateShaderFromFile(char const* shaderName,VertexType vertexType)
{
	for (Shader* curShader:m_loadedShaders)
	{
		if (curShader->GetName() == shaderName)
		{
			return curShader;
		}
	}
	std::string shaderPath = std::string(shaderName)+".hlsl";
	std::string outShaderString;
	FileReadToString(outShaderString,shaderPath);
	return CreateShaderFromSource(shaderName, outShaderString.c_str(),vertexType);
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* enetryPoint, char const* target)
{
	bool result=false;

	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;

	HRESULT hr;
	hr = D3DCompile(
		source, strlen(source),
		name, nullptr, nullptr,
		enetryPoint, target, shaderFlags,
		0, &shaderBlob, &errorBlob);
	if (SUCCEEDED(hr))
	{
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(
			outByteCode.data(),
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize()
		);
		result = true;
	}
	else
	{
		if (errorBlob != NULL)
		{
			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
		}
		ERROR_AND_DIE(Stringf("Could not compile vertex shader."));
		//result = false;
	}

	shaderBlob->Release();
	if (errorBlob != NULL)
	{
		errorBlob->Release();
	}

	return result;
}

void Renderer::BindShader(Shader* shader)
{
	if (shader == nullptr)
		shader = m_defaultShader;
	m_deviceContext->IASetInputLayout(shader->m_inputLayout);
	m_deviceContext->VSSetShader(shader->m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader(shader->m_pixelShader, nullptr, 0);
}
//-------------------------------------------------------------

VertexBuffer* Renderer::CreateVertexBuffer(const unsigned int verticeCount, unsigned int stride)
{
	VertexBuffer* curVertexBuffer =new VertexBuffer(m_device, verticeCount, stride);
	return curVertexBuffer;
}

void Renderer::CopyCPUToGPU(const void* data, unsigned int verticeCount, VertexBuffer* vbo)
{
 	//if (vbo->GetVerticeCount() < verticeCount)
 	//{
		if (verticeCount == 0) {
			return;
		}
		vbo->Resize(verticeCount);
 	//}

	//Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, verticeCount * vbo->m_stride);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

void Renderer::BindVertextBuffer(VertexBuffer* vbo)
{
	UINT stride = vbo->GetStride();
	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &startOffset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BindVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo)
{
	UINT stride = vbo->GetStride();
	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &startOffset);
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer,DXGI_FORMAT_R32_UINT,0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, unsigned int vertexCount)
{
	SetStateIfChanged();

	BindVertextBuffer(vbo);
	//Draw
	m_deviceContext->Draw(vertexCount, 0);
}

IndexBuffer* Renderer::CreateIndexBuffer(unsigned int size)
{
	//size=count*stride
	IndexBuffer* curIndexBuffer = new IndexBuffer(m_device, size);
	return curIndexBuffer;
}

void Renderer::InitializeShadowMapping()
{
	D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT isD3D9ShadowSupported;
	ZeroMemory(&isD3D9ShadowSupported, sizeof(isD3D9ShadowSupported));
	m_device->CheckFeatureSupport(
		D3D11_FEATURE_D3D9_SHADOW_SUPPORT,
		&isD3D9ShadowSupported,
		sizeof(D3D11_FEATURE_D3D9_SHADOW_SUPPORT)
	);

	if (isD3D9ShadowSupported.SupportsDepthAsTextureWithLessEqualComparisonFilter)
	{
		//create depth buffer
		D3D11_TEXTURE2D_DESC shadowMapDesc;
		ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		shadowMapDesc.MipLevels = 1;
		shadowMapDesc.ArraySize = 1;
		shadowMapDesc.SampleDesc.Count = 1;
		shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		shadowMapDesc.Height = static_cast<UINT>(m_shadowDimensionX);
		shadowMapDesc.Width = static_cast<UINT>(m_shadowDimensionY);

		HRESULT hr = m_device->CreateTexture2D(
			&shadowMapDesc,
			nullptr,
			&m_shadowMapTexture
		);

		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("Could not createe shadow depth buffer."));
		}

		// create 2 views
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		hr = m_device->CreateDepthStencilView(
			m_shadowMapTexture,
			&depthStencilViewDesc,
			&m_shadowDepthView
		);

		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("Could not createe shadow CreateDepthStencilView."));
		}

		hr = m_device->CreateShaderResourceView(
			m_shadowMapTexture,
			&shaderResourceViewDesc,
			&m_shadowResourceView
		);

		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("Could not createe shadow CreateShaderResourceView."));
		}

		// create compare state
		D3D11_SAMPLER_DESC comparisonSamplerDesc;
		ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
		comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.BorderColor[0] = 1.0f;
		comparisonSamplerDesc.BorderColor[1] = 1.0f;
		comparisonSamplerDesc.BorderColor[2] = 1.0f;
		comparisonSamplerDesc.BorderColor[3] = 1.0f;
		comparisonSamplerDesc.MinLOD = 0.f;
		comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		comparisonSamplerDesc.MipLODBias = 0.f;
		comparisonSamplerDesc.MaxAnisotropy = 0;
		comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

		// Point filtered shadows can be faster, and may be a good choice when
		// rendering on hardware with lower feature levels. This sample has a
		// UI option to enable/disable filtering so you can see the difference
		// in quality and speed.

		hr = m_device->CreateSamplerState(
			&comparisonSamplerDesc,
			&m_comparisonSampler_point
		);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("Could not createe shadow CreateSamplerState."));
		}
	}	

	m_shadowViewport = new D3D11_VIEWPORT();
	ZeroMemory(m_shadowViewport, sizeof(D3D11_VIEWPORT));
	m_shadowViewport->TopLeftX = 0.0f;
	m_shadowViewport->TopLeftY = 0.0f;
	m_shadowViewport->Width = m_shadowDimensionX;  
	m_shadowViewport->Height = m_shadowDimensionY;
	m_shadowViewport->MinDepth = 0.0f;
	m_shadowViewport->MaxDepth = 1.0f;
}

void Renderer::BeginShadowMapRender(Mat44 const& lightViewProjection)
{
	// 首先解绑阴影贴图的着色器资源视图（从槽位1）
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	m_deviceContext->PSSetShaderResources(1, 1, nullSRV);  // 注意是槽位1

	// 暂时解绑所有渲染目标
	ID3D11RenderTargetView* nullRTV[1] = { nullptr };
	ID3D11DepthStencilView* nullDSV = nullptr;
	m_deviceContext->OMSetRenderTargets(1, nullRTV, nullDSV);

	// Set render target
	m_deviceContext->OMSetRenderTargets(0, nullptr, m_shadowDepthView);
	//m_deviceContext->PSSetShaderResources(1, 1, &m_shadowResourceView);
	// clear depth view
	m_deviceContext->ClearDepthStencilView(m_shadowDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set viewport
	m_deviceContext->RSSetViewports(1, m_shadowViewport);

	m_deviceContext->PSSetShader(nullptr, nullptr, 0);

	// bind shadow constants
// 	ShadowConstants shadowConstants;
// 	shadowConstants.LightViewProjection = lightViewProjection;
//  	CopyCPUToGPU(&shadowConstants, sizeof(ShadowConstants), m_shadowCBO);
//  	BindConstantBuffer(k_shadowConstantsSlot, m_shadowCBO);
	SetShadowConstants(lightViewProjection);

	// set raster state
	//m_deviceContext->RSSetState(m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
}

void Renderer::EndShadowMapRender()
{
	ID3D11RenderTargetView* nullRTV[1] = { nullptr };
	ID3D11DepthStencilView* nullDSV = nullptr;
	m_deviceContext->OMSetRenderTargets(1, nullRTV, nullDSV);

	// 只有在完全解绑阴影贴图的深度视图后，才重新设置主渲染目标
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilDSV);
}

Mat44 Renderer::GetDirectLightProjectionMat( Vec3 const& sunDirection,Vec3 const& sceneCenter,float sceneRadius)
{		
	// 1. 标准化光照方向
	Vec3 normalizedSunDir = sunDirection.GetNormalized();

	// 2. 计算光源位置
	Vec3 lightPos = sceneCenter - normalizedSunDir * sceneRadius;

	// 3. 创建视图空间基向量
	// 注意：光源的前方向是光线方向的反向，所以是normalizedSunDir
	Vec3 forward = normalizedSunDir;
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);

	// 处理特殊情况：前向量和上向量近乎平行
	if (abs(DotProduct3D(forward, worldUp)) > 0.99f)
	{
		worldUp = Vec3(0.0f, 1.0f, 0.0f);
	}

	// 计算右向量和上向量
	Vec3 right = CrossProduct3D(worldUp, forward).GetNormalized();
	Vec3 up = CrossProduct3D(forward, right).GetNormalized();

	// 4. 使用列主序或行主序创建模型矩阵（取决于您的引擎）
	// 假设您的引擎使用列主序矩阵
	Mat44 modelMatrix;
	modelMatrix.SetIJK3D(forward, right, up); // 设置旋转部分
	modelMatrix.SetTranslation3D(lightPos);   // 设置平移部分

	// 5. 计算视图矩阵（模型矩阵的逆）
	// 这种方法避免了手动计算正交逆矩阵
	Mat44 viewMatrix = modelMatrix.GetOrthonormalInverse();

	Mat44 renderMat(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 0.f));
	//BMat44 renderMat;
	renderMat.Append(viewMatrix);

	// 6. 创建正交投影矩阵
	float orthoSize = sceneRadius * 1.f; // 调整尺寸
	float nearPlane = 0.1f;
	float farPlane = sceneRadius * 2.5f;

	Mat44 projMatrix = Mat44::MakeOrthoProjection(
		-orthoSize, orthoSize,
		-orthoSize, orthoSize,
		nearPlane, farPlane);

	// 7. 组合视图和投影矩阵
	Mat44 viewProjMatrix = projMatrix;
	viewProjMatrix.Append(renderMat);

	return viewProjMatrix;
}

void Renderer::BindShadowTexture()
{
	if (m_shadowMapTexture)
	{
		m_deviceContext->PSSetShaderResources(1, 1, &m_shadowResourceView);
	}
	else
	{
		ERROR_AND_DIE("Try to bind shadow textuer but the shadow map is nullptr.");
	}
	
// 	else
// 	{
// 		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
// 	}
}

void Renderer::SetShadowSampleState()
{
	m_deviceContext->PSSetSamplers(1, 1, &m_comparisonSampler_point);
}

void Renderer::DrawIndexedVertexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, unsigned int indexCount)
{
	SetStateIfChanged();
	BindVertexAndIndexBuffer(vbo, ibo);
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

void Renderer::CopyCPUToGPU(const void* data, unsigned int count, IndexBuffer* ibo)
{
	if (count == 0) {
		return;
	}

	ibo->Resize(count);

	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

	memcpy(resource.pData, data, count * ibo->GetStride());

	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

ConstantBuffer* Renderer::CreateConstantBuffer(const unsigned int size)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ConstantBuffer* cbo = new ConstantBuffer(size);

	HRESULT hr;
	hr=m_device->CreateBuffer(&bufferDesc,nullptr,&cbo->m_buffer);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create constant vertex buffer");
	}
	return cbo;
}

void Renderer::CopyCPUToGPU(const void* data, unsigned int size, ConstantBuffer* cbo)
{
	//Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}
//-------------------------------------------------------------
void Renderer::SetStateIfChanged()
{
	ID3D11BlendState* desiredState = m_blendStates[(int)m_desiredBlendMode];
	if (desiredState != m_blendState)
	{
		m_blendState = desiredState;
		float blendFactor[4] = { 0.0f,0.0f,0.0f,0.0f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState(m_blendState, blendFactor, sampleMask);
	}

	if (m_samplerStates[(int)m_desiredSamplerMode] != m_samplerState)
	{
		m_samplerState = m_samplerStates[(int)m_desiredSamplerMode];
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	}

	if (m_rasterizerStates[(int)m_desiredRasterizerMode] != m_rasterizerState)
	{
		m_rasterizerState = m_rasterizerStates[(int)m_desiredRasterizerMode];
		m_deviceContext->RSSetState(m_rasterizerState);
	}

	if (m_depthStencilStates[(int)m_desiredDepthMode] != m_depthStencilState)
	{
		m_depthStencilState=m_depthStencilStates[(int)m_desiredDepthMode];
		m_deviceContext->OMSetDepthStencilState(m_depthStencilState,0);
	}
}

void Renderer::SetModelConstants(const Mat44& modelToWorldTransform, const Rgba8& modelColor)
{
	ModelConstants modelConstants;
	modelColor.GetAsFloats(&modelConstants.ModelColor[0]);
	modelConstants.ModelToWorldTransform = modelToWorldTransform;
	CopyCPUToGPU(&modelConstants, sizeof(modelConstants), m_modelCBO);
	BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
}

void Renderer::SetLightConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity)
{
	LightConstants lightConstants;
	lightConstants.SunDirection = sunDirection;
	lightConstants.SunIntensity = sunIntensity;
	lightConstants.AmbientIntensity = ambientIntensity;
	CopyCPUToGPU(&lightConstants, sizeof(lightConstants), m_lightCBO);
	BindConstantBuffer(k_lightConstantsSlot, m_lightCBO);
}

void Renderer::SetPointLightsConstants(const std::vector<PointLight>& lights)
{
	PointLightConstants pointLightConstants;
	for (int i = 0; i < (int)lights.size(); i++)
	{
		pointLightConstants.PointLights[i] = lights[i];
	}	
	pointLightConstants.ActivePointLightCount =(int) lights.size();
	CopyCPUToGPU(&pointLightConstants, sizeof(pointLightConstants), m_pointLightCBO);
	BindConstantBuffer(k_pointLightConstantsSlot, m_pointLightCBO);
}

void Renderer::SetSpotLightsConstants(const std::vector<SpotLight>& lights)
{
	SpotLightConstants spotLightConstants;
	for (int i = 0; i < (int)lights.size(); i++)
	{
		spotLightConstants.SpotLights[i] = lights[i];
	}
	spotLightConstants.ActiveSpotLightCount = (int)lights.size();
	CopyCPUToGPU(&spotLightConstants, sizeof(spotLightConstants), m_spotLightCBO);
	BindConstantBuffer(k_spotLightConstantsSlot, m_spotLightCBO);
}

void Renderer::SetShadowConstants(Mat44 const& lightViewProjectionMat)
{
	ShadowConstants shadowConstants;
	shadowConstants.LightViewProjection = lightViewProjectionMat;
	CopyCPUToGPU(&shadowConstants, sizeof(shadowConstants), m_shadowCBO);
	BindConstantBuffer(k_shadowConstantsSlot, m_shadowCBO);
}












