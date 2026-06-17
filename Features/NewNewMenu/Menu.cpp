#include "Menu.h"
#include "MenuControls.h"
#include "Dropdown.h"

#include "babagui/imgui.h"
#include "babagui/imgui_impl_dx9.h"
#include "babagui/imgui_impl_win32.h"
#include "babagui/imgui_internal.h"

#include "MenuBackground.h"
#include "MenuFonts.h"
#include "blur.hpp"
#include "../Vars.h"

extern ImFont* menuFont;
extern ImFont* boldMenuFont;
ImFont* tabFont;
ImFont* tabFont2;
ImFont* tabFont3;
ImFont* controlFont;
IDirect3DTexture9* menuBg;

static int tab = 0;

extern bool unload;

inline void ColorToFloat4(const Color& color, float out[4]) { // p100 code
	out[0] = color.r() / 255.f;
	out[1] = color.g() / 255.f;
	out[2] = color.b() / 255.f;
	out[3] = color.a() / 255.f;
}

inline Color Float4ToColor(const float in[4]) {
	return Color(in[0], in[1], in[2], in[3]);
}

void Menu::ColorPicker(const char* name, float* color, bool alpha) {

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiStyle* style = &ImGui::GetStyle();

	auto alphaSliderFlag = alpha ? ImGuiColorEditFlags_AlphaBar : ImGuiColorEditFlags_NoAlpha;

	ImGui::SameLine(219.f);
		ImGui::ColorEdit4(std::string{ "##" }.append(name).append("Picker").c_str(), color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
}

void Menu::Render(IDirect3DDevice9* pDevice) {

	static bool bInitImGui = false;
	BlurData::device = pDevice;
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);


	if (!bInitImGui) {
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(FindWindowW(_(L"Valve001"), 0));
		ImGui_ImplDX9_Init(pDevice);

		ImGuiIO& io = ImGui::GetIO();
		menuFont = io.Fonts->AddFontFromMemoryCompressedTTF(
			verdana_compressed_data,
			verdana_compressed_size,
			13.0f, // size is 13 because it fits better
			nullptr,
			io.Fonts->GetGlyphRangesDefault()
		);

		boldMenuFont = io.Fonts->AddFontFromMemoryCompressedTTF(
			verdanab_compressed_data,
			verdanab_compressed_size,
			14.0f,
			nullptr,
			io.Fonts->GetGlyphRangesDefault()
		);

		tabFont = io.Fonts->AddFontFromMemoryTTF(
			skeet,
			skeet_size,
			51.0f,
			NULL,
			io.Fonts->GetGlyphRangesDefault()
		);

		// arrow font (combo arrow, vb)
		controlFont = io.Fonts->AddFontFromMemoryCompressedTTF(
			comboarrow_compressed_data,
			comboarrow_compressed_size,
			20.0f
		);

		bInitImGui = true;
	}


	if (GetAsyncKeyState(VK_INSERT) & 1) { // Can we please fix this in WndProcHook...? :( // edit - 21.11.2025: fixed
		I::VGuiSurface->SetCursorAlwaysVisible(g_Menu.isOpen = !g_Menu.isOpen);
		g_Menu.flTimeOnChange = I::EngineClient->GetTimescale();
	}

	g_Menu.m_flFadeElapsed = I::EngineClient->GetTimescale() - g_Menu.flTimeOnChange;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	ImGui::GetIO().MouseDrawCursor = g_Menu.isOpen;

	if (g_Menu.isOpen)
	{
		ImGuiStyle* style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(6, 6);

		ImGui::PushFont(menuFont);

		ImGui::SetNextWindowSize(ImVec2(660.f, 560.f));
		ImGui::BeginMenuBackground("ThighHighs & Tranny hake", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar); {
			ImGui::BeginChild("Complete Border", ImVec2(648.f, 548.f), false); {

				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 size = ImGui::GetWindowSize();

				float spacing = 10.f;
				ImU32 dot_color = IM_COL32(255, 255, 255, 20);

				draw_list->AddCircleFilled(ImVec2(pos.x + size.x, pos.y + size.y), 1.0f, dot_color);

				ImU32 background_color = IM_COL32(20, 20, 20, 255);
				draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), background_color);

			} ImGui::EndChild();

			ImGui::SameLine(6.f);

			style->Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0, 0);

			ImGui::BeginChild("Menu Contents", ImVec2(648.f, 548.f), false); {

				ImGui::ColorBar("unicorn", ImVec2(648.f, 2.f));

				style->ItemSpacing = ImVec2(0.f, -1.f);

				ImGui::BeginTabs("Tabs", ImVec2(75.f, 542.f), false); {

					style->ItemSpacing = ImVec2(0.f, 0.f);

					style->ButtonTextAlign = ImVec2(0.5f, 0.47f);

					ImGui::PopFont();
					ImGui::PushFont(tabFont);

					switch (tab) {

					case 0:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::SelectedTab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::Tab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::Tab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::Tab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::Tab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::Tab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::Tab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					case 1:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::Tab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::SelectedTab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::Tab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::Tab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::Tab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::Tab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::Tab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					case 2:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::Tab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::Tab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::SelectedTab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::Tab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::Tab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::Tab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::Tab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					case 3:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::Tab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::Tab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::Tab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::SelectedTab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::Tab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::Tab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::Tab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					case 4:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::Tab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::Tab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::Tab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::Tab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::SelectedTab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::Tab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::Tab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					case 5:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::Tab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::Tab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::Tab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::Tab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::Tab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::SelectedTab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::Tab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					case 6:
						ImGui::TabSpacer("##Top Spacer", ImVec2(75.f, 10.f));

						if (ImGui::Tab("C", ImVec2(75.f, 75.f))) tab = 0;
						if (ImGui::Tab("B", ImVec2(75.f, 75.f))) tab = 1;
						if (ImGui::Tab("D", ImVec2(75.f, 75.f))) tab = 2;
						if (ImGui::Tab("E", ImVec2(75.f, 75.f))) tab = 3;
						if (ImGui::Tab("F", ImVec2(75.f, 75.f))) tab = 4;
						if (ImGui::Tab("G", ImVec2(75.f, 75.f))) tab = 5;
						if (ImGui::SelectedTab("H", ImVec2(75.f, 75.f))) tab = 6;

						ImGui::TabSpacer2("##Bottom Spacer", ImVec2(75.f, 7.f));
						break;
					}

					ImGui::PopFont();
					ImGui::PushFont(menuFont);

					style->ButtonTextAlign = ImVec2(0.5f, 0.5f);

				} ImGui::EndTabs();

				ImGui::SameLine(75.f);

				ImGui::BeginChild("Tab Contents", ImVec2(572.f, 542.f), false); {

					style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);

					switch (tab) {

					case 0:
						Aimbot();
						break;
					case 1:
						Antiaim();
						break;
					case 2:
						Legit();
						break;
					case 3:
						Visuals();
						break;
					case 4:
						Misc();
						break;
					case 5:
						Skins();
						break;
					case 6:
						Players();
						break;
					}

					style->Colors[ImGuiCol_Border] = ImColor(10, 10, 10, 255);

				} ImGui::EndChild();

				style->ItemSpacing = ImVec2(4.f, 4.f);
				style->Colors[ImGuiCol_ChildBg] = ImColor(17, 17, 17, 255);

			} ImGui::EndChild();

			ImGui::PopFont();

		} ImGui::End();
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
}

