struct vs_input_t
{
    float3 modelPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct v2p_t
{
    float4 clipPosition : SV_Position;
    float3 texcoord : TEXCOORD;
};

cbuffer CameraConstants : register(b2)
{
    float4x4 WorldToCameraTransform;
    float4x4 CameraToRenderTransform;
    float4x4 RenderToClipTransform;
};

cbuffer ModelConstants : register(b3)
{
    float4x4 ModelToWorldTransform;
    float4 ModelColor;
};

TextureCube skyboxTexture : register(t0);
SamplerState samplerState : register(s0);

v2p_t VertexMain(vs_input_t input)
{
    v2p_t output;
    
    float4 modelPos = float4(input.modelPosition, 1);
    float4 worldPos = mul(ModelToWorldTransform, modelPos);
    
    float4x4 viewNoTranslation = WorldToCameraTransform;
    //viewNoTranslation[3] = float4(0, 0, 0, 1);
    viewNoTranslation._14 = 0; 
    viewNoTranslation._24 = 0; 
    viewNoTranslation._34 = 0; 
    viewNoTranslation._44 = 1; 
    
    float4 camPos = mul(viewNoTranslation, worldPos);
    float4 renderPos = mul(CameraToRenderTransform, camPos);
    float4 clipPos = mul(RenderToClipTransform, renderPos);
    
    clipPos.z = clipPos.w;
    
    output.clipPosition = clipPos;
    output.texcoord = float3(-input.modelPosition.y,-input.modelPosition.z,input.modelPosition.x);
    //output.texcoord = float3(input.modelPosition.x,-input.modelPosition.z,-input.modelPosition.y);
    
    return output;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
    return skyboxTexture.Sample(samplerState, normalize(input.texcoord));
}
