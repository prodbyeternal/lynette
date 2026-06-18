#pragma once
#include "../Hooks.h"
#include "../../SDK/DirectX/DirectX.h"

namespace EndSceneHook
{
	inline Hook::CTable Table;
	using fn = HRESULT(__stdcall*)(IDirect3DDevice9*);
	HRESULT __stdcall Func(IDirect3DDevice9* pDevice);

	// D3D9 device Reset (vtable index 16). Hooked to release/recreate ImGui DX9
	// resources around a device reset (alt-tab out of fullscreen), which otherwise
	// leaves stale/frozen rendering.
	using ResetFn = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	HRESULT __stdcall ResetHook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams);

	void Init();
}