#pragma once
const char* g_shaderSource = R"(
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

cbuffer CameraConstants : register(b2)
{
	float4x4 WorldToCameraTransform;
	float4x4 CameraToRenderTransform;
	float4x4 RenderToClipTransform;
}

cbuffer ModelConstants : register(b3)
{
	float4x4 ModelToWorldTransform;
	float4 ModelColor;
}

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition=float4(input.localPosition,1);
	float4 worldPosition=mul(ModelToWorldTransform,localPosition);
	float4 camPosition=mul(WorldToCameraTransform,worldPosition);
	float4 renderPosition=mul(CameraToRenderTransform,camPosition);
	float4 clipPosition=mul(RenderToClipTransform,renderPosition);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor=input.color;
	float4 color = textureColor*vertexColor*ModelColor;
	clip(color.a-0.01f);
	return float4(color);
}

)";