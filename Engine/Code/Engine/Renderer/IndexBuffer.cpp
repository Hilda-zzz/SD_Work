#include "IndexBuffer.hpp"
#include <d3d11.h>
#include <Engine/Core/ErrorWarningAssert.hpp>
#include "Engine/Core/StringUtils.hpp"

IndexBuffer::IndexBuffer(ID3D11Device* device, unsigned int size):m_device(device),m_size(size)
{
	Create();
}

IndexBuffer::~IndexBuffer()
{
	m_buffer->Release();
	m_buffer = nullptr;
}

void IndexBuffer::Create()
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; 
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create index buffer");
	}
}

unsigned int IndexBuffer::GetSize()
{
	return m_size;
}

unsigned int IndexBuffer::GetStride()
{
	return sizeof(unsigned int);
}

unsigned int IndexBuffer::GetCount()
{
	return m_size / GetStride();
}

void IndexBuffer::Resize(unsigned int indexCount)
{
	int stride = GetStride();
	m_size = indexCount* stride;
	m_buffer->Release();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex buffer HResult: %d", hr));
	}
}
