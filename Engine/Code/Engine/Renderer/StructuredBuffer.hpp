// StructuredBuffer.hpp
// GPU结构化缓冲区 - 用于存储大量结构化数据（如粒子数组）
#pragma once

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

//==========================================================================
// StructuredBuffer
// 用途：在GPU上存储大量结构化数据（数组）
// 
// 与ConstantBuffer的区别：
//   - ConstantBuffer: 小数据，频繁更新（如相机矩阵）
//   - StructuredBuffer: 大数据，较少更新（如粒子数组）
// 
// 使用场景：
//   - 粒子系统：存储50万个粒子
//   - 物理系统：存储物理对象数组
//   - 渲染系统：存储实例化数据
//==========================================================================
class StructuredBuffer
{
	friend class Renderer;

public:
	//----------------------------------------------------------------------
	// 构造函数
	// 参数：
	//   elementCount - 元素数量（例如：500,000个粒子）
	//   elementSize  - 每个元素的大小（例如：48 bytes/粒子）
	//   isReadWrite  - 是否支持Shader写入（UAV模式）
	//----------------------------------------------------------------------
	StructuredBuffer(unsigned int elementCount, unsigned int elementSize, bool isReadWrite);
	StructuredBuffer(const StructuredBuffer& copy) = delete;
	virtual ~StructuredBuffer();

	//----------------------------------------------------------------------
	// 查询
	//----------------------------------------------------------------------
	unsigned int GetElementCount() const { return m_elementCount; }
	unsigned int GetElementSize() const { return m_elementSize; }
	size_t GetTotalSize() const { return m_elementCount * m_elementSize; }
	bool IsReadWrite() const { return m_isReadWrite; }

private:
	//----------------------------------------------------------------------
	// D3D11资源
	//----------------------------------------------------------------------
	ID3D11Buffer* m_buffer = nullptr;                           // GPU Buffer
	ID3D11ShaderResourceView* m_srv = nullptr;                  // 只读视图（t寄存器）
	ID3D11UnorderedAccessView* m_uav = nullptr;                 // 读写视图（u寄存器，仅isReadWrite=true时创建）

	//----------------------------------------------------------------------
	// 元数据
	//----------------------------------------------------------------------
	unsigned int m_elementCount = 0;                            // 元素数量
	unsigned int m_elementSize = 0;                             // 每个元素大小（bytes）
	bool m_isReadWrite = false;                                 // 是否支持写入
};