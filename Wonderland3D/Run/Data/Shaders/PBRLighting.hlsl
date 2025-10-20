
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define EPSILON 0.0001
#define MIN_ROUGHNESS 0.045

struct VertexInput
{
	float3 a_modelPosition : POSITION;
	float4 a_color : COLOR;
	float2 a_uv : TEXCOORD;
 	float3 a_modelTangent : TANGENT;
 	float3 a_modelBitangent : BITANGENT;
 	float3 a_modelNormal : NORMAL;
};

struct VertexOutPixelIn
{
	float4 v_worldPosition: WORLD_POSITION;
	float4 v_clipPosition : SV_Position;
	float4 v_color : COLOR;
	float2 v_uv : TEXCOORD;
 	float4 v_worldTangent : WORLD_TANGENT;
 	float4 v_worldBitangent : WORLD_BITANGENT;
 	float4 v_worldNormal : WORLD_NORMAL;

 	float4 v_modelTangent : MODEL_TANGENT;      
 	float4 v_modelBitangent : MODEL_BITANGENT;  
 	float4 v_modelNormal : MODEL_NORMAL;     

	//float4 v_lightSpacePos : LIGHT_SPACE;   
};

cbuffer LightConstants : register(b1)
{
	float3 c_sunDirection;
	float c_sunIntensity;
	float c_ambientIntensity;
};

cbuffer CameraConstants : register(b2)
{
	float4x4 c_worldToCameraTransform;		// View Mat
	float4x4 c_cameraToRenderTransform;		// Non-standard transform from game to DirectX conventions (Game Mat)
	float4x4 c_renderToClipTransform;		// Projection Mat
};

cbuffer ModelConstants : register(b3)
{
	float4x4 c_modelToWorldTransform;		// Model transform
	float4 c_modelColor;
};

cbuffer PerFrameConstants : register(b7)
{
	float		c_time;
	int			c_debugInt;
	float		c_debugFloat;
	float		EMPTY_PADDING;
};

cbuffer PBRMaterialConstants : register(b8)
{
    float3 c_albedo;          
    float c_metallic;        
    
    float c_roughness;         
    float c_ao;                
    float2 c_padding1;
    
    float3 c_emissive;        
    float c_emissiveStrength;
};

Texture2D t_albedoTexture : register(t0);
Texture2D t_normalTexture : register(t1);
Texture2D t_MetallicRoughnessTexture : register(t2);  // R=Metallic, G=Roughness
Texture2D t_AOTexture : register(t3);
Texture2D t_EmissiveTexture : register(t4);

SamplerState s_samplerState : register(s0);
SamplerState s_normalSampler : register(s1);

//================================ Help Functions=========================
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if(inEnd!=inStart)
	{
		float fraction = (inValue - inStart) / (inEnd - inStart);
		float outValue = outStart + fraction * (outEnd - outStart);
		return outValue;
	}
	return outStart;
}

float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	float fraction = saturate( (inValue - inStart) / (inEnd - inStart) );
	float outValue = outStart + fraction * (outEnd - outStart);
	return outValue;
}

// x,y,z in [-1,1] to rgb [0,1]
float3 EncodeXYZToRGB( float3 vec )
{
	return (vec + 1.0) * 0.5;
}
// rgb in [0,1] to xyz in [-1,1]
float3 DecodeRGBToXYZ( float3 color )
{
	return (color * 2.0) - 1.0;
}

float3 GetCameraWorldPosition(float4x4 viewMatrix)
{
    float3x3 rotation = transpose((float3x3)viewMatrix);
    float3 translation = -viewMatrix._14_24_34;
    return mul(rotation, translation);
}
//======================================================================

float DistributionGGX(float3 N, float3 H, float a)
{
	float alpha = max(a * a, MIN_ROUGHNESS);
	float a2=alpha*alpha;
	float NdotH=max(dot(N,H),0.0f);
	float NdotH2=NdotH*NdotH;

	float nom=a2;
	float denom=(NdotH2*(a2-1.f)+1.f);
	denom=PI * denom * denom;

	denom = max(denom, EPSILON);

	return nom/denom;
}

float GeometrySchlickGGX(float NdotV,float k)
{
	float nom=NdotV;
	float denom=NdotV*(1.f-k)+k;
	return nom/denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float k)
{
	float NdotV=max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);

	float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0+(1.f-F0)*pow(1.0-cosTheta,5.0);
}

