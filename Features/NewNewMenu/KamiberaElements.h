#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "babagui/imgui.h"
#include "babagui/imgui_internal.h"
#include "KamiberaAnimations.h"
#include "KamiberaMenu.h"
#include "../../SDK/L4D2/Includes/color.h"
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

// ImVec2 arithmetic helpers (in case IMGUI_DEFINE_MATH_OPERATORS wasn't active
// when imgui_internal.h was first included in this translation unit)
static inline ImVec2 KmVec2Add(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
static inline ImVec2 KmVec2Sub(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
static inline ImVec2 KmVec2Mul(const ImVec2& a, float s) { return ImVec2(a.x * s, a.y * s); }

// Returns the end pointer of the *visible* portion of a label, i.e. everything
// before a "##" id-suffix. Raw ImDrawList::AddText does not strip "##" the way
// ImGui widgets do, so we stop text there ourselves.
static inline const char* km_label_end(const char* label)
{
	return ImGui::FindRenderedTextEnd(label);
}

// ============================================================================
//  Child boxes
// ============================================================================

inline void km_child(const char* name, const ImVec2& size, const std::function<void()>& func)
{
	const float title_bar_height = 25.0f;
	const float spacing[2]{ 10.0f, 5.0f };
	const float rounding = 4.0f;

	ImGuiID id = ImGui::GetID(name);
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float anim_progress = storage->GetFloat(id, 0.0f);

	float dt = ImGui::GetIO().DeltaTime;
	anim_progress = smoothLerp(anim_progress, 1.0f, dt * 6.0f);
	storage->SetFloat(id, anim_progress);

	ImGui::BeginChild(name, size, 0,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollWithMouse);

	const auto draw_position = ImVec2(ImGui::GetCurrentWindow()->Pos.x, ImGui::GetCurrentWindow()->Pos.y);

	ImColor base_color = ImColor(16, 16, 16, 0);
	ImColor full_color = ImColor(16, 16, 16, int(255 * g_kmenu.m_animation_progress));
	ImColor final_color = smoothLerpColor(base_color, full_color, anim_progress);

	ImGui::GetWindowDrawList()->AddRectFilled(draw_position, draw_position + ImVec2(size.x, title_bar_height),
		final_color, rounding, ImDrawFlags_RoundCornersTop);

	ImGui::GetWindowDrawList()->AddRectFilled(draw_position + ImVec2(0.0f, title_bar_height), draw_position + size,
		final_color, rounding, ImDrawFlags_RoundCornersBottom);

	ImColor glow_base = ImColor(11, 11, 11, 0);
	ImColor glow_full = ImColor(11, 11, 11, int(255 * g_kmenu.m_animation_progress));
	ImColor glow_final = smoothLerpColor(glow_base, glow_full, anim_progress);

	ImGui::GetWindowDrawList()->AddRect(draw_position, draw_position + size,
		glow_final, rounding, ImDrawFlags_RoundCornersAll, 1.0f);

	ImColor text_base = ImColor(150, 150, 150, 0);
	ImColor text_full = ImColor(150, 150, 150, int(255 * g_kmenu.m_animation_progress));
	ImColor text_final = smoothLerpColor(text_base, text_full, anim_progress);

	ImFont* title_font = menuFontSmall ? menuFontSmall : ImGui::GetFont();
	ImGui::GetForegroundDrawList()->AddText(title_font, title_font->FontSize,
		draw_position + ImVec2(10.0f, (title_bar_height / 2) - 5.0f),
		text_final, name);

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + title_bar_height);
	ImGui::SetCursorPos({ ImGui::GetCursorPosX() + spacing[0], ImGui::GetCursorPosY() + spacing[1] - 2.0f });
	ImGui::BeginGroup();
	func();
	ImGui::EndGroup();

	ImGui::EndChild();
}

inline void km_scrollable_child(const char* name, const ImVec2& size, const std::function<void()>& func)
{
	const float title_bar_height = 25.0f;
	const float spacing[2]{ 10.0f, 5.0f };
	const float rounding = 4.0f;

	ImGuiID id = ImGui::GetID(name);
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float anim_progress = storage->GetFloat(id, 0.0f);

	float dt = ImGui::GetIO().DeltaTime;
	anim_progress = smoothLerp(anim_progress, 1.0f, dt * 6.0f);
	storage->SetFloat(id, anim_progress);

	ImGui::BeginChild(name, size, 0,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoMove);

	const auto draw_position = ImVec2(ImGui::GetCurrentWindow()->Pos.x, ImGui::GetCurrentWindow()->Pos.y);

	ImColor base_color = ImColor(16, 16, 16, 0);
	ImColor full_color = ImColor(16, 16, 16, int(255 * g_kmenu.m_animation_progress));
	ImColor final_color = smoothLerpColor(base_color, full_color, anim_progress);

	ImGui::GetWindowDrawList()->AddRectFilled(draw_position, draw_position + ImVec2(size.x, title_bar_height),
		final_color, rounding, ImDrawFlags_RoundCornersTop);
	ImGui::GetWindowDrawList()->AddRectFilled(draw_position + ImVec2(0.0f, title_bar_height), draw_position + size,
		final_color, rounding, ImDrawFlags_RoundCornersBottom);

	ImColor glow_base = ImColor(11, 11, 11, 0);
	ImColor glow_full = ImColor(11, 11, 11, int(255 * g_kmenu.m_animation_progress));
	ImColor glow_final = smoothLerpColor(glow_base, glow_full, anim_progress);
	ImGui::GetWindowDrawList()->AddRect(draw_position, draw_position + size,
		glow_final, rounding, ImDrawFlags_RoundCornersAll, 1.0f);

	ImColor text_base = ImColor(150, 150, 150, 0);
	ImColor text_full = ImColor(150, 150, 150, int(255 * g_kmenu.m_animation_progress));
	ImColor text_final = smoothLerpColor(text_base, text_full, anim_progress);

	ImFont* title_font = menuFontSmall ? menuFontSmall : ImGui::GetFont();
	ImGui::GetForegroundDrawList()->AddText(title_font, title_font->FontSize,
		draw_position + ImVec2(10.0f, (title_bar_height / 2) - 5.0f),
		text_final, name);

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + title_bar_height);
	ImGui::SetCursorPos({ ImGui::GetCursorPosX() + spacing[0], ImGui::GetCursorPosY() + spacing[1] - 2.0f });
	ImGui::BeginGroup();
	func();
	ImGui::EndGroup();

	ImGui::EndChild();
}

// ============================================================================
//  Checkbox
// ============================================================================

