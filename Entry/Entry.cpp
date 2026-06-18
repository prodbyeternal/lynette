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

void CGlobal_ModuleEntry::Load()
{
	// Allocate Console immediately
	AllocConsole();
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

	// Enable ANSI escape sequence support for colors
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	if (hOut != INVALID_HANDLE_VALUE) {
		GetConsoleMode(hOut, &dwMode);
		SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}

	// Print pink ascii art from lyn.txt (\x1b[38;5;205m is a vibrant pink/magenta)
	std::ifstream file("lyn.txt");
	if (file.is_open()) {
		std::cout << "\x1b[38;5;205m";
		std::string line;
		while (std::getline(file, line)) {
			std::cout << line << "\n";
		}
		std::cout << "\x1b[0m\n";
		file.close();
	} else {
		// Fallback ASCII art or warning if file is not in current working dir
		std::ifstream file2("c:\\Users\\ethhh\\Downloads\\l4d2 src with skeet menu\\lyn.txt");
		if (file2.is_open()) {
			std::cout << "\x1b[38;5;205m";
			std::string line;
			while (std::getline(file2, line)) {
				std::cout << line << "\n";
			}
			std::cout << "\x1b[0m\n";
			file2.close();
		}
	}

	std::cout << "Loading lynette mod menu...\n";
	
	const HMODULE hClient = GetModuleHandleA("client.dll");
	const HMODULE hEngine = GetModuleHandleA("engine.dll");
	const HMODULE hVstdlib = GetModuleHandleA("vstdlib.dll");
	const HMODULE hVgui2 = GetModuleHandleA("vgui2.dll");
	const HMODULE hVguiMatSurface = GetModuleHandleA("vguimatsurface.dll");
	const HMODULE hMaterialSystem = GetModuleHandleA("materialsystem.dll");
	const HMODULE hShaderApi = GetModuleHandleA("shaderapidx9.dll");

	std::cout << "Base Memory Addresses:\n";
	if (hClient) std::cout << "  client.dll:         0x" << std::hex << reinterpret_cast<uintptr_t>(hClient) << std::dec << "\n";
	if (hEngine) std::cout << "  engine.dll:         0x" << std::hex << reinterpret_cast<uintptr_t>(hEngine) << std::dec << "\n";
	if (hVgui2) std::cout << "  vgui2.dll:          0x" << std::hex << reinterpret_cast<uintptr_t>(hVgui2) << std::dec << "\n";
	if (hVguiMatSurface) std::cout << "  vguimatsurface.dll: 0x" << std::hex << reinterpret_cast<uintptr_t>(hVguiMatSurface) << std::dec << "\n";
	if (hMaterialSystem) std::cout << "  materialsystem.dll: 0x" << std::hex << reinterpret_cast<uintptr_t>(hMaterialSystem) << std::dec << "\n";
	if (hShaderApi) std::cout << "  shaderapidx9.dll:   0x" << std::hex << reinterpret_cast<uintptr_t>(hShaderApi) << std::dec << "\n";
	std::cout << "\n";

	while (!GetModuleHandleA("serverbrowser.dll"))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << "Initializing Offsets...\n";
	U::Offsets.Init();

	//Interfaces
	{
		std::cout << "Getting Interfaces...\n";
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

	std::cout << "Hooking functions...\n";
	
	std::cout << "  Drawing Manager...\n";
	G::Draw.Init();
	
	std::cout << "  Hooks...\n";
	G::Hooks.Init();

	std::cout << "Hooked functions:\n";
	std::cout << "  - LevelInitPreEntity, LevelInitPostEntity, LevelShutdown, FrameStageNotify\n";
	std::cout << "  - CalcPlayerView, CalcViewModelView\n";
	std::cout << "  - CL_Move\n";
	std::cout << "  - ShouldDrawFog, CreateMove, DoPostScreenSpaceEffects, GetViewModelFOV, OverrideView\n";
	std::cout << "  - InPrediction\n";
	std::cout << "  - PaintTraverse\n";
	std::cout << "  - ForcedMaterialOverride, DrawModelExecute\n";
	std::cout << "  - DrawModels (ModelRenderSystem)\n";
	std::cout << "  - GetSeqIdForGroup\n";
	std::cout << "  - GetTerrorRules\n";
	std::cout << "  - WndProc (Window Hook)\n";
	std::cout << "  - EndScene (D3D9 Hook)\n";
	std::cout << "  - GetVelocity (BaseAnimating)\n";
	
	std::cout << "\nlynette successfully loaded!\n\n";
}