#pragma once
#include <vector>
#include <string>
#include "KamiberaAnimations.h"
#include "KamiberaMenu.h"
#include "babagui/imgui.h"
#include "babagui/imgui_internal.h"

struct tab_t {
	std::string name;
	std::vector<std::string> sub_tabs;
};

inline int tab_number = 0;
inline int subtab_number = 0;

inline void custom_tabs(const std::vector<tab_t>& tabs)
{
	float y_offset_adjustment = 0.f;

	const float spacing[2]{ 13.0f, 60.0f };
	const float size[2]{ 175.0f, 30.0f };
	const float text_spacing{ 15.0f };
	const float rounding{ 2.0f };

	ImGuiStorage* storage = ImGui::GetStateStorage();
	float dt = ImGui::GetIO().DeltaTime;
	float smoothing_factor = dt * 8.0f;

	for (size_t i = 0; i < tabs.size(); ++i) {
		auto windowPos = ImGui::GetWindowPos();

		float x_offset = windowPos.x + spacing[0] + text_spacing;
		float y_offset = windowPos.y + spacing[1] + (i * size[1]) + 7 + y_offset_adjustment;
		auto text_position = ImVec2(x_offset, y_offset);

		auto tab_min = ImVec2(windowPos.x + spacing[0], windowPos.y + spacing[1] + i * size[1] + y_offset_adjustment);
		auto tab_max = ImVec2(windowPos.x + size[0], windowPos.y + spacing[1] + (i + 1) * size[1] + y_offset_adjustment);

		bool hovered = ImGui::IsMouseHoveringRect(tab_min, tab_max);
		bool selected = (i == tab_number);

		ImGuiID tabAnimId = ImGui::GetID("##tabs") + (ImGuiID)i;
		float tabHoverA = storage->GetFloat(tabAnimId, 0.0f);

		float hover_target = hovered ? 1.0f : 0.0f;
		tabHoverA = smoothLerp(tabHoverA, hover_target, dt * 8.0f);
		storage->SetFloat(tabAnimId, tabHoverA);

		if (selected) {
			auto active_tab_max = ImVec2(windowPos.x + spacing[0] + 175.0f, windowPos.y + spacing[1] + (i + 1) * size[1] + y_offset_adjustment);

			ImColor base_active_color = ImColor(23, 23, 23, 255);
			ImColor hover_active_color = ImColor(28, 28, 28, 255);
			ImColor final_active_color = smoothLerpColor(base_active_color, hover_active_color, tabHoverA);

			ImGui::GetWindowDrawList()->AddRectFilled(tab_min, active_tab_max, final_active_color, rounding, ImDrawFlags_RoundCornersAll);
		}
		else if (hovered) {
			ImColor hover_color = ImColor(18, 18, 18, int(255 * tabHoverA));
			ImGui::GetWindowDrawList()->AddRectFilled(tab_min, tab_max, hover_color, rounding, ImDrawFlags_RoundCornersAll);
		}

		ImColor base_text_color = ImColor(160, 160, 160, 255);
		ImColor selected_text_color = ImColor(245, 245, 245, 255);
		ImColor hover_text_color = ImColor(200, 200, 200, 255);

		ImColor final_text_color = smoothLerpColor(
			smoothLerpColor(base_text_color, hover_text_color, tabHoverA),
			selected_text_color,
			selected ? 1.0f : 0.0f
		);

		ImFont* tab_font = menuFontBold ? menuFontBold : menuFont;
		if (tab_font)
			ImGui::GetWindowDrawList()->AddText(tab_font, tab_font->FontSize, text_position, final_text_color, tabs[i].name.c_str());

		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			tab_number = static_cast<int>(i);
			subtab_number = 0;
		}

		if (!tabs[i].sub_tabs.empty() && (selected || storage->GetFloat(tabAnimId + 100, 0.0f) > 0.001f)) {
			float targetAnim = selected ? 1.0f : 0.0f;
			float subAnimProgress = storage->GetFloat(tabAnimId + 100, targetAnim);
			subAnimProgress = smoothLerp(subAnimProgress, targetAnim, smoothing_factor);
			storage->SetFloat(tabAnimId + 100, subAnimProgress);
			float groupHeight = tabs[i].sub_tabs.size() * size[1] * subAnimProgress;

			auto group_min = ImVec2(windowPos.x + spacing[0], windowPos.y + spacing[1] + (i + 1) * size[1] + y_offset_adjustment);
			auto group_max = ImVec2(windowPos.x + size[0], windowPos.y + spacing[1] + (i + 1) * size[1] + y_offset_adjustment + groupHeight);

			ImGui::GetWindowDrawList()->AddRectFilled(group_min, group_max, ImColor(10, 10, 10, int(255 * g_kmenu.m_animation_progress)),
				rounding, ImDrawFlags_RoundCornersBottom);

			for (size_t j = 0; j < tabs[i].sub_tabs.size(); ++j) {
				auto sub_tab_name = tabs[i].sub_tabs[j];
				auto sub_tab_min = ImVec2(group_min.x, group_min.y + j * size[1]);
				auto sub_tab_max = ImVec2(group_max.x, group_min.y + (j + 1) * size[1]);
				auto sub_text_position = ImVec2(x_offset, group_min.y + j * size[1] + 5);

				bool sub_hovered = ImGui::IsMouseHoveringRect(sub_tab_min, sub_tab_max);
				bool sub_selected = (subtab_number == static_cast<int>(j));

				if (sub_hovered) {
					ImColor sub_hover_color = ImColor(15, 15, 15, 255);
					ImGui::GetWindowDrawList()->AddRectFilled(sub_tab_min, sub_tab_max, sub_hover_color, 2.0f, ImDrawFlags_RoundCornersAll);
				}

				if (selected) {
					ImColor sub_text_color = sub_selected ? ImColor(GetAccentU32(1.f)) : ImColor(255, 255, 255, 255);
					ImFont* st_font = menuFont ? menuFont : ImGui::GetFont();
					ImGui::GetWindowDrawList()->AddText(st_font, st_font->FontSize, sub_text_position, sub_text_color, sub_tab_name.c_str());
				}

				if (sub_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					subtab_number = static_cast<int>(j);
			}

			y_offset_adjustment += tabs[i].sub_tabs.size() * size[1] * subAnimProgress;
		}
	}
}
