#include "WndProc.h"
#include "../../Features/NewNewMenu/Menu.h"
#include "../../Features/NewNewMenu/babagui/imgui.h"
#include "../../Features/NewNewMenu/babagui/imgui_impl_win32.h"
#include "../../Features/NewNewMenu/babagui/imgui_impl_dx9.h"
#include "../../Features/NewNewMenu/babagui/imgui_internal.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LONG WINAPI WndProc::Detour(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (g_Menu.isOpen) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

		// Only block input messages — let system messages (paint, close, focus, etc.) pass through
		switch (uMsg) {
		case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
		case WM_MOUSEWHEEL: case WM_MOUSEMOVE:
		case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
		case WM_CHAR: case WM_SYSCHAR:
		case WM_XBUTTONDOWN: case WM_XBUTTONUP:
			return 1; // Eat input messages so the game doesn't process them
		}
	}
	return CallWindowProcW(WndProc, hWnd, uMsg, wParam, lParam);
}

void WndProc::Initialize()
{
	while (!hwWindow)
	{
		hwWindow = FindWindowW(_(L"Valve001"), 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	WndProc = reinterpret_cast<WNDPROC>(SetWindowLongW(hwWindow, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(Detour)));
}

void WndProc::UnInitialize()
{
	SetWindowLongW(hwWindow, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
}