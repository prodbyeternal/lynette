#include "ParticleColor.h"
#include "../../Features/Vars.h"
#include <cstring>
#include <string>
#include <unordered_set>
#include <iostream>

using namespace Hooks;

// ===========================================================================
// CParticleCollection layout — VERIFIED against this build's client.dll via IDA
// (CParticleCollection::Simulate = sub_1036DB80,
//  InitializeNewParticles = sub_1036D1D0, name accessor = sub_103D8B20).
//
//   +0x30  int     m_nActiveParticles          (confirmed: count passed to
//                                                InitializeNewParticles)
//   +0x48  void*   m_pDef                       (confirmed)
//   +0xC8  int     m_nPerParticleInitializedAttributeMask (confirmed)
//   +0xCC  int     m_nPerParticleUpdatedAttributeMask     (confirmed)
//   +0xD4  float*  m_pParticleAttributes[32]    (derived: immediately follows
//                                                the 3 mask ints; GUARDED below)
//
// CParticleSystemDefinition name (CUtlString) at m_pDef + 0x2F8:
//   sub_103D8B20: name = (nameObj[3] != 0) ? *(char**)nameObj : "";
// ===========================================================================

namespace PCOffsets
{
	constexpr int ActiveParticles = 0x30;
	constexpr int Def             = 0x48;
	constexpr int ParticleAttribs = 0xD4;  // m_pParticleAttributes[0]
	constexpr int DefNameString   = 0x2F8; // CUtlString inside the definition
}

// TINT_RGB is attribute index 6 (DEFPARTICLE_ATTRIBUTE(TINT_RGB,6)).
// Per-particle color is stored SoA in groups of 4 particles: 4 R's, then 4 G's,
// then 4 B's (12 floats per group).
constexpr int TINT_ATTR_INDEX   = 6;
constexpr int TINT_FLOATS_GROUP = 12;

static int GetActiveParticles(void* pCollection)
{
	int n = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(pCollection) + PCOffsets::ActiveParticles);
	if (n < 0 || n > 5000) // MAX_PARTICLES_IN_A_SYSTEM clamp
		return 0;
	return n;
}

// Reads the particle system's name directly (CUtlString), no engine call.
static const char* GetSystemName(void* pCollection)
{
	uintptr_t def = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pCollection) + PCOffsets::Def);
	if (def < 0x10000 || def > 0x7FFFFFFF)
		return nullptr;

	int* nameObj = reinterpret_cast<int*>(def + PCOffsets::DefNameString);
	// sub_103D8B20: valid string only when nameObj[3] (length/heap flag) != 0.
	if (nameObj[3] == 0)
		return nullptr;

	const char* name = *reinterpret_cast<const char**>(nameObj);
	uintptr_t v = reinterpret_cast<uintptr_t>(name);
	if (v < 0x10000 || v > 0x7FFFFFFF)
		return nullptr;
	return name;
}

// Decide which color (if any) a named particle system should be tinted with.
static bool ResolveTargetColor(const char* name, float& r, float& g, float& b)
{
	if (!name)
		return false;

	auto contains = [&](const char* sub) { return std::strstr(name, sub) != nullptr; };

	// Blood is matched FIRST and takes priority. L4D2 blood systems are named
	// with "blood" (blood_impact, blood_pool/puddle, etc.). Checking this before
	// the spitter rule prevents generic spitter tokens (puddle/aoe/ground) from
	// stealing blood pools and decals.
	if (Vars::Grenade::BloodColorChanger && contains("blood"))
	{
		r = Vars::Grenade::BloodColor.r() / 255.f;
		g = Vars::Grenade::BloodColor.g() / 255.f;
		b = Vars::Grenade::BloodColor.b() / 255.f;
		return true;
	}

	if (Vars::Grenade::ProjectileColorChanger &&
		(contains("molotov") || contains("inferno") || contains("fire")))
	{
		r = Vars::Grenade::MolotovColor.r() / 255.f;
		g = Vars::Grenade::MolotovColor.g() / 255.f;
		b = Vars::Grenade::MolotovColor.b() / 255.f;
		return true;
	}

	if (Vars::Grenade::ProjectileColorChanger &&
		(contains("boomer") || contains("vomit") || contains("bile")))
	{
		r = Vars::Grenade::BileColor.r() / 255.f;
		g = Vars::Grenade::BileColor.g() / 255.f;
		b = Vars::Grenade::BileColor.b() / 255.f;
		return true;
	}

	// Spitter acid: the spit trail AND the acid pool/AoE on the ground. Match only
	// spitter-specific tokens. The previously-used generic words (goo/aoe/puddle/
	// burn_ground) also matched blood pools and other ground decals, so they're
	// gone — every real spitter system name contains "spit" (spitter_projectile,
	// spitter_slime_aoe, spit_trail, etc.).
	if (Vars::Grenade::ProjectileColorChanger && contains("spit"))
	{
		r = Vars::Grenade::SpitterColor.r() / 255.f;
		g = Vars::Grenade::SpitterColor.g() / 255.f;
		b = Vars::Grenade::SpitterColor.b() / 255.f;
		return true;
	}

	// Smoker smoke cloud (spawned on death / from the smoker).
	if (Vars::Grenade::ProjectileColorChanger &&
		(contains("smoker") || contains("smoke")))
	{
		r = Vars::Grenade::SmokerColor.r() / 255.f;
		g = Vars::Grenade::SmokerColor.g() / 255.f;
		b = Vars::Grenade::SmokerColor.b() / 255.f;
		return true;
	}

	return false;
}

