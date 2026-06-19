#pragma once

// Utilities for manipulating the game's ConVar/ConCommand flags at runtime:
//   - Unhiding developer/hidden console commands
//   - Stripping FCVAR_CHEAT so cheat-flagged cvars work without sv_cheats
class CCvarTools
{
public:
	// Walk the entire cvar linked list and clear FCVAR_HIDDEN / FCVAR_DEVELOPMENTONLY
	// so hidden developer commands show up in find/autocomplete and can be used.
	// Safe to call multiple times.
	void UnhideAllCommands();

	// Remove FCVAR_CHEAT from every cvar so cheat-protected commands can be changed
	// without sv_cheats being set. Use with care.
	void RemoveCheatFlags();

	// Strip cheat/hidden flags from a single named cvar (e.g. "r_flashlightfov").
	// Returns true if the cvar was found.
	bool UnlockCvar(const char* name);

	// Force client-side prediction on. On a listen/local server you have server
	// authority so prediction-driven movement (bhop, edgejump, edgebug, etc.)
	// behaves; on a remote/dedicated server those same features only work if the
	// client is actually predicting. L4D2 ships with cl_predict locked off, so we
	// unlock and enable cl_predict / cl_predictweapons / cl_lagcompensation. Safe
	// to call repeatedly; only writes when a value is wrong.
	void EnsurePredictionEnabled();
};

namespace G { inline CCvarTools CvarTools; }