inline void km_checkbox(const char* name, bool* variable, bool disabled = false)
{
	if (ImGui::GetCurrentWindow()->SkipItems)
		return;

	ImFont* label_font = menuFont ? menuFont : ImGui::GetFont();
	const float label_font_size = label_font->FontSize;
	const char* name_end = km_label_end(name);
	const ImVec2 name_text_size = label_font->CalcTextSizeA(label_font_size, FLT_MAX, 0.f, name, name_end);
	const float width = 120.0f;
	const float sz = 15.0f;
	const float spacing = 5.0f;
	const float rounding = 4.0f;
	const float fix_shit = 5.0f;

	ImVec2 position = ImGui::GetCurrentWindow()->DC.CursorPos;
	position.x += 261.0f;

	ImRect total_bb(position, ImVec2(position.x + width, position.y + sz));
	ImRect check_bb(total_bb.Min, ImVec2(total_bb.Min.x + sz, total_bb.Min.y + sz));

	ImGui::ItemSize(total_bb, sz + 15.f);

	if (!ImGui::ItemAdd(total_bb, ImGui::GetID(name)))
		return;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, ImGui::GetID(name), &hovered, &held);

	if (pressed && !disabled)
		*variable = !(*variable);

	ImGuiID id = ImGui::GetID(name);
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float animation_progress = storage->GetFloat(id, *variable ? 1.0f : 0.0f);
	float hover_progress = storage->GetFloat(id + 1000, 0.0f);

	float target = *variable ? 1.0f : 0.0f;
	float hover_target = hovered ? 1.0f : 0.0f;

	float dt = ImGui::GetIO().DeltaTime;
	animation_progress = smoothLerp(animation_progress, target, dt * 8.0f);
	hover_progress = smoothLerp(hover_progress, hover_target, dt * 6.0f);

	storage->SetFloat(id, animation_progress);
	storage->SetFloat(id + 1000, hover_progress);

	ImGui::PushClipRect(check_bb.Min, check_bb.Max, false);

	ImColor unchecked_color = ImColor(13, 13, 13, 255);
	ImColor checked_color = ImColor(GetAccentU32(1.f));
	ImColor hover_color = ImColor(20, 20, 20, 255);

	ImColor background_color = smoothLerpColor(
		smoothLerpColor(unchecked_color, hover_color, hover_progress),
		checked_color, animation_progress);

	ImGui::GetWindowDrawList()->AddRectFilled(check_bb.Min, check_bb.Max, background_color, rounding, ImDrawFlags_RoundCornersAll);

	ImColor base_outline = ImColor(40, 40, 40, 255);
	ImColor hover_outline = ImColor(60, 60, 60, 255);
	ImColor accent_outline = ImColor(GetAccentU32(1.f));

	ImColor outline_color = smoothLerpColor(
		smoothLerpColor(base_outline, hover_outline, hover_progress),
		accent_outline, animation_progress);

	ImGui::GetWindowDrawList()->AddRect(check_bb.Min, check_bb.Max, outline_color, rounding, ImDrawFlags_RoundCornersAll, 1.0f);

	if (animation_progress > 0.01f) {
		float pad = ImMax(1.0f, sz / 6.0f);
		ImColor checkmark_color = ImColor(0, 0, 0, (int)(255 * animation_progress));
		ImGui::RenderCheckMark(ImGui::GetWindowDrawList(), ImVec2(check_bb.Min.x + pad, check_bb.Min.y + pad), checkmark_color, sz - pad * 2.0f);
	}

	ImGui::PopClipRect();

	ImColor inactive_text = ImColor(150, 150, 150, 255);
	ImColor active_text = ImColor(255, 255, 255, 255);
	ImColor hover_text = ImColor(200, 200, 200, 255);

	ImColor text_color = smoothLerpColor(
		smoothLerpColor(inactive_text, hover_text, hover_progress),
		active_text, animation_progress);

	ImGui::GetWindowDrawList()->AddText(label_font, label_font_size,
		ImVec2(check_bb.Max.x + spacing - 279.0f, check_bb.Min.y + (sz - name_text_size.y - fix_shit) / 2),
		text_color, name, name_end);
}

// ============================================================================
//  Keybind selector (adapted for int* key codes)
// ============================================================================

static const char* km_key_names[] = {
	"-",   "m1",  "m2",  "cn",  "m3",  "m4",  "m5",  "-",   "bac", "tab", "-",   "-",   "clr", "ret", "-",   "-",
	"shi", "ctl", "men", "pau", "cap", "kan", "-",   "jun", "fin", "kan", "-",   "esc", "con", "nco", "acc", "mad",
	"spa", "pgu", "pgd", "end", "hom", "lef", "up",  "rig", "dow", "sel", "pri", "exe", "pri", "ins", "del", "hel",
	"0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9",   "-",   "-",   "-",   "-",   "-",   "-",
	"-",   "a",   "b",   "c",   "d",   "e",   "f",   "g",   "h",   "i",   "j",   "k",   "l",   "m",   "n",   "o",
	"p",   "q",   "r",   "s",   "t",   "u",   "v",   "w",   "x",   "y",   "z",   "win", "win", "app", "-",   "sle",
	"num", "num", "num", "num", "num", "num", "num", "num", "num", "num", "mul", "add", "sep", "min", "dec", "div",
	"f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",  "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15", "f16",
	"f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "-",   "-",   "-",   "-",   "-",   "-",   "-",   "-",
	"num", "scr", "equ", "mas", "toy", "oya", "oya", "-",   "-",   "-",   "-",   "-",   "-",   "-",   "-",   "-",
	"shi", "shi", "ctr", "ctr", "alt", "alt"
};

