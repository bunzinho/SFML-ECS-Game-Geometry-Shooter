#pragma once
#include <random>

constexpr float pi  = 3.141592f;
constexpr float tau = 6.283185f;

inline float random_float()
{
	static std::uniform_real_distribution<float> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}

float Clamp01(const float v);
float Lerp(const float a, const float b, const float t);
float InvLerp(const float a, const float b, const float v);

double Clamp01(const double v);
double Lerp(const double a, const double b, const double t);
double InvLerp(const double a, const double b, const double v);