void Menu::Shutdown() {

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

void Menu::Aimbot() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Aimbot", 506.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertCheckbox("Enable Aimbot", Vars::Hitscan::bEnable);
			InsertCheckbox("Use Key For Aimbot", Vars::Hitscan::bKey); // Main Key is LSHIFT, you can change or add a keybind yourself.
			InsertCheckbox("Silent Aim", Vars::Hitscan::bSilentAim);
			InsertCheckbox("Auto Shoot", Vars::Hitscan::bAutoShoot);
			InsertCheckbox("Interp", Vars::Misc::DisableInterp);

			ImVec4 FovColor = ImColor(Vars::Hitscan::AimFov.r(), Vars::Hitscan::AimFov.g(), Vars::Hitscan::AimFov.b(), Vars::Hitscan::AimFov.a());
			InsertCheckbox("FOV", Vars::Hitscan::AimFovCircle);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Fov Color", (float*)&FovColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::Hitscan::AimFov = {
					(int)(FovColor.x * 255.f),
					(int)(FovColor.y * 255.f),
					(int)(FovColor.z * 255.f),
					(int)(FovColor.w * 255.f)
				};
			}
			InsertSliderInt("Radius", Vars::Hitscan::iFov, 1, 180, "%.0f");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxLeft("Aimbot Cover", "Aimbot");
	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Other", 506.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertCheckbox("No Recoil", Vars::Removals::NoRecoil);
			InsertCheckbox("No Spread", Vars::Removals::NoSpread);

			InsertCheckbox("Ignore Steam Friends", Vars::Ignore::IgnoreFriends);
			InsertCheckbox("Ignore Teammates", Vars::Ignore::IgnoreTeammates);
			InsertCheckbox("Ignore Idle Witch", Vars::Ignore::IgnoreWitchUntilStartled);
			InsertCheckbox("Ignore Common Infected", Vars::Ignore::IgnoreCommonInfected);
			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Other Cover", "Other");
	}
}

