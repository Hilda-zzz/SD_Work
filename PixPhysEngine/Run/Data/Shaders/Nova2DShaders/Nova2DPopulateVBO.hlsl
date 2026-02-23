#include "Nova2DShaders/Nova2DParticleStruct.hlsli"
#include "Nova2DShaders/Nova2DCommon.hlsli"

StructuredBuffer<uint> AliveList : register(t0);
StructuredBuffer<GPUParticle2D> Particles : register(t1);
StructuredBuffer<uint> Counters : register(t2); 

StructuredBuffer<Nova2DEmitterDefinitionGPU> g_Definitions : register(t5);
StructuredBuffer<Nova2DEmitterInstanceGPU> g_Instances : register(t6);

RWByteAddressBuffer VertexBuffer : register(u0); 


void WriteVertex(uint vertexIndex, float2 pos, float4 color, float2 uv, uint textureIndex, float depth, float emission)
{
    uint byteOffset = vertexIndex * 32;  // sizeof(Vertex_PCU) = 24
    
    // Position (12 bytes) - float3
    float3 pos3D = float3(pos, depth);
    VertexBuffer.Store3(byteOffset + 0, asuint(pos3D));
    
    // Color (4 bytes) - uint (Rgba8 packed)
    uint packedColor = PackColor(color); 
    VertexBuffer.Store(byteOffset + 12, packedColor);
    
    // UV (8 bytes) - float2
    VertexBuffer.Store2(byteOffset + 16, asuint(uv));

    VertexBuffer.Store(byteOffset + 24, textureIndex);

    VertexBuffer.Store(byteOffset + 28, asuint(emission));
}

float4 CalculateSpriteUV(uint spriteIndex, uint animFrame, uint sheetDimX, uint sheetDimY)
{
    uint currentFrame = spriteIndex + animFrame;
    
    uint col = currentFrame % sheetDimX;
    uint row = currentFrame / sheetDimX;
    
    float uvWidth = 1.0f / float(sheetDimX);
    float uvHeight = 1.0f / float(sheetDimY);
    
    float uvMinX = float(col) * uvWidth;
    float uvMinY = float(row) * uvHeight;
    float uvMaxX = uvMinX + uvWidth;
    float uvMaxY = uvMinY + uvHeight;
    
    return float4(uvMinX, uvMinY, uvMaxX, uvMaxY);
}

[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    
    uint aliveCount = Counters[3];
    
    if (particleIndex >= aliveCount)
        return;
    
    uint pIdx = AliveList[particleIndex];
    GPUParticle2D p = Particles[pIdx];
    
    float cosR = cos(p.rotation);
    float sinR = sin(p.rotation);
    float halfSize = p.size * 0.5;
    
    float2 right = float2(cosR, sinR) * halfSize;
    float2 up = float2(-sinR, cosR) * halfSize;
    
    float2 pos0 = p.position - right - up;  
    float2 pos1 = p.position + right - up;  
    float2 pos2 = p.position + right + up; 
    float2 pos3 = p.position - right + up;  

    uint baseVertexIndex = particleIndex * 4;

    // Calculate UV from def
    Nova2DEmitterInstanceGPU inst = g_Instances[p.instanceID];
    Nova2DEmitterDefinitionGPU def = g_Definitions[inst.m_definitionIndex];
    uint textureIndex = def.m_appearance.m_textureIndex;
    uint sheetDimX = def.m_appearance.m_spriteSheetDimensionsX;
    uint sheetDimY = def.m_appearance.m_spriteSheetDimensionsY;
    
    float4 uvs = CalculateSpriteUV(p.spriteIndex, p.animFrame, sheetDimX, sheetDimY);
    float uvMinX = uvs.x;
    float uvMinY = uvs.y;
    float uvMaxX = uvs.z;
    float uvMaxY = uvs.w;
    
    float textureUVScaleX = def.m_appearance.m_textureUVScaleX;
    float textureUVScaleY = def.m_appearance.m_textureUVScaleY;

    uvMinX *= textureUVScaleX;
    uvMaxX *= textureUVScaleX;
    uvMinY *= textureUVScaleY;
    uvMaxY *= textureUVScaleY;

    WriteVertex(baseVertexIndex + 0, pos0, p.color, float2(uvMinX, uvMinY), textureIndex, p.depth, p.emission);
    WriteVertex(baseVertexIndex + 1, pos1, p.color, float2(uvMaxX, uvMinY), textureIndex, p.depth, p.emission);
    WriteVertex(baseVertexIndex + 2, pos2, p.color, float2(uvMaxX, uvMaxY), textureIndex, p.depth, p.emission);
    WriteVertex(baseVertexIndex + 3, pos3, p.color, float2(uvMinX, uvMaxY), textureIndex, p.depth, p.emission);

}