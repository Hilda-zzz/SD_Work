//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 modelPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
 	float3 modelTangent : TANGENT;
 	float3 modelBitangent : BITANGENT;
 	float3 modelNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
 	float4 worldTangent : TANGENT;
 	float4 worldBitangent : BITANGENT;
 	float4 worldNormal : NORMAL;
    float4 worldPosition : POSITION;
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
};

cbuffer PointLightBuffer : register(b4)
{
	struct PointLight
	{
		float3 Position;
		float Range;
		float4 Color;
		float Intensity;
		float3 Attenuation;
	};
	PointLight pointLights[10];
    uint ActivePointLightCount;
}

cbuffer SpotLightBuffer : register(b5)
{
	struct SpotLight
	{
		float3 Position;
		float Range;
		float4 Color;
		float Intensity;
		float3 Attenuation;
        float Cone;
	    float3 Direction;
        float SpotCutoffAngle;
        float3 padding;
	};
	SpotLight spotLights[10];
    uint ActiveSpotLightCount;
}


//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 WorldToCameraTransform;	// View transform
	float4x4 CameraToRenderTransform;	// Non-standard transform from game to DirectX conventions
	float4x4 RenderToClipTransform;		// Projection transform
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelToWorldTransform;		// Model transform
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);

//------------------------------------------------------------------------------------------------
SamplerState samplerState : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 modelPosition = float4(input.modelPosition, 1);
	float4 worldPosition = mul(ModelToWorldTransform, modelPosition);
	float4 cameraPosition = mul(WorldToCameraTransform, worldPosition);
	float4 renderPosition = mul(CameraToRenderTransform, cameraPosition);
	float4 clipPosition = mul(RenderToClipTransform, renderPosition);

 	float4 worldTangent = mul(ModelToWorldTransform, float4(input.modelNormal, 0.0f));
 	float4 worldBitangent = mul(ModelToWorldTransform, float4(input.modelNormal, 0.0f));
 	float4 worldNormal = mul(ModelToWorldTransform, float4(input.modelNormal, 0.0f));

	v2p_t v2p;
	v2p.clipPosition = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
 	v2p.worldTangent = worldTangent;
 	v2p.worldBitangent = worldBitangent;
 	v2p.worldNormal = worldNormal;
    v2p.worldPosition=worldPosition;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
    float ambient = AmbientIntensity;
    float directional = SunIntensity * saturate(dot(normalize(input.worldNormal.xyz), -SunDirection));
    float3 totalLighting = float3(ambient + directional, ambient + directional, ambient + directional);
    
    for(uint i = 0; i < ActivePointLightCount; i++)
    {
        float3 lightVector = pointLights[i].Position - input.worldPosition.xyz;
        float distance = length(lightVector);
        
        if(distance < pointLights[i].Range)
        {
            lightVector = normalize(lightVector);
            
            float NdotL = saturate(dot(normalize(input.worldNormal.xyz), lightVector));
            
            float attenuation = 1.0 / (pointLights[i].Attenuation.x + 
                                     pointLights[i].Attenuation.y* distance + 
                                     pointLights[i].Attenuation.z * distance * distance);
            
            totalLighting += pointLights[i].Color.xyz * pointLights[i].Intensity * NdotL * attenuation;
        }
    }
       
    for(uint j = 0; j < ActiveSpotLightCount; j++)
    {
        float3 lightToPixelVec = spotLights[j].Position - input.worldPosition.xyz;
        float dist = length(lightToPixelVec);
        if(dist < spotLights[j].Range)
        {
            float3 lightDir = normalize(lightToPixelVec);
            float spotEffect = dot(lightDir, normalize(-spotLights[j].Direction));
            float spotFactor = 0.0;
        
            if (spotEffect > cos(spotLights[j].SpotCutoffAngle))
                spotFactor = pow(spotEffect, spotLights[j].Cone);
            
            float diffuseFactor = max(dot(input.worldNormal.xyz, lightDir), 0.0);
            float attenuation = 1.0 / (spotLights[j].Attenuation.x + 
                                      spotLights[j].Attenuation.y * dist + 
                                      spotLights[j].Attenuation.z * dist * dist);
                                  
            totalLighting += spotLights[j].Color.xyz * spotLights[j].Intensity * diffuseFactor * attenuation * spotFactor;
        }
    }
    
    
    float4 lightColor = float4(totalLighting, 1);
    
    float4 textureColor = diffuseTexture.Sample(samplerState, input.uv);
    float4 vertexColor = input.color;
    float4 modelColor = ModelColor;
    float4 color = lightColor * textureColor * vertexColor * modelColor;
    
    clip(color.a - 0.01f);
    return color;
}