inline bool km_keybind(const char* name, int* key)
{
	if (ImGui::GetCurrentWindow()->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;

	char buf_display[64] = "none";
	if (*key != 0 && *key < IM_ARRAYSIZE(km_key_names) && g.ActiveId != ImGui::GetCurrentWindow()->GetID(name))
		strcpy_s(buf_display, km_key_names[*key]);

	const auto buffer_text_size = ImGui::CalcTextSize(buf_display, NULL, true);
	const float width = 40.0f;
	const float height = 20.0f;
	const float spacing = 5.0f;
	const float fix_shit = 5.0f;
	const float rounding = 2.0f;

	ImGui::SameLine(ImGui::GetCurrentWindow()->Size.x - (width + (height - spacing)) - 9.0f);

	const ImRect frame_bb(ImGui::GetCurrentWindow()->DC.CursorPos,
		ImGui::GetCurrentWindow()->DC.CursorPos + ImVec2(width, height));

	ImGui::ItemSize(frame_bb, height);

	if (!ImGui::ItemAdd(frame_bb, ImGui::GetCurrentWindow()->GetID(name)))
		return false;

	bool value_changed = false;
	int k = *key;

	const bool hovered = ImGui::IsItemHovered();

	if (hovered && io.MouseClicked[0]) {
		if (g.ActiveId != ImGui::GetCurrentWindow()->GetID(name)) {
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*key = 0;
		}
		ImGui::SetActiveID(ImGui::GetCurrentWindow()->GetID(name), ImGui::GetCurrentWindow());
		ImGui::FocusWindow(ImGui::GetCurrentWindow());
	}
	else if (io.MouseClicked[0]) {
		if (g.ActiveId == ImGui::GetCurrentWindow()->GetID(name))
			ImGui::ClearActiveID();
	}

	if (g.ActiveId == ImGui::GetCurrentWindow()->GetID(name)) {
		for (auto i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
			if (io.MouseDown[i]) {
				switch (i) {
				case 0: k = 0x01; break;
				case 1: k = 0x02; break;
				case 2: k = 0x04; break;
				case 3: k = 0x05; break;
				case 4: k = 0x06; break;
				}
				value_changed = true;
				ImGui::ClearActiveID();
			}
		}

		if (!value_changed) {
			for (auto i = 0x08; i <= 0xA5; i++) {
				if (io.KeysDown[i]) {
					k = i;
					value_changed = true;
					ImGui::ClearActiveID();
				}
			}
		}

		if (ImGui::IsKeyPressedMap(ImGuiKey_Escape)) {
			*key = 0;
			ImGui::ClearActiveID();
		}
		else {
			*key = k;
		}
	}

	ImGui::GetCurrentWindow()->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max,
		ImColor(16, 16, 16, 255), rounding);
	ImGui::GetCurrentWindow()->DrawList->AddRect(frame_bb.Min, frame_bb.Max,
		ImColor(33, 33, 33, 255), rounding, ImDrawFlags_RoundCornersAll, 1.0f);

	ImVec2 text_pos = frame_bb.Min + (frame_bb.GetSize() - buffer_text_size - ImVec2(0, fix_shit)) * 0.5f;

	ImFont* font = menuFont ? menuFont : ImGui::GetFont();
	ImGui::GetCurrentWindow()->DrawList->AddText(font, font->FontSize, text_pos,
		g.ActiveId == ImGui::GetCurrentWindow()->GetID(name)
		? ImColor(230, 210, 255, 255)
		: *key != 0 ? ImColor(230, 210, 255, 255)
		: ImColor(150, 150, 150, 255),
		buf_display);

	return value_changed;
}

// ============================================================================
//  Slider
// ============================================================================

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
	ImGuiContext& g = *GImGui;
	if (items_count <= 0)
		return FLT_MAX;
	return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2) + 20.0f;
}

