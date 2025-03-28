#pragma once
struct ID3D11Buffer;
struct ID3D11Device;

class IndexBuffer
{
	friend class Renderer;

public: 
	IndexBuffer(ID3D11Device* device, unsigned int size);
	IndexBuffer(const IndexBuffer& copy) = delete;
	~IndexBuffer();

	void Create();
	unsigned int GetSize();
	unsigned int GetStride();
	unsigned int GetCount();
	void Resize(unsigned int indexCount);

private:
	ID3D11Device* m_device = nullptr;
	ID3D11Buffer* m_buffer = nullptr;
	unsigned int m_size = 0;
};