void Menu::Antiaim() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Anti-aimbot angles", 506.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			InsertCheckbox("Master Switch", Vars::HvH::Enabled);
			InsertCheckbox("Enable Anti-Aim", Vars::HvH::AntiAim);

			const char* yawModes[] = {
				"None",
				"Backwards",
				"Right",
				"Left",
				"Jitter",
				"Spin",
				"Random",
				"Fake Jitter"
			};
			InsertCombo("Real Yaw", Vars::HvH::RealYaw, yawModes, IM_ARRAYSIZE(yawModes));

			if (Vars::HvH::RealYaw == 5)
			{
				InsertSliderFloat("Spin Speed", Vars::HvH::SpinSpeed, -360, 360, "%.1f");
			}

			const char* pitchModes[] = {
				"None",
				"Fake Down",
				"Fake Up",
				"Zero",
				"Jitter Pitch",
				"Random Pitch"
			};
			InsertCombo("Pitch Mode", Vars::HvH::PitchMode, pitchModes, IM_ARRAYSIZE(pitchModes));

			InsertCheckbox("Enable Fake Angle", Vars::HvH::FakeAngle);
			InsertSliderFloat("Fake Offset", Vars::HvH::FakeOffset, -180.f, 180.f, "%.1f");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxLeft("Anti-aimbot angles Cover", "Anti-aimbot angles");

	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Fake lag", 331.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");
			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Fake lag Cover", "Fake lag");

		InsertSpacer("Fake lag - Other Spacer");

		InsertGroupBoxRight("Other", 157.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");
			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Other Cover", "Other");
	}
}

void Menu::Legit() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	InsertGroupBoxTop("Weapon Selection", ImVec2(535.f, 61.f)); {
		style->ItemSpacing = ImVec2(4, 2);
		style->WindowPadding = ImVec2(4, 4);
		ImGui::CustomSpacing(9.f);
		InsertText("cheat does not have that function");
		style->ItemSpacing = ImVec2(0, 0);
		style->WindowPadding = ImVec2(6, 6);
	} InsertEndGroupBoxTop("Weapon Selection Cover", "Weapon Selection", ImVec2(536.f, 11.f));

	InsertSpacer("Weapon Selection - Main Group boxes Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Aimbot", 427.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");
			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxLeft("Aimbot Cover", "Aimbot");
	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Triggerbot", 277.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");
			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Triggerbot Cover", "Triggerbot");

		InsertSpacer("Triggerbot - Other Spacer");

		InsertGroupBoxRight("Other", 132.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");
			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Other Cover", "Other");
	}
}

