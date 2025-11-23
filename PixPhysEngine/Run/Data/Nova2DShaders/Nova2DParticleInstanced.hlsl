// ========== Vertex Shader ==========
struct VS_INPUT 
{
    // Per-Vertex
    float3 localPosition : POSITION;   // (0,0), (1,0), (0,1), (1,1)
    float2 uv : TEXCOORD;
    
    // Per-Instance
    float2 worldPosition : INSTANCE_POS;   
    float2 size : INSTANCE_SIZE;           
    float4 color : INSTANCE_COLOR;         
    float rotation : INSTANCE_ROTATION;   
    
    uint instanceID : SV_InstanceID;       // auto supply
};

struct VS_OUTPUT 
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

VS_OUTPUT VertexMain(VS_INPUT input) 
{
    VS_OUTPUT output;
    
    // 1. Scale and Center
	float4 localPosition=float4(input.localPosition,1);
    localPosition.xy=(localPosition.xy-0.5f)* input.size;  

    // 2. #TODO: Rotation
    // float2 rotated;
    // rotated.x = localPos.x * cos(input.rotation) - localPos.y * sin(input.rotation);
    // rotated.y = localPos.x * sin(input.rotation) + localPos.y * cos(input.rotation);
    // localPos = rotated;
    
    // 3. Translation
    float2 worldPos = input.worldPosition.xy + localPosition.xy;
    
    // 4. Cam
	float4 worldPosition = float4(worldPos, 0, 1);
	float4 camPosition=mul(WorldToCameraTransform,worldPosition);
	float4 renderPosition=mul(CameraToRenderTransform,camPosition);
	float4 clipPosition=mul(RenderToClipTransform,renderPosition);
    
    output.position=clipPosition;
    output.color = input.color;
    output.uv = input.uv;
    
    return output;
}

// ========== Pixel Shader ==========
Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

float4 PixelMain(VS_OUTPUT input) : SV_Target 
{
    float4 color = input.color;
    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, input.uv);

	clip(color.a-0.01f);
	return float4(color);
}