inline bool km_draw_slider(const char* name, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format,
	float width = 75.f, float value_box_width = -1.f)
{
	if (ImGui::GetCurrentWindow()->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	char value_buf[64];
	const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

	const float height = 20.0f;
	const float grabber_size = 12.0f;
	const float slide_height = 1.0f;
	const float rounding = 2.0f;
	const auto value_text_size = ImGui::CalcTextSize(value_buf, value_buf_end, true);

	const ImRect frame_bb(ImGui::GetCurrentWindow()->DC.CursorPos + ImVec2(15.0f, -2.0f),
		ImGui::GetCurrentWindow()->DC.CursorPos + ImVec2(15.0f + width, height - 2.0f));

	float box_right_edge_offset = 292.0f;
	ImVec2 child_pos = ImGui::GetCurrentWindow()->Pos;

	float box_width;
	if (value_box_width > 0) {
		box_width = value_box_width;
	}
	else {
		switch (data_type) {
		case ImGuiDataType_Float: box_width = 70.0f; break;
		case ImGuiDataType_S32: box_width = 50.0f; break;
		case ImGuiDataType_Double: box_width = 80.0f; break;
		default: box_width = 60.0f; break;
		}
	}

	ImVec2 box_pos = ImVec2(
		child_pos.x + box_right_edge_offset - box_width,
		ImGui::GetCurrentWindow()->DC.CursorPos.y - (height - slide_height) / 2 - 2.0f + 15.0f - 6.0f - 16.0f + 13.0f
	);

	const float slider_end_x = box_pos.x - 12.0f;
	const float slider_start_x = slider_end_x - width;

	const ImRect total_bb(
		ImVec2(slider_start_x, ImGui::GetCurrentWindow()->DC.CursorPos.y + (height - slide_height) / 2 - 2.0f + 6.0f - 6.0f),
		ImVec2(slider_end_x, ImGui::GetCurrentWindow()->DC.CursorPos.y + (height - slide_height) / 2 + slide_height - 2.0f + 6.0f - 6.0f)
	);

	float progress = 0.0f;
	switch (data_type) {
	case ImGuiDataType_Float:
		progress = (*static_cast<float*>(p_data) - *static_cast<const float*>(p_min)) /
			(*static_cast<const float*>(p_max) - *static_cast<const float*>(p_min));
		break;
	case ImGuiDataType_S32:
		progress = (*static_cast<int*>(p_data) - *static_cast<const int*>(p_min)) /
			(float)(*static_cast<const int*>(p_max) - *static_cast<const int*>(p_min));
		break;
	case ImGuiDataType_Double:
		progress = static_cast<float>((*static_cast<double*>(p_data) - *static_cast<const double*>(p_min)) /
			(*static_cast<const double*>(p_max) - *static_cast<const double*>(p_min)));
		break;
	default:
		progress = (*static_cast<int*>(p_data) - *static_cast<const int*>(p_min)) /
			(float)(*static_cast<const int*>(p_max) - *static_cast<const int*>(p_min));
		break;
	}
	progress = std::clamp(progress, 0.0f, 1.0f);

	ImVec2 grabber_center = ImVec2(
		total_bb.Min.x + progress * total_bb.GetWidth(),
		total_bb.Min.y + total_bb.GetHeight() / 2
	);

	ImRect grabber_bb(
		ImVec2(grabber_center.x - grabber_size, grabber_center.y - grabber_size),
		ImVec2(grabber_center.x + grabber_size, grabber_center.y + grabber_size)
	);

	ImRect interaction_bb = total_bb;
	interaction_bb.Expand(ImVec2(0, grabber_size));

	ImGui::ItemSize(frame_bb, height);

	const ImGuiID slider_id = ImGui::GetID(name);
	if (!ImGui::ItemAdd(interaction_bb, slider_id))
		return false;

	const bool hovered = ImGui::ItemHoverable(interaction_bb, slider_id);
	const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false);
	const bool grabber_hovered = ImGui::IsMouseHoveringRect(grabber_bb.Min, grabber_bb.Max);
	const bool grabber_clicked = grabber_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false);

	ImGuiID id = slider_id;
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float hover_a = storage->GetFloat(id, 0.0f);
	float grab_a = storage->GetFloat(id + 2000, 0.0f);

	float dt = ImGui::GetIO().DeltaTime;
	hover_a = smoothLerp(hover_a, hovered ? 1.0f : 0.0f, dt * 8.0f);
	grab_a = smoothLerp(grab_a, grabber_hovered ? 1.0f : 0.0f, dt * 10.0f);
	storage->SetFloat(id, hover_a);
	storage->SetFloat(id + 2000, grab_a);

	if (clicked || grabber_clicked) {
		ImGui::SetActiveID(slider_id, window);
		ImGui::SetFocusID(slider_id, window);
		ImGui::FocusWindow(window);
		g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
	}

	ImRect grab_bb;
	const bool value_changed = ImGui::SliderBehavior(total_bb, slider_id, data_type, p_data, p_min, p_max, format,
		ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp, &grab_bb);

	if (value_changed)
		ImGui::MarkItemEdited(slider_id);

	ImColor base_track = ImColor(88, 88, 88, 255);
	ImColor hover_track = ImColor(100, 100, 100, 255);
	ImColor final_track = smoothLerpColor(base_track, hover_track, hover_a);
	ImGui::GetWindowDrawList()->AddRectFilled(total_bb.Min, total_bb.Max, final_track, rounding, ImDrawFlags_RoundCornersAll);

	ImVec2 filled_end = ImVec2(total_bb.Min.x + progress * total_bb.GetWidth(), total_bb.Max.y);
	if (progress > 0.0f) {
		ImColor base_fill = ImColor(GetAccentU32(0.78f));
		ImColor hover_fill = ImColor(GetAccentU32(1.f));
		ImColor final_fill = smoothLerpColor(base_fill, hover_fill, hover_a);
		ImGui::GetWindowDrawList()->AddRectFilled(total_bb.Min, filled_end, final_fill, rounding, ImDrawFlags_RoundCornersAll);
	}

	ImFont* font = menuFont ? menuFont : ImGui::GetFont();
	float font_size = font->FontSize;

	ImColor base_text = ImColor(150, 150, 150, 255);
	ImColor hover_text = ImColor(200, 200, 200, 255);
	ImColor final_text = smoothLerpColor(base_text, hover_text, hover_a);
	ImGui::GetWindowDrawList()->AddText(font, font_size, frame_bb.Min + ImVec2(-13.0f, 0.0f), final_text, name);

	ImColor base_grabber = ImColor(GetAccentU32(1.f));
	ImGui::GetWindowDrawList()->AddCircleFilled(grabber_center, grabber_size / 2, base_grabber);

	ImColor base_grab_outline = ImColor(255, 255, 255, 100);
	ImColor hover_grab_outline = ImColor(255, 255, 255, 200);
	ImColor final_grab_outline = smoothLerpColor(base_grab_outline, hover_grab_outline, grab_a);
	ImGui::GetWindowDrawList()->AddCircle(grabber_center, grabber_size / 2, final_grab_outline, 0, 1.0f);

	const float box_height = 20.0f;
	ImRect value_box(box_pos, ImVec2(box_pos.x + box_width, box_pos.y + box_height));
	const std::string value_box_label = std::string(name) + "##_value";
	const ImGuiID value_box_id = ImGui::GetID(value_box_label.c_str());
	bool value_box_hovered = false;
	bool value_box_held = false;
	bool value_box_pressed = false;
	bool value_box_active = ImGui::TempInputIsActive(value_box_id);

	if (!value_box_active) {
		if (ImGui::ItemAdd(value_box, value_box_id))
			value_box_pressed = ImGui::ButtonBehavior(value_box, value_box_id, &value_box_hovered, &value_box_held);
		value_box_active = value_box_pressed;
	}

	ImColor base_box = ImColor(16, 16, 16, 255);
	ImColor hover_box = ImColor(22, 22, 22, 255);
	ImColor final_box = smoothLerpColor(base_box, hover_box, value_box_hovered ? 1.0f : hover_a);
	ImGui::GetWindowDrawList()->AddRectFilled(value_box.Min, value_box.Max, final_box, 2.0f);

	ImColor base_box_outline = ImColor(32, 32, 32, 255);
	ImColor hover_box_outline = ImColor(50, 50, 50, 255);
	ImColor final_box_outline = smoothLerpColor(base_box_outline, hover_box_outline, value_box_hovered ? 1.0f : hover_a);
	float box_outline_thickness = 1.0f + hover_a * 0.5f;
	ImGui::GetWindowDrawList()->AddRect(value_box.Min, value_box.Max, final_box_outline, 2.0f, 0, box_outline_thickness);

	if (value_box_active) {
		if (ImGui::TempInputScalar(value_box, value_box_id, value_box_label.c_str(), data_type, p_data, format, p_min, p_max))
			ImGui::MarkItemEdited(value_box_id);
		return value_changed || ImGui::IsItemEdited();
	}

	ImVec2 text_pos = ImVec2(value_box.Min.x + 8.0f, value_box.Min.y + (box_height - value_text_size.y) / 2);

	ImColor base_value = ImColor(150, 150, 150, 255);
	ImColor hover_value = ImColor(200, 200, 200, 255);
	ImColor final_value = smoothLerpColor(base_value, hover_value, hover_a);
	ImGui::GetWindowDrawList()->AddText(font, font_size, text_pos, final_value, value_buf, value_buf_end);

	return value_changed;
}

