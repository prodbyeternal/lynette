#include "BaseAnimating.h"

using namespace Hooks;

IClientModelRenderable* __fastcall BaseAnimating::GetClientModelRenderable::Detour(C_BaseAnimating* pThis, void* edx)
{
	//This commented example below is one way to make infected go trough DME.

	if (pThis)
	{
		ClientClass* pCC = pThis->GetClientClass();

		if (pCC && (pCC->m_ClassID == Infected))
			return NULL;
	}


	return Func.Original<FN>()(pThis, edx);
}

void BaseAnimating::Init()
{

	//GetClientModelRenderable
	{
		using namespace GetClientModelRenderable;

		const FN pfGetClientModelRenderable = reinterpret_cast<FN>(U::Offsets.m_dwGetClientModelRenderable);
		XASSERT(pfGetClientModelRenderable == 0x0);

		if (pfGetClientModelRenderable)
			XASSERT(Func.Init(pfGetClientModelRenderable, &Detour) == false);
	}
}