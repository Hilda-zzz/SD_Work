#include "VertexBuffer.hpp"
#include <Engine/Core/Vertex_PCU.hpp>
#include "Engine/Core/StringUtils.hpp"
#include <d3d11.h>
#include <Engine/Core/ErrorWarningAssert.hpp>

VertexBuffer::VertexBuffer(ID3D11Device* device, unsigned int verticeCount, unsigned int stride, 
	bool isPerInstance, bool enableUAV)
	:m_device(device),
	m_verticeCount(verticeCount),
	m_stride(stride),
	m_isPerInstance(isPerInstance),
	m_enableUAV(enableUAV)
{
	Create();
}

VertexBuffer::~VertexBuffer()
{
	if (m_uav)
	{
		m_uav->Release();
		m_uav = nullptr;
	}

	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}
}

void VertexBuffer::Create()
{
	//---------------------------------------------------------
	//CREATE VERTEX BUFFER
	D3D11_BUFFER_DESC bufferDesc = {};

	if (m_enableUAV)
	{
		// ===== UAV-enabled VBO（用于 PopulateVBO）=====
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;  // ✅ 必须是 DEFAULT，不是 DYNAMIC
		bufferDesc.ByteWidth = m_stride * m_verticeCount;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;  // ✅ 关键
		bufferDesc.CPUAccessFlags = 0;  // ✅ UAV 不能有 CPU access
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;  // ✅ 支持 ByteAddressBuffer
	}
	else
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = m_stride * m_verticeCount;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer");
	}

	if (m_enableUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;  // ✅ ByteAddressBuffer 用 R32_TYPELESS
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = (m_stride * m_verticeCount) / 4;  // ✅ 以 4 字节为单位
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;  // ✅ RAW 模式

		hr = m_device->CreateUnorderedAccessView(m_buffer, &uavDesc, &m_uav);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create UAV for vertex buffer");
		}
	}
}

void VertexBuffer::Resize(unsigned int verticeCount)
{
	m_verticeCount = verticeCount;

	if (m_uav)
	{
		m_uav->Release();
		m_uav = nullptr;
	}
	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}

	D3D11_BUFFER_DESC bufferDesc = {};

	if (m_enableUAV)
	{
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = m_stride * verticeCount;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	}
	else
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = m_stride * verticeCount;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex buffer HResult: %d", hr));
	}

	if (m_enableUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = (m_stride * verticeCount) / 4;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

		hr = m_device->CreateUnorderedAccessView(m_buffer, &uavDesc, &m_uav);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not recreate UAV for vertex buffer");
		}
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
