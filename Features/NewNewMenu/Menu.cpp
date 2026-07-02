#define IMGUI_DEFINE_MATH_OPERATORS
#include "Menu.h"
#include "babagui/imgui.h"
#include "babagui/imgui_impl_dx9.h"
#include "babagui/imgui_impl_win32.h"
#include "babagui/imgui_internal.h"
#include "../Vars.h"
#include "../Movement/Movement.h"
#include "../Config/Config.h"

#include "KamiberaMenu.h"
#include "KamiberaAnimations.h"
#include "KamiberaTabs.h"
#include "KamiberaElements.h"

#pragma comment(lib, "d3dx9.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImFont* verdanaFont = nullptr;
ImFont* indicatorFont = nullptr;

// menuFont, menuFontSmall, menuFontBold are inline globals in KamiberaMenu.h

// ============================================================================
//  Combo item arrays
// ============================================================================

static const char* hitboxes[] = { "Auto Target", "Head Only", "Body Only", "Pelvis" };
static const char* aimModes[] = { "Always On", "Hold", "Toggle" };
static const char* yawModes[] = { "Disabled", "Backwards", "Right", "Left", "Jitter", "Spin", "Random", "Fake Jitter" };
static const char* pitchModes[] = { "Disabled", "Down", "Up", "Zero", "Fake Down", "Fake Up", "Fake Zero", "Random" };
static const char* chamsMaterials[] = { "Solid", "Flat", "Wireframe", "Glow", "Shaded" };

// Layout constants (kamibera-calibrated: children are 300px wide)
static constexpr float KM_LEFT_X = 215.0f;
static constexpr float KM_RIGHT_X = 530.0f;
static constexpr float KM_TOP_Y = 12.0f;
static constexpr float KM_CHILD_W = 300.0f;
static constexpr float KM_CHILD_H = 555.0f;

// ============================================================================
//  Tab content renderers — Aimbot
// ============================================================================

static void RenderAimbot_General()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Aimbot", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Enabled", &Vars::Hitscan::bEnable);
		km_checkbox("Silent Aim", &Vars::Hitscan::bSilentAim);
		km_checkbox("Auto Shoot", &Vars::Hitscan::bAutoShoot);
		km_checkbox("Draw FOV Circle", &Vars::Hitscan::AimFovCircle);

		float fovVal = static_cast<float>(Vars::Hitscan::iFov);
		if (km_slider_float("FOV Angle", &fovVal, 1.0f, 180.0f, "%.0f"))
			Vars::Hitscan::iFov = static_cast<int>(fovVal);

		km_combo("Hitbox", Vars::Hitscan::iHitbox, hitboxes, IM_ARRAYSIZE(hitboxes));
		km_combo("Aim Mode", Vars::Hitscan::AimMode, aimModes, IM_ARRAYSIZE(aimModes));

		km_text_with_keybind("Aim Key");
		km_keybind("##aimkey", &Vars::Hitscan::AimKey.m_Var);
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Target Filters", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Ignore Teammates", &Vars::Ignore::IgnoreTeammates);
		km_checkbox("Ignore Friends", &Vars::Ignore::IgnoreFriends);
		km_checkbox("Ignore Common Infected", &Vars::Ignore::IgnoreCommonInfected);
		km_checkbox("Ignore Sleeping Witch", &Vars::Ignore::IgnoreWitchUntilStartled);
	});
}

static void RenderAimbot_AntiAim()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Yaw & Pitch", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Enabled", &Vars::HvH::AntiAim);
		km_combo("Real Yaw Mode", Vars::HvH::RealYaw, yawModes, IM_ARRAYSIZE(yawModes));
		km_slider_float("Spin Speed", &Vars::HvH::SpinSpeed, 0.0f, 100.0f, "%.0f");
		km_combo("Pitch Mode", Vars::HvH::PitchMode, pitchModes, IM_ARRAYSIZE(pitchModes));
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Desync Angles", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Fake Angle", &Vars::HvH::FakeAngle);
		km_slider_float("Fake Yaw Offset", &Vars::HvH::FakeOffset, -180.f, 180.f, "%.0f");
		km_slider_float("Desync Offset", &Vars::HvH::DesyncOffset, -58.f, 58.f, "%.0f");
	});
}

// ============================================================================
//  Tab content renderers — Visuals
// ============================================================================

