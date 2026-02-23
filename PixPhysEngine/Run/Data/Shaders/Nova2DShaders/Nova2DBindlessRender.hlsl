struct vs_input_t
{
    float3 localPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    uint textureIndex : TEXINDEX;
    float emission : EMISSION;
};

struct v2p_t
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    uint textureIndex : TEXINDEX; 
    float emission : EMISSION; 
};

cbuffer CameraConstants : register(b2)
{
    float4x4 WorldToCameraTransform;
    float4x4 CameraToRenderTransform;
    float4x4 RenderToClipTransform;
    float2 ViewportSize; 
    float2 _Padding;
}

cbuffer ModelConstants : register(b3)
{
    float4x4 ModelToWorldTransform;
    float4 ModelColor;
}

Texture2DArray g_ParticleTextures : register(t10);
SamplerState diffuseSampler : register(s0);

v2p_t VertexMain(vs_input_t input)
{
    float4 localPosition = float4(input.localPosition, 1);
    float4 worldPosition = mul(ModelToWorldTransform, localPosition);
    float4 camPosition = mul(WorldToCameraTransform, worldPosition);
    float4 renderPosition = mul(CameraToRenderTransform, camPosition);
    float4 clipPosition = mul(RenderToClipTransform, renderPosition);
    
    v2p_t v2p;
    v2p.position = clipPosition;
    v2p.color = input.color;
    v2p.uv = input.uv;
    v2p.textureIndex = input.textureIndex;
    v2p.emission = input.emission;
    return v2p;
}

struct PSOutput
{
    float4 color : SV_Target0;   // Main color
    float4 bloom : SV_Target2;   // Bloom
};

PSOutput PixelMain(v2p_t input)
{    
    float4 textureColor =  g_ParticleTextures.Sample(diffuseSampler, float3(input.uv, input.textureIndex));
    float4 vertexColor = input.color;
    float4 finalColor = textureColor * vertexColor * ModelColor;
   
    PSOutput output;
    output.color = finalColor;
    output.bloom = finalColor * input.emission; 
    
    return output;
}