void Menu::Visuals() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Player ESP", 331.f); {

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			InsertCheckbox("Enable ESP", Vars::ESP::Enabled);

			ImVec4 playerCol = ImColor(Vars::ESP::PlayerColor.r(), Vars::ESP::PlayerColor.g(), Vars::ESP::PlayerColor.b(), Vars::ESP::PlayerColor.a());
			ImVec4 infectedCol = ImColor(Vars::ESP::InfectedColor.r(), Vars::ESP::InfectedColor.g(), Vars::ESP::InfectedColor.b(), Vars::ESP::InfectedColor.a());
			ImVec4 playerinfectedCol = ImColor(Vars::ESP::PlayerInfectedColor.r(), Vars::ESP::PlayerInfectedColor.g(), Vars::ESP::PlayerInfectedColor.b(), Vars::ESP::PlayerInfectedColor.a());


			InsertCheckbox("Survivors", Vars::ESP::players);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Player color##esp", (float*)&playerCol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::ESP::PlayerColor = {
					(int)(playerCol.x * 255.f),
					(int)(playerCol.y * 255.f),
					(int)(playerCol.z * 255.f),
					(int)(playerCol.w * 255.f)
				};
			}
			InsertCheckbox("Infected", Vars::ESP::infected);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Infected color", (float*)&infectedCol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::ESP::InfectedColor = {
					(int)(infectedCol.x * 255.f),
					(int)(infectedCol.y * 255.f),
					(int)(infectedCol.z * 255.f),
					(int)(infectedCol.w * 255.f)
				};
			}
			InsertCheckbox("Special Infected", Vars::ESP::specialinfected);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Player Infected color", (float*)&playerinfectedCol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::ESP::PlayerInfectedColor = {
					(int)(playerinfectedCol.x * 255.f),
					(int)(playerinfectedCol.y * 255.f),
					(int)(playerinfectedCol.z * 255.f),
					(int)(playerinfectedCol.w * 255.f)
				};
			}

			InsertCheckbox("Box", Vars::ESP::Box);
			InsertCheckbox("Healthbar", Vars::ESP::Healthbar);
			InsertCheckbox("Name", Vars::ESP::Name);

			InsertCheckbox("Weapons", Vars::ESP::Weapons);
			InsertCheckbox("Minigun", Vars::ESP::Minigun);


			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxLeft("Player ESP Cover", "Player ESP");

		InsertSpacer("Player ESP - Colored models Spacer");

		InsertGroupBoxLeft("Colored models", 157.f); {

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			ImVec4 playerColor = ImColor(Vars::Chams::PlayerColor.r(), Vars::Chams::PlayerColor.g(), Vars::Chams::PlayerColor.b(), Vars::Chams::PlayerColor.a());
			//ImVec4 infectedColor = ImColor(Vars::Chams::InfectedColor.r(), Vars::Chams::InfectedColor.g(), Vars::Chams::InfectedColor.b(), Vars::Chams::InfectedColor.a());
			ImVec4 specialColor = ImColor(Vars::Chams::PlayerInfectedColor.r(), Vars::Chams::PlayerInfectedColor.g(), Vars::Chams::PlayerInfectedColor.b(), Vars::Chams::PlayerInfectedColor.a());
			ImVec4 skyboxColor = ImColor(Vars::SkyBox::color.r(), Vars::SkyBox::color.g(), Vars::SkyBox::color.b(), Vars::SkyBox::color.a());
			ImVec4 weaponColor = ImColor(Vars::Chams::ViewmodelGunColor.r(), Vars::Chams::ViewmodelGunColor.g(), Vars::Chams::ViewmodelGunColor.b(), Vars::Chams::ViewmodelGunColor.a());


			InsertCheckbox("Player Chams", Vars::Chams::Players);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Player Chams color", (float*)&playerColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::Chams::PlayerColor = {
					(int)(playerColor.x * 255.f),
					(int)(playerColor.y * 255.f),
					(int)(playerColor.z * 255.f),
					(int)(playerColor.w * 255.f)
				};
			}

			InsertCheckbox("Special Infected Chams", Vars::Chams::PlayerInfected);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Player Infected Chams color", (float*)&specialColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::Chams::PlayerInfectedColor = {
					(int)(specialColor.x * 255.f),
					(int)(specialColor.y * 255.f),
					(int)(specialColor.z * 255.f),
					(int)(specialColor.w * 255.f)
				};
			}

			InsertCheckbox("Skybox Color", Vars::SkyBox::Enable);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Skybox color", (float*)&skyboxColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::SkyBox::color = {
					(int)(skyboxColor.x * 255.f),
					(int)(skyboxColor.y * 255.f),
					(int)(skyboxColor.z * 255.f),
					(int)(skyboxColor.w * 255.f)
				};
			}

			InsertCheckbox("Local Weapon Chams", Vars::Chams::ViewmodelGun);
			ImGui::SameLine(219.f);
			if (ImGui::ColorEdit4("##Local Weapon Chams color", (float*)&weaponColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip))
			{
				Vars::Chams::ViewmodelGunColor = {
					(int)(weaponColor.x * 255.f),
					(int)(weaponColor.y * 255.f),
					(int)(weaponColor.z * 255.f),
					(int)(weaponColor.w * 255.f)
				};
			}

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxLeft("Colored models Cover", "Colored models");
	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Other ESP", 199.f); {

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			
			InsertCheckbox("Fov Modifier", Vars::ESP::m_bFovChanger);
			if (Vars::ESP::m_bFovChanger)
			{
				InsertSliderFloat("Viewmodel Fov", Vars::ESP::m_fViewFov, 0.0f, 130.0f, _("%.0f"));
				InsertSliderFloat("Game Fov", Vars::ESP::m_fFov, 0.0f, 180.0f, _("%.0f"));
			}

			InsertCheckbox("Viewmodel Modifier", Vars::ESP::ViewModelChanger);
			if (Vars::ESP::ViewModelChanger)
			{
				InsertSliderFloat("Viewmodel X", Vars::ESP::viewmodel_x, -50.f, 50.f, "%.0f");
				InsertSliderFloat("Viewmodel Y", Vars::ESP::viewmodel_y, -50.f, 50.f, "%.0f");
				InsertSliderFloat("Viewmodel Z", Vars::ESP::viewmodel_z, -50.f, 50.f, "%.0f");
			}

			InsertCheckbox("No Visual Recoil", Vars::Removals::NoVisualRecoil);

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxRight("Other ESP Cover", "Other ESP");

		InsertSpacer("Other ESP - Effects Spacer");

		InsertGroupBoxRight("Effects", 289.f); {

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			InsertCheckbox("Remove Boomer Goon", Vars::Removals::BoomerVisual);

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxRight("Effects Cover", "Effects");
	}
}