inline bool km_slider_float(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f", float width = 75.f)
{
	return km_draw_slider(label, ImGuiDataType_Float, v, &v_min, &v_max, format, width);
}

inline bool km_slider_int(const char* label, int* v, int v_min, int v_max, const char* format = "%d", float width = 75.f)
{
	return km_draw_slider(label, ImGuiDataType_S32, v, &v_min, &v_max, format, width);
}

// ============================================================================
//  Combobox
// ============================================================================

inline bool km_header_box(const char* name, const char* preview_value, int items_count, ImVec2 sz)
{
	ImGuiContext& g = *GImGui;

	if (ImGui::GetCurrentWindow()->SkipItems)
		return false;

	const auto name_text_size = ImGui::CalcTextSize(name, NULL, true);
	const float height = sz.y;
	const float rounding = 4.0f;
	const float thickness = 1.0f;
	const float padding = 5.0f;
	const float spacing = 5.0f;

	const float combo_width = sz.x;
	const float combo_x = ImGui::GetCurrentWindow()->DC.CursorPos.x;
	const float combo_y = ImGui::GetCurrentWindow()->DC.CursorPos.y - 5.0f;

	if (combo_width <= 0 || height <= 0)
		return false;

	const ImRect frame_bb(ImVec2(combo_x, combo_y), ImVec2(combo_x + combo_width, combo_y + height));
	const ImRect total_bb(ImVec2(combo_x, combo_y + name_text_size.y + 5.0f), frame_bb.Max);

	ImGui::ItemSize(frame_bb, height);

	if (!ImGui::ItemAdd(frame_bb, ImGui::GetID(name)))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, ImGui::GetID(name), &hovered, &held);
	bool popup_open = ImGui::IsPopupOpen(ImGui::GetID(name), ImGuiPopupFlags_None);

	ImGui::GetWindowDrawList()->PushClipRect(total_bb.Min, total_bb.Max, false);

	ImColor final_bg = ImColor(16, 16, 16, int(255 * g_kmenu.m_animation_progress));
	ImGui::GetWindowDrawList()->AddRectFilled(total_bb.Min, total_bb.Max, final_bg, rounding, ImDrawFlags_RoundCornersAll);

	ImColor base_outline = ImColor(33, 33, 33, int(255 * g_kmenu.m_animation_progress));
	ImColor hover_outline = ImColor(60, 60, 60, int(255 * g_kmenu.m_animation_progress));
	ImColor accent_outline = ImColor(GetAccentU32(g_kmenu.m_animation_progress));

	ImColor final_outline;
	if (popup_open) {
		final_outline = accent_outline;
	}
	else {
		final_outline = smoothLerpColor(base_outline, hover_outline, hovered ? 1.0f : 0.0f);
	}

	float animated_thickness = thickness + (popup_open ? 0.5f : 0.0f);
	ImGui::GetWindowDrawList()->AddRect(total_bb.Min, total_bb.Max, final_outline, rounding, ImDrawFlags_RoundCornersAll, animated_thickness);

	ImColor base_text = ImColor(150, 150, 150, int(255 * g_kmenu.m_animation_progress));
	ImVec2 text_pos = total_bb.Min + ImVec2(5.0f, 3.0f);

	float available_width = total_bb.GetWidth() - 30.0f;
	ImVec2 text_size = ImGui::CalcTextSize(preview_value);

	ImFont* font = menuFont ? menuFont : ImGui::GetFont();
	if (text_size.x <= available_width) {
		ImGui::GetWindowDrawList()->AddText(font, font->FontSize, text_pos, base_text, preview_value);
	}
	else {
		ImGui::GetWindowDrawList()->AddText(font, font->FontSize, text_pos, base_text, preview_value);
		ImVec2 gradient_start = text_pos + ImVec2(available_width - 20.0f, 0.0f);
		ImVec2 gradient_end = text_pos + ImVec2(available_width, text_size.y);
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(gradient_start, gradient_end,
			ImColor(0, 0, 0, 0), final_bg, final_bg, ImColor(0, 0, 0, 0));
	}

	ImGuiID combo_id = ImGui::GetID(name);
	ImGuiStorage* arrowStorage = ImGui::GetStateStorage();
	float arrowRot = arrowStorage->GetFloat(combo_id + 5000, 0.0f);

	float dt = ImGui::GetIO().DeltaTime;
	float target_rotation = popup_open ? 180.0f : 0.0f;
	arrowRot = smoothLerp(arrowRot, target_rotation, dt * 8.0f);
	arrowStorage->SetFloat(combo_id + 5000, arrowRot);

	ImVec2 arrow_center = ImVec2(total_bb.Max.x - 15.0f, total_bb.Min.y + total_bb.GetHeight() / 2.0f);
	ImColor arrow_color = ImColor(150, 150, 150, int(255 * g_kmenu.m_animation_progress));

	float line_thickness = 1.0f;
	float arrow_size = 5.0f;
	float arrow_angle = 70.0f;
	float half_angle_rad = (arrow_angle * 0.5f) * 3.14159f / 180.0f;
	float left_angle = -half_angle_rad;
	float right_angle = half_angle_rad;

	ImVec2 p1, p2, p3;
	if (arrowRot < 90.0f) {
		p1 = ImVec2(arrow_center.x, arrow_center.y - arrow_size * 0.5f);
		p2 = ImVec2(arrow_center.x + arrow_size * 0.5f * sinf(left_angle), arrow_center.y + arrow_size * 0.5f * cosf(left_angle));
		p3 = ImVec2(arrow_center.x + arrow_size * 0.5f * sinf(right_angle), arrow_center.y + arrow_size * 0.5f * cosf(right_angle));
	}
	else {
		p1 = ImVec2(arrow_center.x, arrow_center.y + arrow_size * 0.5f);
		p2 = ImVec2(arrow_center.x + arrow_size * 0.5f * sinf(left_angle), arrow_center.y - arrow_size * 0.5f * cosf(left_angle));
		p3 = ImVec2(arrow_center.x + arrow_size * 0.5f * sinf(right_angle), arrow_center.y - arrow_size * 0.5f * cosf(right_angle));
	}

	ImGui::GetWindowDrawList()->AddLine(p1, p2, arrow_color, line_thickness);
	ImGui::GetWindowDrawList()->AddLine(p1, p3, arrow_color, line_thickness);

	ImGui::GetWindowDrawList()->PopClipRect();

	if ((pressed || g.NavActivateId == ImGui::GetID(name)) && !popup_open) {
		if (ImGui::GetCurrentWindow()->DC.NavLayerCurrent == 0)
			ImGui::GetCurrentWindow()->NavLastIds[0] = ImGui::GetID(name);
		ImGui::OpenPopupEx(ImGui::GetID(name), ImGuiPopupFlags_None);
		popup_open = true;
	}

	if (!popup_open)
		return false;

	ImGui::SetNextWindowPos(total_bb.Min + ImVec2(0, (height / 2.0f) + spacing + 8.0f));

	float full_popup_height = CalcMaxPopupHeightFromItemCount(items_count);
	const float max_popup_height = 300.f;
	float constrained_height = full_popup_height > max_popup_height ? max_popup_height : full_popup_height;

	ImGui::SetNextWindowSizeConstraints(ImVec2(combo_width, constrained_height), ImVec2(FLT_MAX, max_popup_height));

	ImGuiWindowFlags flags = ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoMove;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));

	bool call_once = ImGui::BeginPopup(name, flags);

	if (call_once) {
		ImGui::PushClipRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), false);
		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(),
			ImColor(13, 13, 13, int(255 * g_kmenu.m_animation_progress)), rounding, ImDrawFlags_RoundCornersAll);
		ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(),
			ImColor(10, 10, 10, int(255 * g_kmenu.m_animation_progress)), rounding, ImDrawFlags_RoundCornersAll, thickness);
		ImGui::PopClipRect();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
	}
	else {
		ImGui::EndPopup();
		return false;
	}

	ImGui::PopStyleVar();
	return true;
}

constexpr float COMBO_TEXT_OFFSET_Y = 3.0f;

