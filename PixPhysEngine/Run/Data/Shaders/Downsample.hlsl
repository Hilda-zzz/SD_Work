Texture2D sourceTexture : register(t0);
SamplerState diffuseSampler : register(s0);

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

// Box filter downsample (2x2)
float4 PixelMain(v2p_t input) : SV_Target0
{
    // Simple bilinear sample (hardware does the filtering)
    return sourceTexture.Sample(diffuseSampler, input.uv);
}