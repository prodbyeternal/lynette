#include "CvarTools.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"

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