static void RenderVisuals_ESP()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Player ESP", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Master Switch", &Vars::ESP::Enabled);
		km_checkbox("Survivors", &Vars::ESP::players);
		km_checkbox("Special Infected", &Vars::ESP::specialinfected);
		km_checkbox("Common Infected", &Vars::ESP::infected);
		km_checkbox("Bounding Box", &Vars::ESP::Box);
		km_checkbox("Health Bar", &Vars::ESP::Healthbar);
		km_checkbox("Draw Name", &Vars::ESP::Name);
		km_checkbox("Draw Weapon", &Vars::ESP::Weapons);
		km_checkbox("Minigun Glow", &Vars::ESP::Minigun);
		km_checkbox("Ammunition Glow", &Vars::ESP::Ammo);
		km_checkbox("Local ESP", &Vars::ESP::LocalESP);
		km_checkbox("Glow Survivors", &Vars::ESP::GlowSurvivors);
		km_checkbox("Glow Special Infected", &Vars::ESP::GlowSI);
		km_checkbox("Glow Common Infected", &Vars::ESP::GlowCI);
		km_checkbox("Glow Witch", &Vars::ESP::GlowWitch);
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("ESP Colors & HUD", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_text_with_color_picker("Survivor Color", &Vars::ESP::PlayerColor);
		km_text_with_color_picker("SI Color", &Vars::ESP::PlayerInfectedColor);
		km_text_with_color_picker("CI Color", &Vars::ESP::InfectedColor);

		km_checkbox("Velocity HUD", &Vars::ESP::VelocityHUD);
		km_checkbox("Velocity Graph", &Vars::ESP::VelocityGraph);
		km_slider_float("Speedo X", &Vars::ESP::VelocityX, 0.0f, 1.0f, "%.2f");
		km_slider_float("Speedo Y", &Vars::ESP::VelocityY, 0.0f, 1.0f, "%.2f");
	});
}

static void RenderVisuals_Chams()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Chams", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Survivors Chams", &Vars::Chams::Players);
		km_combo("Survivor Material", Vars::Chams::SurvivorMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		km_checkbox("Survivor Through Walls", &Vars::Chams::SurvivorThroughWalls);
		km_text_with_color_picker("Survivor Color##chams", &Vars::Chams::PlayerColor);

		km_checkbox("SI Chams", &Vars::Chams::PlayerInfected);
		km_combo("SI Material", Vars::Chams::SIMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		km_checkbox("SI Through Walls", &Vars::Chams::SIThroughWalls);
		km_text_with_color_picker("SI Color##chams", &Vars::Chams::PlayerInfectedColor);

		km_checkbox("CI Chams", &Vars::Chams::Infected);
		km_combo("CI Material", Vars::Chams::CIMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		km_checkbox("CI Through Walls", &Vars::Chams::CIThroughWalls);
		km_text_with_color_picker("CI Color##chams", &Vars::Chams::InfectedColor);
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Viewmodel & Weapon", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Local Weapon Chams", &Vars::Chams::ViewmodelGun);
		km_combo("Weapon Material", Vars::Chams::GunMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		km_text_with_color_picker("Weapon Color", &Vars::Chams::ViewmodelGunColor);

		km_checkbox("Viewmodel Changer", &Vars::ESP::ViewModelChanger);
		km_slider_float("Viewmodel X", &Vars::ESP::viewmodel_x, -10.f, 10.f, "%.1f");
		km_slider_float("Viewmodel Y", &Vars::ESP::viewmodel_y, -10.f, 10.f, "%.1f");
		km_slider_float("Viewmodel Z", &Vars::ESP::viewmodel_z, -10.f, 10.f, "%.1f");
		km_slider_float("Viewmodel Roll", &Vars::ESP::viewmodel_roll, -180.f, 180.f, "%.1f");
	});
}

