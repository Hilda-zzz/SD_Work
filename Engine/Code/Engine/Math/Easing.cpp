#include "Easing.hpp"
#include "MathUtils.hpp"



EasingFunc g_easingFunctions[] = {
	{"SmoothStart2", SmoothStart2},
	{"SmoothStart3", SmoothStart3},
	{"SmoothStart4", SmoothStart4},
	{"SmoothStart5", SmoothStart5},
	{"SmoothStart6", SmoothStart6},
	{"SmoothStop2", SmoothStop2},
	{"SmoothStop3", SmoothStop3},
	{"SmoothStop4", SmoothStop4},
	{"SmoothStop5", SmoothStop5},
	{"SmoothStop6", SmoothStop6},
	{"SmoothStep3", SmoothStep3},
	{"SmoothStep5", SmoothStep5},
	{"Hesitate3", Hesitate3},
	{"Hesitate5", Hesitate5},
	{"CustomFunky", CustomFunkyEasingFunction}
};
const size_t g_numEasingFunctions = sizeof(g_easingFunctions) / sizeof(EasingFunc);

float SmoothStart2(float t)
{
	return t * t;
}

float SmoothStart3(float t)
{
	return t * t * t;
}

float SmoothStart4(float t)
{
	return t * t * t * t;
}

float SmoothStart5(float t)
{
	return t * t * t * t * t;
}

float SmoothStart6(float t)
{
	return t * t * t * t * t * t;
}

float SmoothStop2(float t)
{
	float m = 1.f - t;
	return 1.f - m * m;
}

float SmoothStop3(float t)
{
	float m = 1.f - t;
	return 1.f - m * m * m;
}

float SmoothStop4(float t)
{
	float m = 1.f - t;
	return 1.f - m * m * m * m;
}

float SmoothStop5(float t)
{
	float m = 1.f - t;
	return 1.f - m * m * m * m * m;
}

float SmoothStop6(float t)
{
	float m = 1.f - t;
	return 1.f - m * m * m * m * m * m;
}

float SmoothStep3(float t)
{
	return ComputeCubicBezier1D(0.f, 0.f, 1.f, 1.f, t);
}

float SmoothStep5(float t)
{
	return ComputeQuinticBezier1D(0.f, 0.f, 0.f, 1.f, 1.f, 1.f, t);
}

float Hesitate3(float t)
{
	return ComputeCubicBezier1D(0.f, 1.f, 0.f, 1.f, t);
}

float Hesitate5(float t)
{
	return ComputeQuinticBezier1D(0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t);
}

float CustomFunkyEasingFunction(float t)
{
	return (float)abs(abs(sin(4.1f*t)*cos(6.f*t+1.9f)) * 1.2f - 0.1f);
}
