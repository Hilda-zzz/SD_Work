//Third party rng code from https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/shaders/globals.hlsli and Squirrel Noise

#include "Nova2DShaders/Math.hlsli"

static const float fSQRT_3_OVER_3 = 0.57735026918962576450914878050196f;

// Random number generator based on: https://github.com/diharaw/helios/blob/master/src/engine/shader/random.glsl
struct RNG
{
	uint2 s; // state

	// xoroshiro64* random number generator.
	// http://prng.di.unimi.it/xoroshiro64star.c
	uint rotl(uint x, uint k)
	{
		return (x << k) | (x >> (32 - k));
	}
	// Xoroshiro64* RNG
	uint next()
	{
		uint result = s.x * 0x9e3779bb;

		s.y ^= s.x;
		s.x = rotl(s.x, 26) ^ s.y ^ (s.y << 9);
		s.y = rotl(s.y, 13);

		return result;
	}
	// Thomas Wang 32-bit hash.
	// http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
	uint hash(uint seed)
	{
		seed = (seed ^ 61) ^ (seed >> 16);
		seed *= 9;
		seed = seed ^ (seed >> 4);
		seed *= 0x27d4eb2d;
		seed = seed ^ (seed >> 15);
		return seed;
	}

	void init(uint2 id, uint frameIndex)
	{
		uint s0 = (id.x << 16) | id.y;
		uint s1 = frameIndex;
		s.x = hash(s0);
		s.y = hash(s1);
		next();
	}
	float next_float()
	{
		uint u = 0x3f800000 | (next() >> 9);
		return asfloat(u) - 1.0;
	}

	float get_random_float_in_range(float f1, float f2)
	{
		float random01 = next_float();
		float range = f2 - f1;
		return random01 * range + f1;
	}

	uint get_random_uint_in_range(uint u1, uint u2)
	{
		float random01 = next_float();
		float range = (u2 + 1) - u1;
		return ((uint)(random01 * range)) + u1;
	}

	uint next_uint(uint nmax)
	{
		float f = next_float();
		return uint(floor(f * nmax));
	}
	float2 next_float2()
	{
		return float2(next_float(), next_float());
	}
	float3 next_float3()
	{
		return float3(next_float(), next_float(), next_float());
	}
};

//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 3D, gradients are unit-length vectors in random (3D) directions.
//

//-----------------------------------------------------------------------------------------------
// Fast hash of an int32 into a different (unrecognizable) uint32.
//
// Returns an uinteger containing 32 reasonably-well-scrambled bits, based on the hash
//	of a given (signed) integer input parameter (position/index) and [optional] seed.  Kind of
//	like looking up a value in an infinitely large table of previously generated random numbers.
//
// The bit-noise constants and bit-shifts were evolved by a genetic algorithm using the
//	"BigCrush" statistical tests for fitness, and have so far produced excellent test results.
//
// I call this particular approach SquirrelNoise (version 4).
//
uint Get1dNoiseUint(int positionX, uint seed)
{
	uint BIT_NOISE1 = 0xd2a80a23;
	uint BIT_NOISE2 = 0xa884f197;
	uint BIT_NOISE3 = 0x1b56c4e9;

	uint mangledBits = (uint)positionX;
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 7);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits >> 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 11);
	return mangledBits;
}

//-----------------------------------------------------------------------------------------------
static const uint Get2dNoiseUint(int indexX, int indexY, uint seed)
{
	static const int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
	return Get1dNoiseUint(indexX + (PRIME_NUMBER * indexY), seed);
}

//-----------------------------------------------------------------------------------------------
uint Get3dNoiseUint(int indexX, int indexY, int indexZ, uint seed)
{
	int PRIME1 = 198491317; // Large prime number with non-boring bits
	int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint(indexX + (PRIME1 * indexY) + (PRIME2 * indexZ), seed);
}

//-----------------------------------------------------------------------------------------------
uint Get4dNoiseUint(int indexX, int indexY, int indexZ, int indexT, uint seed)
{
	static const int PRIME1 = 198491317; // Large prime number with non-boring bits
	static const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
	static const int PRIME3 = 357239; // Large prime number with distinct and non-boring bits
	return Get1dNoiseUint(indexX + (PRIME1 * indexY) + (PRIME2 * indexZ) + (PRIME3 * indexT), seed);
}

//-----------------------------------------------------------------------------------------------
static const float Get1dNoiseZeroToOne(int index, uint seed)
{
	static const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get1dNoiseUint(index, seed));
}

static const float Get1dNoiseInRange(float min, float max, int index, uint seed)
{
	return lerp(min, max, Get1dNoiseZeroToOne(index, seed));
}

