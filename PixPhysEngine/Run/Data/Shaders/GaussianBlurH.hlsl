Texture2D sourceTexture : register(t0);
SamplerState diffuseSampler : register(s0);

cbuffer BlurParams : register(b0)
{
    float2 texelSize;
    float2 padding;
};

struct vs_input_t
{
    float3 localPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct v2p_t
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

v2p_t VertexMain(vs_input_t input)
{
    v2p_t v2p;
    v2p.position = float4(input.localPosition, 1.0);
    v2p.uv = input.uv;
    return v2p;
}

// 5-tap Gaussian (horizontal)
float4 PixelMain(v2p_t input) : SV_Target0
{
    float weights[5] = { 0.06, 0.24, 0.40, 0.24, 0.06 };
    int offsets[5] = { -2, -1, 0, 1, 2 };
    
    float4 result = float4(0, 0, 0, 0);
    
    for (int i = 0; i < 5; i++)
    {
        float2 uv = input.uv + float2(offsets[i] * texelSize.x, 0);
        result += sourceTexture.Sample(diffuseSampler, uv) * weights[i];
    }
    
    return result;
}