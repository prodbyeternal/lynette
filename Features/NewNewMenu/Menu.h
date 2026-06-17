#pragma once
#include "../../SDK/SDK.h"
#include "Singleton.hpp"
#include <d3d9.h>
#include <d3d9helper.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#pragma comment(lib, "d3dx9.lib")
#include <d3dx9xof.h>

class Menu : public Singleton<Menu> {

public:

	void Render(IDirect3DDevice9* pDevice);
	void Shutdown();
	void ColorPicker(const char* name, float* color, bool alpha);
	float m_flFadeElapsed = 0.0f;
	float m_flFadeAlpha = 0.0f;
	float flTimeOnChange = 0.0f;
	float m_flFadeDuration = 0.1f;
	void Legit();
	void Aimbot();
	void Antiaim();
	void Visuals();
	void Misc();
	void Skins();
	void Players();

	bool isOpen = false;
};

inline Menu g_Menu;