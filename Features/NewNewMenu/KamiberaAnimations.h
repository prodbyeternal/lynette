#pragma once
#include "KamiberaEasing.h"
#include "babagui/imgui.h"

inline float smoothLerp(float a, float b, float t)
{
	float eased_t = static_cast<float>(Easing::OutCubic(t));
	return a + (b - a) * eased_t;
}

inline ImColor smoothLerpColor(const ImColor& a, const ImColor& b, float t)
{
	float eased_t = static_cast<float>(Easing::OutCubic(t));
	float ra = a.Value.x, ga = a.Value.y, ba = a.Value.z, aa = a.Value.w;
	float rb = b.Value.x, gb = b.Value.y, bb_val = b.Value.z, ab = b.Value.w;
	float r = ra + (rb - ra) * eased_t;
	float g = ga + (gb - ga) * eased_t;
	float b_col = ba + (bb_val - ba) * eased_t;
	float a_col = aa + (ab - aa) * eased_t;
	return ImColor(r, g, b_col, a_col);
}
