#pragma once
#include "babagui/imgui.h"
#include "../../SDK/L4D2/Includes/color.h"
#include "../Vars.h"

struct MenuState {
	bool m_opened = false;
	float m_animation_progress = 0.f;
	float m_animation_speed = 15.f;
};

inline MenuState g_kmenu{};

inline ImFont* menuFont = nullptr;
inline ImFont* menuFontSmall = nullptr;
inline ImFont* menuFontBold = nullptr;

inline ImU32 GetAccentU32(float alpha = 1.f)
{
	return IM_COL32(
		Vars::Menu::AccentColor.r(),
		Vars::Menu::AccentColor.g(),
		Vars::Menu::AccentColor.b(),
		static_cast<int>(Vars::Menu::AccentColor.a() * alpha)
	);
}

inline ImVec4 GetAccentVec4(float alpha = 1.f)
{
	return ImVec4(
		Vars::Menu::AccentColor.r() / 255.f,
		Vars::Menu::AccentColor.g() / 255.f,
		Vars::Menu::AccentColor.b() / 255.f,
		(Vars::Menu::AccentColor.a() / 255.f) * alpha
	);
}

inline ImU32 ColorToU32(const Color& c, float alpha = 1.f)
{
	return IM_COL32(c.r(), c.g(), c.b(), static_cast<int>(c.a() * alpha));
}

inline ImVec4 ColorToVec4(const Color& c, float alpha = 1.f)
{
	return ImVec4(c.r() / 255.f, c.g() / 255.f, c.b() / 255.f, (c.a() / 255.f) * alpha);
}
