#include "ClientMode.h"

#include "../../Features/Vars.h"
#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Hitscan/Hitscan.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Sequence Freezing/SequenceFreezing.h"
#include "../../Features/AutoShove/AutoShove.h"
#include "../../Features/Anti Aim/Anti Aim.h"
#include "../../Features/Movement/MovementPrediction.h"
#include "../../Features/Movement/Movement.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
using namespace Hooks;

bool __fastcall ClientMode::ShouldDrawFog::Detour(void* ecx, void* edx)
{
	return Table.Original<FN>(Index)(ecx, edx);
}

bool __fastcall ClientMode::CreateMove::Detour(void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd)
{
	if (!cmd || !cmd->command_number)
		return Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd);

	if (Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd))
		I::Prediction->SetLocalViewAngles(cmd->viewangles);

	DWORD pep;
	__asm mov pep, ebp;
	bool* BSendPacket = reinterpret_cast<bool*>(*reinterpret_cast<char**>(pep) - 0x1C);

	auto* pEnt = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	C_TerrorPlayer* pLocal = pEnt ? pEnt->As<C_TerrorPlayer*>() : nullptr;

	if (pLocal && !pLocal->deadflag())
	{
		auto* pWpnEnt = pLocal->GetActiveWeapon();
		C_TerrorWeapon* pWeapon = pWpnEnt ? pWpnEnt->As<C_TerrorWeapon*>() : nullptr;

		if (pWeapon)
		{
			// Initialize movement feature globals
			l4d2::local = pLocal;
			l4d2::cmd = cmd;
			static auto Sv_gravity = I::Cvars->FindVar("sv_gravity");
			l4d2::half_gravity_per_tick = -(((Sv_gravity->GetFloat()) / 2.f) * I::GlobalVars->interval_per_tick);

			Vector SavedViewAngles = cmd->viewangles;

			// 1. Update prediction state
			Prediction::Update();

			// 2. Bunnyhop (pre-prediction)
			Movement::Bhop();

			if (Vars::Misc::Teleport)
			{
				if ((GetAsyncKeyState(Vars::Misc::TeleportKey.m_Var)) < 0) { // AirStuck with ViewAngles reset (((Teleport to 0, 0, 0))) (Reversed from some L4D2 hack)
					cmd->viewangles.x = 3.4028235e38f;
					cmd->viewangles.y = 3.4028235e38f;
					cmd->viewangles.z = 3.4028235e38f;
					cmd->upmove = 3.4028235e38f;
					cmd->forwardmove = 3.4028235e38f;
					cmd->sidemove = 3.4028235e38f;
				}
			}
			f::misc->run(pLocal, cmd);

			// 3. Sim single prediction step for EdgeJump / LongJump / MiniJump flags check
			// Only run when at least one edge feature is active (it's expensive)
			if (Vars::Movement::bEdgeJump || Vars::Movement::bLongJump || Vars::Movement::bMiniJump)
			{
				Prediction::Begin(cmd);
				Prediction::Finish();
			}

			// 4. EdgeJump, LongJump, and MiniJump
			Movement::EdgeJump();
			Movement::LongJump();
			Movement::MiniJump();

			// 5. Engine prediction and rage features
			F::EnginePrediction.Start(pLocal, cmd);
			{
				f::SequenceFreezing.Run(cmd, pLocal);
				f::hitscan.run(pLocal, cmd);
				f::autoshove.run(pLocal, cmd);
				F::NoSpread.Run(pLocal, pWeapon, cmd);
				f::aa.run(cmd, BSendPacket);
			}
			F::EnginePrediction.Finish(pLocal, cmd);

			// 6. Fix movement orientation and execute EdgeBug scanning
			Movement::FixMovement(SavedViewAngles);
			Movement::EdgeBug();

			viewangle = cmd->viewangles;
		}
	}

	return false;
}

void __fastcall ClientMode::DoPostScreenSpaceEffects::Detour(void* ecx, void* edx, const void* pSetup)
{
	Table.Original<FN>(Index)(ecx, edx, pSetup);
}

