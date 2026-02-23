#include "IndirectArgsBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <d3d11.h>

IndirectArgsBuffer::IndirectArgsBuffer(ID3D11Device* device, unsigned int sizeInBytes)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeInBytes;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS |
		D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	bufferDesc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (FAILED(hr)) {
		ERROR_AND_DIE("Failed to create IndirectArgs buffer");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = sizeInBytes / 4;  // uint32??
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

	hr = device->CreateUnorderedAccessView(m_buffer, &uavDesc, &m_uav);
	if (FAILED(hr)) {
		ERROR_AND_DIE("Failed to create IndirectArgs UAV");
	}
}

IndirectArgsBuffer::~IndirectArgsBuffer()
{
	if (m_uav) {
		m_uav->Release();
		m_uav = nullptr;
	}
	if (m_buffer) {
		m_buffer->Release();
		m_buffer = nullptr;
	}
}