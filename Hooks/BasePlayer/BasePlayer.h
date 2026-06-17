#pragma once

#include "../../SDK/SDK.h"

namespace Hooks
{
	namespace BasePlayer
	{
		namespace CalcPlayerView
		{
			inline Hook::CFunction Func;
			using FN = void(__fastcall*)(C_BasePlayer*, void*, Vector&, Vector&, float&);

			void __fastcall Detour(C_BasePlayer* pThis, void* edx, Vector& eyeOrigin, Vector& eyeAngles, float& fov);
		}

		namespace CalcViewModelView
		{
			inline Hook::CFunction Func;
			using FN = void(__fastcall*)(void*, void*, int, Vector&, const Vector&);

			void __fastcall Detour(void* ecx, void* edx, int a2, Vector& eyePosition, const Vector& eyeAngles); // thanks to motrinyt on github for this offset and code
		}

		void Init();
	}
}