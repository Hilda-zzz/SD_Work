// BlitWithBloom Shader - Composite main color + bloom
Texture2D mainColorTexture : register(t0);
Texture2D bloomTexture : register(t1);
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

// Vertex Shader - Direct pass-through (no camera transform)
v2p_t VertexMain(vs_input_t input)
{
    v2p_t v2p;
    v2p.position = float4(input.localPosition, 1.0);
    v2p.uv = input.uv;
    return v2p;
}

// Pixel Shader - Composite main color + bloom
float4 PixelMain(v2p_t input) : SV_Target0
{
    float4 mainColor = mainColorTexture.Sample(diffuseSampler, input.uv);
    float4 bloom = bloomTexture.Sample(diffuseSampler, input.uv);
    
    // Additive blend with adjustable intensity
    // You can tweak the 0.8 multiplier to control bloom strength
    return mainColor + bloom * 0.8;
}