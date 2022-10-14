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
	return (1.0f - t) * a + b * Clamp01(t);
}

float InvLerp(const float a, const float b, const float v)
{
	auto r = (v - a) / (b - a);
	return Clamp01(r);
}