//-----------------------------------------------------------------------------------------------
static const float Get2dNoiseZeroToOne(int indexX, int indexY, uint seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get2dNoiseUint(indexX, indexY, seed));
}

static const float GetFloatNoiseInRange(float min, float max, int index, uint seed)
{
	float normalizedTime = Get1dNoiseZeroToOne(index, seed);
	return lerp(min, max, normalizedTime);
}

static const float2 GetFloat2NoiseInRange(float2 min, float2 max, int index, uint seed, bool seperateAxis = true)
{
	if (seperateAxis)
	{
		float normalizedX = Get1dNoiseZeroToOne(index, seed);
		float normalizedY = Get1dNoiseZeroToOne(index, seed + 1);
		float2 output;
		output.x = lerp(min.x, max.x, normalizedX);
		output.y = lerp(min.y, max.y, normalizedY);

		return output;
	}
	else
	{
		float normalizedTime = Get1dNoiseZeroToOne(index, seed);
		return lerp(min, max, normalizedTime);
	}

}

static const float3 GetFloat3NoiseInRange(float3 min, float3 max, int index, uint seed, bool seperateAxis = true)
{
	if (seperateAxis)
	{
		float normalizedX = Get1dNoiseZeroToOne(index, seed);
		float normalizedY = Get1dNoiseZeroToOne(index, seed + 1);
		float normalizedZ = Get1dNoiseZeroToOne(index, seed + 2);
		float3 output;
		output.x = lerp(min.x, max.x, normalizedX);
		output.y = lerp(min.y, max.y, normalizedY);
		output.z = lerp(min.z, max.z, normalizedZ);

		return output;
	}
	else
	{
		float normalizedTime = Get1dNoiseZeroToOne(index, seed);
		return lerp(min, max, normalizedTime);
	}
}

//-----------------------------------------------------------------------------------------------
static const float Get3dNoiseZeroToOne(int indexX, int indexY, int indexZ, uint seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get3dNoiseUint(indexX, indexY, indexZ, seed));
}

//-----------------------------------------------------------------------------------------------
static const float Get4dNoiseZeroToOne(int indexX, int indexY, int indexZ, int indexT, uint seed)
{
	const double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
	return (float)(ONE_OVER_MAX_UINT * (double)Get4dNoiseUint(indexX, indexY, indexZ, indexT, seed));
}


//-----------------------------------------------------------------------------------------------
static const float Get1dNoiseNegOneToOne(int index, uint seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get1dNoiseUint(index, seed));
}


//-----------------------------------------------------------------------------------------------
static const float Get2dNoiseNegOneToOne(int indexX, int indexY, uint seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get2dNoiseUint(indexX, indexY, seed));
}


//-----------------------------------------------------------------------------------------------
static const float Get3dNoiseNegOneToOne(int indexX, int indexY, int indexZ, uint seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get3dNoiseUint(indexX, indexY, indexZ, seed));
}


//-----------------------------------------------------------------------------------------------
static const float Get4dNoiseNegOneToOne(int indexX, int indexY, int indexZ, int indexT, uint seed)
{
	const double ONE_OVER_MAX_INT = (1.0 / (double)0x7FFFFFFF);
	return (float)(ONE_OVER_MAX_INT * (double)(int)Get4dNoiseUint(indexX, indexY, indexZ, indexT, seed));
}

