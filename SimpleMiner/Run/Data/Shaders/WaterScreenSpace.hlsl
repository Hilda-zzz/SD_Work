//=============================================================================
// WaterBasic.hlsl
// Basic water shader with normal mapping and lighting
//
// Features:
// - Dual-layer animated normal mapping
// - Lighting (ambient + directional + specular)
// - Water color blending
// - Transparency
//
// TODO:
// - Phase 2: Screen-space refraction
// - Phase 3: Skybox reflection
// - Phase 4: Fresnel effect
// - Phase 5: Depth fog
//=============================================================================

//------------------------------------------------------------------------------------------------
// Input Structures
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
// Vertex to Pixel Structure
//------------------------------------------------------------------------------------------------
struct v2p_t
{
    float4 clipPosition : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 worldTangent : TANGENT;
    float3 worldBitangent : BITANGENT;
    float3 worldNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
// Constant Buffers
//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
    float3 SunDirection;
    float SunIntensity;
    float AmbientIntensity;
    float3 padding1;
};

cbuffer CameraConstants : register(b2)
{
    float4x4 WorldToCameraTransform;
    float4x4 CameraToRenderTransform;
    float4x4 RenderToClipTransform;
	float2 ViewportSize; 
    float2 Padding2;
};

cbuffer ModelConstants : register(b3)
{
    float4x4 ModelToWorldTransform;
    float4 ModelColor;
};

cbuffer WaterConstants : register(b4)
{
    float Time;
    float WaveSpeed;
    float WaveScale;
    float SpecularPower;
    
    float3 DeepWaterColor;
    float SpecularIntensity;
    
    float3 ShallowWaterColor;
    float WaterAlpha;

    float RefractionStrength;  
    float3 _Padding;
};

//------------------------------------------------------------------------------------------------
// Textures
//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D sceneColorTexture : register(t2);  
Texture2D sceneDepthTexture : register(t3);  

//------------------------------------------------------------------------------------------------
// Samplers
//------------------------------------------------------------------------------------------------
SamplerState samplerState : register(s0);

//=============================================================================
// Vertex Shader
//=============================================================================
v2p_t VertexMain(vs_input_t input)
{
    // Transform to clip space
    float4 modelPosition = float4(input.modelPosition, 1.0);
    float4 worldPosition = mul(ModelToWorldTransform, modelPosition);
    float4 cameraPosition = mul(WorldToCameraTransform, worldPosition);
    float4 renderPosition = mul(CameraToRenderTransform, cameraPosition);
    float4 clipPosition = mul(RenderToClipTransform, renderPosition);
    
    // Transform TBN to world space
    float3 worldTangent = normalize(mul((float3x3)ModelToWorldTransform, input.modelTangent));
    float3 worldBitangent = normalize(mul((float3x3)ModelToWorldTransform, input.modelBitangent));
    float3 worldNormal = normalize(mul((float3x3)ModelToWorldTransform, input.modelNormal));
    
    // Output
    v2p_t v2p;
    v2p.clipPosition = clipPosition;
    v2p.color = input.color;
    v2p.uv = input.uv;
    v2p.worldPosition = worldPosition.xyz;
    v2p.worldTangent = worldTangent;
    v2p.worldBitangent = worldBitangent;
    v2p.worldNormal = worldNormal;
    
    return v2p;
}

