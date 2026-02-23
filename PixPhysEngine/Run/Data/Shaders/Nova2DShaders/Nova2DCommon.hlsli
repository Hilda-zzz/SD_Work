#include "Nova2DShaders/Nova2DParticleStruct.hlsli"
#include "Nova2DShaders/RNG.hlsli"

#ifndef NOVA2D_COMMON_HLSLI
#define NOVA2D_COMMON_HLSLI

//==========================================================================
// Random Number Generation
//==========================================================================
uint Hash(uint seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

float Random01(uint seed, uint index)
{
	return float(Hash(seed + index * 747796405u)) / 4294967296.0;
}

float RandomRange(uint seed, uint index, float minVal, float maxVal)
{
	return lerp(minVal, maxVal, Random01(seed, index));
}

float2 RandomRange2D(uint seed, uint indexOffset, float2 minVal, float2 maxVal)
{
	return float2(
		RandomRange(seed, indexOffset + 0, minVal.x, maxVal.x),
		RandomRange(seed, indexOffset + 1, minVal.y, maxVal.y)
	);
}

//==========================================================================
// Math Utilities
//==========================================================================
#define PI 3.14159265359
#define TWO_PI 6.28318530718

float2 Rotate2D(float2 v, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	return float2(
		v.x * c - v.y * s,
		v.x * s + v.y * c
	);
}


uint LerpColor(uint colorA, uint colorB, float t)
{
	uint rA = (colorA >> 0) & 0xFF;
	uint gA = (colorA >> 8) & 0xFF;
	uint bA = (colorA >> 16) & 0xFF;
	uint aA = (colorA >> 24) & 0xFF;

	uint rB = (colorB >> 0) & 0xFF;
	uint gB = (colorB >> 8) & 0xFF;
	uint bB = (colorB >> 16) & 0xFF;
	uint aB = (colorB >> 24) & 0xFF;

	uint r = uint(lerp(float(rA), float(rB), t));
	uint g = uint(lerp(float(gA), float(gB), t));
	uint b = uint(lerp(float(bA), float(bB), t));
	uint a = uint(lerp(float(aA), float(aB), t));

	return (a << 24) | (b << 16) | (g << 8) | r;
}

uint PackColor(float4 color)
{
    uint r = uint(saturate(color.r) * 255.0);
    uint g = uint(saturate(color.g) * 255.0);
    uint b = uint(saturate(color.b) * 255.0);
    uint a = uint(saturate(color.a) * 255.0);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

//==========================================================================
// Curve Evaluation
//==========================================================================

//==========================================================================
// Curve Evaluation
//==========================================================================

// Evaluate color curve with linear interpolation between keyframes
float4 EvaluateColorCurve(N2D_AppearanceModule appearance, float t)
{
    t = saturate(t);
    uint numKeyframes = appearance.m_numColorKeyframes;
    
    // Initialize result to white
    float4 result = float4(1, 1, 1, 1);
    
    if (numKeyframes > 0)
    {
        if (numKeyframes == 1)
        {
            result = appearance.m_colorKeyframes[0].m_colorPacked;
        }
        else
        {
            // Before first keyframe
            if (t <= appearance.m_colorKeyframes[0].m_time)
            {
                result = appearance.m_colorKeyframes[0].m_colorPacked;
            }
            // After last keyframe
            else if (t >= appearance.m_colorKeyframes[numKeyframes - 1].m_time)
            {
                result = appearance.m_colorKeyframes[numKeyframes - 1].m_colorPacked;
            }
            // Interpolate between keyframes
            else
            {
                result = appearance.m_colorKeyframes[numKeyframes - 1].m_colorPacked;
                for (uint i = 0; i < numKeyframes - 1; ++i)
                {
                    float t0 = appearance.m_colorKeyframes[i].m_time;
                    float t1 = appearance.m_colorKeyframes[i + 1].m_time;
                    
                    if (t >= t0 && t <= t1)
                    {
                        float localT = (t - t0) / max(t1 - t0, 0.0001);
                        float4 c0 = appearance.m_colorKeyframes[i].m_colorPacked;
                        float4 c1 = appearance.m_colorKeyframes[i + 1].m_colorPacked;
                        result = lerp(c0, c1, localT);
                        break;
                    }
                }
            }
        }
    }
    
    return result;
}

// Evaluate float curve (generic function)
float EvaluateFloatCurve(N2D_FloatCurve curve, float t)
{
    t = saturate(t);
    uint numKeyframes = curve.m_numKeyframes;
    
    // Initialize result to 0
    float result = 0.0f;
    
    if (numKeyframes > 0)
    {
        if (numKeyframes == 1)
        {
            result = curve.m_keyframes[0].m_value;
        }
        else
        {
            // Before first keyframe
            if (t <= curve.m_keyframes[0].m_time)
            {
                result = curve.m_keyframes[0].m_value;
            }
            // After last keyframe
            else if (t >= curve.m_keyframes[numKeyframes - 1].m_time)
            {
                result = curve.m_keyframes[numKeyframes - 1].m_value;
            }
            // Interpolate between keyframes
            else
            {
                result = curve.m_keyframes[numKeyframes - 1].m_value;
                for (uint i = 0; i < numKeyframes - 1; ++i)
                {
                    float t0 = curve.m_keyframes[i].m_time;
                    float t1 = curve.m_keyframes[i + 1].m_time;
                    
                    if (t >= t0 && t <= t1)
                    {
                        float localT = (t - t0) / max(t1 - t0, 0.0001);
                        float v0 = curve.m_keyframes[i].m_value;
                        float v1 = curve.m_keyframes[i + 1].m_value;
                        result = lerp(v0, v1, localT);
                        break;
                    }
                }
            }
        }
    }
    
    return result;
}

// Find and evaluate Size curve from definition
float EvaluateSizeCurve(Nova2DEmitterDefinitionGPU def, float t)
{
    float result = 5.0f;
    
    for (uint i = 0; i < def.m_numCurves; ++i)
    {
        if (def.m_curves[i].m_type == 0)
        {
            result = EvaluateFloatCurve(def.m_curves[i], t);
            break;
        }
    }
    
    return result;
}

// Find and evaluate Rotation Speed curve from definition
float EvaluateRotationSpeedCurve(Nova2DEmitterDefinitionGPU def, float t)
{
    float result = 3.14159f; 
    
    for (uint i = 0; i < def.m_numCurves; ++i)
    {
        if (def.m_curves[i].m_type == 1)  // ROTATION_SPEED = 1
        {
            result = EvaluateFloatCurve(def.m_curves[i], t);
            break;
        }
    }
    
    return result;
}

//==========================================================================
// Curl Noise Force Field (using Matthew's implementation)
//==========================================================================

// Compute Curl Noise using central differences
float3 SampleCurlNoise(float3 position, float delta, float curlNoiseScale, int noiseOctaves)
{
    float span = 2.f * delta;
    
    // Compute partial derivatives using central differences
    float dXy = (Compute3dPerlinNoise(position.x + delta, position.y, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 1) -
                 Compute3dPerlinNoise(position.x - delta, position.y, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 1)) / span;
    
    float dXz = (Compute3dPerlinNoise(position.x + delta, position.y, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 2) -
                 Compute3dPerlinNoise(position.x - delta, position.y, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 2)) / span;
    
    float dYx = (Compute3dPerlinNoise(position.x, position.y + delta, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 0) -
                 Compute3dPerlinNoise(position.x, position.y - delta, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 0)) / span;
    
    float dYz = (Compute3dPerlinNoise(position.x, position.y + delta, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 2) -
                 Compute3dPerlinNoise(position.x, position.y - delta, position.z, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 2)) / span;
    
    float dZx = (Compute3dPerlinNoise(position.x, position.y, position.z + delta, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 0) -
                 Compute3dPerlinNoise(position.x, position.y, position.z - delta, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 0)) / span;
    
    float dZy = (Compute3dPerlinNoise(position.x, position.y, position.z + delta, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 1) -
                 Compute3dPerlinNoise(position.x, position.y, position.z - delta, curlNoiseScale, noiseOctaves, .5f, 2.f, true, 1)) / span;
    
    // Curl = (dFz/dy - dFy/dz, dFx/dz - dFz/dx, dFy/dx - dFx/dy)
    float3 curl = float3(dZy - dYz, dXz - dZx, dYx - dXy);
    return normalize(curl);
}

// Wrapper for 2D particles: use (x, y, time) as 3D input
float2 ComputeCurlForce2D(float2 position, float time, float scale, float strength)
{
    // Sample position in 3D noise space (x, y, time)
    float3 pos3D = float3(position.x * scale, position.y * scale, time * 0.1);
    
    // Compute curl noise with default parameters
    float delta = 0.01; // Sample delta for derivatives
    float noiseScale = 1.0; // Noise scale
    int octaves = 3; // Noise octaves
    
    float3 curl3D = SampleCurlNoise(pos3D, delta, noiseScale, octaves);
    
    // Project to 2D (take XY components) and apply strength
    float2 force = float2(curl3D.x, curl3D.y) * strength;
    
    return force;
}

bool ColorMatch(float4 a, float4 b)
{
    const float tolerance = 1.0 / 255.0;
    return abs(a.r - b.r) < tolerance &&
           abs(a.g - b.g) < tolerance &&
           abs(a.b - b.b) < tolerance;
}

#endif // NOVA2D_COMMON_HLSLI