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
	float OrthoMinX;
	float OrthoMinY;
	float OrthoMinZ;
	float OrthoMaxX;
	float OrthoMaxY;
	float OrthoMaxZ;
	float pad0;
	float pad1;
};
static const int k_cameraConstantsSlot = 2;
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
	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));

	//---------------------------------------------------------
	//Set rasterizer state
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

	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create rasterizer state.");
	}

	m_deviceContext->RSSetState(m_rasterizerState);
	//---------------------------------------------------------

	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));

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
	Image defaultImage = Image(IntVec2(2, 2), Rgba8::WHITE);
	m_defaultTexture = CreateTextureFromImage(defaultImage);
	BindTexture(nullptr);
	//---------------------------------------------------------
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

	g_testFont =CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
}

void Renderer::BeginFrame()
{
	//Set render target
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
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
	DX_SAFE_RELEASE(m_rasterizerState);
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

	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	for (int i = 0; i < (int)BlendMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_blendStates[i]);
	}
	//DX_SAFE_RELEASE(m_blendState);

	for (int i = 0; i < (int)SamplerMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}

	for (BitmapFont* bitFont : m_loadedFonts)
	{
		delete bitFont;
		bitFont = nullptr;
	}
	for (Texture* texture : m_loadedTextures)
	{
		delete texture;
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
}

void Renderer::BeginCamera(const Camera& camera)
{
	//Set viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.Width = (float)g_theWindow->GetClientDimensions().x;
	viewport.Height = (float)g_theWindow->GetClientDimensions().y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	m_deviceContext->RSSetViewports(1, &viewport);

	CameraConstants camConstants;
	camConstants.OrthoMinX = camera.GetOrthoBottomLeft().x;
	camConstants.OrthoMinY = camera.GetOrthoBottomLeft().y;
	camConstants.OrthoMinZ = 0.f;
	camConstants.OrthoMaxX = camera.GetOrthoTopRight().x;
	camConstants.OrthoMaxY = camera.GetOrthoTopRight().y;
	camConstants.OrthoMaxZ = 1.f;
	CopyCPUToGPU(&camConstants, sizeof(CameraConstants), m_cameraCBO);
	BindConstantBuffer(k_cameraConstantsSlot, m_cameraCBO);
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
		if (m_loadedFonts[i]->GetTexture().m_name == imageFilePath)
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
//-------------------------------------------------------------

Shader* Renderer::CreateShaderFromSource(char const* shaderName, char const* shaderSource)
{
	ShaderConfig config =ShaderConfig("defaultConfig");
	m_currentShader = new Shader(config);
	CompileShaderToByteCode(m_vertexShaderByteCode,shaderName,shaderSource, "VertexMain", "vs_5_0");
	HRESULT hr;
	hr = m_device->CreateVertexShader(
		m_vertexShaderByteCode.data(),
		m_vertexShaderByteCode.size(),
		NULL, &m_currentShader->m_vertexShader
	);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex shader."));
	}

	CompileShaderToByteCode(m_pixelShaderByteCode, shaderName, shaderSource, "PixelMain", "ps_5_0");

	hr = m_device->CreatePixelShader(
		m_pixelShaderByteCode.data(),
		m_pixelShaderByteCode.size(),
		NULL, &m_currentShader->m_pixelShader
	);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create pixel shader."));
	}

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
		m_vertexShaderByteCode.data(),
		m_vertexShaderByteCode.size(),
		&m_currentShader->m_inputLayout
	);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not reate vertex layout");
	}

	m_loadedShaders.push_back(m_currentShader);
	return m_currentShader;
}

Shader* Renderer::CreateShaderFromFile(char const* shaderName)
{
	std::string shaderPath = "Data/Shaders/"+std::string(shaderName)+".hlsl";
	std::string outShaderString;
	FileReadToString(outShaderString,shaderPath);
	return CreateShaderFromSource(shaderName, outShaderString.c_str());
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

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, unsigned int vertexCount)
{
	SetStateIfChanged();

	BindVertextBuffer(vbo);
	//Draw
	m_deviceContext->Draw(vertexCount, 0);
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
}











