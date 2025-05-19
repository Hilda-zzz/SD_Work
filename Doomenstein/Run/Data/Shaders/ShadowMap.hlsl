struct vs_input_t
{
	float3 modelPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
 	float3 modelTangent : TANGENT;
 	float3 modelBitangent : BITANGENT;
 	float3 modelNormal : NORMAL;
};

struct v2p_t
{
    float4 clipPosition : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer ShadowConstants : register(b6)
{
    float4x4 LightViewProjection;     
};

cbuffer ModelConstants : register(b3)
{
    float4x4 ModelToWorldTransform;    
    float4 ModelColor;
};

 Texture2D diffuseTexture : register(t0);
 SamplerState samplerState : register(s0); 

v2p_t VertexMain(vs_input_t input)
{
    v2p_t output;
    
    float4 modelPosition = float4(input.modelPosition, 1.0f);
    float4 worldPosition = mul(ModelToWorldTransform, modelPosition);
    output.clipPosition = mul(LightViewProjection, worldPosition);
    output.uv = input.uv;
    return output;
}

void PixelMain(v2p_t input)
{
    float4 textureColor = diffuseTexture.Sample(samplerState, input.uv);
    if (textureColor.a < 0.01f)
    {
        discard;  
    }
}