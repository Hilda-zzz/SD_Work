// CellGPUStructs.hlsli
#ifndef CELL_GPU_STRUCTS_HLSLI
#define CELL_GPU_STRUCTS_HLSLI

struct GPUCellData
{
    uint worldX; // 4 bytes
    uint worldY; // 4 bytes
    uint colorPacked; // 4 bytes: RGBA8888
    uint attributes; // 4 bytes: PhysicsType + Emission
};

float4 UnpackColor(uint packed)
{
    return float4(
        (packed & 0xFF) / 255.0, // R
        ((packed >> 8) & 0xFF) / 255.0, // G
        ((packed >> 16) & 0xFF) / 255.0, // B
        ((packed >> 24) & 0xFF) / 255.0 // A
    );
}

void UnpackAttributes(uint attributes, out uint physicsType, out float emission)
{
    physicsType = attributes & 0x7; // 3 bits
    emission = ((attributes >> 3) & 0x1F) / 31.0; // 5 bits ? [0,31]
}

void UnpackCellData(GPUCellData cellData, out float2 worldPos, out float4 color, out uint physicsType, out float emission)
{
    worldPos = float2((float) cellData.worldX, (float) cellData.worldY);
    color = UnpackColor(cellData.colorPacked);
    UnpackAttributes(cellData.attributes, physicsType, emission);
}

#endif