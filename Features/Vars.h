#pragma once
#include "../SDK/L4D2/Includes/color.h"
#include "../SDK/SDK.h"
template <class T>
class CVar
{
public:
	T m_Var;
	const wchar_t* m_szDisplayName;
};

namespace Vars
{
	namespace Menu
	{
		inline bool Opened = false;
		//inline Rect_t Position = { 100, 100, 870, 480 };
		//inline const auto Title = L"Team Fortress 2";
		inline int TitleBarH = 16;

		inline int ShadowSize = 128;

		inline int SpacingX = 8;
		inline int SpacingY = 4;
		inline int SpacingText = 4;

		inline int CheckBoxW = 10;
		inline int CheckBoxH = 10;
		inline int CheckBoxFillSize = 4;

		inline int ButtonTabW = 198;
		inline int ButtonTabH = 25;

		inline int ButtonW = 90;
		inline int ButtonH = 20;

		inline int ButtonWSmall = 70;
		inline int ButtonHSmall = 16;

		inline int ComboBoxW = 90;
		inline int ComboBoxH = 14;

		inline int InputBoxW = 90;
		inline int InputBoxH = 14;
		inline int InputColorBoxW = 30;


		namespace Colors
		{
			//inline Color_t WindowBackground = { 30, 30, 30, 255 };
			inline Color Text = { 255, 255, 255, 190 };
			inline Color Widget = { 255, 255, 255, 190 };
			inline Color WidgetActive = { 240, 137, 229, 255 };
			inline Color ShadowColor = { 240, 137, 229, 255 };
			inline Color OutlineMenu = { 50, 50, 50, 255 };
		}
	}
	namespace Hitscan {
		inline bool bEnable = false;
		inline bool bAutoShoot = false;
		inline bool bKey = false;
		inline bool bSilentAim = false;
		inline int  iFov = 30;
		inline int  MaxPlayers = 2;
		inline int  iHitbox = 0; // 0 - Auto 1 - Head 2 - Body 3 - Pelvis 
		inline int  AimMode = 1; // 0 - Always On, 1 - Hold, 2 - Toggle
		//inline int  Key; // R.
		inline CVar<int> AimKey{ VK_LSHIFT, L"Aim Key" };
		inline bool MultiPointIfNoHitboxesAreVisible = false;
		inline bool AimFovCircle = false;
		inline Color AimFov = {255, 255, 255, 255};
	}
	namespace Exploits {
		inline bool Speedhack = false;
		inline CVar<int> Key{ VK_CONTROL, L"SpeedHack" };
		inline int SpeedHackValue = 5;
	}
	namespace Misc {
		inline bool Namestealer = false;
		inline bool AutoStrafe = false;
		inline bool SequenceFreezing = false;
		inline bool DisableInterp = false;
		inline int SequenceFreezingValue = 1;
		//inline int  SequenceFreezingKey = 8; // R.
		inline CVar<int> Key{ VK_MENU, L"Sequence Freezing" };
		inline bool AutoShove = false;
		inline bool ThirdPerson = false;
		inline int ThirdPersonDistance = 100;
		inline CVar<int> ThirdPersonKey{ VK_B, L"Third Person" };
		inline bool Teleport = false;
		inline CVar<int> TeleportKey{ VK_P, L"Teleport Key" };

	}
	namespace SkyBox
	{
		inline bool Enable = false;
		inline bool RainbowSkyBox = false;
		inline Color color = { 240, 137, 229, 255 };
	}
	namespace Fog
	{
		inline bool Enable = false;          // override fog color
		inline Color color = { 120, 120, 140, 255 };
		inline bool RemoveFog = false;       // disable fog entirely
	}
	namespace Grenade
	{
		inline bool TrajectoryPrediction = false; // draw throw arc for thrown items
		inline Color TrajectoryColor = { 210, 100, 185, 255 };

		// Projectile color changer (boomer bile / molotov fire chams)
		inline bool ProjectileColorChanger = false;
		inline Color BileColor = { 0, 255, 0, 255 };
		inline Color MolotovColor = { 255, 100, 0, 255 };
		inline Color SpitterColor = { 180, 255, 0, 255 };

		// Molotov fire range visual (ground circle at predicted landing)
		inline bool MolotovRangeVisual = false;
		inline float MolotovRangeRadius = 150.f; // approximate inferno spread radius
		inline Color MolotovRangeColor = { 255, 80, 0, 200 };