static bool LooksLikeValidPtr(const void* p)
{
	uintptr_t v = reinterpret_cast<uintptr_t>(p);
	if (v < 0x10000 || v > 0x7FFFFFFF)
		return false;
	if (v & 0xF) // SIMD attribute data is 16-byte aligned
		return false;
	return true;
}

// SEH-guarded tint write. Kept in its own function with NO C++ objects so it can
// use __try/__except (which can't coexist with object unwinding in the caller).
static void WriteTint(void* ecx, float r, float g, float b)
{
	int nActive = GetActiveParticles(ecx);
	if (nActive <= 0)
		return;

	// m_pParticleAttributes[TINT_RGB] -> base of the SoA tint buffer.
	float* tintBase = *reinterpret_cast<float**>(
		reinterpret_cast<uintptr_t>(ecx) + PCOffsets::ParticleAttribs + TINT_ATTR_INDEX * sizeof(float*));

	if (!LooksLikeValidPtr(tintBase))
		return; // layout wrong for this build — no-op instead of corrupting memory

	__try
	{
		for (int i = 0; i < nActive; i += 4)
		{
			float* group = tintBase + (i / 4) * TINT_FLOATS_GROUP;
			int groupCount = (nActive - i) < 4 ? (nActive - i) : 4;
			for (int lane = 0; lane < groupCount; ++lane)
			{
				group[0 * 4 + lane] = r; // R block
				group[1 * 4 + lane] = g; // G block
				group[2 * 4 + lane] = b; // B block
			}
		}
	}
	__except (1)
	{
		// Bad offset/access — silently disable this write.
	}
}

void __fastcall ParticleColor::Detour(void* ecx, void* edx, float dt)
{
	// Let the engine simulate first so particle state/colours are populated.
	Func.Original<FN>()(ecx, edx, dt);

	if (!ecx)
		return;

	const char* name = GetSystemName(ecx);

	// Debug: print each unique particle system name once so exact names (acid
	// pool, smoke cloud, etc.) can be discovered for matching.
	if (Vars::Grenade::LogParticleNames && name)
	{
		static std::unordered_set<std::string> seen;
		if (seen.insert(name).second)
			std::cout << "[particle] " << name << "\n";
	}

	if (!Vars::Grenade::ProjectileColorChanger && !Vars::Grenade::BloodColorChanger)
		return;

	float r, g, b;
	if (!ResolveTargetColor(name, r, g, b))
		return;

	WriteTint(ecx, r, g, b);
}

void ParticleColor::Init()
{
	// CParticleCollection::Simulate(float) — VERIFIED signature (sub_1036DB80):
	// push ebp / mov ebp,esp / sub esp,44 / mov eax, g_Telemetry / push esi /
	// mov esi,ecx / xor ecx,ecx / mov [ebp-34],ecx / mov [ebp-30],ecx / push edi /
	// mov [ebp-10],esi / cmp [eax+8],ecx
	const DWORD dwSimulate = U::Pattern.Find(_("client.dll"),
		_("55 8B EC 83 EC 44 A1 ? ? ? ? 56 8B F1 33 C9 89 4D CC 89 4D D0 57 89 75 F0 39 48 08"));

	if (!dwSimulate)
	{
		std::cout << "    [ParticleColor] Simulate signature not found — particle recolor disabled.\n";
		return;
	}

	if (!Func.Init(reinterpret_cast<void*>(dwSimulate), &Detour))
		std::cout << "    [ParticleColor] failed to install Simulate hook.\n";
	else
		std::cout << "    [ParticleColor] Simulate hook installed at 0x" << std::hex << dwSimulate << std::dec << "\n";
}