float Compute3dPerlinNoise(float posX, float posY, float posZ, float scale, uint numOctaves, float octavePersistence, float octaveScale, bool renormalize, uint seed)
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	static const float3 gradients[8] = // Traditional "12 edges" requires modulus and isn't any better.
	{
		float3(+fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3), // Normalized unit 3D vectors
		float3(-fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  pointing toward cube
		float3(+fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  corners, so components
		float3(-fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3), //  are all sqrt(3)/3, i.e.
		float3(+fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // 0.5773502691896257645091f.
		float3(-fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // These are slightly better
		float3(+fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3), // than axes (1,0,0) and much
		float3(-fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3)  // faster than edges (1,1,0).
	};

	float totalNoise = 0.f;
	float totalAmplitude = 0.f;
	float currentAmplitude = 1.f;
	float invScale = (1.f / scale);
	float3 currentPos = float3(posX * invScale, posY * invScale, posZ * invScale);

	for (uint octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
	{
		// Determine random unit "gradient vectors" for surrounding corners
		float3 cellMins = float3((float)floor(currentPos.x), (float)floor(currentPos.y), (float)floor(currentPos.z));
		float3 cellMaxs = float3(cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f);
		int indexWestX = (int)cellMins.x;
		int indexSouthY = (int)cellMins.y;
		int indexBelowZ = (int)cellMins.z;
		int indexEastX = indexWestX + 1;
		int indexNorthY = indexSouthY + 1;
		int indexAboveZ = indexBelowZ + 1;

		uint noiseBelowSW = Get3dNoiseUint(indexWestX, indexSouthY, indexBelowZ, seed);
		uint noiseBelowSE = Get3dNoiseUint(indexEastX, indexSouthY, indexBelowZ, seed);
		uint noiseBelowNW = Get3dNoiseUint(indexWestX, indexNorthY, indexBelowZ, seed);
		uint noiseBelowNE = Get3dNoiseUint(indexEastX, indexNorthY, indexBelowZ, seed);
		uint noiseAboveSW = Get3dNoiseUint(indexWestX, indexSouthY, indexAboveZ, seed);
		uint noiseAboveSE = Get3dNoiseUint(indexEastX, indexSouthY, indexAboveZ, seed);
		uint noiseAboveNW = Get3dNoiseUint(indexWestX, indexNorthY, indexAboveZ, seed);
		uint noiseAboveNE = Get3dNoiseUint(indexEastX, indexNorthY, indexAboveZ, seed);

		float3 gradientBelowSW = gradients[noiseBelowSW & 0x00000007];
		float3 gradientBelowSE = gradients[noiseBelowSE & 0x00000007];
		float3 gradientBelowNW = gradients[noiseBelowNW & 0x00000007];
		float3 gradientBelowNE = gradients[noiseBelowNE & 0x00000007];
		float3 gradientAboveSW = gradients[noiseAboveSW & 0x00000007];
		float3 gradientAboveSE = gradients[noiseAboveSE & 0x00000007];
		float3 gradientAboveNW = gradients[noiseAboveNW & 0x00000007];
		float3 gradientAboveNE = gradients[noiseAboveNE & 0x00000007];

		// Dot each corner's gradient with displacement from corner to position
		float3 displacementFromBelowSW = float3(currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z);
		float3 displacementFromBelowSE = float3(currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z);
		float3 displacementFromBelowNW = float3(currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z);
		float3 displacementFromBelowNE = float3(currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z);
		float3 displacementFromAboveSW = float3(currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z);
		float3 displacementFromAboveSE = float3(currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z);
		float3 displacementFromAboveNW = float3(currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z);
		float3 displacementFromAboveNE = float3(currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z);

		float dotBelowSW = dot(gradientBelowSW, displacementFromBelowSW);
		float dotBelowSE = dot(gradientBelowSE, displacementFromBelowSE);
		float dotBelowNW = dot(gradientBelowNW, displacementFromBelowNW);
		float dotBelowNE = dot(gradientBelowNE, displacementFromBelowNE);
		float dotAboveSW = dot(gradientAboveSW, displacementFromAboveSW);
		float dotAboveSE = dot(gradientAboveSE, displacementFromAboveSE);
		float dotAboveNW = dot(gradientAboveNW, displacementFromAboveNW);
		float dotAboveNE = dot(gradientAboveNE, displacementFromAboveNE);

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast = SmoothStep3(displacementFromBelowSW.x);
		float weightNorth = SmoothStep3(displacementFromBelowSW.y);
		float weightAbove = SmoothStep3(displacementFromBelowSW.z);
		float weightWest = 1.f - weightEast;
		float weightSouth = 1.f - weightNorth;
		float weightBelow = 1.f - weightAbove;

		// 8-way blend (8 -> 4 -> 2 -> 1)
		float blendBelowSouth = (weightEast * dotBelowSE) + (weightWest * dotBelowSW);
		float blendBelowNorth = (weightEast * dotBelowNE) + (weightWest * dotBelowNW);
		float blendAboveSouth = (weightEast * dotAboveSE) + (weightWest * dotAboveSW);
		float blendAboveNorth = (weightEast * dotAboveNE) + (weightWest * dotAboveNW);
		float blendBelow = (weightSouth * blendBelowSouth) + (weightNorth * blendBelowNorth);
		float blendAbove = (weightSouth * blendAboveSouth) + (weightNorth * blendAboveNorth);
		float blendTotal = (weightBelow * blendBelow) + (weightAbove * blendAbove);
		float noiseThisOctave = blendTotal * (1.f / 0.793856621f); // 3D Perlin is in [-.793856621,.793856621]; map to ~[-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise += noiseThisOctave * currentAmplitude;
		totalAmplitude += currentAmplitude;
		currentAmplitude *= octavePersistence;
		currentPos *= octaveScale;
		currentPos.x += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z += OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if (renormalize && totalAmplitude > 0.f)
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3(totalNoise);		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}