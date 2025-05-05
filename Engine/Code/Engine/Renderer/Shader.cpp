#include "Shader.hpp"
#include <d3d11.h>
#include <d3dcompiler.h>




Shader::Shader(const ShaderConfig& config):m_config(config)
{
//	//Compile vertex shader
//	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
//#if defined(ENGINE_DEBUG_RENDER)
//	shaderFlags = D3DCOMPILE_DEBUG;
//	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
//	shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
//#endif
//
//	ID3DBlob* shaderBlob = NULL;
//	ID3DBlob* errorBlob = NULL;
//
//	HRESULT hr;
//	hr = D3DCompile(
//		shaderSource, strlen(shaderSource),
//		"VertexShader", nullptr, nullptr,
//		"VertexMain", "vs_5_0", shaderFlags,
//		0, &shaderBlob, &errorBlob);
//	if (SUCCEEDED(hr))
//	{
//		m_vertexShaderByteCode.resize(shaderBlob->GetBufferSize());
//		memcpy(
//			m_vertexShaderByteCode.data(),
//			shaderBlob->GetBufferPointer(),
//			shaderBlob->GetBufferSize()
//		);
//	}
//	else
//	{
//		if (errorBlob != NULL)
//		{
//			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
//		}
//		ERROR_AND_DIE(Stringf("Could not compile vertex shader."));
//	}
//
//	shaderBlob->Release();
//	if (errorBlob != NULL)
//	{
//		errorBlob->Release();
//	}
}

Shader::~Shader()
{
	m_vertexShader->Release();
	m_pixelShader->Release();
	m_inputLayout->Release();
}

const std::string& Shader::GetName() const
{
	return m_config.m_name;
}

ShaderConfig::ShaderConfig(std::string name):m_name(name)
{
}
