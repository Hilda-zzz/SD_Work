#include "CellGPUStructs.hlsli"

// === Input Buffers ===
StructuredBuffer<GPUCellData> CellDataBuffer : register(t0);

// === Output Buffers ===
struct Vertex_GPU {
    float3 position;          // 12 bytes
    uint colorPacked;         // 4 bytes
    float2 uv;                 // 8
    uint physicsType;         // 4 bytes
    float emission;           // 4 bytes
};  

RWByteAddressBuffer VertexBuffer : register(u0); 
RWByteAddressBuffer IndirectArgs : register(u1);        

// === Constants ===
cbuffer PopulateConstants : register(b0)
{
    uint g_totalCells;
    float3 g_padding;
};

// === Helper: Pack Color ===
uint PackColor(float4 color)
{
    return (uint(color.r * 255.0) << 0) |
           (uint(color.g * 255.0) << 8) |
           (uint(color.b * 255.0) << 16) |
           (uint(color.a * 255.0) << 24);
}

// === Helper: Write Vertex to ByteAddressBuffer ===
void WriteVertex(uint vertexIndex, float2 worldPos, float4 color, float2 uv, uint physicsType, float emission)
{
    uint byteOffset = vertexIndex * 32;  // ????32??
    
    // Write position (3 floats = 12 bytes)
    VertexBuffer.Store3(byteOffset, asuint(float3(worldPos, 0.0)));
    
    // Write colorPacked (1 uint = 4 bytes)
    VertexBuffer.Store(byteOffset + 12, PackColor(color));
    
    // Write uv (2 floats = 8 bytes)
    VertexBuffer.Store2(byteOffset + 16, asuint(uv));
    
    // Write physicsType (1 uint = 4 bytes)
    VertexBuffer.Store(byteOffset + 24, physicsType);
    
    // Write emission (1 float = 4 bytes)
    VertexBuffer.Store(byteOffset + 28, asuint(emission));
}

// === Helper: Generate Quad (6 vertices) ===
void GenerateQuad(uint baseVertexIndex, float2 worldPos, float4 color, uint physicsType, float emission)
{
    // Quad corners (1×1 pixel)
    float2 mins = worldPos;
    float2 maxs = worldPos + float2(1.0, 1.0);
    
    WriteVertex(baseVertexIndex + 0, float2(mins.x, mins.y), color, float2(0, 0), physicsType, emission); 
    WriteVertex(baseVertexIndex + 1, float2(maxs.x, mins.y), color, float2(1, 0), physicsType, emission); 
    WriteVertex(baseVertexIndex + 2, float2(maxs.x, maxs.y), color, float2(1, 1), physicsType, emission); 
    WriteVertex(baseVertexIndex + 3, float2(mins.x, maxs.y), color, float2(0, 1), physicsType, emission); 
}

// === Main Compute Shader ===
[numthreads(256, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint cellIndex = dispatchThreadID.x;
    
    if (cellIndex >= g_totalCells) return;
    
    GPUCellData cellData = CellDataBuffer[cellIndex];
    
    float2 worldPos;
    float4 color;
    uint physicsType;
    float emission;
    UnpackCellData(cellData, worldPos, color, physicsType, emission);
    
    if (physicsType == 0) return;
    
    uint baseVertexIndex = cellIndex * 4;
    //IndirectArgs.InterlockedAdd(0, 4, baseVertexIndex);
    
    GenerateQuad(baseVertexIndex, worldPos, color, physicsType, emission);

    uint dummy;
    IndirectArgs.InterlockedAdd(0, 6, dummy);
}