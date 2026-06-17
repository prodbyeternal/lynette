#pragma once

#include "MovementPrediction.h"
#include "../../Util/Offsets/Offsets.h"

C_BasePlayer** Prediction_Player = nullptr;

void Prediction::Update()
{
	Prediction::BackupPrediction();

	// Map client_state fields via our offset scanning or direct interfaces if available.
	// Since engine client_state isn't fully exposed as a public global in SDK.h, 
	// we will map directly to what we can. 
	// If needed, we can also query the engine client to perform standard checks,
	// or fallback to calling Prediction->Update if we signature scan the engine client state pointer.
	// Let's signature scan for engine client state to get the delta ticks, similar to how the movement code does.
	struct CClientState
	{
		std::byte pad0[0x10];
		int m_nDeltaTick;
		std::byte pad1[0x4C];
		int lastoutgoingcommand;
		int chokedcommands;
		int last_command_ack;
	};

	static CClientState* client_state = nullptr;
	if (!client_state)
	{
		client_state = *reinterpret_cast<CClientState**>(U::Pattern.Find("engine.dll", "A1 ? ? ? ? 83 C0 08 C3") + 1);
	}

	if (client_state)
	{
		const bool valid = client_state->m_nDeltaTick > 0;
		if (valid)
		{
			I::Prediction->Update(
				client_state->m_nDeltaTick, 
				valid, 
				client_state->last_command_ack, 
				client_state->lastoutgoingcommand + client_state->chokedcommands
			);
		}
	}
}

int Prediction::GetTickBase(CUserCmd* cmd)
{
	static int s_nTick = 0;
	static CUserCmd* s_pLastCommand = nullptr;

	if (cmd && l4d2::local)
	{
		if (!s_pLastCommand || s_pLastCommand->hasbeenpredicted)
			s_nTick = l4d2::local->m_nTickBase();
		else
			s_nTick++;

		s_pLastCommand = cmd;
	}

	return s_nTick;
}

void UpdateButtonState(CUserCmd* cmd)
{
	if (!l4d2::local)
		return;

	const int buttons = cmd->buttons;
	const int local_buttons = l4d2::local->m_nButtons();
	const int buttons_changed = buttons ^ local_buttons;

	// In L4D2 SDK, m_afButtonLast, m_afButtonPressed, m_afButtonReleased are inline methods or direct offsets.
	// Let's verify that the base player has direct accessors.
	// If m_afButtonLast/m_afButtonPressed/m_afButtonReleased are not standard netvars in the target project,
	// we can compute them directly or write them to the local player offsets.
	// Let's check if the target project defines player button state variables. 
	// Usually they are located at standard offsets:
	// m_nButtons = 0xE4 (or m_nButtons netvar), m_afButtonLast = m_nButtons - 4, m_afButtonPressed = m_nButtons + 4, m_afButtonReleased = m_nButtons + 8
	// Let's map them safely via offsets or cast.
	int* pButtons = const_cast<int*>(&(l4d2::local->m_nButtons()));
	if (pButtons)
	{
		int* pButtonLast = pButtons - 1;
		int* pButtonPressed = pButtons + 1;
		int* pButtonReleased = pButtons + 2;

		*pButtonLast = local_buttons;
		*pButtons = buttons;
		*pButtonPressed = buttons_changed & buttons;
		*pButtonReleased = buttons_changed & (~buttons);
	}
}

