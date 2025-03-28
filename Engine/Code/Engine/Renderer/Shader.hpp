#pragma once
#include <string>
#include <vector>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct ShaderConfig
{
	ShaderConfig(std::string name);
	std::string m_name;
	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";
};

class Shader
{
	friend class Renderer;
public:
	Shader(const ShaderConfig& config);
	Shader(const Shader& copy) = delete;
	~Shader();

	const std::string& GetName() const;

	std::vector<uint8_t>	m_vertexShaderByteCode;
	std::vector<uint8_t>	m_pixelShaderByteCode;
private:
	ShaderConfig m_config;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
};