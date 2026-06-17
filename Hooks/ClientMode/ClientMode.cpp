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

	C_TerrorPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<C_TerrorPlayer*>();

	if (pLocal && !pLocal->deadflag())
	{
		C_TerrorWeapon* pWeapon = pLocal->GetActiveWeapon()->As<C_TerrorWeapon*>();

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

			// 3. Sim single prediction step for EdgeJump / LongJump flags check
			Prediction::Begin(cmd);
			Prediction::Finish();

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
void __fastcall ClientMode::OverrideView::Detour(void* ecx, void* edx, CViewSetup* View)
{

	if (Vars::ESP::m_bFovChanger)
		View->fov = Vars::ESP::m_fFov;

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