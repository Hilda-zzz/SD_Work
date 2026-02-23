#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <vector>

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class Texture2DArray
{
	friend class Renderer;

private:
	Texture2DArray() {};
	Texture2DArray(Texture2DArray const& copy) = delete;
	~Texture2DArray();

public:
	IntVec2 GetDimensions() const { return m_dimensions; }
	int GetArraySize() const { return m_arraySize; }
	std::string const& GetFirstPath() const { return m_firstPath; }

protected:
	std::string m_firstPath;  // 第一张纹理路径（用于查找）
	IntVec2 m_dimensions;     // 每张纹理的尺寸
	int m_arraySize = 0;      // 数组大小

	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;
};