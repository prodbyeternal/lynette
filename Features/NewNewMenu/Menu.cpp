#include "Menu.h"
#include "Header.h"
#include "babagui/imgui.h"
#include "babagui/imgui_impl_dx9.h"
#include "babagui/imgui_impl_win32.h"
#include "babagui/imgui_internal.h"
#include "../Vars.h"
#include "../Movement/Movement.h"
#include "../Config/Config.h"

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

// ============================================================================
//  Tab content renderers
//
//  Each menu tab (and its sub-tabs) lives in its own function so the per-tab
//  logic is grouped in one place instead of one giant Render() body. The
//  left/right MenuChild panels are kept in their respective sub-tab function.
// ============================================================================

// ---------------------------- Aimbot tab ----------------------------------

static void RenderAimbot_General(const ImVec4& main_color) {
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

		const char* aimModes[] = { "Always On", "Hold", "Toggle" };
		ImGui::Combo("Aim Mode", &Vars::Hitscan::AimMode, aimModes, IM_ARRAYSIZE(aimModes));

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

static void RenderAimbot_AntiAim(const ImVec4& main_color) {
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

static void RenderAimbotTab(const ImVec4& main_color) {
	ImGui::SetCursorPosY(54);
	ImGui::SetCursorPosX(30);
	if (ImGui::subtab("General", tab2 == 0)) tab2 = 0;

	ImGui::SetCursorPosY(79);
	ImGui::SetCursorPosX(30);
	if (ImGui::subtab("Anti-Aim", tab2 == 1)) tab2 = 1;

	if (tab2 == 0)      RenderAimbot_General(main_color);
	else if (tab2 == 1) RenderAimbot_AntiAim(main_color);
}

// ---------------------------- Visuals tab ----------------------------------

static void RenderVisuals_ESP(const ImVec4& main_color) {
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

static void RenderVisuals_Chams(const ImVec4& main_color) {
	// Chams
	ImGui::SetCursorPosY(38);
	ImGui::SetCursorPosX(122);
	ImGui::MenuChild("Chams Settings", ImVec2(226, 337), false);
	{
		ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

		const char* chamsMaterials[] = { "Solid", "Flat", "Wireframe", "Glow", "Shaded" };

		// Survivors
		ImGui::Checkbox("Survivors Chams", &Vars::Chams::Players);
		ImGui::Combo("Survivor Material", &Vars::Chams::SurvivorMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		ImGui::Checkbox("Survivor Through Walls", &Vars::Chams::SurvivorThroughWalls);
		ImGui::Separator();

		// Special Infected
		ImGui::Checkbox("SI Chams", &Vars::Chams::PlayerInfected);
		ImGui::Combo("SI Material", &Vars::Chams::SIMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		ImGui::Checkbox("SI Through Walls", &Vars::Chams::SIThroughWalls);
		ImGui::Separator();

		// Common Infected
		ImGui::Checkbox("CI Chams", &Vars::Chams::Infected);
		ImGui::Combo("CI Material", &Vars::Chams::CIMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
		ImGui::Checkbox("CI Through Walls", &Vars::Chams::CIThroughWalls);
		ImGui::Separator();

		// Local weapon
		ImGui::Checkbox("Local Weapon Chams", &Vars::Chams::ViewmodelGun);
		ImGui::Combo("Weapon Material", &Vars::Chams::GunMaterial, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));

		ImGui::PopStyleColor(2);

		float wCol[4];
		ColorToFloat4(Vars::Chams::ViewmodelGunColor, wCol);
		if (ImGui::ColorEdit4("Weapon Color", wCol, ImGuiColorEditFlags_NoInputs)) {
			Vars::Chams::ViewmodelGunColor = Float4ToColor(wCol);
		}

		float survCol[4], siCol[4], ciCol[4];
		ColorToFloat4(Vars::Chams::PlayerColor, survCol);
		ColorToFloat4(Vars::Chams::PlayerInfectedColor, siCol);
		ColorToFloat4(Vars::Chams::InfectedColor, ciCol);
		if (ImGui::ColorEdit4("Survivor Color", survCol, ImGuiColorEditFlags_NoInputs))
			Vars::Chams::PlayerColor = Float4ToColor(survCol);
		if (ImGui::ColorEdit4("SI Color", siCol, ImGuiColorEditFlags_NoInputs))
			Vars::Chams::PlayerInfectedColor = Float4ToColor(siCol);
		if (ImGui::ColorEdit4("CI Color", ciCol, ImGuiColorEditFlags_NoInputs))
			Vars::Chams::InfectedColor = Float4ToColor(ciCol);

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
		ImGui::Checkbox("Aspect Ratio Changer", &Vars::ESP::m_bAspectRatioChanger);
		ImGui::Checkbox("Flashlight FOV Changer", &Vars::ESP::m_bFlashlightFovChanger);
		ImGui::Checkbox("Unhide Dev Commands", &Vars::ESP::m_bUnhideCommands);
		ImGui::PopStyleColor(2);

		ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);
		ImGui::SliderFloat("View Fov", &Vars::ESP::m_fViewFov, 20.f, 150.f, "%.0f");
		ImGui::SliderFloat("Game Fov", &Vars::ESP::m_fFov, 20.f, 150.f, "%.0f");
		ImGui::SliderFloat("Aspect Ratio", &Vars::ESP::m_fAspectRatio, 0.0f, 2.0f, "%.1f");
		ImGui::SliderFloat("Flashlight FOV", &Vars::ESP::m_fFlashlightFov, 5.0f, 170.f, "%.0f");
		ImGui::PopStyleColor(2);

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::Checkbox("Custom Fog Color", &Vars::Fog::Enable);
		ImGui::Checkbox("Remove Fog", &Vars::Fog::RemoveFog);
		ImGui::PopStyleColor(2);

		float fogCol[4];
		ColorToFloat4(Vars::Fog::color, fogCol);
		if (ImGui::ColorEdit4("Fog Color", fogCol, ImGuiColorEditFlags_NoInputs))
			Vars::Fog::color = Float4ToColor(fogCol);

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::Checkbox("Grenade Trajectory", &Vars::Grenade::TrajectoryPrediction);
		ImGui::Checkbox("Molotov Range Visual", &Vars::Grenade::MolotovRangeVisual);
		ImGui::Checkbox("Projectile Model Color", &Vars::Grenade::ProjectileColorChanger);
		ImGui::Checkbox("Blood Color (particle)", &Vars::Grenade::BloodColorChanger);
		ImGui::PopStyleColor(2);

		ImGui::PushStyleColor(ImGuiCol_SliderGrab, main_color);
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, main_color);
		ImGui::SliderFloat("Molotov Range", &Vars::Grenade::MolotovRangeRadius, 30.f, 250.f, "%.0f");
		ImGui::PopStyleColor(2);

		float trajCol[4], bileCol[4], moloCol[4], rangeCol[4], bloodCol[4], spitCol[4];
		ColorToFloat4(Vars::Grenade::TrajectoryColor, trajCol);
		ColorToFloat4(Vars::Grenade::BileColor, bileCol);
		ColorToFloat4(Vars::Grenade::MolotovColor, moloCol);
		ColorToFloat4(Vars::Grenade::MolotovRangeColor, rangeCol);
		ColorToFloat4(Vars::Grenade::BloodColor, bloodCol);
		ColorToFloat4(Vars::Grenade::SpitterColor, spitCol);
		if (ImGui::ColorEdit4("Trajectory Color", trajCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::TrajectoryColor = Float4ToColor(trajCol);
		if (ImGui::ColorEdit4("Range Color", rangeCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::MolotovRangeColor = Float4ToColor(rangeCol);
		if (ImGui::ColorEdit4("Bile Jar Color", bileCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::BileColor = Float4ToColor(bileCol);
		if (ImGui::ColorEdit4("Molotov Color", moloCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::MolotovColor = Float4ToColor(moloCol);
		if (ImGui::ColorEdit4("Spitter Goo Color", spitCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::SpitterColor = Float4ToColor(spitCol);
		if (ImGui::ColorEdit4("Blood Color", bloodCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::BloodColor = Float4ToColor(bloodCol);

		float smokeCol[4];
		ColorToFloat4(Vars::Grenade::SmokerColor, smokeCol);
		if (ImGui::ColorEdit4("Smoker Smoke Color", smokeCol, ImGuiColorEditFlags_NoInputs))
			Vars::Grenade::SmokerColor = Float4ToColor(smokeCol);

		ImGui::PushStyleColor(ImGuiCol_CheckMark, main_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::Checkbox("Log Particle Names (debug)", &Vars::Grenade::LogParticleNames);
		ImGui::PopStyleColor(2);
	}
	ImGui::EndChild();
}

static void RenderVisualsTab(const ImVec4& main_color) {
	ImGui::SetCursorPosY(54);
	ImGui::SetCursorPosX(30);
	if (ImGui::subtab("ESP Options", tab2 == 0)) tab2 = 0;

	ImGui::SetCursorPosY(79);
	ImGui::SetCursorPosX(30);
	if (ImGui::subtab("Chams & Visuals", tab2 == 1)) tab2 = 1;

	if (tab2 == 0)      RenderVisuals_ESP(main_color);
	else if (tab2 == 1) RenderVisuals_Chams(main_color);
}

// ----------------------------- Misc tab ------------------------------------

static void RenderMisc_Movement(const ImVec4& main_color) {
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
		ImGui::Checkbox("Debug Path Visual", &Vars::Movement::EdgeBugDebugVisual);

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

static void RenderMisc_Exploits(const ImVec4& main_color) {
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

static void RenderMiscTab(const ImVec4& main_color) {
	ImGui::SetCursorPosY(54);
	ImGui::SetCursorPosX(30);
	if (ImGui::subtab("Movement", tab2 == 0)) tab2 = 0;

	ImGui::SetCursorPosY(79);
	ImGui::SetCursorPosX(30);
	if (ImGui::subtab("Exploits", tab2 == 1)) tab2 = 1;

	if (tab2 == 0)      RenderMisc_Movement(main_color);
	else if (tab2 == 1) RenderMisc_Exploits(main_color);
}

// ---------------------------- Config tab -----------------------------------

static void RenderConfigTab(const ImVec4& main_color) {
	static char s_configName[64] = "default";
	static int  s_selectedConfig = -1;
	static float s_statusTime = 0.f;
	static char s_statusMsg[128] = "";

	auto setStatus = [&](const char* msg) {
		strncpy(s_statusMsg, msg, sizeof(s_statusMsg) - 1);
		s_statusMsg[sizeof(s_statusMsg) - 1] = '\0';
		s_statusTime = (float)ImGui::GetTime();
	};

	// Ensure the config system is initialised and the list is fresh.
	static bool s_initedList = false;
	if (!s_initedList) {
		G::Config.Init();
		s_initedList = true;
	}

	// Config management panel
	ImGui::SetCursorPosY(38);
	ImGui::SetCursorPosX(122);
	ImGui::MenuChild("Config Management", ImVec2(226, 337), false);
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::Text("Config Name");
		ImGui::PushItemWidth(200.f);
		ImGui::InputText("##cfgname", s_configName, sizeof(s_configName));
		ImGui::PopItemWidth();
		ImGui::PopStyleColor();

		ImGui::Spacing();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, main_color);

		if (ImGui::Button("Save Config", ImVec2(200, 24))) {
			if (G::Config.Save(s_configName))
				setStatus("Config saved.");
			else
				setStatus("Failed to save config.");
		}
		if (ImGui::Button("Load Config", ImVec2(200, 24))) {
			if (G::Config.Load(s_configName))
				setStatus("Config loaded.");
			else
				setStatus("Failed to load config.");
		}
		if (ImGui::Button("Refresh List", ImVec2(200, 24))) {
			G::Config.RefreshList();
			setStatus("List refreshed.");
		}

		ImGui::PopStyleColor(2);

		// Status line (fades after a few seconds)
		if (s_statusMsg[0] && (float)ImGui::GetTime() - s_statusTime < 3.f) {
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(main_color), "%s", s_statusMsg);
		}
	}
	ImGui::EndChild();

	// Saved configs list
	ImGui::SetCursorPosY(38);
	ImGui::SetCursorPosX(358);
	ImGui::MenuChild("Saved Configs", ImVec2(224, 337), false);
	{
		const auto& configs = G::Config.GetConfigs();
		if (configs.empty()) {
			ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, 0.5f), "No saved configs.");
		}
		else {
			for (int i = 0; i < (int)configs.size(); i++) {
				bool selected = (s_selectedConfig == i);
				if (ImGui::Selectable(configs[i].c_str(), selected)) {
					s_selectedConfig = i;
					strncpy(s_configName, configs[i].c_str(), sizeof(s_configName) - 1);
					s_configName[sizeof(s_configName) - 1] = '\0';
				}
			}
		}
	}
	ImGui::EndChild();
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

	// Smoother UI feel: rounded widgets and built-in hover/active color transitions.
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding    = 4.0f;
	style.FrameRounding     = 4.0f;
	style.GrabRounding      = 4.0f;
	style.PopupRounding     = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.ChildRounding     = 4.0f;
	style.TabRounding       = 4.0f;
	// HoverFlashSpeed / colour-transition tuning so hovered widgets ease in/out
	// instead of snapping between states.
	style.Colors[ImGuiCol_FrameBgHovered].w = 0.55f;
	style.Colors[ImGuiCol_ButtonHovered].w  = 0.65f;

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

	// Smoothly animate the menu open/close. Opening eases in; closing is much
	// faster so the menu disappears almost immediately when you press Insert.
	float targetAlpha = g_Menu.isOpen ? 1.0f : 0.0f;
	bool closing = (targetAlpha < window_alpha);
	float animSpeed = ImGui::GetIO().DeltaTime * (closing ? 20.0f : 10.0f);
	animSpeed = animSpeed < 0.f ? 0.f : (animSpeed > 1.f ? 1.f : animSpeed);
	window_alpha += (targetAlpha - window_alpha) * animSpeed;
	// Snap to the endpoints quickly so we don't linger on a near-invisible window.
	if (window_alpha < 0.02f) window_alpha = 0.0f;
	if (window_alpha > 0.98f) window_alpha = 1.0f;

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
				if (ImGui::tab("Misc", tab == 2)) { tab = 2; tab2 = 0; } ImGui::SameLine();
				if (ImGui::tab("Config", tab == 3)) { tab = 3; tab2 = 0; }
				ImGui::EndGroup();
				ImGui::PopFont();
			}

			// Render active tab content. Each tab's widgets live in its own
			// function (see the "Tab content renderers" section above).
			{
				ImGui::PushFont(verdanaFont);

				if (tab == 0)      RenderAimbotTab(main_color);
				else if (tab == 1) RenderVisualsTab(main_color);
				else if (tab == 2) RenderMiscTab(main_color);
				else if (tab == 3) RenderConfigTab(main_color);

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