//=============================================================================
// Helper Functions
//=============================================================================
float3 SampleWaterNormal(float2 baseUV, float time)
{
    float3 normal = float3(0, 0, 0);
    
    float wave1 = sin(time * 0.2 + baseUV.x * 2.0) * 0.05;  
    float2 uv1 = baseUV * (WaveScale * 0.6)  
               + float2(time * WaveSpeed * 0.4f, wave1);
    normal += (normalTexture.Sample(samplerState, uv1).rgb * 2.0 - 1.0) * 0.5; 
 
    float wave2 = sin(time * 0.35 + baseUV.y * 2.0) * 0.05; 
    float2 uv2 = baseUV * (WaveScale * 1.2)  
               + float2(-time * WaveSpeed * 0.25f, time * WaveSpeed * 0.2f + wave2);
    normal += (normalTexture.Sample(samplerState, uv2).rgb * 2.0 - 1.0) * 0.6;
    
    float wave3 = sin(time * 0.5 + baseUV.x * 4.0 + baseUV.y * 4.0) * 0.05; 
    float2 uv3 = baseUV * (WaveScale * 1.5) 
               + float2(time * WaveSpeed * 0.4f + wave3, -time * WaveSpeed * 0.35f);
    normal += (normalTexture.Sample(samplerState, uv3).rgb * 2.0 - 1.0) * 0.5; 
    
    return normalize(normal);
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

//=============================================================================
// Pixel Shader
//=============================================================================

struct SSRResult
{
    bool hit;
    float3 color;
    float confidence;  
};

float NDCDepthToViewDepth(float ndcDepth, float near, float far)
{
    return (near * far) / (far - ndcDepth * (far - near));
}

SSRResult TraceScreenSpaceRay(float3 worldPos, float3 reflectDir, float2 screenUV, float maxDistance)
{
    SSRResult result;
    result.hit = false;
    result.color = float3(0, 0, 0);
    result.confidence = 0.0;
    
    const int maxSteps = 1024;
    float stepSize = 0.05;
    const float depthThreshold = 0.2; 
    
    float3 rayPos = worldPos;
    float3 rayDir = reflectDir;
    
    float oriZ = 0.f;
    
    for (int i = 0; i < maxSteps; i++)
    {
        rayPos += rayDir * stepSize;
        
        float4 cameraPosition = mul(WorldToCameraTransform, float4(rayPos, 1.0));
        float4 renderPosition = mul(CameraToRenderTransform, cameraPosition);
        float4 clipPos = mul(RenderToClipTransform, renderPosition);
        
        if (clipPos.w <= 0.0) break;
        
        float3 ndc = clipPos.xyz / clipPos.w;
        
        // in screen
        if (abs(ndc.x) > 1.0 || abs(ndc.y) > 1.0 || ndc.z < 0.0 || ndc.z > 1.0)
            break;
        
        float2 rayUV = ndc.xy * 0.5 + 0.5;
        rayUV.y = 1.0 - rayUV.y;
        
        float sceneDepth = sceneDepthTexture.SampleLevel(samplerState, rayUV, 0).r;
        float rayDepth = ndc.z;
        float sceneDepthView = NDCDepthToViewDepth(sceneDepth, 0.1, 1000);
        float rayDepthView = clipPos.w;

        float depthDiff = rayDepthView - sceneDepthView;
        
        if (depthDiff > 0.0 && depthDiff < depthThreshold)
        {
            result.hit = true;
            result.color = sceneColorTexture.SampleLevel(samplerState, rayUV, 0).rgb;
            
            float2 edgeFactor = 1.0 - abs(rayUV * 2.0 - 1.0);
            result.confidence = min(edgeFactor.x, edgeFactor.y);
            
            result.confidence *= saturate(1.0 - (depthDiff / depthThreshold));
            
            break;
        }
    }
    
    return result;
}



float4 PixelMain(v2p_t input) : SV_Target0
{
    // ========== 1. Sample normal map ==========
    float3 normalTS = SampleWaterNormal(input.uv, Time);

    // ========== 2. Tangent space to world space ==========
    float3x3 TBN = float3x3(
        normalize(input.worldTangent),
        normalize(input.worldBitangent),
        normalize(input.worldNormal)
    );

    float3 worldNormal = normalize(mul(normalTS, TBN));
    float3 cameraWorldPos = GetCameraWorldPosition(WorldToCameraTransform);
    float3 viewDir = normalize(cameraWorldPos - input.worldPosition);
    
    // ========== 2.5 Screen-space refraction (NEW) ==========
    float2 screenUV = input.clipPosition.xy / ViewportSize;
    float2 refractUV = screenUV + worldNormal.xy * RefractionStrength;
    refractUV = saturate(refractUV);
    float3 underwaterColor = sceneColorTexture.Sample(samplerState, refractUV).rgb;

    // ========== 2.6 Screen-space reflection (NEW) ==========

    float3 reflectDir = reflect(-viewDir, worldNormal);
    
    SSRResult ssrResult = TraceScreenSpaceRay(
        input.worldPosition,
        reflectDir,
        screenUV,
        50.0  
    );
    
    float3 reflectionColor;
    if (ssrResult.hit)
    {
        reflectionColor = ssrResult.color;
    }
    else
    {
        reflectionColor = float3(0.5, 0.7, 1.0);  
    }

    // ========== 3. Lighting ==========
    // Ambient
    float ambient = AmbientIntensity;
    // Diffuse
    float NdotL = saturate(dot(worldNormal, -SunDirection));
    float diffuse = SunIntensity * NdotL;
    // Specular (Blinn-Phong)
    float3 halfVector = normalize(-SunDirection + viewDir);
    float NdotH = saturate(dot(worldNormal, halfVector));
    float specular = pow(max(NdotH,0), SpecularPower) * SunIntensity;

    // ========== 4. Water color (TODO: use Fresnel) ==========
    float NdotV = saturate(dot(worldNormal, viewDir));
    float fresnel = 0.02 + 0.98 * pow(1.0 - NdotV, 2.0);

    //float3 waterColor = lerp(ShallowWaterColor, DeepWaterColor, 0.5);

    // ========== 4.1 Calculate water depth ==========
    float pixelDepth = sceneDepthTexture.Sample(samplerState, screenUV).r;
    float pixelViewDepth = NDCDepthToViewDepth(pixelDepth, 0.1, 1000.0);
    
    float4 waterClipPos = float4(input.worldPosition,1.0);
    float waterNDCDepth = waterClipPos.z / waterClipPos.w;
    float waterViewDepth = NDCDepthToViewDepth(waterNDCDepth, 0.1, 1000.0);
    
    float underwaterDepth = abs(pixelViewDepth - waterViewDepth);

    float transmissionAmount = (1.0 - fresnel);
    float depthAttenuation = exp(-underwaterDepth * 0.05);
    float3 transmission = underwaterColor * transmissionAmount * depthAttenuation;

    float depthFactor = saturate(underwaterDepth / 40.0); // Adjust 10.0 based on your scene scale
    float3 waterColor = lerp(ShallowWaterColor, DeepWaterColor, depthFactor);
    float3 absorption = waterColor * (1.0 - transmissionAmount * depthAttenuation);

    //float3 underwaterFade = underwaterColor * exp(-underwaterDepth * 0.1); 

    float3 baseColor = transmission + absorption; //lerp(underwaterFade, waterColor, 0.5 + depthFactor * 0.3);

    // ========== 5. Final color composition ==========
    // Apply lighting to water color
    
    float3 lightColor = (ambient + diffuse) * baseColor;
    
    // Add sun specular highlight
    float3 sunColor = float3(1.0, 0.95, 0.8);
    lightColor += specular * sunColor;
    
    // Apply vertex and model color
    float3 finalColor = lightColor * ModelColor.rgb;

    float reflectionMix = fresnel * ssrResult.confidence;
    if(!ssrResult.hit) 
        reflectionMix=0.f;
    finalColor = lerp(finalColor, reflectionColor, reflectionMix);
    
    // ========== 6. Return with alpha ==========
    return float4(finalColor, 1.f);
}