float3 CalculatePBR(float3 N, float3 V, float3 L, 
                    float3 albedo, float metallic, float roughness)
{
	float3 H = normalize(V + L);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);

	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
    float G = GeometrySmith(N, V, L, k);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular     = numerator / denominator;  

	float3 kS = F;
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metallic;

	float NdotL = max(dot(N, L), 0.0); 
	float3 diffuse = kD * albedo / PI;

	return (diffuse + specular) * NdotL * c_sunIntensity;
}

//======================================================================
VertexOutPixelIn VertexMain(VertexInput input)
{
	float4 modelPosition = float4(input.a_modelPosition, 1);
	float4 worldPosition = mul(c_modelToWorldTransform, modelPosition);      
	float4 cameraPosition = mul(c_worldToCameraTransform, worldPosition);    
	float4 renderPosition = mul(c_cameraToRenderTransform, cameraPosition);  
	float4 clipPosition = mul(c_renderToClipTransform, renderPosition);      

 	float4 worldTangent = mul(c_modelToWorldTransform, float4(input.a_modelTangent, 0.0f));     
 	float4 worldBitangent = mul(c_modelToWorldTransform, float4(input.a_modelBitangent, 0.0f)); 
 	float4 worldNormal = mul(c_modelToWorldTransform, float4(input.a_modelNormal, 0.0f));       

	//float4 lightSpacePos=mul(LightViewProjection,worldPosition);

	VertexOutPixelIn v2p;
	v2p.v_clipPosition = clipPosition;
	v2p.v_color = input.a_color;
	v2p.v_uv = input.a_uv;
	v2p.v_worldPosition=worldPosition;
 	v2p.v_worldTangent = worldTangent;
 	v2p.v_worldBitangent = worldBitangent;
 	v2p.v_worldNormal = worldNormal;

	v2p.v_modelTangent = float4(input.a_modelTangent, 0.0f);      
	v2p.v_modelBitangent = float4(input.a_modelBitangent, 0.0f);  
	v2p.v_modelNormal = float4(input.a_modelNormal, 0.0f);      

	//v2p.v_lightSpacePos=lightSpacePos;

	return v2p;
}

float4 PixelMain(VertexOutPixelIn input) : SV_Target0
{
	// === Sample Material Texture ===
	float3 texAlbedo = t_albedoTexture.Sample(s_samplerState, input.v_uv).rgb;
    float2 texMR = t_MetallicRoughnessTexture.Sample(s_samplerState, input.v_uv).rg;
    float texAO = t_AOTexture.Sample(s_samplerState, input.v_uv).r;
	float3 texEmissive=t_EmissiveTexture.Sample(s_samplerState, input.v_uv).rgb;

	// === Apply parameters ===
    float3 albedo = texAlbedo * c_albedo  * c_modelColor.rgb * input.v_color.rgb;
    float metallic = texMR.r * c_metallic;
    float roughness = texMR.g * c_roughness;
    float ao = texAO * c_ao;
	float3 emissive=texEmissive*c_emissive;

	// === Normal ===
	float4 normalTextureColor = t_normalTexture.Sample(s_normalSampler, input.v_uv);
	float3 pixelNormalTBNSpace=normalize(DecodeRGBToXYZ(normalTextureColor.rgb));

	// input world && model normal
	float3 surfaceTangentWorldSpace		= normalize( input.v_worldTangent.xyz );
	float3 surfaceBitangentWorldSpace	= normalize( input.v_worldBitangent.xyz );
	float3 surfaceNormalWorldSpace		= normalize( input.v_worldNormal.xyz );

	float3 surfaceTangentModelSpace		= normalize( input.v_modelTangent.xyz );
	float3 surfaceBitangentModelSpace	= normalize( input.v_modelBitangent.xyz );
	float3 surfaceNormalModelSpace		= normalize( input.v_modelNormal.xyz );

	// directional light
	float3 sunDir=-normalize(c_sunDirection.xyz);

	float3 cameraWorldPos = GetCameraWorldPosition(c_worldToCameraTransform);
	float3 viewDir = normalize(cameraWorldPos - input.v_worldPosition.xyz);
    float3 halfVector = normalize(sunDir + viewDir);

	// combine pixel normal with world normal
	float3x3 tbnToWorldMat = float3x3(surfaceTangentWorldSpace, surfaceBitangentWorldSpace, surfaceNormalWorldSpace);
	float3 pixelNormalWorldSpace = mul(pixelNormalTBNSpace, tbnToWorldMat);

	// ambient
	float3 ambient=albedo*c_ambientIntensity*ao;

	float3 directLightColor = CalculatePBR(surfaceNormalWorldSpace, viewDir, sunDir,
										 albedo, metallic, roughness);

	float4 finalColor=float4(directLightColor+ambient,1.f);

	//========================================

 	clip(finalColor.a - 0.01f);
 	return finalColor;
}