static void RenderVisuals_World()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("World & View", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Skybox Custom Color", &Vars::SkyBox::Enable);
		km_checkbox("Rainbow Skybox", &Vars::SkyBox::RainbowSkyBox);
		km_text_with_color_picker("Skybox Color", &Vars::SkyBox::color);

		km_checkbox("Custom Fog Color", &Vars::Fog::Enable);
		km_checkbox("Remove Fog", &Vars::Fog::RemoveFog);
		km_text_with_color_picker("Fog Color", &Vars::Fog::color);

		km_checkbox("No Spread", &Vars::Removals::NoSpread);
		km_checkbox("No Recoil", &Vars::Removals::NoRecoil);
		km_checkbox("No Visual Recoil", &Vars::Removals::NoVisualRecoil);
		km_checkbox("Remove Boomer Vomit", &Vars::Removals::BoomerVisual);

		km_checkbox("Fov Changer", &Vars::ESP::m_bFovChanger);
		km_slider_float("View Fov", &Vars::ESP::m_fViewFov, 20.f, 150.f, "%.0f");
		km_slider_float("Game Fov", &Vars::ESP::m_fFov, 20.f, 150.f, "%.0f");
		km_checkbox("Aspect Ratio Changer", &Vars::ESP::m_bAspectRatioChanger);
		km_slider_float("Aspect Ratio", &Vars::ESP::m_fAspectRatio, 0.0f, 2.0f, "%.1f");
		km_checkbox("Flashlight FOV Changer", &Vars::ESP::m_bFlashlightFovChanger);
		km_slider_float("Flashlight FOV", &Vars::ESP::m_fFlashlightFov, 5.0f, 170.f, "%.0f");
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Grenades & Projectiles", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Grenade Trajectory", &Vars::Grenade::TrajectoryPrediction);
		km_text_with_color_picker("Trajectory Color", &Vars::Grenade::TrajectoryColor);

		km_checkbox("Projectile Model Color", &Vars::Grenade::ProjectileColorChanger);
		km_text_with_color_picker("Bile Color", &Vars::Grenade::BileColor);
		km_text_with_color_picker("Molotov Color", &Vars::Grenade::MolotovColor);
		km_text_with_color_picker("Spitter Color", &Vars::Grenade::SpitterColor);
		km_text_with_color_picker("Smoker Color", &Vars::Grenade::SmokerColor);

		km_checkbox("Molotov Range Visual", &Vars::Grenade::MolotovRangeVisual);
		km_slider_float("Molotov Range", &Vars::Grenade::MolotovRangeRadius, 30.f, 250.f, "%.0f");
		km_text_with_color_picker("Range Color", &Vars::Grenade::MolotovRangeColor);

		km_checkbox("Blood Color Changer", &Vars::Grenade::BloodColorChanger);
		km_text_with_color_picker("Blood Color", &Vars::Grenade::BloodColor);

		km_checkbox("Log Particles (debug)", &Vars::Grenade::LogParticleNames);
	});
}

// ============================================================================
//  Tab content renderers — Misc
// ============================================================================

static void RenderMisc_Movement()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Movement", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Bunnyhop", &Vars::Movement::bBhop);
		km_checkbox("Auto Air Strafe", &Vars::Misc::AutoStrafe);
		km_checkbox("Auto Shove", &Vars::Misc::AutoShove);

		km_checkbox("Edge Jump", &Vars::Movement::bEdgeJump);
		km_text_with_keybind("Edge Jump Key");
		km_keybind("##ejkey", &Vars::Movement::kEdgeJump);

		km_checkbox("Long Jump", &Vars::Movement::bLongJump);
		km_text_with_keybind("Long Jump Key");
		km_keybind("##ljkey", &Vars::Movement::kLongJump);

		km_checkbox("Mini Jump", &Vars::Movement::bMiniJump);
		km_text_with_keybind("Mini Jump Key");
		km_keybind("##mjkey", &Vars::Movement::kMiniJump);
		km_checkbox("Mini Jump Hold Duck", &Vars::Movement::bMiniJumpHoldDuck);

		km_checkbox("Edge Bug", &Vars::Movement::bEdgeBug);
		km_text_with_keybind("Edge Bug Key");
		km_keybind("##ebkey", &Vars::Movement::kEdgeBug);
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Edge Bug Advanced", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_slider_int("Scan Limit Ticks", &Vars::Movement::EdgeBugTicks, 1, 64);
		km_checkbox("Extended Search Paths", &Vars::Movement::ExtendedEdgeBugPaths);
		km_slider_int("Search Path Density", &Vars::Movement::EdgeBugPaths, 1, 10);
		km_slider_float("Scan Angle Limit", &Vars::Movement::EdgeBugAngle, 5.0f, 90.0f, "%.0f");
		km_checkbox("Force Prediction", &Vars::Movement::bForcePrediction);
		km_checkbox("CS Style GameMovement", &Vars::Movement::bCSGameMovement);
		km_checkbox("Suppress Fall Damage", &Vars::Movement::EdgeBugNoDamage);
		km_checkbox("Debug Path Visual", &Vars::Movement::EdgeBugDebugVisual);
	});
}

