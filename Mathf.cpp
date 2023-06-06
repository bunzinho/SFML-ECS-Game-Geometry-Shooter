#include "Mathf.h"

float Clamp01(float value)
{
	if (value < 0.0f)
	{
		return 0.0f;
	}

	if (value > 1.0f)
	{
		return 1.0f;
	}

	return value;
}

float Lerp(const float a, const float b, const float t)
{
	return (1.0f - t) * a + t * b;
}

float InvLerp(const float a, const float b, const float v)
{
	auto r = (v - a) / (b - a);
	return Clamp01(r);
}

double Clamp01(double value)
{
	if (value < 0.0)
	{
		return 0.0;
	}

	if (value > 1.0)
	{
		return 1.0;
	}

	return value;
}

double Lerp(const double a, const double b, const double t)
{
	return (1.0 - t) * a + t * b;
}

double InvLerp(const double a, const double b, const double v)
{
	auto r = (v - a) / (b - a);
	return Clamp01(r);
}
