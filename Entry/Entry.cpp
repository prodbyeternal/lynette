#include "Entry.h"
#include "../SDK/L4D2/Interfaces/IConVar.h"
#include "../SDK/L4D2/Interfaces/Models.h"
#include "../SDK/L4D2/Interfaces/IInput.h"
#include "../SDK/DirectX/DirectX.h"
#include "Entry.h"
#include "../SDK/L4D2/Interfaces/IConVar.h"
#include "../SDK/L4D2/Interfaces/Models.h"
#include "../SDK/L4D2/Interfaces/IInput.h"
#include "../SDK/DirectX/DirectX.h"
#include <fstream>
#include <string>
#include <iostream>
#include "../Features/Config/Config.h"

// ANSI color constants for console output
constexpr const char* ANSI_GREEN = "\x1b[32m";
constexpr const char* ANSI_PINK = "\x1b[38;5;205m";
constexpr const char* ANSI_RESET = "\x1b[0m";

void CGlobal_ModuleEntry::Load()
{
	// Allocate Console immediately
	AllocConsole();
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

	// Enable ANSI escape sequence support for colors
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	bool ansiSupported = false;
	if (hOut != INVALID_HANDLE_VALUE) {
		GetConsoleMode(hOut, &dwMode);
		ansiSupported = SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}

	// Print pink ascii art from lyn.txt (preserve original pink color)
	if (ansiSupported) {
		std::cout << ANSI_PINK;
	}
	std::cout << R"(  .---.       ____     __ ,---.   .--.    .-''-. ,---------. ,---------.    .-''-.   
  | ,_|       \   \   /  /|    \  |  |  .'_ _   \\          \\          \ .'_ _   \  
,-./  )        \  _. /  ' |  ,  \ |  | / ( ` )   '`--.  ,---' `--.  ,---'/ ( ` )   ' 
\  '_ '`)       _( )_ .'  |  |\_ \|  |. (_ o _)  |   |   \       |   \  . (_ o _)  | 
 > (_)  )   ___(_ o _)'   |  _( )_\  ||  (_,_)___|   :_ _:       :_ _:  |  (_,_)___| 
(  .  .-'  |   |(_,_)'    | (_ o _)  |'  \   .---.   (_I_)       (_I_)  '  \   .---. 
 `-'`-'|___|   `-'  /     |  (_,_)\  | \  `-'    /  (_(=)_)     (_(=)_)  \  `-'    / 
  |        \\      /      |  |    |  |  \       /    (_I_)       (_I_)    \       /  
  `--------` `-..-'       '--'    '--'   `'-..-'     '---'       '---'     `'-..-'   )";
	if (ansiSupported) {
		std::cout << ANSI_RESET;
	}
	std::cout << "\n\n";

	if (ansiSupported) {
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << " loading lynette mod menu...\n";
	} else {
		std::cout << "[+] loading lynette mod menu...\n";
	}
	
	const HMODULE hClient = GetModuleHandleA("client.dll");
	const HMODULE hEngine = GetModuleHandleA("engine.dll");
	const HMODULE hVstdlib = GetModuleHandleA("vstdlib.dll");
	const HMODULE hVgui2 = GetModuleHandleA("vgui2.dll");
	const HMODULE hVguiMatSurface = GetModuleHandleA("vguimatsurface.dll");
	const HMODULE hMaterialSystem = GetModuleHandleA("materialsystem.dll");
	const HMODULE hShaderApi = GetModuleHandleA("shaderapidx9.dll");

	if (ansiSupported) {
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << " base memory addresses:\n";
		if (hClient) std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   client.dll:         0x" << std::hex << reinterpret_cast<uintptr_t>(hClient) << std::dec << "\n";
		if (hEngine) std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   engine.dll:         0x" << std::hex << reinterpret_cast<uintptr_t>(hEngine) << std::dec << "\n";
		if (hVgui2) std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   vgui2.dll:          0x" << std::hex << reinterpret_cast<uintptr_t>(hVgui2) << std::dec << "\n";
		if (hVguiMatSurface) std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   vguimatsurface.dll: 0x" << std::hex << reinterpret_cast<uintptr_t>(hVguiMatSurface) << std::dec << "\n";
		if (hMaterialSystem) std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   materialsystem.dll: 0x" << std::hex << reinterpret_cast<uintptr_t>(hMaterialSystem) << std::dec << "\n";
		if (hShaderApi) std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   shaderapidx9.dll:   0x" << std::hex << reinterpret_cast<uintptr_t>(hShaderApi) << std::dec << "\n";
	} else {
		std::cout << "[+] base memory addresses:\n";
		if (hClient) std::cout << "[+]   client.dll:         0x" << std::hex << reinterpret_cast<uintptr_t>(hClient) << std::dec << "\n";
		if (hEngine) std::cout << "[+]   engine.dll:         0x" << std::hex << reinterpret_cast<uintptr_t>(hEngine) << std::dec << "\n";
		if (hVgui2) std::cout << "[+]   vgui2.dll:          0x" << std::hex << reinterpret_cast<uintptr_t>(hVgui2) << std::dec << "\n";
		if (hVguiMatSurface) std::cout << "[+]   vguimatsurface.dll: 0x" << std::hex << reinterpret_cast<uintptr_t>(hVguiMatSurface) << std::dec << "\n";
		if (hMaterialSystem) std::cout << "[+]   materialsystem.dll: 0x" << std::hex << reinterpret_cast<uintptr_t>(hMaterialSystem) << std::dec << "\n";
		if (hShaderApi) std::cout << "[+]   shaderapidx9.dll:   0x" << std::hex << reinterpret_cast<uintptr_t>(hShaderApi) << std::dec << "\n";
	}
	std::cout << "\n";

	while (!GetModuleHandleA("serverbrowser.dll"))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	if (ansiSupported) {
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << " initializing offsets...\n";
	} else {
		std::cout << "[+] initializing offsets...\n";
	}
	U::Offsets.Init();

	//Interfaces
	{
		if (ansiSupported) {
			std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << " getting interfaces...\n";
		} else {
			std::cout << "[+] getting interfaces...\n";
		}
		I::BaseClient       = U::Interface.Get<IBaseClientDLL*>("client.dll", "VClient016");
		I::ClientEntityList = U::Interface.Get<IClientEntityList*>("client.dll", "VClientEntityList003");
		I::Prediction       = U::Interface.Get<IPrediction*>("client.dll", "VClientPrediction001");
		I::GameMovement     = U::Interface.Get<IGameMovement*>("client.dll", "GameMovement001");

		I::EngineClient     = U::Interface.Get<IVEngineClient*>("engine.dll", "VEngineClient013");
		I::EngineTrace      = U::Interface.Get<IEngineTrace*>("engine.dll", "EngineTraceClient003");
		I::EngineVGui       = U::Interface.Get<IEngineVGui*>("engine.dll", "VEngineVGui001");
		I::RenderView       = U::Interface.Get<IVRenderView*>("engine.dll", "VEngineRenderView013");
		I::DebugOverlay     = U::Interface.Get<IVDebugOverlay*>("engine.dll", "VDebugOverlay003");
		I::ModelInfo        = U::Interface.Get<IVModelInfo*>("engine.dll", "VModelInfoClient004");
		I::ModelRender      = U::Interface.Get<IVModelRender*>("engine.dll", "VEngineModel016");

		I::VGuiPanel        = U::Interface.Get<IVGuiPanel*>("vgui2.dll", "VGUI_Panel009");
		I::VGuiSurface      = U::Interface.Get<IVGuiSurface*>("vgui2.dll", "VGUI_Surface031");

		I::MatSystemSurface = U::Interface.Get<IMatSystemSurface*>("vguimatsurface.dll", "VGUI_Surface031");

		I::MaterialSystem   = U::Interface.Get<IMaterialSystem*>("materialsystem.dll", "VMaterialSystem080");
		I::Cvars	        = U::Interface.Get<ICvar*>("vstdlib.dll", "VEngineCvar007"); // 007 for l4d2, 004 for tf2 i believe.
		I::steamclient = U::Interface.Get<ISteamClient017*>("SteamClient.dll", "SteamClient017");
		/* steam */
		HSteamPipe hNewPipe = I::steamclient->CreateSteamPipe();
		HSteamUser hNewUser = I::steamclient->ConnectToGlobalUser(hNewPipe);

		I::steamfriends002 = reinterpret_cast<ISteamFriends002*>(I::steamclient->GetISteamFriends(hNewUser, hNewPipe, STEAMFRIENDS_INTERFACE_VERSION_002));
		I::steamuser = reinterpret_cast<ISteamUser017*>(I::steamclient->GetISteamUser(hNewUser, hNewPipe, STEAMUSER_INTERFACE_VERSION_017));
		I::Friends015 = reinterpret_cast<ISteamFriends015*>(I::steamclient->GetISteamFriends(hNewUser, hNewPipe, STEAMFRIENDS_INTERFACE_VERSION_015));
		I::Utils007 = reinterpret_cast<ISteamUtils007*>(I::steamclient->GetISteamUtils(hNewUser, STEAMUTILS_INTERFACE_VERSION_007));
		{
			I::ClientMode = **reinterpret_cast<void***>(U::Offsets.m_dwClientMode);
			XASSERT(I::ClientMode == nullptr);

			I::GlobalVars = **reinterpret_cast<CGlobalVarsBase***>(U::Offsets.m_dwGlobalVars);
			XASSERT(I::GlobalVars == nullptr);

			I::MoveHelper = **reinterpret_cast<IMoveHelper***>(U::Offsets.m_dwMoveHelper);
			XASSERT(I::MoveHelper == nullptr);

			I::Input = **reinterpret_cast<IInput_t***>(U::Offsets.m_dwIInput);
			XASSERT(I::Input == nullptr);

			g_dwDirectXDevice = **reinterpret_cast<void***>(U::Pattern.Find(_("shaderapidx9.dll"), _("A1 ? ? ? ? 8B 08 8B 51 ? 50 FF D2 8B F8")) + 0x1);
		}
	}

	if (ansiSupported) {
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << " hooking functions...\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   drawing manager...\n";
	} else {
		std::cout << "[+] hooking functions...\n";
		std::cout << "[+]   drawing manager...\n";
	}
	G::Draw.Init();
	
	if (ansiSupported) {
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   hooks...\n";
	} else {
		std::cout << "[+]   hooks...\n";
	}
	G::Hooks.Init();

	// Initialise the config system. We intentionally do NOT auto-load a config
	// here — loading is opt-in via the Config tab's "Load Config" button. Auto
	// loading a leftover "default" config silently re-enabled features (Anti-Aim,
	// Fog, etc.) on map load, which is surprising and hard to diagnose.
	G::Config.Init();

	if (ansiSupported) {
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << " hooked functions:\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - levelinitpreentity, levelinitpostentity, levelshutdown, framestagenotify\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - calcplayerview, calcviewmodelview\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - cl_move\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - shoulddrawfog, createmove, dopostscreenspaceeffects, getviewmodelfov, overrideview\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - inprediction\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - painttraverse\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - forcedmaterialoverride, drawmodelexecute\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - drawmodels (modelrendersystem)\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - getseqidforgroup\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - getterrorrules\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - wndproc (window hook)\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - endscene (d3d9 hook)\n";
		std::cout << ANSI_GREEN << "[+]" << ANSI_RESET << "   - getvelocity (baseanimating)\n";
		std::cout << "\n" << ANSI_GREEN << "[+]" << ANSI_RESET << " lynette successfully loaded!\n\n";
	} else {
		std::cout << "[+] hooked functions:\n";
		std::cout << "[+]   - levelinitpreentity, levelinitpostentity, levelshutdown, framestagenotify\n";
		std::cout << "[+]   - calcplayerview, calcviewmodelview\n";
		std::cout << "[+]   - cl_move\n";
		std::cout << "[+]   - shoulddrawfog, createmove, dopostscreenspaceeffects, getviewmodelfov, overrideview\n";
		std::cout << "[+]   - inprediction\n";
		std::cout << "[+]   - painttraverse\n";
		std::cout << "[+]   - forcedmaterialoverride, drawmodelexecute\n";
		std::cout << "[+]   - drawmodels (modelrendersystem)\n";
		std::cout << "[+]   - getseqidforgroup\n";
		std::cout << "[+]   - getterrorrules\n";
		std::cout << "[+]   - wndproc (window hook)\n";
		std::cout << "[+]   - endscene (d3d9 hook)\n";
		std::cout << "[+]   - getvelocity (baseanimating)\n";
		std::cout << "\n[+] lynette successfully loaded!\n\n";
	}
}