		// Blood color changer (particle-based; see note in code)
		inline bool BloodColorChanger = false;
		inline Color BloodColor = { 255, 0, 0, 255 };
	}
	namespace Removals {
		inline bool NoSpread = false;
		inline bool NoRecoil = false;
		inline bool NoVisualRecoil = false; 
		inline bool BoomerVisual = false;
	}
	namespace ESP {
		inline bool Enabled = false;
		inline bool players = false;
		inline bool infected = false;
		inline bool specialinfected = false;
		inline bool Box = false;
		inline bool Weapons = false;
		inline bool Minigun = false;
		inline bool Ammo = false;
		inline bool Healthbar = false;
		inline bool EnemyOnly = true;
		inline bool Name = false;
		inline bool LocalESP = false;

		inline bool ViewModelChanger = false;
		inline float viewmodel_x = 1.0f;
		inline float viewmodel_y = 1.0f;
		inline float viewmodel_z = 0.0f;
		inline float viewmodel_roll = 0.0f;

		inline bool m_bFovChanger = false;
		inline float m_fViewFov = 50.0f;
		inline float m_fFov = 90.0f;

		// Aspect ratio changer (drives r_aspectratio, 0.0 - 2.0)
		inline bool m_bAspectRatioChanger = false;
		inline float m_fAspectRatio = 0.0f;

		// Flashlight FOV changer (drives r_flashlightfov, bypasses sv_cheats)
		inline bool m_bFlashlightFovChanger = false;
		inline float m_fFlashlightFov = 45.0f;

		// Unlock hidden/developer console commands
		inline bool m_bUnhideCommands = false;

		inline Color PlayerColor = { 0, 255, 0, 255 };
		inline Color PlayerInfectedColor = { 255, 255, 0, 255 };
		inline Color InfectedColor = { 0, 255, 255, 255 };

		// Glow ESP variables
		inline bool GlowSurvivors = false;
		inline bool GlowSI = false;
		inline bool GlowCI = false;
		inline bool GlowWitch = false;

		// Velocity Speedometer & Graph Variables
		inline bool VelocityHUD = false;
		inline bool VelocityGraph = false;
		inline float VelocityX = 0.5f;
		inline float VelocityY = 0.85f;
	}
	namespace Chams
	{
		inline bool Players = false;
		inline bool PlayerInfected = false;
		inline bool Infected = false;
		inline bool ViewmodelGun = false;
		inline Color ViewmodelGunColor = { 240, 137, 229, 255 };
		inline int  ViewmodelMaterial = 0; // legacy global material (kept for config compat)
		inline bool ThroughWalls = false;

		// Per-category material selectors.
		// 0 - Solid, 1 - Flat, 2 - Wireframe, 3 - Glow/Additive, 4 - Shaded (normal2)
		inline int  SurvivorMaterial = 0;
		inline int  SIMaterial = 0;
		inline int  CIMaterial = 0;
		inline int  GunMaterial = 0;

		// Per-category through-walls toggles.
		inline bool SurvivorThroughWalls = false;
		inline bool SIThroughWalls = false;
		inline bool CIThroughWalls = false;

		inline Color PlayerColor = { 0, 255, 0, 255 };
		inline Color PlayerInfectedColor = { 255, 255, 0, 255 };
		inline Color InfectedColor = { 0, 255, 255, 255 };
	}
	namespace Ignore {
		inline bool IgnoreWitchUntilStartled = true;
		inline bool IgnoreCommonInfected = false;
		inline bool IgnoreFriends = true;
		inline bool IgnoreTeammates = true;
	}
	namespace HvH {
		inline bool Enabled = false;
		inline bool AntiAim = false;

		inline bool FakeAngle = false;
		inline float FakeOffset = 0.f;

		inline int RealYaw = 0;
		inline int PitchMode = 0;
		inline float SpinSpeed = 0.f;
		inline float DesyncOffset = 0.f;
	}
	namespace DT
	{
		inline bool Enable = false;
		inline bool Recharging = false;
		inline bool shifting = false;
		inline int ticks = 0;
		inline int Shifted = 0;
	}
	namespace Movement
	{
		inline bool bBhop = false;
		inline bool bEdgeJump = false;
		inline int kEdgeJump = 0;
		inline bool bLongJump = false;
		inline int kLongJump = 0;
		inline bool bMiniJump = false;
		inline int kMiniJump = 0;
		inline bool bEdgeBug = false;
		inline int kEdgeBug = 0;
		inline int EdgeBugTicks = 12;
		inline bool ExtendedEdgeBugPaths = false;
		inline int EdgeBugPaths = 2;
		inline float EdgeBugAngle = 15.f;
		inline bool bLAN = true;
		inline bool bCSGameMovement = false;
		inline bool EdgeBugNoDamage = false;
		inline bool EdgeBugDebugVisual = false; // draw the predicted edgebug path in-world
		inline float fAiraccelerate = 10.f;
	}
}