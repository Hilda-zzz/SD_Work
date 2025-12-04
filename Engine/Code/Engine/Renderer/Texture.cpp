#include "Engine/Renderer/Texture.hpp"
#include <d3d11.h>

Texture::~Texture()
{
	m_texture->Release();
	m_texture = nullptr;
	m_shaderResourceView->Release();
	m_shaderResourceView = nullptr;

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}
}