static void RenderMisc_Exploits()
{
	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Exploits", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Speedhack", &Vars::Exploits::Speedhack);
		km_text_with_keybind("Speedhack Key");
		km_keybind("##shkey", &Vars::Exploits::Key.m_Var);
		km_slider_int("Speed Scale", &Vars::Exploits::SpeedHackValue, 1, 25);

		km_checkbox("Sequence Freezing", &Vars::Misc::SequenceFreezing);
		km_text_with_keybind("Freezing Key");
		km_keybind("##sfkey", &Vars::Misc::Key.m_Var);
		km_slider_int("Freezing Ticks", &Vars::Misc::SequenceFreezingValue, 1, 150);

		km_checkbox("Teleport Exploit", &Vars::Misc::Teleport);
		km_text_with_keybind("Teleport Key");
		km_keybind("##tpkey", &Vars::Misc::TeleportKey.m_Var);

		km_checkbox("Third Person", &Vars::Misc::ThirdPerson);
		km_text_with_keybind("Third Person Key");
		km_keybind("##3pkey", &Vars::Misc::ThirdPersonKey.m_Var);
		km_slider_int("Cam Distance", &Vars::Misc::ThirdPersonDistance, 10, 500);
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Other", ImVec2(KM_CHILD_W, KM_CHILD_H), []()
	{
		km_checkbox("Watermark", &Vars::ESP::Watermark);
		km_checkbox("Unhide Dev Commands", &Vars::ESP::m_bUnhideCommands);
	});
}

// ============================================================================
//  Tab content renderers — Config
// ============================================================================

static void RenderConfigTab()
{
	static char s_configName[64] = "default";
	static int  s_selectedConfig = -1;
	static float s_statusTime = 0.f;
	static char s_statusMsg[128] = "";

	auto setStatus = [&](const char* msg) {
		strncpy(s_statusMsg, msg, sizeof(s_statusMsg) - 1);
		s_statusMsg[sizeof(s_statusMsg) - 1] = '\0';
		s_statusTime = (float)ImGui::GetTime();
	};

	static bool s_initedList = false;
	if (!s_initedList) {
		G::Config.Init();
		s_initedList = true;
	}

	ImGui::SetCursorPos(ImVec2(KM_LEFT_X, KM_TOP_Y));
	km_child("Config Management", ImVec2(KM_CHILD_W, KM_CHILD_H), [&]()
	{
		km_text("Config Name");
		km_input_text("##cfgname", "config name...", s_configName, sizeof(s_configName), 280.f, 22.f);
		ImGui::Dummy(ImVec2(0, 6));

		if (km_button("Save Config", ImVec2(280, 24))) {
			if (G::Config.Save(s_configName)) setStatus("Config saved.");
			else setStatus("Failed to save config.");
		}
		if (km_button("Load Config", ImVec2(280, 24))) {
			if (G::Config.Load(s_configName)) setStatus("Config loaded.");
			else setStatus("Failed to load config.");
		}
		if (km_button("Refresh List", ImVec2(280, 24))) {
			G::Config.RefreshList();
			setStatus("List refreshed.");
		}

		if (s_statusMsg[0] && (float)ImGui::GetTime() - s_statusTime < 3.f) {
			ImGui::Dummy(ImVec2(0, 4));
			km_text(s_statusMsg);
		}
	});

	ImGui::SetCursorPos(ImVec2(KM_RIGHT_X, KM_TOP_Y));
	km_child("Saved Configs", ImVec2(KM_CHILD_W, KM_CHILD_H), [&]()
	{
		const auto& configs = G::Config.GetConfigs();
		if (configs.empty()) {
			km_text("No saved configs.");
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.09f, 0.09f, 0.09f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
			for (int i = 0; i < (int)configs.size(); i++) {
				bool selected = (s_selectedConfig == i);
				if (ImGui::Selectable(configs[i].c_str(), selected)) {
					s_selectedConfig = i;
					strncpy(s_configName, configs[i].c_str(), sizeof(s_configName) - 1);
					s_configName[sizeof(s_configName) - 1] = '\0';
				}
			}
			ImGui::PopStyleColor(3);
		}
	});
}

// ============================================================================
//  Init
// ============================================================================

