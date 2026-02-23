#include "ComputeShader.hpp"
#include <d3d11.h>

ComputeShaderConfig::ComputeShaderConfig(std::string name)
	: m_name(name)
{
}

ComputeShader::ComputeShader(const ComputeShaderConfig& config)
	: m_config(config)
{
}

ComputeShader::~ComputeShader()
{
	if (m_computeShader)
	{
		m_computeShader->Release();
		m_computeShader = nullptr;
	}
}

const std::string& ComputeShader::GetName() const
{
	return m_config.m_name;
}

