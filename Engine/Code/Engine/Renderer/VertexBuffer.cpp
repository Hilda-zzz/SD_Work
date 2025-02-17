#include "VertexBuffer.hpp"
#include <Engine/Core/Vertex_PCU.hpp>
#include "Engine/Core/StringUtils.hpp"
#include <d3d11.h>
#include <Engine/Core/ErrorWarningAssert.hpp>

VertexBuffer::VertexBuffer(ID3D11Device* device, unsigned int verticeCount, unsigned int stride):m_device(device),m_verticeCount(verticeCount),m_stride(stride)
{
	Create();
}

VertexBuffer::~VertexBuffer()
{
	m_buffer->Release();
	m_buffer = nullptr;
}

void VertexBuffer::Create()
{
	//---------------------------------------------------------
	//CREATE VERTEX BUFFER
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_stride*m_verticeCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer");
	}
}

void VertexBuffer::Resize(unsigned int verticeCount)
{
	m_verticeCount = verticeCount;
	m_buffer->Release();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_stride * verticeCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex buffer HResult: %d", hr));
	}

}

unsigned int VertexBuffer::GetVerticeCount()
{
	return m_verticeCount;
}

unsigned int VertexBuffer::GetStride()
{
	return m_stride;
}