void Menu::Init(IDirect3DDevice9* pDevice) {
	if (isInitialized) return;

	ImGui::CreateContext();

	HWND hwWindow = FindWindowW(L"Valve001", nullptr);
	ImGui_ImplWin32_Init(hwWindow);
	ImGui_ImplDX9_Init(pDevice);

	ImGuiIO& io = ImGui::GetIO();

	// High-quality rasterization: 3x horizontal oversampling kills the blur the
	// old OversampleH=1 config produced, and a slight rasterizer multiply thickens
	// thin strokes so small text stays crisp.
	ImFontConfig font_config;
	font_config.OversampleH = 3;
	font_config.OversampleV = 3;
	font_config.PixelSnapH = false;
	font_config.RasterizerMultiply = 1.10f;

	// Legacy fonts (watermark / indicators) keep their own crisp config too.
	ImFontConfig legacy_config;
	legacy_config.OversampleH = 3;
	legacy_config.OversampleV = 3;
	legacy_config.PixelSnapH = false;

	static const ImWchar ranges[] = {
		0x0020, 0x00FF,
		0x0400, 0x044F,
		0,
	};

	verdanaFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 13.0f, &legacy_config, io.Fonts->GetGlyphRangesDefault());
	indicatorFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 14.0f, &legacy_config, io.Fonts->GetGlyphRangesDefault());

	// Menu fonts: Segoe UI for body/small, Segoe UI Semibold for titles/tabs.
	menuFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 14.0f, &font_config, ranges);
	menuFontSmall = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 12.0f, &font_config, ranges);
	menuFontBold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 15.0f, &font_config, ranges);
	if (!menuFontBold)
		menuFontBold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 15.0f, &font_config, ranges);

	io.Fonts->Build();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding    = 6.0f;
	style.FrameRounding     = 4.0f;
	style.GrabRounding      = 4.0f;
	style.PopupRounding     = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.ChildRounding     = 4.0f;
	style.TabRounding       = 4.0f;
	style.WindowPadding     = ImVec2(0, 0);
	style.ItemSpacing       = ImVec2(8, 6);
	style.ScrollbarSize     = 4.0f;
	style.WindowBorderSize  = 0.0f;
	style.ChildBorderSize   = 0.0f;

	style.Colors[ImGuiCol_WindowBg]       = ImVec4(16 / 255.f, 16 / 255.f, 16 / 255.f, 1.0f);
	style.Colors[ImGuiCol_ChildBg]        = ImVec4(16 / 255.f, 16 / 255.f, 16 / 255.f, 0.0f);
	style.Colors[ImGuiCol_FrameBg]        = ImVec4(13 / 255.f, 13 / 255.f, 13 / 255.f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(20 / 255.f, 20 / 255.f, 20 / 255.f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive]  = ImVec4(24 / 255.f, 24 / 255.f, 24 / 255.f, 1.0f);
	style.Colors[ImGuiCol_Border]         = ImVec4(11 / 255.f, 11 / 255.f, 11 / 255.f, 1.0f);
	style.Colors[ImGuiCol_Text]           = ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg]    = ImVec4(15 / 255.f, 15 / 255.f, 15 / 255.f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarGrab]  = ImVec4(40 / 255.f, 40 / 255.f, 40 / 255.f, 1.0f);

	isInitialized = true;
}

// ============================================================================
//  Render — kamibera two-panel layout
// ============================================================================