inline void km_combo(const char* label, int& selected_index, const char* items[], int items_count, ImVec2 sz = ImVec2(180, 39))
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f, 1.0f));
	ImGui::Text("%s", label);
	ImGui::PopStyleColor();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 9.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 15.0f);

	const float combo_width = ImGui::GetContentRegionAvail().x - 12.0f;

	{
		const ImVec2 frame_min = ImGui::GetCurrentWindow()->DC.CursorPos + ImVec2(0.0f, -5.0f);
		const ImVec2 frame_max = frame_min + ImVec2(combo_width, sz.y);
		const bool popup_already_open = ImGui::IsPopupOpen(ImGui::GetID(label), ImGuiPopupFlags_None);
		if (!popup_already_open && items_count > 0 && ImGui::IsMouseHoveringRect(frame_min, frame_max)) {
			const float wheel = ImGui::GetIO().MouseWheel;
			if (wheel != 0.0f) {
				const int delta = (wheel > 0.0f) ? -1 : 1;
				selected_index = (selected_index + delta + items_count) % items_count;
				ImGui::GetIO().MouseWheel = 0.0f;
			}
		}
	}

	if (km_header_box(label, items[selected_index], items_count, ImVec2(combo_width, sz.y))) {
		for (int i = 0; i < items_count; i++) {
			ImGui::SetCursorPosX(25.0f);

			ImVec2 item_start_pos = ImGui::GetCursorScreenPos();
			ImVec2 item_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y);

			bool item_hovered = ImGui::IsMouseHoveringRect(item_start_pos, item_start_pos + item_size);

			ImGuiID itemAnimId = ImGui::GetID(items[i]) + 6000;
			ImGuiStorage* iStorage = ImGui::GetStateStorage();
			float itemHoverA = iStorage->GetFloat(itemAnimId, 0.0f);

			float dt = ImGui::GetIO().DeltaTime;
			itemHoverA = smoothLerp(itemHoverA, item_hovered ? 1.0f : 0.0f, dt * 10.0f);
			iStorage->SetFloat(itemAnimId, itemHoverA);

			if (itemHoverA > 0.0f) {
				ImVec2 hover_start = item_start_pos + ImVec2(-15.0f, 0.0f);
				ImVec2 hover_end = item_start_pos + item_size - ImVec2(5.0f, 0.0f);
				ImColor hover_color = ImColor(21, 21, 21, int(255 * itemHoverA * g_kmenu.m_animation_progress));
				ImGui::GetWindowDrawList()->AddRectFilled(hover_start, hover_end, hover_color, 4.0f, ImDrawFlags_RoundCornersAll);
			}

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + COMBO_TEXT_OFFSET_Y);

			ImGui::PushStyleColor(ImGuiCol_Text,
				(selected_index == i)
				? ImVec4(255, 255, 255, g_kmenu.m_animation_progress)
				: ImVec4(150, 150, 150, g_kmenu.m_animation_progress));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));

			bool selected = (selected_index == i);
			if (ImGui::Selectable(items[i], selected))
				selected_index = i;

			if (selected) {
				ImVec2 dot_pos = item_start_pos + ImVec2(-8.0f, item_size.y / 2.0f - 2.0f);
				ImGui::GetWindowDrawList()->AddCircleFilled(dot_pos + ImVec2(2.0f, 2.0f), 3.0f, ImColor(GetAccentU32(g_kmenu.m_animation_progress)));
			}

			ImGui::PopStyleColor(4);
		}
		ImGui::EndPopup();
	}
}

inline void km_multi_combo(const char* label, bool combos[], const char* items[], int items_count, ImVec2 sz = ImVec2(180, 39))
{
	std::vector<std::string> selected;
	for (int i = 0; i < items_count; i++)
		if (combos[i])
			selected.push_back(items[i]);

	std::string preview = selected.empty() ? "none" : selected[0];
	for (size_t i = 1; i < selected.size(); i++)
		preview += ", " + selected[i];

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f, 1.0f));
	ImGui::Text("%s", label);
	ImGui::PopStyleColor();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 9.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 15.0f);

	const float combo_width = ImGui::GetContentRegionAvail().x - 12.0f;

	if (km_header_box(label, preview.c_str(), items_count, ImVec2(combo_width, sz.y))) {
		for (int i = 0; i < items_count; i++) {
			ImGui::SetCursorPosX(25.0f);
			ImVec2 item_start_pos = ImGui::GetCursorScreenPos();
			ImVec2 item_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y);

			bool item_hovered = ImGui::IsMouseHoveringRect(item_start_pos, item_start_pos + item_size);

			ImGuiID itemAnimId = ImGui::GetID(items[i]) + 6000;
			ImGuiStorage* iStorage = ImGui::GetStateStorage();
			float itemHoverA = iStorage->GetFloat(itemAnimId, 0.0f);

			float dt = ImGui::GetIO().DeltaTime;
			itemHoverA = smoothLerp(itemHoverA, item_hovered ? 1.0f : 0.0f, dt * 10.0f);
			iStorage->SetFloat(itemAnimId, itemHoverA);

			if (itemHoverA > 0.0f) {
				ImVec2 hover_start = item_start_pos + ImVec2(-15.0f, 0.0f);
				ImVec2 hover_end = item_start_pos + item_size - ImVec2(5.0f, 0.0f);
				ImColor hover_color = ImColor(21, 21, 21, int(255 * itemHoverA * g_kmenu.m_animation_progress));
				ImGui::GetWindowDrawList()->AddRectFilled(hover_start, hover_end, hover_color, 4.0f, ImDrawFlags_RoundCornersAll);
			}

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + COMBO_TEXT_OFFSET_Y);

			ImGui::PushStyleColor(ImGuiCol_Text,
				combos[i] ? ImVec4(255, 255, 255, g_kmenu.m_animation_progress) : ImVec4(150, 150, 150, g_kmenu.m_animation_progress));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));

			bool temp = combos[i];
			if (ImGui::Selectable(items[i], &temp, ImGuiSelectableFlags_DontClosePopups))
				combos[i] = temp;

			if (combos[i]) {
				ImVec2 dot_pos = item_start_pos + ImVec2(-8.0f, item_size.y / 2.0f - 2.0f);
				ImGui::GetWindowDrawList()->AddCircleFilled(dot_pos + ImVec2(2.0f, 2.0f), 3.0f, ImColor(GetAccentU32(g_kmenu.m_animation_progress)));
			}

			ImGui::PopStyleColor(4);
		}
		ImGui::EndPopup();
	}
}

// ============================================================================
//  Button
// ============================================================================

