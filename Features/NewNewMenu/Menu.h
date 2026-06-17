#pragma once
#include "../../SDK/SDK.h"
#include "Singleton.hpp"
#include <d3d9.h>
#include <d3d9helper.h>

class Menu : public Singleton<Menu> {
public:
	void Render(IDirect3DDevice9* pDevice);
	void Shutdown();

	bool isOpen = false;
};

inline Menu g_Menu;