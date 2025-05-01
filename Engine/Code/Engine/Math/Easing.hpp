#pragma once
#include <string>

typedef float(*EasingFunctionPtr) (float);

struct EasingFunc
{
	std::string m_funcName;
	EasingFunctionPtr m_funcPointer;
};

extern EasingFunc g_easingFunctions[];
extern const size_t g_numEasingFunctions;

float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);
float SmoothStart5(float t);
float SmoothStart6(float t);

float SmoothStop2(float t);
float SmoothStop3(float t);
float SmoothStop4(float t);
float SmoothStop5(float t);
float SmoothStop6(float t);

float SmoothStep3(float t);
float SmoothStep5(float t);

float Hesitate3(float t);
float Hesitate5(float t);

float CustomFunkyEasingFunction(float t);