inline bool km_button(const char* name, ImVec2 pos = ImVec2(280, 22))
{
	if (ImGui::GetCurrentWindow()->SkipItems)
		return false;

	ImFont* font = menuFont ? menuFont : ImGui::GetFont();
	float font_size = font->FontSize;
	const auto name_text_size = font->CalcTextSizeA(font_size, FLT_MAX, NULL, name);

	const float width = pos.x;
	const float height = pos.y;
	const float rounding = 4.0f;

	const ImVec2 position = ImGui::GetCurrentWindow()->DC.CursorPos;
	ImRect total_bb(position, position + ImVec2(width, height));

	ImGui::ItemSize(total_bb, height);
	if (!ImGui::ItemAdd(total_bb, ImGui::GetID(name)))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, ImGui::GetID(name), &hovered, &held);

	float dt = ImGui::GetIO().DeltaTime;
	ImGuiID id = ImGui::GetID(name);
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float pressA = storage->GetFloat(id + 3000, 0.0f);
	float hoverA = storage->GetFloat(id + 3001, 0.0f);

	pressA = smoothLerp(pressA, held ? 1.0f : 0.0f, dt * 12.0f);
	hoverA = smoothLerp(hoverA, hovered ? 1.0f : 0.0f, dt * 8.0f);
	storage->SetFloat(id + 3000, pressA);
	storage->SetFloat(id + 3001, hoverA);

	ImColor baseColor(16, 16, 16, int(255 * g_kmenu.m_animation_progress));
	ImColor accentColor(GetAccentU32(g_kmenu.m_animation_progress));
	ImColor hoverColor(25, 25, 25, int(255 * g_kmenu.m_animation_progress));

	ImColor finalColor = smoothLerpColor(
		smoothLerpColor(baseColor, hoverColor, hoverA),
		accentColor, pressA);

	ImGui::GetWindowDrawList()->AddRectFilled(total_bb.Min, total_bb.Max, finalColor, rounding, ImDrawFlags_RoundCornersAll);

	ImColor baseOutline(33, 33, 33, int(255 * g_kmenu.m_animation_progress));
	ImColor accentOutline(GetAccentU32(g_kmenu.m_animation_progress));
	ImColor hoverOutline(50, 50, 50, int(255 * g_kmenu.m_animation_progress));

	ImColor finalOutline = smoothLerpColor(
		smoothLerpColor(baseOutline, hoverOutline, hoverA),
		accentOutline, pressA);

	float outlineThickness = 1.0f + hoverA * 0.5f;
	ImGui::GetWindowDrawList()->AddRect(total_bb.Min, total_bb.Max, finalOutline, rounding, ImDrawFlags_RoundCornersAll, outlineThickness);

	const float text_x = total_bb.Min.x + (width - name_text_size.x) * 0.5f;
	const float text_y = total_bb.Min.y + (height - name_text_size.y) * 0.5f - 1.0f;

	ImColor textColor(255, 255, 255, int(255 * g_kmenu.m_animation_progress));
	ImGui::GetWindowDrawList()->AddText(font, font_size, ImVec2(text_x, text_y), textColor, name);

	return pressed;
}

// ============================================================================
//  Color picker button (13x12 swatch)
// ============================================================================

inline bool km_color_picker_button(const char* name, Color* color, const ImVec2& size = ImVec2(13.0f, 12.0f))
{
	if (ImGui::GetCurrentWindow()->SkipItems)
		return false;

	const float width = size.x;
	const float height = size.y;
	const float rounding = 4.0f;

	const ImVec2 position = ImGui::GetCurrentWindow()->DC.CursorPos;
	ImRect total_bb(position, position + ImVec2(width, height));

	ImGui::ItemSize(total_bb, height);
	if (!ImGui::ItemAdd(total_bb, ImGui::GetID(name)))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, ImGui::GetID(name), &hovered, &held);

	float dt = ImGui::GetIO().DeltaTime;
	ImGuiID id = ImGui::GetID(name);
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float cpHoverA = storage->GetFloat(id + 4000, 0.0f);
	cpHoverA = smoothLerp(cpHoverA, hovered ? 1.0f : 0.0f, dt * 8.0f);
	storage->SetFloat(id + 4000, cpHoverA);

	ImColor bgColor = smoothLerpColor(ImColor(16, 16, 16, 255), ImColor(25, 25, 25, 255), cpHoverA);
	ImGui::GetWindowDrawList()->AddRectFilled(total_bb.Min, total_bb.Max, bgColor, rounding, ImDrawFlags_RoundCornersAll);

	ImColor outlineColor = smoothLerpColor(ImColor(33, 33, 33, 255), ImColor(50, 50, 50, 255), cpHoverA);
	ImGui::GetWindowDrawList()->AddRect(total_bb.Min, total_bb.Max, outlineColor, rounding, ImDrawFlags_RoundCornersAll, 1.0f);

	ImRect color_bb(total_bb.Min + ImVec2(1.0f, 1.0f), total_bb.Max - ImVec2(1.0f, 1.0f));

	ImGui::GetWindowDrawList()->AddRectFilled(color_bb.Min, color_bb.Max, IM_COL32(204, 204, 204, 255), rounding - 1.0f, ImDrawFlags_RoundCornersAll);

	const float grid_step = 4.0f;
	int yi = 0;
	for (float y = color_bb.Min.y; y < color_bb.Max.y; y += grid_step, yi++) {
		for (float x = color_bb.Min.x + (yi & 1) * grid_step; x < color_bb.Max.x; x += grid_step * 2.0f) {
			ImVec2 p1(x, y);
			ImVec2 p2(ImMin(x + grid_step, color_bb.Max.x), ImMin(y + grid_step, color_bb.Max.y));
			ImGui::GetWindowDrawList()->AddRectFilled(p1, p2, IM_COL32(128, 128, 128, 255), rounding - 1.0f, ImDrawFlags_RoundCornersAll);
		}
	}

	ImGui::GetWindowDrawList()->AddRectFilled(color_bb.Min, color_bb.Max, ColorToU32(*color), rounding - 1.0f, ImDrawFlags_RoundCornersAll);

	return pressed;
}

// ============================================================================
//  Custom color picker popup window
// ============================================================================

