#pragma once
#include <string>
#include <vector>

struct ID3D11ComputeShader;

struct ComputeShaderConfig
{
	ComputeShaderConfig(std::string name);
	std::string m_name;
	std::string m_entryPoint = "CSMain";
};

class ComputeShader
{
	friend class Renderer;
public:
	ComputeShader(const ComputeShaderConfig& config);
	ComputeShader(const ComputeShader& copy) = delete;
	~ComputeShader();

	const std::string& GetName() const;

	std::vector<uint8_t> m_shaderByteCode;
private:
	ComputeShaderConfig m_config;
	ID3D11ComputeShader* m_computeShader = nullptr;
};