#include "Menu.h"
#include "Header.h"
#include "babagui/imgui.h"
#include "babagui/imgui_impl_dx9.h"
#include "babagui/imgui_impl_win32.h"
#include "babagui/imgui_internal.h"
#include "../Vars.h"
#include "../Movement/Movement.h"

#pragma comment(lib, "d3dx9.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Fonts
ImFont* verdanaFont = nullptr;
static ImFont* tabFont = nullptr;

static int tab = 0;
static int tab2 = 0;

static float window_alpha = 0.0f;

// Color conversion helpers
inline void ColorToFloat4(const Color& color, float out[4]) {
	out[0] = color.r() / 255.f;
	out[1] = color.g() / 255.f;
	out[2] = color.b() / 255.f;
	out[3] = color.a() / 255.f;
}

inline Color Float4ToColor(const float in[4]) {
	return Color(
		static_cast<int>(in[0] * 255.f),
		static_cast<int>(in[1] * 255.f),
		static_cast<int>(in[2] * 255.f),
		static_cast<int>(in[3] * 255.f)
	);
}

// Custom clamp helper
inline float CustomClamp(float value, float min, float max) {
	return (value < min) ? min : (value > max) ? max : value;
}

void Menu::Init(IDirect3DDevice9* pDevice) {
	if (isInitialized) return;

	ImGui::CreateContext();
	
	// Find target game window
	HWND hwWindow = FindWindowW(L"Valve001", nullptr);
	ImGui_ImplWin32_Init(hwWindow);
	ImGui_ImplDX9_Init(pDevice);

	ImGuiIO& io = ImGui::GetIO();
	
	ImFontConfig font_config;
	font_config.OversampleH = 1;
	font_config.OversampleV = 1;
	font_config.PixelSnapH = 1;

	static const ImWchar ranges[] = {
		0x0020, 0x00FF,
		0x0400, 0x044F,
		0,
	};

	// Load assets from Header.h
	tabFont = io.Fonts->AddFontFromMemoryTTF((void*)icon, sizeof(icon), 44, &font_config, ranges);
	
	// Load Verdana
	verdanaFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 13.0f, &font_config, io.Fonts->GetGlyphRangesJapanese());

	isInitialized = true;
}