void Menu::Render(IDirect3DDevice9* pDevice) {
	if (!isInitialized) {
		Init(pDevice);
	}

	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	float dt = ImGui::GetIO().DeltaTime;
	float target = g_Menu.isOpen ? 1.0f : 0.0f;
	g_kmenu.m_opened = g_Menu.isOpen;
	g_kmenu.m_animation_progress = std::clamp(
		g_kmenu.m_animation_progress + (target - g_kmenu.m_animation_progress) * std::min(dt * g_kmenu.m_animation_speed, 1.0f),
		0.0f, 1.0f);

	ImGui::GetIO().MouseDrawCursor = g_Menu.isOpen;

	if (!g_Menu.isOpen && g_kmenu.m_animation_progress < 0.02f)
		return;

	const float progress = g_kmenu.m_animation_progress;
	const int a = int(255 * progress); // panel alpha follows open/close animation

	const float menu_w = 850.0f;
	const float menu_h = 582.0f;

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 center = io.DisplaySize * 0.5f;
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(menu_w, menu_h), ImGuiCond_Always);

	ImGui::Begin("##KamiberaMenu", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);
	{
		ImVec2 wp = ImGui::GetWindowPos();
		ImVec2 ws = ImGui::GetWindowSize();
		auto bg = ImGui::GetBackgroundDrawList();

		// Soft drop shadow: stacked low-alpha rounded rects behind the (opaque)
		// panels — the panels cover the interior so only the feathered edge shows.
		for (int i = 5; i >= 1; --i) {
			float o = i * 3.0f;
			int sa = int(12 * progress);
			bg->AddRectFilled(wp - ImVec2(o, o), wp + ws + ImVec2(o, o), ImColor(0, 0, 0, sa), 6.0f + o);
		}

		// Sidebar panel (200 wide)
		ImVec2 sb_min = wp;
		ImVec2 sb_max = wp + ImVec2(200, 582);
		bg->AddRectFilled(sb_min, sb_max, ImColor(15, 15, 15, a), 6.0f, ImDrawFlags_RoundCornersLeft);

		// Main content panel (650 wide)
		ImVec2 mp = wp + ImVec2(200, 0);
		ImVec2 mp_max = mp + ImVec2(650, 582);
		bg->AddRectFilled(mp, mp_max, ImColor(13, 13, 13, a), 6.0f, ImDrawFlags_RoundCornersRight);

		// Outer border + separator
		bg->AddRect(wp, wp + ws, ImColor(11, 11, 11, a), 6.0f, ImDrawFlags_RoundCornersAll, 1.0f);
		bg->AddRect(wp + ImVec2(1, 1), wp + ws - ImVec2(1, 1), ImColor(28, 28, 28, int(a * 0.5f)), 6.0f, ImDrawFlags_RoundCornersAll, 1.0f);
		bg->AddLine(wp + ImVec2(200, 12), wp + ImVec2(200, 570), ImColor(11, 11, 11, a), 1.0f);

		// Title + subtitle
		if (menuFontBold)
			bg->AddText(menuFontBold, menuFontBold->FontSize, wp + ImVec2(20, 15), ImColor(245, 245, 245, a), "lynette");
		if (menuFontSmall)
			bg->AddText(menuFontSmall, menuFontSmall->FontSize, wp + ImVec2(21, 33), ImColor(110, 110, 110, a), "l4d2");
		// Accent underline (gradient fade to the right)
		{
			ImVec2 ul0 = wp + ImVec2(20, 49);
			ImVec2 ul1 = wp + ImVec2(180, 50);
			ImU32 ca = GetAccentU32(0.9f * progress);
			ImU32 c0 = GetAccentU32(0.0f);
			bg->AddRectFilledMultiColor(ul0, ul1, ca, c0, c0, ca);
		}

		// Tabs (writes tab_number / subtab_number)
		static const std::vector<tab_t> tabs = {
			{ "Aimbot", { "General", "Anti-Aim" } },
			{ "Visuals", { "ESP", "Chams", "World" } },
			{ "Misc", { "Movement", "Exploits" } },
			{ "Config", {} }
		};
		custom_tabs(tabs);

		// Sidebar accent color picker (bottom)
		{
			auto* font = menuFontSmall ? menuFontSmall : ImGui::GetFont();
			ImVec2 accent_text_pos = wp + ImVec2(20, 552);
			bg->AddText(font, font->FontSize, accent_text_pos, ImColor(150, 150, 150, a), "Accent");

			ImVec2 sq_min = wp + ImVec2(150, 551);
			ImVec2 sq_max = sq_min + ImVec2(22, 15);
			bg->AddRectFilled(sq_min, sq_max, GetAccentU32(progress), 3.0f);
			bg->AddRect(sq_min, sq_max, IM_COL32(50, 50, 50, a), 3.0f, 0, 1.0f);

			ImRect sq_rect(sq_min, sq_max);
			if (sq_rect.Contains(io.MousePos) && ImGui::IsMouseClicked(0))
				ImGui::OpenPopup("##sidebar_accent_picker");
			km_color_picker_window("##sidebar_accent_picker", &Vars::Menu::AccentColor);
		}

		// Content
		ImGui::PushFont(menuFont);
		switch (tab_number) {
		case 0:
			if (subtab_number == 0)      RenderAimbot_General();
			else if (subtab_number == 1) RenderAimbot_AntiAim();
			break;
		case 1:
			if (subtab_number == 0)      RenderVisuals_ESP();
			else if (subtab_number == 1) RenderVisuals_Chams();
			else if (subtab_number == 2) RenderVisuals_World();
			break;
		case 2:
			if (subtab_number == 0)      RenderMisc_Movement();
			else if (subtab_number == 1) RenderMisc_Exploits();
			break;
		case 3:
			RenderConfigTab();
			break;
		}
		ImGui::PopFont();
	}
	ImGui::End();
}

void Menu::Shutdown() {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
