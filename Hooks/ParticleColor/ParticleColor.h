#pragma once

#include "../../SDK/SDK.h"

// -----------------------------------------------------------------------------
// Particle recolor hook.
//
// Hooks CParticleCollection::Simulate (a non-virtual client.dll function found by
// pattern scan) and, after the engine has simulated a frame, rewrites the
// per-particle TINT_RGB attribute for systems whose name matches a target
// (molotov fire, boomer bile, blood).
//
// All struct offsets were verified against this build's client.dll in IDA (see
// ParticleColor.cpp). Every memory access is guarded so a wrong offset no-ops
// instead of crashing.
// -----------------------------------------------------------------------------

namespace Hooks
{
	namespace ParticleColor
	{
		inline Hook::CFunction Func;

		// CParticleCollection::Simulate(float dt) — __thiscall
		using FN = void(__fastcall*)(void* ecx, void* edx, float dt);
		void __fastcall Detour(void* ecx, void* edx, float dt);

		void Init();
	}
}
