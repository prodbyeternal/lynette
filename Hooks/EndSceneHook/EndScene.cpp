#include "EndScene.h"
#include "../../Features/NewNewMenu/Menu.h"
#include <intrin.h>
HRESULT __stdcall EndSceneHook::Func(IDirect3DDevice9* pDevice)
{
	static void* firstAddress = _ReturnAddress();
	if (firstAddress != _ReturnAddress())
		return Table.Original<fn>(42)(pDevice);

	g_Menu.Render(pDevice);
	return Table.Original<fn>(42)(pDevice);
}

void EndSceneHook::Init()
{
	XASSERT(Table.Init(g_dwDirectXDevice) == false);
	XASSERT(Table.Hook(&Func, 42) == false);
}