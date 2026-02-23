
static const float PI = 3.14159265f;

float StepCompare(float a, float b)
{
	return step(a, b) * step(b, a);
}

float StepCompare(int a, int b)
{
	return StepCompare((float)a, (float)b);
}

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
	float s = 1.f - t;
	return 1.f - (s * s);
}

float SmoothStop3(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s);
}

float SmoothStop4(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s * s);
}

float SmoothStop5(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s * s * s);
}

float SmoothStop6(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s * s * s * s);
}

float SmoothStep3(float t)
{
	return lerp(SmoothStart2(t), SmoothStop2(t), t);
}

float ComputeCubicBezier1D(float a, float b, float c, float d, float t)
{
	float AB = lerp(a, b, t);
	float BC = lerp(b, c, t);
	float CD = lerp(c, d, t);

	float ABC = lerp(AB, BC, t);
	float BCD = lerp(BC, CD, t);
	float ABCD = lerp(ABC, BCD, t);
	return ABCD;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float AB = lerp(A, B, t);
	float BC = lerp(B, C, t);
	float CD = lerp(C, D, t);
	float DE = lerp(D, E, t);
	float EF = lerp(E, F, t);

	float ABC = lerp(AB, BC, t);
	float BCD = lerp(BC, CD, t);
	float CDE = lerp(CD, DE, t);
	float DEF = lerp(DE, EF, t);

	float ABCD = lerp(ABC, BCD, t);
	float BCDE = lerp(BCD, CDE, t);
	float CDEF = lerp(CDE, DEF, t);

	float ABCDE = lerp(ABCD, BCDE, t);
	float BCDEF = lerp(BCDE, CDEF, t);

	float ABCDEF = lerp(ABCDE, BCDEF, t);
	return ABCDEF;
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

float GetValueFromEasingFunciton(int easingFunciton, float t)
{
	float output = t;
	if (easingFunciton == 0)
	{
		output = t;
	}
	else if (easingFunciton == 1)
	{
		output = SmoothStart2(t);
	}
	else if (easingFunciton == 2)
	{
		output = SmoothStart3(t);
	}
	else if (easingFunciton == 3)
	{
		output = SmoothStart4(t);
	}
	else if (easingFunciton == 4)
	{
		output = SmoothStart5(t);
	}
	else if (easingFunciton == 5)
	{
		output = SmoothStart6(t);
	}
	else if (easingFunciton == 6)
	{
		output = SmoothStop2(t);
	}
	else if (easingFunciton == 7)
	{
		output = SmoothStop3(t);
	}
	else if (easingFunciton == 8)
	{
		output = SmoothStop4(t);
	}
	else if (easingFunciton == 9)
	{
		output = SmoothStop5(t);
	}
	else if (easingFunciton == 10)
	{
		output = SmoothStop6(t);
	}
	else if (easingFunciton == 11)
	{
		output = SmoothStep3(t);
	}
	else if (easingFunciton == 12)
	{
		output = SmoothStep5(t);
	}
	else if (easingFunciton == 13)
	{
		output = Hesitate3(t);
	}
	else if (easingFunciton == 14)
	{
		output = Hesitate5(t);
	}
	else if (easingFunciton == 15)
	{
		output = .5f * t + .5f * SmoothStart2(t);
	}
	else if (easingFunciton == 16)
	{
		output = .5f * t + .5f * SmoothStop2(t);
	}

	return output;
}
