// ProceduralSky.hlsl - Procedural Sky Shader
// Stage 1: Basic solid color rendering to verify geometry and depth setup

struct vs_input_t
{
    float3 modelPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct v2p_t
{
    float4 clipPosition : SV_Position;
    float3 worldPosition : POSITION0;
    float3 viewDirection : TEXCOORD0;
    float4 color : COLOR;
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

cbuffer SkyConstants : register(b4)
{
    float3 SunDirection;
    float TimeOfDay;
    float3 MoonDirection;
    float SunIntensity;
    float3 RayleighScattering;
    float AtmosphereRadius;
    float3 MieScattering;
    float PlanetRadius;
    float RayleighScaleHeight;
    float MieScaleHeight;
    float MieG;
    float MoonIntensity;
    float3 CameraPosition;
    float StarBrightness;

    float3 SkyHorizonColor;    
    float Padding2;
    
    float3 SkyZenithColor;     
    float Padding3;
    
    float3 UnderHorizonColor;  
    float Padding4;
    
    float3 UnderNadirColor;    
    float Padding5;
};

v2p_t VertexMain(vs_input_t input)
{
    v2p_t output;
    
    float4 modelPos = float4(input.modelPosition, 1.0);
    float4 worldPos = mul(ModelToWorldTransform, modelPos);

    float4x4 viewNoTranslation = WorldToCameraTransform;
    viewNoTranslation._14 = 0; 
    viewNoTranslation._24 = 0; 
    viewNoTranslation._34 = 0; 
    viewNoTranslation._44 = 1; 

    output.worldPosition = worldPos.xyz;
    output.viewDirection = normalize(worldPos.xyz - CameraPosition);
    
    float4 camPos = mul(viewNoTranslation, worldPos);
    float4 renderPos = mul(CameraToRenderTransform, camPos);
    float4 clipPos = mul(RenderToClipTransform, renderPos);
    
    clipPos.z = clipPos.w;  // Force depth to farthest
    
    output.clipPosition = clipPos;
    output.color = input.color;
    return output;
}

Texture2D moonTexture : register(t0);
SamplerState samplerState : register(s0);

float GetSunMask(float sunViewDot, float sunRadius)
{
    float threshold = 1.0 - sunRadius * sunRadius;
    return step(threshold, sunViewDot);
}

float GetGlow(float viewDot, float glowSize, float glowPower)
{
    float distance = 1.0 - viewDot; 
    float glow = saturate(1.0 - distance / glowSize);  
    return pow(glow, glowPower);
}

float3 GetAtmosphereGlowDisk(
    float3 viewDir,
    float3 sunDir,
    float sunHeight,
    float diskRadius,      
    float diskCenterOffset, 
    float glowFalloff,     
    float glowIntensity,   
    float3 glowColor   
)
{
    float verticalOffset = diskCenterOffset + sunHeight * 0.5;  
    
    float3 horizonSunDir = normalize(float3(sunDir.x, 0.0, sunDir.z)); 
    float3 diskCenter = horizonSunDir * cos(verticalOffset) + float3(0, verticalOffset, 0);
    
    diskCenter = sunDir + float3(0, verticalOffset, 0);
    
    float distToCenter = length(viewDir - diskCenter);
    float distToEdge = distToCenter - diskRadius;
    
    float edgeGlow = 0.0;
    
    if (distToEdge < glowFalloff) 
    {

        float t = saturate(1.0 - distToEdge / glowFalloff);
        edgeGlow = pow(t, 2.0) * glowIntensity;
    }
    
    float sunLowFactor = saturate(1.0 - abs(sunHeight) / 0.3);
    sunLowFactor = pow(sunLowFactor, 2.0);
    
    float viewHorizonFactor = saturate(1.0 - abs(viewDir.y) / 0.4);
    viewHorizonFactor = pow(viewHorizonFactor, 2.0);
    
    return glowColor * edgeGlow * sunLowFactor * viewHorizonFactor;
}

// From Inigo Quilez, https://www.iquilezles.org/www/articles/intersectors/intersectors.htm
float sphIntersect(float3 rayDir, float3 spherePos, float radius)
{
    float3 oc = -spherePos;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;

    float result = -1.0;  
    if (h >= 0.0)
    {
        float t = -b - sqrt(h);
        if (t >= 0.0)
        {
            result = t;
        }
    }
    
    return result; 
}

// -------------------------------------------------------------------------------

float MiePhaseFunction(float cosAngle)
{
    float g = MieG; 
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159 * pow(abs(1.0 + g2 - 2.0 * g * cosAngle), 1.5));
}

void ComputeOutLocalDensity(float3 position, float3 lightDir, out float localDPA, out float DPC)
{
    float3 planetCenter = float3(0, -PlanetRadius, 0);
    
    float height = distance(position, planetCenter) - PlanetRadius;
    
    localDPA = exp(-height / MieScaleHeight); 
    
    DPC = 0;
}

float4 IntegrateInscattering(float3 rayStart, float3 rayDir, float rayLength, 
                             float3 lightDir, float sampleCount)
{
    float ExtinctionM=0.055f;

    //----------------
    float3 stepVector = rayDir * (rayLength / sampleCount);
    float stepSize = length(stepVector);
    
    float scatterMie = 0;
    float densityCP = 0;
    float densityPA = 0;
    float localDPA = 0;
    float prevLocalDPA = 0;
    float prevTransmittance = 0;
    
    ComputeOutLocalDensity(rayStart, lightDir, localDPA, densityCP);
    densityPA += localDPA * stepSize;
    prevLocalDPA = localDPA;
    
    float Transmittance = exp(-(densityCP + densityPA) * ExtinctionM) * localDPA;
    prevTransmittance = Transmittance;
    
    for(float i = 1.0; i < sampleCount; i += 1.0)
    {
        float3 P = rayStart + stepVector * i;
        
        ComputeOutLocalDensity(P, lightDir, localDPA, densityCP);
        
        densityPA += (prevLocalDPA + localDPA) * stepSize / 2.0;
        
        Transmittance = exp(-(densityCP + densityPA) * ExtinctionM) * localDPA;

        scatterMie += (prevTransmittance + Transmittance) * stepSize / 2.0;
        
        prevTransmittance = Transmittance;
        prevLocalDPA = localDPA;
    }
    
    scatterMie = scatterMie * MiePhaseFunction(dot(rayDir, -lightDir));
    
    float3 lightInscatter = MieScattering * scatterMie;
    
    return float4(lightInscatter, 1);
}

//------------------------------------------------------------------------

float4 PixelMain(v2p_t input) : SV_Target0
{
    float3 viewDir = normalize(input.viewDirection);
    float3 sunPosDirection=-SunDirection;
    float3 moonPosDirection=-MoonDirection;
    // Main angles
    float sunViewDot = dot(sunPosDirection, viewDir);
    float sunZenithDot = sunPosDirection.y;
    float viewZenithDot = viewDir.y;
    float sunMoonDot = dot(sunPosDirection, moonPosDirection);

    float height = viewDir.z;

    float3 skyColor;
    
    if (height > 0.0)  // Above horizon
    {
        float t = saturate(height);
        skyColor = lerp(SkyHorizonColor, SkyZenithColor, t);
        
        // sun and moon mask
        float sunMask = GetSunMask(sunViewDot, 0.04f);
        float3 sunColor = float3(0.8f,0.6f,0.6f) * sunMask;
        float moonIntersect = sphIntersect(viewDir, moonPosDirection, 0.06);
        float moonMask = moonIntersect > -1 ? 1 : 0;
        float3 moonNormal = normalize(viewDir * moonIntersect - moonPosDirection);
        float moonNdotL = saturate(dot(moonNormal, sunPosDirection-moonPosDirection));

        float u = 0.5 + atan2(moonNormal.x, moonNormal.z) / (2.0 * 3.14159);
        float v = 0.5 - asin(moonNormal.y) / 3.14159;
        float3 texColor = moonTexture.Sample(samplerState, float2(u, v)).rgb;
        float exposureMultiplier = exp2(0.f);
        float3 moonColor = moonMask * moonNdotL*texColor*exposureMultiplier;

        //float3 atmosphereGlow = GetAtmosphereGlowDisk(
        //    viewDir,
        //    sunPosDirection,
        //    sunPosDirection.z,
        //    15.0,           
        //    -0.5,          
        //    2.0,         
        //    2.0,        
        //    float3(1.0, 0.6, 0.3)  
        //);
        float sunGlow=GetGlow(sunViewDot, 0.01f, 3.f);

        skyColor = skyColor+sunColor+moonColor+sunGlow;
        //skyColor =atmosphereGlow;
    }
    else  // Below horizon
    {
        float t = saturate(-height);
        skyColor = lerp(SkyHorizonColor, UnderNadirColor, t);
    }
    
    return float4(skyColor, 1.0);
}