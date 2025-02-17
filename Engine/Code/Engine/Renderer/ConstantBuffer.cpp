#include "ConstantBuffer.hpp"

#include <d3d11.h>

#define DX_SAFE_RELEASE(dxObject)			\
{											\
	if ((dxObject) != nullptr)				\
	{										\
		(dxObject)->Release();				\
		(dxObject)=nullptr;					\
	}										\
}

ConstantBuffer::ConstantBuffer(size_t size)
{
	m_size = size;
}

ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