void Menu::Misc() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Miscellaneous", 506.f); {

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			InsertCheckbox("Bunnyhop", Vars::Misc::Bunnyhop);
			InsertCheckbox("Auto Strafe", Vars::Misc::AutoStrafe);
			InsertCheckbox("Auto Shove", Vars::Misc::AutoShove); // SO USELESS ONG

			InsertCheckbox("ThirdPerson", Vars::Misc::ThirdPerson);
			InsertSliderInt("ThirdPerson Distance", Vars::Misc::ThirdPersonDistance, 50, 300, "%.0f");
			//thirdperson key is b

			InsertCheckbox("Lag Exploit", Vars::Misc::SequenceFreezing);
			InsertSliderInt("Lag Value", Vars::Misc::SequenceFreezingValue, 1, 1000, "%.0f");
			//lag key is alt

			InsertCheckbox("Teleport", Vars::Misc::Teleport);
			//teleport key is b

			InsertCheckbox("Speed Hax", Vars::Exploits::Speedhack);
			InsertSliderInt("Speed Value", Vars::Exploits::SpeedHackValue, 1, 100, "%.0f");
			//speedhax key is ctrl

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxLeft("Miscellaneous Cover", "Miscellaneous");

	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Settings", 156.f); {

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			InsertText("cheat does not have that function");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxRight("Settings Cover", "Settings");

		InsertSpacer("Settings - Other Spacer");

		InsertGroupBoxRight("Other", 332.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);

			InsertText("cheat does not have that function");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Other Cover", "Other");
	}
}

void Menu::Skins() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Knife options", 112.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);

			InsertText("cheat does not have that function");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxLeft("Knife options Cover", "Knife options");

		InsertSpacer("Fake lag - Other Spacer");

		InsertGroupBoxLeft("Glove options", 376.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxLeft("Glove options Cover", "Glove options");
	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Weapon skin", 506.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxRight("Weapon skin Cover", "Weapon skin");
	}
}

void Menu::Players() {

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false); {

		InsertGroupBoxLeft("Players", 506.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			ImGui::CustomSpacing(9.f);
			InsertText("Nate Higgerson - ID:1");
			InsertText("Thundercock - ID:2");
			InsertText("SkbidiHunter - ID:3");
			InsertText("WitchTheBitch - ID:4");
			InsertText("ISmokeALot - ID:5");
			InsertText("GoonerBoomer - ID:6");
			InsertText("v^ uses aimware - ID:7");
			InsertText("@@@@@@@@@@@@@@@ - ID:8");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);

		} InsertEndGroupBoxLeft("Players Cover", "Players");
	}
	ImGui::NextColumn(); {

		InsertGroupBoxRight("Adjustments", 506.f); {
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);

			ImGui::CustomSpacing(9.f);
			InsertText("cheat does not have that function");

			style->ItemSpacing = ImVec2(0, 0);
			style->WindowPadding = ImVec2(6, 6);
		} InsertEndGroupBoxRight("Adjustments Cover", "Adjustments");
	}
}