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
	float3 worldPosition : WORLD_POS;
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

cbuffer WorldConstants : register(b9)
{
    float4 CameraPosition;
    float4 IndoorLightColor;
    float4 OutdoorLightColor;
    float4 SkyColor;
    float FogNearDistance;
    float FogFarDistance;
    float2 Padding;
};

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
	v2p.worldPosition = worldPosition.xyz;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor=input.color;

	float outdoorInfluence = input.color.r;
	float indoorInfluence = input.color.g;
	float directionalShade = input.color.b;

	float3 outerLight = outdoorInfluence * OutdoorLightColor.rgb;

    float3 innerLight = indoorInfluence * IndoorLightColor.rgb;
	float3 diffuseLight = 1.0f - (1.0f - outerLight) * (1.0f - innerLight);

	float3 litColor = textureColor.rgb * diffuseLight * directionalShade * ModelColor.rgb;

	// fog
	float distanceToCamera = length(input.worldPosition - CameraPosition.xyz);
    float fogFraction = saturate((distanceToCamera - FogNearDistance) / (FogFarDistance - FogNearDistance));

	float3 finalColor = lerp(litColor, SkyColor.rgb, fogFraction);
	//float3 finalColor =litColor;

	clip(textureColor.a-0.01f);
	return float4(finalColor,textureColor.a);
}