void Menu::Render(IDirect3DDevice9* pDevice) {
	if (!isInitialized) {
		Init(pDevice);
	}
	
	// Reset/configure render state for ImGui overlay
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	window_alpha = g_Menu.isOpen ? 1.0f : 0.0f;
	ImGui::GetIO().MouseDrawCursor = g_Menu.isOpen;

	if (window_alpha > 0.0f) {
		ImVec4 main_color = ImVec4(210.0f / 255.0f, 100.0f / 255.0f, 185.0f / 255.0f, 1.0f); // Sleek signature theme color

		auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ 600, 420 };
		ImGui::SetNextWindowSize(gs);
		ImGui::SetNextWindowBgAlpha(window_alpha);
		
		ImGui::Begin("##GUI", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
		{
			s = ImVec2(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y * 2);
			p = ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x, ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y);
			auto draw = ImGui::GetWindowDrawList();

			// Draw window background and design panels
			draw->AddRectFilled(ImVec2(p.x, p.y + 0), ImVec2(p.x + s.x, p.y + s.y - 0), ImColor(8, 8, 8), 4);
			draw->AddRect(ImVec2(p.x + 1, p.y + 1), ImVec2(p.x + s.x - 1, p.y + s.y - 1), ImColor(27, 27, 27, 255), 4.5);
			draw->AddRect(ImVec2(p.x - -10, p.y + 35.3f), ImVec2(p.x + s.x - 480, p.y + s.y - 36.85f), ImColor(26, 26, 26, 255));

			draw->AddLine(ImVec2(p.x, p.y + s.y - 27), ImVec2(p.x + s.x, p.y + s.y - 27), ImColor(27, 27, 27, 255));
			draw->AddLine(ImVec2(p.x, p.y + 25), ImVec2(p.x + s.x, p.y + 25), ImColor(27, 27, 27, 255));

			// Horizontal gradient glow bar
			int fade_line_count = 60;
			float fade_stop = s.x;
			float center_point = fade_stop / 2.0f;

			for (int i = 0; i < fade_line_count; i++) {
				float alpha = 1.0f - (i * (1.0f / fade_line_count));
				ImVec2 start_right = ImVec2(p.x + fade_stop - i * (center_point / fade_line_count), p.y + 25);
				ImVec2 end_right = ImVec2(p.x + fade_stop - (i + 1) * (center_point / fade_line_count), p.y + 25);
				ImColor fade_color(main_color.x, main_color.y, main_color.z, alpha);
				draw->AddLine(start_right, end_right, fade_color);
			}

			ImGui::AlignTextToFramePadding();
			ImGui::PushFont(verdanaFont);
			draw->AddText(ImVec2(p.x + 9.5f, p.y + 7), ImColor(main_color), "lynette");
			draw->AddText(ImVec2(p.x + 9.5f, p.y + 384), ImColor(255, 255, 255, 100), "Build:");
			draw->AddText(ImVec2(p.x + 45.f, p.y + 384), ImColor(main_color), "Stable");
			ImGui::PopFont();

			// Main tabs
			{
				ImGui::PushFont(verdanaFont);
				ImGui::SetCursorPosX(112);
				ImGui::SetCursorPosY(10);
				ImGui::BeginGroup();
				if (ImGui::tab("Aimbot", tab == 0)) { tab = 0; tab2 = 0; } ImGui::SameLine();
				if (ImGui::tab("Visuals", tab == 1)) { tab = 1; tab2 = 0; } ImGui::SameLine();
				if (ImGui::tab("Misc", tab == 2)) { tab = 2; tab2 = 0; }
				ImGui::EndGroup();
				ImGui::PopFont();
			}

			// Render active tab content
			{
				ImGui::PushFont(verdanaFont);
				
				if (tab == 0) // Aimbot
				{
					ImGui::SetCursorPosY(54);
					ImGui::SetCursorPosX(30);
					if (ImGui::subtab("General", tab2 == 0)) tab2 = 0;
					
					ImGui::SetCursorPosY(79);
					ImGui::SetCursorPosX(30);
					if (ImGui::subtab("Anti-Aim", tab2 == 1)) tab2 = 1;

					if (tab2 == 0) {
						// Aimbot Settings Group
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(122);
						ImGui::MenuChild("Aimbot Options", ImVec2(226, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Enabled", &Vars::Hitscan::bEnable);
							ImGui::Checkbox("Silent Aim", &Vars::Hitscan::bSilentAim);
							ImGui::Checkbox("Auto Shoot", &Vars::Hitscan::bAutoShoot);
							ImGui::Checkbox("Draw FOV Circle", &Vars::Hitscan::AimFovCircle);

							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);

							// FOV is int in Vars.h, cast to float for slider and restore
							float fovVal = static_cast<float>(Vars::Hitscan::iFov);
							if (ImGui::SliderFloat("FOV Angle", &fovVal, 1.0f, 180.0f, "%.0f")) {
								Vars::Hitscan::iFov = static_cast<int>(fovVal);
							}

							ImGui::PopStyleColor(2);

							const char* hitboxes[] = { "Auto Target", "Head Only", "Body Only", "Pelvis" };
							ImGui::Combo("Target Hitbox", &Vars::Hitscan::iHitbox, hitboxes, IM_ARRAYSIZE(hitboxes));

							ImGui::Spacing();
							KeybindSelector("Aim Assist Key", &Vars::Hitscan::AimKey.m_Var);
						}
						ImGui::EndChild();

						// Target Filter Modifiers
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(358);
						ImGui::MenuChild("Target Filters", ImVec2(224, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Ignore Teammates", &Vars::Ignore::IgnoreTeammates);
							ImGui::Checkbox("Ignore Friends", &Vars::Ignore::IgnoreFriends);
							ImGui::Checkbox("Ignore Common Infected", &Vars::Ignore::IgnoreCommonInfected);
							ImGui::Checkbox("Ignore Sleeping Witch", &Vars::Ignore::IgnoreWitchUntilStartled);

							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();
					}
					else if (tab2 == 1) {
						// Anti-Aim Group
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(122);
						ImGui::MenuChild("Yaw & Pitch", ImVec2(226, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Enabled", &Vars::HvH::AntiAim);

							ImGui::PopStyleColor(2);

							const char* yawModes[] = { "Disabled", "Backwards", "Right", "Left", "Jitter", "Spin", "Random", "Fake Jitter" };
							ImGui::Combo("Real Yaw Mode", &Vars::HvH::RealYaw, yawModes, IM_ARRAYSIZE(yawModes));

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);
							ImGui::SliderFloat("Spin Speed", &Vars::HvH::SpinSpeed, 0.0f, 100.0f);
							ImGui::PopStyleColor(2);

							const char* pitchModes[] = { "Disabled", "Down", "Up", "Zero", "Fake Down", "Fake Up", "Fake Zero", "Random" };
							ImGui::Combo("Pitch Mode", &Vars::HvH::PitchMode, pitchModes, IM_ARRAYSIZE(pitchModes));
						}
						ImGui::EndChild();

						// Desync Angles
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(358);
						ImGui::MenuChild("Desync Angles", ImVec2(224, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Fake Angle", &Vars::HvH::FakeAngle);

							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);

							ImGui::SliderFloat("Fake Yaw Offset", &Vars::HvH::FakeOffset, -180.f, 180.f);
							ImGui::SliderFloat("Desync Offset", &Vars::HvH::DesyncOffset, -58.f, 58.f);

							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();
					}
				}
				else if (tab == 1) // Visuals
				{
					ImGui::SetCursorPosY(54);
					ImGui::SetCursorPosX(30);
					if (ImGui::subtab("ESP Options", tab2 == 0)) tab2 = 0;

					ImGui::SetCursorPosY(79);
					ImGui::SetCursorPosX(30);
					if (ImGui::subtab("Chams & Visuals", tab2 == 1)) tab2 = 1;

					if (tab2 == 0) {
						// ESP General Options
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(122);
						ImGui::MenuChild("Player ESP", ImVec2(226, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Master Switch", &Vars::ESP::Enabled);
							ImGui::Checkbox("Survivors", &Vars::ESP::players);
							ImGui::Checkbox("Special Infected", &Vars::ESP::specialinfected);
							ImGui::Checkbox("Common Infected", &Vars::ESP::infected);
							ImGui::Checkbox("Bounding Box", &Vars::ESP::Box);
							ImGui::Checkbox("Health Bar", &Vars::ESP::Healthbar);
							ImGui::Checkbox("Draw Name", &Vars::ESP::Name);
							ImGui::Checkbox("Draw Weapon", &Vars::ESP::Weapons);
							ImGui::Checkbox("Minigun Glow", &Vars::ESP::Minigun);
							ImGui::Checkbox("Ammunition Glow", &Vars::ESP::Ammo);
							ImGui::Checkbox("Local ESP", &Vars::ESP::LocalESP);



							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();

						// ESP Colors
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(358);
						ImGui::MenuChild("Colors Configuration", ImVec2(224, 337), false);
						{
							float colS[4], colSI[4], colCI[4];
							ColorToFloat4(Vars::ESP::PlayerColor, colS);
							ColorToFloat4(Vars::ESP::PlayerInfectedColor, colSI);
							ColorToFloat4(Vars::ESP::InfectedColor, colCI);

							if (ImGui::ColorEdit4("Survivor Color", colS, ImGuiColorEditFlags_NoInputs)) {
								Vars::ESP::PlayerColor = Float4ToColor(colS);
							}
							if (ImGui::ColorEdit4("Special Infected", colSI, ImGuiColorEditFlags_NoInputs)) {
								Vars::ESP::PlayerInfectedColor = Float4ToColor(colSI);
							}
							if (ImGui::ColorEdit4("Common Infected", colCI, ImGuiColorEditFlags_NoInputs)) {
								Vars::ESP::InfectedColor = Float4ToColor(colCI);
							}

							ImGui::Separator();
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
							ImGui::Checkbox("Velocity HUD", &Vars::ESP::VelocityHUD);
							ImGui::Checkbox("Velocity Graph", &Vars::ESP::VelocityGraph);
							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);
							ImGui::SliderFloat("Speedo X", &Vars::ESP::VelocityX, 0.0f, 1.0f, "%.2f");
							ImGui::SliderFloat("Speedo Y", &Vars::ESP::VelocityY, 0.0f, 1.0f, "%.2f");
							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();
					}
					else if (tab2 == 1) {
						// Chams
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(122);
						ImGui::MenuChild("Chams Settings", ImVec2(226, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Survivors Chams", &Vars::Chams::Players);
							ImGui::Checkbox("SI Chams", &Vars::Chams::PlayerInfected);
							ImGui::Checkbox("CI Chams", &Vars::Chams::Infected);
							ImGui::Checkbox("Local Weapon Chams", &Vars::Chams::ViewmodelGun);

							ImGui::PopStyleColor(2);

							const char* chamsMaterials[] = { "Solid Material", "Flat Shaded", "Wireframe" };
							ImGui::Combo("Chams Material", &Vars::Chams::ViewmodelMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));

							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
							ImGui::Checkbox("Chams Through Walls", &Vars::Chams::ThroughWalls);
							ImGui::PopStyleColor(2);

							float wCol[4];
							ColorToFloat4(Vars::Chams::ViewmodelGunColor, wCol);
							if (ImGui::ColorEdit4("Weapon Color", wCol, ImGuiColorEditFlags_NoInputs)) {
								Vars::Chams::ViewmodelGunColor = Float4ToColor(wCol);
							}

							ImGui::Separator();
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
							ImGui::Checkbox("Viewmodel Changer", &Vars::ESP::ViewModelChanger);
							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);
							ImGui::SliderFloat("Viewmodel X", &Vars::ESP::viewmodel_x, -10.f, 10.f, "%.1f");
							ImGui::SliderFloat("Viewmodel Y", &Vars::ESP::viewmodel_y, -10.f, 10.f, "%.1f");
							ImGui::SliderFloat("Viewmodel Z", &Vars::ESP::viewmodel_z, -10.f, 10.f, "%.1f");
							ImGui::SliderFloat("Viewmodel Roll", &Vars::ESP::viewmodel_roll, -180.f, 180.f, "%.1f");
							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();

						// Removals and Sky
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(358);
						ImGui::MenuChild("World & View Modifiers", ImVec2(224, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Rainbow Skybox", &Vars::SkyBox::RainbowSkyBox);
							ImGui::Checkbox("Skybox Custom Color", &Vars::SkyBox::Enable);

							ImGui::PopStyleColor(2);

							float sCol[4];
							ColorToFloat4(Vars::SkyBox::color, sCol);
							if (ImGui::ColorEdit4("Skybox Color", sCol, ImGuiColorEditFlags_NoInputs)) {
								Vars::SkyBox::color = Float4ToColor(sCol);
							}

							ImGui::Separator();
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("No Spread", &Vars::Removals::NoSpread);
							ImGui::Checkbox("No Recoil", &Vars::Removals::NoRecoil);
							ImGui::Checkbox("No Visual Recoil", &Vars::Removals::NoVisualRecoil);
							ImGui::Checkbox("Remove Boomer Vomit", &Vars::Removals::BoomerVisual);

							ImGui::PopStyleColor(2);

							ImGui::Separator();
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
							ImGui::Checkbox("Fov Changer Enabled", &Vars::ESP::m_bFovChanger);
							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);
							ImGui::SliderFloat("View Fov", &Vars::ESP::m_fViewFov, 20.f, 150.f, "%.0f");
							ImGui::SliderFloat("Game Fov", &Vars::ESP::m_fFov, 20.f, 150.f, "%.0f");
							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();
					}
				}
				else if (tab == 2) // Miscellaneous
				{
					ImGui::SetCursorPosY(54);
					ImGui::SetCursorPosX(30);
					if (ImGui::subtab("Movement", tab2 == 0)) tab2 = 0;

					ImGui::SetCursorPosY(79);
					ImGui::SetCursorPosX(30);
					if (ImGui::subtab("Exploits", tab2 == 1)) tab2 = 1;

					if (tab2 == 0) {
						// Ported Movement Options
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(122);
						ImGui::MenuChild("Movement Settings", ImVec2(226, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Bunnyhop", &Vars::Movement::bBhop);
							ImGui::Checkbox("Auto Air Strafe", &Vars::Misc::AutoStrafe);
							ImGui::Checkbox("Auto Shove", &Vars::Misc::AutoShove);

							ImGui::Separator();

							ImGui::Checkbox("Edge Jump", &Vars::Movement::bEdgeJump);
							ImGui::Checkbox("Long Jump", &Vars::Movement::bLongJump);
							ImGui::Checkbox("Mini Jump", &Vars::Movement::bMiniJump);
							ImGui::Checkbox("Edge Bug", &Vars::Movement::bEdgeBug);

							ImGui::PopStyleColor(2);

							ImGui::Spacing();
							KeybindSelector("Edge Jump Key", &Vars::Movement::kEdgeJump);
							KeybindSelector("Long Jump Key", &Vars::Movement::kLongJump);
							KeybindSelector("Mini Jump Key", &Vars::Movement::kMiniJump);
							KeybindSelector("Edge Bug Key", &Vars::Movement::kEdgeBug);
						}
						ImGui::EndChild();

						// EdgeBug Advanced Details
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(358);
						ImGui::MenuChild("Edge Bug Advanced", ImVec2(224, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Extended Search Paths", &Vars::Movement::ExtendedEdgeBugPaths);
							ImGui::Checkbox("LAN Game Compatibility", &Vars::Movement::bLAN);
							ImGui::Checkbox("CS Style GameMovement", &Vars::Movement::bCSGameMovement);
							ImGui::Checkbox("Suppress Fall Damage", &Vars::Movement::EdgeBugNoDamage);

							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);

							ImGui::SliderInt("Scan Limit Ticks", &Vars::Movement::EdgeBugTicks, 1, 64);
							ImGui::SliderInt("Search Path Density", &Vars::Movement::EdgeBugPaths, 1, 10);
							ImGui::SliderFloat("Scan Angle Limit", &Vars::Movement::EdgeBugAngle, 5.0f, 90.0f, "%.0f");

							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();
					}
					else if (tab2 == 1) {
						// Exploits
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(122);
						ImGui::MenuChild("Exploits", ImVec2(226, 337), false);
						{
							ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

							ImGui::Checkbox("Speedhack", &Vars::Exploits::Speedhack);
							ImGui::Checkbox("Sequence Freezing", &Vars::Misc::SequenceFreezing);
							ImGui::Checkbox("Teleport Exploit", &Vars::Misc::Teleport);
							ImGui::Checkbox("Third Person", &Vars::Misc::ThirdPerson);

							ImGui::PopStyleColor(2);

							ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
							ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);

							ImGui::SliderInt("Speed Scale", &Vars::Exploits::SpeedHackValue, 1, 25);
							ImGui::SliderInt("Freezing Ticks", &Vars::Misc::SequenceFreezingValue, 1, 150);
							ImGui::SliderInt("Cam Distance", &Vars::Misc::ThirdPersonDistance, 10, 500);

							ImGui::PopStyleColor(2);
						}
						ImGui::EndChild();

						// Bind selectors
						ImGui::SetCursorPosY(38);
						ImGui::SetCursorPosX(358);
						ImGui::MenuChild("Keybind Selectors", ImVec2(224, 337), false);
						{
							KeybindSelector("Speedhack Key", &Vars::Exploits::Key.m_Var);
							KeybindSelector("Freezing Key", &Vars::Misc::Key.m_Var);
							KeybindSelector("Teleport Key", &Vars::Misc::TeleportKey.m_Var);
							KeybindSelector("Third Person Key", &Vars::Misc::ThirdPersonKey.m_Var);
						}
						ImGui::EndChild();
					}
				}

				ImGui::PopFont();
			}
		}
		ImGui::End();
	}
}

void Menu::Shutdown() {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}