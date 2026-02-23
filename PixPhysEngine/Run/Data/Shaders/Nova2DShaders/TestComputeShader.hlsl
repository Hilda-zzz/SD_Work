// read only
StructuredBuffer<float> InputBuffer : register(t0);

// rw
RWStructuredBuffer<float> OutputBuffer : register(u0);

cbuffer TestParams : register(b0)
{
    float multiplier;
    float addValue;
    uint elementCount;
    uint padding;
};

[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint index = DTid.x;
    
    if (index >= elementCount)
        return;
    
    float inputValue = InputBuffer[index];
    OutputBuffer[index] = inputValue * multiplier + addValue;
}