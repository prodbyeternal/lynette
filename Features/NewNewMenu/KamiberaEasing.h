#pragma once
#include <cmath>

namespace Easing
{
	constexpr double KM_PI = 3.14159265358979323846;

	__forceinline double OutCubic(double t)
	{
		return 1 + (--t) * t * t;
	}

	__forceinline double OutQuad(const double t)
	{
		return t * (2 - t);
	}

	__forceinline double InOutCubic(double t)
	{
		return t < 0.5 ? 4 * t * t * t : 1 + (--t) * (2 * (--t)) * (2 * t);
	}

	__forceinline double OutBack(double t)
	{
		return 1 + (--t) * t * (2.70158 * t + 1.70158);
	}

	__forceinline double OutElastic(const double t)
	{
		const double t2 = (t - 1) * (t - 1);
		return 1 - t2 * t2 * cos(t * KM_PI * 4.5);
	}

	__forceinline double OutBounce(const double t)
	{
		return 1 - pow(2, -6 * t) * abs(cos(t * KM_PI * 3.5));
	}
}