void Prediction::Begin(CUserCmd* cmd)
{
	if (!I::MoveHelper || !l4d2::local)
		return;

	// Set current command offset: usually m_pCurrentCommand is at m_nButtons - 12 (or similar)
	// Let's locate and assign it safely.
	// In the target SDK, if there is a helper to set prediction random seed, we use it.
	int* pButtons = const_cast<int*>(&(l4d2::local->m_nButtons()));
	if (pButtons)
	{
		CUserCmd** ppCurrentCommand = reinterpret_cast<CUserCmd**>(pButtons - 3);
		*ppCurrentCommand = cmd;
	}

	if (static bool once = false; !once) 
	{
		Prediction_Player = *reinterpret_cast<C_BasePlayer***>(U::Pattern.Find("client.dll", "89 35 ? ? ? ? 0F 57 C0 F3 0F 2A 86 ? ? ? ? F3 0F 59 41 ?") + 2);
		Prediction_RandomSeed = *reinterpret_cast<unsigned int**>(U::Pattern.Find("client.dll", "A1 ? ? ? ? 53 56 57 8B 7D 14 8D 4D 14") + 1);
		once = true;
	}

	InPrediction = true;

	if (Prediction_RandomSeed && cmd)
		*Prediction_RandomSeed = cmd->random_seed;
	if (Prediction_Player)
		*Prediction_Player = l4d2::local;

	m_flOldCurTime = I::GlobalVars->curtime;
	m_flOldFrameTime = I::GlobalVars->frametime;
	m_nOldTickCount = I::GlobalVars->tickcount;

	const int nTickBase = l4d2::local->m_nTickBase();
	
	// Cast IPrediction to our custom mapping to access internal prediction status members
	auto prediction_custom = reinterpret_cast<CPrediction_Custom*>(I::Prediction);
	const bool Old_FirstTimePredicted = prediction_custom->m_Split->m_bFirstTimePredicted;
	const bool Old_InPrediction = prediction_custom->m_bInPrediction;

	I::GlobalVars->curtime = TICKS_TO_TIME(nTickBase);
	I::GlobalVars->frametime = TICK_INTERVAL;
	I::GlobalVars->tickcount = nTickBase;

	// In L4D2 SDK, forced and disabled buttons are usually at m_nButtons + 12 and m_nButtons + 16 respectively.
	if (pButtons)
	{
		int buttonForced = *(pButtons + 3);
		int buttonDisabled = *(pButtons + 4);
		cmd->buttons |= buttonForced;
		cmd->buttons &= ~buttonDisabled;
	}

	I::MoveHelper->SetHost(l4d2::local);

	// StartTrackPredictionErrors
	using StartTrackPredictionErrorsFn = void(__thiscall*)(void*, C_BasePlayer*);
	static auto StartTrackPredictionErrors = reinterpret_cast<StartTrackPredictionErrorsFn>(U::Pattern.Find("client.dll", "55 8B EC 56 8B F1 8B 0D ? ? ? ? 57"));
	if (StartTrackPredictionErrors)
	{
		StartTrackPredictionErrors(I::GameMovement, l4d2::local);
	}

	if (cmd->impulse)
	{
		// Set impulse: usually m_nImpulse is at m_nButtons + 20
		if (pButtons)
		{
			int* pImpulse = pButtons + 5;
			*pImpulse = cmd->impulse;
		}
	}

	UpdateButtonState(cmd);

	I::Prediction->SetLocalViewAngles(cmd->viewangles);

	I::Prediction->SetupMove(l4d2::local, cmd, I::MoveHelper, &mv);
	I::GameMovement->ProcessMovement(l4d2::local, &mv);
	I::Prediction->FinishMove(l4d2::local, cmd, &mv);

	prediction_custom->m_bInPrediction = Old_InPrediction;
	prediction_custom->m_Split->m_bFirstTimePredicted = Old_FirstTimePredicted;
}

void Prediction::Finish()
{
	if (!I::MoveHelper || !l4d2::local)
		return;

	// FinishTrackPredictionErrors
	using FinishTrackPredictionErrorsFn = void(__thiscall*)(void*, C_BasePlayer*);
	static auto FinishTrackPredictionErrors = reinterpret_cast<FinishTrackPredictionErrorsFn>(U::Pattern.Find("client.dll", "55 8B EC 56 8B F1 8B 0D ? ? ? ? 57 ? ? ? ? ? ? 75 14"));
	if (FinishTrackPredictionErrors)
	{
		FinishTrackPredictionErrors(I::GameMovement, l4d2::local);
	}

	I::MoveHelper->SetHost(nullptr);

	InPrediction = false;

	I::GlobalVars->curtime = m_flOldCurTime;
	I::GlobalVars->frametime = m_flOldFrameTime;
	I::GlobalVars->tickcount = m_nOldTickCount;

	if (Prediction_RandomSeed)
		*Prediction_RandomSeed = -1;
	if (Prediction_Player)
		*Prediction_Player = nullptr;

	I::GameMovement->Reset();
}

void Prediction::RestoreEntityToPredictedFrame(int i)
{
	using RestoreEntityToPredictedFramefn = void(__stdcall*)(int, int);
	static auto RestoreEntityToPredictedFrame = reinterpret_cast<RestoreEntityToPredictedFramefn>(U::Pattern.Find("client.dll", "55 8B EC 53 8B 5D 08 53 E8 ? ? ? ?"));
	if (RestoreEntityToPredictedFrame)
	{
		RestoreEntityToPredictedFrame(0, i);
	}
}
