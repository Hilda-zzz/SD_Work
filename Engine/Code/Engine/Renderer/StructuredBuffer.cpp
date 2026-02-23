// StructuredBuffer.cpp
#include "StructuredBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <d3d11.h>

#define DX_SAFE_RELEASE(dxObject)			\
{											\
	if ((dxObject) != nullptr)				\
	{										\
		(dxObject)->Release();				\
		(dxObject) = nullptr;				\
	}										\
}

//==========================================================================
// 构造函数
//==========================================================================
StructuredBuffer::StructuredBuffer(unsigned int elementCount, unsigned int elementSize, bool isReadWrite)
	: m_elementCount(elementCount)
	, m_elementSize(elementSize)
	, m_isReadWrite(isReadWrite)
{
	// 验证参数
	GUARANTEE_OR_DIE(elementCount > 0, "StructuredBuffer: elementCount must be > 0");
	GUARANTEE_OR_DIE(elementSize > 0, "StructuredBuffer: elementSize must be > 0");

	// 元素大小必须是4字节的倍数（GPU对齐要求）
	GUARANTEE_OR_DIE(elementSize % 4 == 0, "StructuredBuffer: elementSize must be multiple of 4 bytes");
}

//==========================================================================
// 析构函数
//==========================================================================
StructuredBuffer::~StructuredBuffer()
{
	DX_SAFE_RELEASE(m_uav);
	DX_SAFE_RELEASE(m_srv);
	DX_SAFE_RELEASE(m_buffer);
}