bool OnGetViewModelFOV(float& fov)
{
	if (!Vars::ESP::m_bFovChanger)
		return false;

	C_TerrorPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<C_TerrorPlayer*>();
	if (pLocal == nullptr || !pLocal->IsAlive())
		return false;

	fov = Vars::ESP::m_fViewFov;
	return true;
}
float __fastcall ClientMode::GetViewModelFOV::Detour(void* ecx, void* edx)
{
	bool replace = false;
	float fov = Table.Original<FN>(Index)(ecx, edx);

	if (OnGetViewModelFOV(fov))
		replace = true;

	if (replace)
		return fov;

	return Table.Original<FN>(Index)(ecx, edx);
}
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../Features/CvarTools/CvarTools.h"
void __fastcall ClientMode::OverrideView::Detour(void* ecx, void* edx, CViewSetup* View)
{

	if (Vars::ESP::m_bFovChanger)
		View->fov = Vars::ESP::m_fFov;

	// Unhide developer/hidden console commands (runs once on the toggle's rising edge).
	{
		static bool prevUnhide = false;
		if (Vars::ESP::m_bUnhideCommands && !prevUnhide)
			G::CvarTools.UnhideAllCommands();
		prevUnhide = Vars::ESP::m_bUnhideCommands;
	}

	// Aspect ratio changer: r_aspectratio is a hidden cvar that SetValue() doesn't
	// reliably apply (the engine reads it through a different path), so instead we
	// send it as a real console command. We unlock the flag first so the command
	// is allowed, and only re-send when the value changes.
	{
		static ConVar* r_aspectratio = nullptr;
		static bool unlocked = false;
		if (!r_aspectratio)
			r_aspectratio = I::Cvars->FindVar("r_aspectratio");
		if (r_aspectratio && !unlocked)
		{
			// Allow the basic ClientCmd to execute this cvar, and strip cheat/hidden.
			r_aspectratio->m_nFlags &= ~(FCVAR_HIDDEN | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);
			r_aspectratio->m_nFlags |= FCVAR_CLIENTCMD_CAN_EXECUTE;
			unlocked = true;
		}

		static float lastApplied = -999.f;
		if (Vars::ESP::m_bAspectRatioChanger)
		{
			if (Vars::ESP::m_fAspectRatio != lastApplied)
			{
				// Use the basic ClientCmd (early, correctly-aligned vtable slot).
				// ClientCmd_Unrestricted sits far later in the vtable and is
				// misaligned in this SDK (it crashed). SetValue applied the value
				// but not through the path that updates the renderer, so the change
				// didn't show — running it as a real console command does.
				char cmd[64];
				sprintf_s(cmd, "r_aspectratio %.3f", Vars::ESP::m_fAspectRatio);
				I::EngineClient->ClientCmd(cmd);
				lastApplied = Vars::ESP::m_fAspectRatio;
			}
		}
		else if (lastApplied != 0.f)
		{
			I::EngineClient->ClientCmd("r_aspectratio 0");
			lastApplied = 0.f;
		}
	}

	// Flashlight FOV changer: r_flashlightfov is cheat-protected; strip the flag
	// so it works without sv_cheats, and only write on change.
	{
		static ConVar* r_flashlightfov = nullptr;
		static bool unlocked = false;
		if (!r_flashlightfov)
			r_flashlightfov = I::Cvars->FindVar("r_flashlightfov");

		if (r_flashlightfov)
		{
			if (!unlocked)
			{
				r_flashlightfov->m_nFlags &= ~(FCVAR_HIDDEN | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);
				r_flashlightfov->m_nFlags |= FCVAR_CLIENTCMD_CAN_EXECUTE;
				unlocked = true;
			}

			static float lastFov = -1.f;
			if (Vars::ESP::m_bFlashlightFovChanger && Vars::ESP::m_fFlashlightFov != lastFov)
			{
				char cmd[64];
				sprintf_s(cmd, "r_flashlightfov %.1f", Vars::ESP::m_fFlashlightFov);
				I::EngineClient->ClientCmd(cmd);
				lastFov = Vars::ESP::m_fFlashlightFov;
			}
		}
	}

	// Fog color override. Uses non-virtual ConVar::SetValue (resolved against our
	// own CvarsB.cpp). ClientCmd_Unrestricted's engine vtable slot is misaligned in
	// this SDK and crashed, so it must not be used.
	{
		static ConVar* fog_override = nullptr;
		static ConVar* fog_color = nullptr;
		static ConVar* fog_colorskybox = nullptr;
		static ConVar* fog_enable = nullptr;
		static bool unlocked = false;
		if (!fog_override)
		{
			fog_override    = I::Cvars->FindVar("fog_override");
			fog_color       = I::Cvars->FindVar("fog_color");
			fog_colorskybox = I::Cvars->FindVar("fog_colorskybox");
			fog_enable      = I::Cvars->FindVar("fog_enable");
		}
		if (!unlocked)
		{
			int strip = FCVAR_HIDDEN | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY;
			ConVar* all[] = { fog_override, fog_color, fog_colorskybox, fog_enable };
			for (ConVar* cv : all)
			{
				if (cv)
				{
					cv->m_nFlags &= ~strip;
					cv->m_nFlags |= FCVAR_CLIENTCMD_CAN_EXECUTE;
				}
			}
			unlocked = true;
		}

		static int lastState = -1; // -1 unknown, 0 off, 1 color, 2 remove
		static Color lastColor = { 0, 0, 0, 0 };

		int desired = Vars::Fog::RemoveFog ? 2 : (Vars::Fog::Enable ? 1 : 0);

		if (desired == 1)
		{
			bool colorChanged = !(lastColor == Vars::Fog::color);
			if (lastState != 1 || colorChanged)
			{
				char cmd[96];
				I::EngineClient->ClientCmd("fog_override 1");
				I::EngineClient->ClientCmd("fog_enable 1");
				sprintf_s(cmd, "fog_color %d %d %d", Vars::Fog::color.r(), Vars::Fog::color.g(), Vars::Fog::color.b());
				I::EngineClient->ClientCmd(cmd);
				sprintf_s(cmd, "fog_colorskybox %d %d %d", Vars::Fog::color.r(), Vars::Fog::color.g(), Vars::Fog::color.b());
				I::EngineClient->ClientCmd(cmd);
				lastState = 1;
				lastColor = Vars::Fog::color;
			}
		}
		else if (desired == 2)
		{
			if (lastState != 2)
			{
				I::EngineClient->ClientCmd("fog_override 1");
				I::EngineClient->ClientCmd("fog_enable 0");
				lastState = 2;
			}
		}
		else
		{
			if (lastState != 0)
			{
				I::EngineClient->ClientCmd("fog_override 0");
				lastState = 0;
			}
		}
	}

	Table.Original<FN>(Index)(ecx, edx, View);
}
void ClientMode::Init()
{
	XASSERT(Table.Init(I::ClientMode) == false);
	XASSERT(Table.Hook(&ShouldDrawFog::Detour, ShouldDrawFog::Index) == false);
	XASSERT(Table.Hook(&CreateMove::Detour, CreateMove::Index) == false);
	XASSERT(Table.Hook(&DoPostScreenSpaceEffects::Detour, DoPostScreenSpaceEffects::Index) == false);
	XASSERT(Table.Hook(&GetViewModelFOV::Detour, GetViewModelFOV::Index) == false);
	XASSERT(Table.Hook(&OverrideView::Detour, OverrideView::Index) == false);
}