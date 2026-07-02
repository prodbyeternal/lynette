#include "CvarTools.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"

// We modify ConCommandBase::m_nFlags DIRECTLY instead of calling the virtual
// RemoveFlags(). The virtual layout in our SDK header does not match the real
// engine vtable, so calling RemoveFlags() through it triggers a calling-
// convention mismatch ("ESP was not properly saved") and crashes. m_nFlags is a
// public member at a stable offset, so a direct write is safe.
//
// To enumerate EVERY registered command we use the engine's official cvar
// Iterator (ICvar::Iterator -> FactoryInternalIterator), which now lands on the
// correct vtable slot after the ICvar header was fixed to match VEngineCvar007.
// (Walking m_pNext from a FindVar() anchor only reached part of the list, which
// is why some commands stayed hidden.)

void CCvarTools::UnhideAllCommands()
{
	if (!I::Cvars)
		return;

	ICvar::Iterator it(I::Cvars);
	for (it.SetFirst(); it.IsValid(); it.Next())
	{
		ConCommandBase* pCmd = it.Get();
		if (pCmd)
			pCmd->m_nFlags &= ~(FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY);
	}
}

void CCvarTools::RemoveCheatFlags()
{
	if (!I::Cvars)
		return;

	ICvar::Iterator it(I::Cvars);
	for (it.SetFirst(); it.IsValid(); it.Next())
	{
		ConCommandBase* pCmd = it.Get();
		if (pCmd)
			pCmd->m_nFlags &= ~FCVAR_CHEAT;
	}
}

bool CCvarTools::UnlockCvar(const char* name)
{
	if (!I::Cvars)
		return false;

	ConCommandBase* pCmd = I::Cvars->FindCommandBase(name);
	if (!pCmd)
		return false;

	pCmd->m_nFlags &= ~(FCVAR_CHEAT | FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED);
	return true;
}

void CCvarTools::EnsurePredictionEnabled()
{
	if (!I::Cvars || !I::EngineClient)
		return;

	// The prediction cvars the movement features depend on. cl_predict is the key
	// one; the others keep weapon state and lag-comp consistent while predicting.
	static const char* kPredictCvars[] = {
		"cl_predict",
		"cl_predictweapons",
		"cl_lagcompensation",
	};

	for (const char* name : kPredictCvars)
	{
		ConVar* cv = I::Cvars->FindVar(name);
		if (!cv)
			continue;

		// Unlock once: strip cheat/hidden so the value sticks without sv_cheats,
		// and allow ClientCmd to drive it (same approach as the fog/aspect code).
		cv->m_nFlags &= ~(FCVAR_HIDDEN | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY);
		cv->m_nFlags |= FCVAR_CLIENTCMD_CAN_EXECUTE;

		// Only issue the command when it isn't already enabled, so we don't spam
		// the console command buffer every frame.
		if (cv->GetInt() != 1)
		{
			char cmd[64];
			sprintf_s(cmd, "%s 1", name);
			I::EngineClient->ClientCmd(cmd);
		}
	}
}
