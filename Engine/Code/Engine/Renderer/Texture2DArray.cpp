#include "Texture2DArray.hpp"
#include <d3d11.h>

Texture2DArray::~Texture2DArray()
{
	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	if (m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = nullptr;
	}
}