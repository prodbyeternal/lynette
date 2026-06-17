#pragma once

#include "../../SDK/SDK.h"

namespace Hooks
{
	namespace BaseAnimating
	{
		namespace GetClientModelRenderable
		{
			inline Hook::CFunction Func;
			using FN = IClientModelRenderable * (__fastcall*)(C_BaseAnimating*, void*);

			IClientModelRenderable* __fastcall Detour(C_BaseAnimating* pThis, void* edx);
		}

		void Init();
	}
}