struct VertexInput
{
    float3 a_modelPosition : POSITION;
    float4 a_color : COLOR;
    float2 a_uv : TEXCOORD;
};

struct VertexOutPixelIn
{
    float4 v_clipPosition : SV_Position;
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
SamplerState s_samplerState : register(s0);

VertexOutPixelIn VertexMain(VertexInput input)
{
    VertexOutPixelIn output;
    
    float4 modelPos = float4(input.a_modelPosition, 1);
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
    
    output.v_clipPosition = clipPos;
    output.texcoord = float3(-input.a_modelPosition.y,-input.a_modelPosition.z,input.a_modelPosition.x);
    //output.texcoord = float3(input.modelPosition.x,-input.modelPosition.z,-input.modelPosition.y);
    
    return output;
}

float4 PixelMain(VertexOutPixelIn input) : SV_Target0
{
    return skyboxTexture.Sample(s_samplerState, normalize(input.texcoord));
}
