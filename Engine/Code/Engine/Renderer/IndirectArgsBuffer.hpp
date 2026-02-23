#pragma once
#include <cstdint>

struct ID3D11Buffer;
struct ID3D11UnorderedAccessView;
struct ID3D11Device;

class IndirectArgsBuffer
{
public:
	IndirectArgsBuffer(ID3D11Device* device, unsigned int sizeInBytes);
	~IndirectArgsBuffer();

	IndirectArgsBuffer(const IndirectArgsBuffer&) = delete;
	IndirectArgsBuffer& operator=(const IndirectArgsBuffer&) = delete;

	ID3D11Buffer* GetBuffer() const { return m_buffer; }
	ID3D11UnorderedAccessView* GetUAV() const { return m_uav; }

private:
	ID3D11Buffer* m_buffer = nullptr;
	ID3D11UnorderedAccessView* m_uav = nullptr;
};