inline bool km_color_picker_window(const char* name, Color* color, const ImVec2& window_size = ImVec2(205.0f, 220.0f))
{
	ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

	if (!ImGui::BeginPopup(name, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
		return false;

	bool color_changed = false;
	const auto draw_list = ImGui::GetWindowDrawList();
	const ImVec2 position = ImGui::GetWindowPos();
	const ImVec2 sz = ImGui::GetWindowSize();

	draw_list->AddRectFilled(position, position + sz, ImColor(13, 13, 13, 255), 8.0f, ImDrawFlags_RoundCornersAll);
	draw_list->AddRect(position, position + sz, ImColor(33, 33, 33, 255), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);

	ImVec4 color_vec4 = ColorToVec4(*color);

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiStyle backup_style = style;

	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.FrameRounding = 4.0f;
	style.GrabRounding = 4.0f;

	ImGui::SetCursorPos(ImVec2(10.0f, 10.0f));
	if (ImGui::ColorPicker4((std::string("##km_picker_") + name).c_str(), &color_vec4.x,
		ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview)) {
		*color = Color(
			static_cast<int>(color_vec4.x * 255.f),
			static_cast<int>(color_vec4.y * 255.f),
			static_cast<int>(color_vec4.z * 255.f),
			static_cast<int>(color_vec4.w * 255.f));
		color_changed = true;
	}

	style = backup_style;

	const ImVec2 mini_button_size = ImVec2(25.0f, 20.0f);
	const float mini_button_spacing = 5.0f;
	const ImVec2 preset_start = ImVec2(10.0f, 95.0f);

	const Color preset_colors[] = {
		Color(255, 255, 255, 255), Color(0, 0, 0, 255),
		Color(255, 0, 0, 255), Color(0, 255, 0, 255),
		Color(0, 0, 255, 255), Color(255, 255, 0, 255),
		Color(255, 0, 255, 255), Color(0, 255, 255, 255)
	};

	for (int i = 0; i < 8; i++) {
		ImVec2 button_pos = ImVec2(
			preset_start.x + (i % 4) * (mini_button_size.x + mini_button_spacing),
			preset_start.y + (i / 4) * (mini_button_size.y + mini_button_spacing));

		ImRect button_bb(button_pos, button_pos + mini_button_size);
		bool btn_hovered = ImGui::IsMouseHoveringRect(button_bb.Min, button_bb.Max);
		bool btn_clicked = btn_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

		ImColor btn_color = btn_hovered ? ImColor(25, 25, 25, 255) : ImColor(16, 16, 16, 255);
		ImColor btn_outline = btn_hovered ? ImColor(50, 50, 50, 255) : ImColor(33, 33, 33, 255);

		draw_list->AddRectFilled(button_bb.Min, button_bb.Max, btn_color, 4.0f, ImDrawFlags_RoundCornersAll);
		draw_list->AddRect(button_bb.Min, button_bb.Max, btn_outline, 4.0f, ImDrawFlags_RoundCornersAll, 1.0f);

		ImRect clr_bb(button_bb.Min + ImVec2(1.0f, 1.0f), button_bb.Max - ImVec2(1.0f, 1.0f));
		draw_list->AddRectFilled(clr_bb.Min, clr_bb.Max, ColorToU32(preset_colors[i]), 3.0f, ImDrawFlags_RoundCornersAll);

		if (btn_clicked) {
			*color = preset_colors[i];
			color_changed = true;
		}
	}

	ImGui::EndPopup();
	return color_changed;
}

// ============================================================================
//  Text with color picker (label + inline swatch that opens full picker)
// ============================================================================

inline void km_text_with_color_picker(const char* name, Color* color, bool disabled = false)
{
	if (ImGui::GetCurrentWindow()->SkipItems)
		return;

	ImFont* font = menuFont ? menuFont : ImGui::GetFont();
	const char* name_end = km_label_end(name);
	const auto name_text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.f, name, name_end);
	const float width = 120.0f;
	const float sz = 15.0f;
	const float spacing = 5.0f;
	const float fix_shit = 5.0f;

	ImVec2 position = ImGui::GetCurrentWindow()->DC.CursorPos;
	position.x += 261.0f;

	ImRect total_bb(position, ImVec2(position.x + width, position.y + sz));

	ImGui::ItemSize(total_bb, sz + 15.f);

	if (!ImGui::ItemAdd(total_bb, ImGui::GetID(name)))
		return;

	ImGuiID id = ImGui::GetID(name);
	ImGuiStorage* storage = ImGui::GetStateStorage();
	float hover_progress = storage->GetFloat(id + 1000, 0.0f);
	float dt = ImGui::GetIO().DeltaTime;

	bool hovered = ImGui::IsMouseHoveringRect(total_bb.Min, total_bb.Max);
	hover_progress = smoothLerp(hover_progress, hovered ? 1.0f : 0.0f, dt * 6.0f);
	storage->SetFloat(id + 1000, hover_progress);

	bool color_picker_hovered = false;
	if (color) {
		const float cpw = 13.0f;
		const float cph = 12.0f;
		const float cps = 12.0f;
		ImVec2 cp_pos = ImVec2(total_bb.Min.x - cps - cpw + 26.0f, total_bb.Min.y + (sz - cph) / 2);
		ImRect cp_bb(cp_pos, ImVec2(cp_pos.x + cpw, cp_pos.y + cph));
		color_picker_hovered = cp_bb.Contains(ImGui::GetIO().MousePos);

		ImGui::PushClipRect(cp_bb.Min, cp_bb.Max, false);
		ImColor picker_bg = color_picker_hovered ? ImColor(25, 25, 25, 255) : ImColor(16, 16, 16, 255);
		ImGui::GetWindowDrawList()->AddRectFilled(cp_bb.Min, cp_bb.Max, picker_bg, 4.0f, ImDrawFlags_RoundCornersAll);
		ImColor picker_outline = color_picker_hovered ? ImColor(50, 50, 50, 255) : ImColor(33, 33, 33, 255);
		ImGui::GetWindowDrawList()->AddRect(cp_bb.Min, cp_bb.Max, picker_outline, 4.0f, ImDrawFlags_RoundCornersAll, 1.0f);

		ImRect clr_bb(cp_bb.Min + ImVec2(1.0f, 1.0f), cp_bb.Max - ImVec2(1.0f, 1.0f));
		ImGui::GetWindowDrawList()->AddRectFilled(clr_bb.Min, clr_bb.Max, IM_COL32(204, 204, 204, 255), 3.0f, ImDrawFlags_RoundCornersAll);
		ImGui::GetWindowDrawList()->AddRectFilled(clr_bb.Min, clr_bb.Max, ColorToU32(*color), 3.0f, ImDrawFlags_RoundCornersAll);
		ImGui::PopClipRect();
	}

	ImColor inactive_text = ImColor(150, 150, 150, 255);
	ImColor hover_text = ImColor(200, 200, 200, 255);
	ImColor text_color = smoothLerpColor(inactive_text, hover_text, hover_progress);

	ImGui::GetWindowDrawList()->AddText(font, font->FontSize,
		ImVec2(total_bb.Min.x + spacing - 260.0f, total_bb.Min.y + (sz - name_text_size.y - fix_shit) / 2),
		text_color, name, name_end);

	if (color_picker_hovered && !disabled && color) {
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			ImGui::OpenPopup((std::string("##ColorPicker_") + name).c_str());
	}

	km_color_picker_window((std::string("##ColorPicker_") + name).c_str(), color);
}

// ============================================================================
//  Input text
// ============================================================================

inline bool km_input_text(const char* name, const char* hint, char* buf, size_t buf_size, float width = 280.f, float height = 22.f, ImGuiInputTextFlags flags = 0)
{
	ImVec2 input_pos = ImGui::GetCursorPos();
	ImVec2 input_min = ImGui::GetWindowPos() + input_pos;
	ImVec2 input_max = input_min + ImVec2(width, height);

	ImGui::GetWindowDrawList()->AddRectFilled(input_min, input_max, ImColor(13, 13, 13, 255), 4.0f, ImDrawFlags_RoundCornersAll);
	ImGui::GetWindowDrawList()->AddRect(input_min, input_max, ImColor(33, 33, 33, 255), 4.0f, ImDrawFlags_RoundCornersAll, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

	bool result = ImGui::InputTextEx(name, hint, buf, static_cast<int>(buf_size), ImVec2(width, height), flags, 0, 0);

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(2);

	return result;
}

// ============================================================================
//  Grey label text helper
// ============================================================================

inline void km_text(const char* text)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f, 1.0f));
	ImGui::Text("%s", text);
	ImGui::PopStyleColor();
}

// Grey label meant to be immediately followed by km_keybind() on the same row
// (km_keybind does its own right-aligning SameLine).
inline void km_text_with_keybind(const char* text)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f, 1.0f));
	ImGui::Text("%s", text);
	ImGui::PopStyleColor();
}
