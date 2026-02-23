#pragma once

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11UnorderedAccessView;

class VertexBuffer
{
	friend class Renderer;

public:
	VertexBuffer(ID3D11Device* device, unsigned int verticeCount, unsigned int stride, 
		bool isPerInstance = false, bool enableUAV=false);
	VertexBuffer(const VertexBuffer& copy) = delete;
	~VertexBuffer();

	void Create();
	void Resize(unsigned int verticeCount);

	unsigned int GetVerticeCount();
	unsigned int GetStride();

	// ===== UAV Support =====
	ID3D11UnorderedAccessView* GetUAV() const { return m_uav; }
	bool IsUAVEnabled() const { return m_enableUAV; }

private:
	ID3D11Device* m_device = nullptr;
	ID3D11Buffer* m_buffer = nullptr;

	ID3D11UnorderedAccessView* m_uav = nullptr;
	bool m_enableUAV = false;

	unsigned int m_verticeCount = 0;
	unsigned int m_stride = 0;

	// -------------------------------
	bool m_isPerInstance = false; 
};