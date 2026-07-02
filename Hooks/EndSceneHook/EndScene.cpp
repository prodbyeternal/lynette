#include "EndScene.h"
#include "../../Features/NewNewMenu/Menu.h"
#include "../../Features/NewNewMenu/babagui/imgui.h"
#include "../../Features/NewNewMenu/babagui/imgui_impl_dx9.h"
#include "../../Features/NewNewMenu/babagui/imgui_impl_win32.h"
#include <intrin.h>
#include <deque>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <cmath>
#include "../../Features/Vars.h"
#include "../../Features/Movement/Movement.h"
#include "../../SDK/GameUtil/GameUtil.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"

extern std::vector<Vector> ebpos;

static bool SafeW2S(const Vector& world, ImVec2& out)
{
	Vector s;
	if (!G::Util.W2S(world, s))
		return false;

	ImGuiIO& io = ImGui::GetIO();
	const float margin = 2000.f;
	if (s.x < -margin || s.x > io.DisplaySize.x + margin ||
		s.y < -margin || s.y > io.DisplaySize.y + margin)
		return false;

	out = ImVec2(s.x, s.y);
	return true;
}

inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

// Menu accent color helpers (drive HUD/indicators from Vars::Menu::AccentColor)
static ImVec4 AccentVec4(float alpha = 1.f) {
	const Color& c = Vars::Menu::AccentColor;
	return ImVec4(c.r() / 255.f, c.g() / 255.f, c.b() / 255.f, (c.a() / 255.f) * alpha);
}
static ImU32 AccentU32(float alpha = 1.f) {
	return ImGui::GetColorU32(AccentVec4(alpha));
}

// Exponential smoothing lerp (kamibera-style)
static float LerpFloat(float current, float target, float smooth, float dt) {
	return current + (target - current) * (1.f - expf(-smooth * dt));
}

// Animated indicator state
struct IndicatorState {
	const char* label;
	float opacity = 0.f;
	float targetOpacity = 0.f;
	float posX = 0.f;
	float targetX = 0.f;
	float detectIntensity = 0.f;
	float targetDetect = 0.f;
	bool active = false;
	bool prevDetect = false;
	std::chrono::steady_clock::time_point detectTime;
};

static IndicatorState g_Indicators[4] = {
	{"eb"}, {"ej"}, {"mj"}, {"lj"}
};

struct ChatNotification {
	bool        active = false;
	float       spawnTime = 0.f;
	float       lifetime = 5.0f;
};
static ChatNotification g_EdgeBugChat;

static void PushEdgeBugChatNotification()
{
	g_EdgeBugChat.active = true;
	g_EdgeBugChat.spawnTime = (float)ImGui::GetTime();
}

static std::deque<float> velocityHistory;

// Watermark animated width
static float g_WatermarkWidth = 0.f;

HRESULT __stdcall EndSceneHook::Func(IDirect3DDevice9* pDevice)
{
	static void* firstAddress = _ReturnAddress();
	if (firstAddress != _ReturnAddress())
		return Table.Original<fn>(42)(pDevice);

	if (!g_Menu.isInitialized)
	{
		g_Menu.Init(pDevice);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1) {
		g_Menu.isOpen = !g_Menu.isOpen;
		I::VGuiSurface->SetCursorAlwaysVisible(g_Menu.isOpen);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	g_Menu.Render(pDevice);

	// Kamibera-style watermark
	if (Vars::ESP::Watermark && !I::EngineVGui->IsGameUIVisible())
	{
		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* fg = ImGui::GetForegroundDrawList();
		ImFont* font = indicatorFont ? indicatorFont : (verdanaFont ? verdanaFont : ImGui::GetFont());
		const float fontSz = 14.f;

		static float fpsSmoothed = 0.f;
		float dt = io.DeltaTime > 0.f ? io.DeltaTime : (1.f / 60.f);
		float instFps = 1.f / dt;
		fpsSmoothed += (instFps - fpsSmoothed) * 0.1f;
		int fps = (int)(fpsSmoothed + 0.5f);

		const char* name = "lynette";
		char tail[32];
		sprintf(tail, " | %d fps", fps);

		const ImU32 white     = IM_COL32(255, 255, 255, 255);
		const ImU32 grey      = IM_COL32(146, 144, 145, 255);
		const ImU32 bgCol     = IM_COL32(16, 16, 16, 255);
		const ImU32 borderCol = IM_COL32(27, 27, 27, 255);

		ImVec2 nameSz = font->CalcTextSizeA(fontSz, FLT_MAX, 0.f, name);
		ImVec2 tailSz = font->CalcTextSizeA(fontSz, FLT_MAX, 0.f, tail);
		float textW = nameSz.x + tailSz.x;

		const float padX = 10.f, padY = 6.f;
		float targetW = textW + padX * 2.f;
		float panelH = fontSz + padY * 2.f;

		g_WatermarkWidth = LerpFloat(g_WatermarkWidth, targetW, 8.f, dt);
		if (g_WatermarkWidth < 10.f) g_WatermarkWidth = targetW;

		float right = io.DisplaySize.x - 12.f;
		float top = 12.f;
		ImVec2 pMin = ImVec2(right - g_WatermarkWidth, top);
		ImVec2 pMax = ImVec2(right, top + panelH);

		fg->AddRectFilled(pMin, pMax, bgCol, 4.f);
		fg->AddRect(pMin, pMax, borderCol, 4.f, 0, 1.f);

		ImVec2 textPos = ImVec2(pMin.x + (g_WatermarkWidth - textW) * 0.5f, pMin.y + padY);
		fg->AddText(font, fontSz, textPos, white, name);
		fg->AddText(font, fontSz, ImVec2(textPos.x + nameSz.x, textPos.y), grey, tail);
	}

	// EdgeBug chat notification
	{
		static bool prevDetectEBChat = false;
		if (Movement::Detect_EB && !prevDetectEBChat)
			PushEdgeBugChatNotification();
		prevDetectEBChat = Movement::Detect_EB;

		if (g_EdgeBugChat.active && I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible())
		{
			float elapsed = (float)ImGui::GetTime() - g_EdgeBugChat.spawnTime;
			if (elapsed >= g_EdgeBugChat.lifetime)
			{
				g_EdgeBugChat.active = false;
			}
			else
			{
				float alpha = 1.f;
				if (elapsed > g_EdgeBugChat.lifetime - 1.f)
					alpha = g_EdgeBugChat.lifetime - elapsed;
				alpha = alpha < 0.f ? 0.f : (alpha > 1.f ? 1.f : alpha);

				ImGuiIO& io = ImGui::GetIO();
				ImDrawList* fg = ImGui::GetForegroundDrawList();
				ImFont* font = verdanaFont ? verdanaFont : ImGui::GetFont();
				float fontSz = 16.f;

				ImVec2 base = ImVec2(20.f, io.DisplaySize.y - 140.f);

				const char* name = "lynette";
				const char* msg = " // edgebugged";

				ImU32 pink = ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, alpha));
				ImU32 white = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, alpha));
				ImU32 shadow = ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.6f * alpha));

				ImVec2 nameSz = font->CalcTextSizeA(fontSz, FLT_MAX, 0.f, name);

				fg->AddText(font, fontSz, ImVec2(base.x + 1.f, base.y + 1.f), shadow, name);
				fg->AddText(font, fontSz, ImVec2(base.x + nameSz.x + 1.f, base.y + 1.f), shadow, msg);
				fg->AddText(font, fontSz, base, pink, name);
				fg->AddText(font, fontSz, ImVec2(base.x + nameSz.x, base.y), white, msg);
			}
		}
	}

	// Grenade trajectory prediction
	if (Vars::Grenade::TrajectoryPrediction &&
		I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible())
	{
		auto* pEnt = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		C_TerrorPlayer* pLocal = pEnt ? pEnt->As<C_TerrorPlayer*>() : nullptr;
		if (pLocal && pLocal->IsAlive() && pLocal->GetActiveWeapon())
		{
			int wid = pLocal->GetActiveWeapon()->As<C_TerrorWeapon*>()->GetWeaponID();
			bool isThrowable = (wid == WEAPON_MOLOTOV || wid == WEAPON_PIPEBOMB || wid == WEAPON_VOMITJAR);

			if (isThrowable)
			{
				Vector eyeAng;
				I::EngineClient->GetViewAngles(eyeAng);
				Vector fwd, right, up;
				Math::AngleVectors(eyeAng, &fwd, &right, &up);

				Vector pos = pLocal->Weapon_ShootPosition() + fwd * 16.f;
				const float throwSpeed = 1100.f;
				Vector vel = fwd * throwSpeed + up * 200.f;

				static ConVar* sv_gravity = I::Cvars->FindVar("sv_gravity");
				float gravity = sv_gravity ? sv_gravity->GetFloat() : 800.f;
				const float dt = 1.f / 30.f;

				ImDrawList* fg = ImGui::GetForegroundDrawList();
				const Color tc = Vars::Grenade::TrajectoryColor;
				ImU32 lineCol = ImGui::GetColorU32(ImVec4(tc.r()/255.f, tc.g()/255.f, tc.b()/255.f, 0.9f));

				bool haveLand = false; Vector landWorld;

				for (int step = 0; step < 120; ++step)
				{
					Vector next = pos + vel * dt;
					vel.z -= gravity * dt;

					Ray_t ray; ray.Init(pos, next);
					CTraceFilter filter; filter.pSkip = pLocal;
					trace_t tr;
					I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);

					Vector segEnd = (tr.fraction < 1.f) ? tr.endpos : next;

					ImVec2 s0, s1;
					bool v0 = SafeW2S(pos, s0);
					bool v1 = SafeW2S(segEnd, s1);
					if (v0 && v1)
						fg->AddLine(s0, s1, lineCol, 2.0f);

					if (tr.fraction < 1.f)
					{
						Vector n = tr.plane.normal;
						float dot = vel.x*n.x + vel.y*n.y + vel.z*n.z;
						vel.x = (vel.x - 2.f*dot*n.x) * 0.45f;
						vel.y = (vel.y - 2.f*dot*n.y) * 0.45f;
						vel.z = (vel.z - 2.f*dot*n.z) * 0.45f;
						pos = tr.endpos + n * 0.5f;

						if (vel.Lenght2D() < 60.f && fabsf(vel.z) < 60.f)
						{
							haveLand = true; landWorld = tr.endpos;
							break;
						}
					}
					else
					{
						pos = next;
					}
				}

				if (haveLand)
				{
					ImVec2 ls;
					if (SafeW2S(landWorld, ls))
						fg->AddCircle(ls, 6.0f, lineCol, 16, 2.0f);
				}
			}
		}
	}

	// Molotov fire range visual
	if (Vars::Grenade::MolotovRangeVisual &&
		I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible())
	{
		float radius = Vars::Grenade::MolotovRangeRadius;
		if (radius <= 0.f) radius = 100.f;

		ImDrawList* fg = ImGui::GetForegroundDrawList();
		const Color rc = Vars::Grenade::MolotovRangeColor;
		ImU32 rcol = ImGui::GetColorU32(ImVec4(rc.r()/255.f, rc.g()/255.f, rc.b()/255.f, rc.a()/255.f));

		const int maxEnt = I::ClientEntityList->GetMaxEntities();
		for (int n = 1; n <= maxEnt; ++n)
		{
			IClientEntity* pEnt2 = I::ClientEntityList->GetClientEntity(n);
			if (!pEnt2 || pEnt2->IsDormant())
				continue;
			ClientClass* cc = pEnt2->GetClientClass();
			if (!cc || cc->m_ClassID != CInferno)
				continue;

			Vector origin = pEnt2->As<C_BaseEntity*>()->m_vecOrigin();

			const int segs = 48;
			ImVec2 prevPt; bool havePt = false;
			for (int i = 0; i <= segs; ++i)
			{
				float ang = (float)i / segs * 6.28318530718f;
				Vector world = origin;
				world.x += cosf(ang) * radius;
				world.y += sinf(ang) * radius;
				ImVec2 sp;
				if (SafeW2S(world, sp))
				{
					if (havePt)
						fg->AddLine(prevPt, sp, rcol, 2.0f);
					prevPt = sp;
					havePt = true;
				}
				else havePt = false;
			}
		}
	}

	// EdgeBug debug visual
	if (Vars::Movement::bEdgeBug && Vars::Movement::EdgeBugDebugVisual &&
		I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible())
	{
		std::vector<Vector> path = ebpos;

		if (!path.empty())
		{
			ImDrawList* fg = ImGui::GetForegroundDrawList();
			ImU32 pathCol = ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 0.9f));
			ImU32 nodeCol = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 0.9f));

			ImVec2 prevScreen;
			bool havePrev = false;
			for (size_t i = 0; i < path.size(); ++i)
			{
				ImVec2 screen;
				if (!SafeW2S(path[i], screen))
				{
					havePrev = false;
					continue;
				}

				fg->AddCircleFilled(screen, 2.5f, nodeCol);

				if (havePrev)
					fg->AddLine(prevScreen, screen, pathCol, 2.0f);

				prevScreen = screen;
				havePrev = true;
			}

			if (Movement::Should_EB && !path.empty())
			{
				ImVec2 landing;
				if (SafeW2S(path.back(), landing))
				{
					ImU32 hot = ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f));
					fg->AddCircle(landing, 6.0f, hot, 16, 2.0f);
				}
			}
		}
	}

	// Velocity HUD + animated indicators
	if (I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible() && Vars::ESP::VelocityHUD)
	{
		auto* pEnt = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		C_TerrorPlayer* pLocal = pEnt ? pEnt->As<C_TerrorPlayer*>() : nullptr;
		if (pLocal && pLocal->IsAlive())
		{
			Vector vel = pLocal->m_vecVelocity();
			float currentSpeed = vel.Lenght2D();

			velocityHistory.push_back(currentSpeed);
			if (velocityHistory.size() > 120)
				velocityHistory.pop_front();

			bool useGraph = Vars::ESP::VelocityGraph;

			ImGuiIO& io = ImGui::GetIO();
			float dt = io.DeltaTime > 0.f ? io.DeltaTime : (1.f / 60.f);

			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * Vars::ESP::VelocityX, io.DisplaySize.y * Vars::ESP::VelocityY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(300, useGraph ? 160 : 80));
			ImGui::Begin("##SpeedometerHUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings);

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			float w = 280.f;
			float h = useGraph ? 100.f : 35.f;

			// Use indicatorFont for velocity HUD
			ImFont* hudFont = indicatorFont ? indicatorFont : (verdanaFont ? verdanaFont : ImGui::GetFont());
			bool pushedFont = false;
			if (indicatorFont) {
				ImGui::PushFont(indicatorFont);
				pushedFont = true;
			} else if (verdanaFont) {
				ImGui::PushFont(verdanaFont);
				pushedFont = true;
			}

			if (useGraph)
			{
				drawList->AddRectFilled(pos + ImVec2(2.f, 2.f), pos + ImVec2(w + 2.f, h + 2.f), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.17f), 8.f);
				drawList->AddRectFilled(pos, pos + ImVec2(w, h), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.84f), 8.f);
				drawList->AddRect(pos, pos + ImVec2(w, h), ImGui::GetColorU32(ImGuiCol_Border, 0.05f), 8.f, 0, 1.f);
			}

			char speedStr[32];
			sprintf(speedStr, "%d", static_cast<int>(std::round(currentSpeed)));

			ImVec2 textSz = hudFont->CalcTextSizeA(26.f, FLT_MAX, 0.f, speedStr);
			float textYOffset = useGraph ? 10.f : 0.f;

			drawList->AddText(hudFont, 26.f, pos + ImVec2((w - textSz.x) * 0.5f, textYOffset), AccentU32(1.f), speedStr);
			drawList->AddText(hudFont, 10.f, pos + ImVec2((w - textSz.x) * 0.5f + textSz.x + 3.f, textYOffset + 12.f), ImGui::GetColorU32(ImVec4(160.f/255.f, 160.f/255.f, 160.f/255.f, 1.f)), "UPS");

			// Draw Line Graph
			if (useGraph && velocityHistory.size() > 2)
			{
				float gWidth = w - 40.f;
				float gHeight = 40.f;
				ImVec2 gOrigin = pos + ImVec2(20.f, h - 15.f);

				float maxSpeed = 300.f;
				for (float s : velocityHistory) {
					if (s > maxSpeed)
						maxSpeed = s;
				}

				for (size_t i = 0; i < velocityHistory.size() - 1; i++) {
					float x1 = (float)i / 120.f * gWidth;
					float x2 = (float)(i + 1) / 120.f * gWidth;
					float v1 = (velocityHistory[i] / maxSpeed) * gHeight;
					float v2 = (velocityHistory[i+1] / maxSpeed) * gHeight;

					ImVec2 p1 = gOrigin + ImVec2(x1, -v1);
					ImVec2 p2 = gOrigin + ImVec2(x2, -v2);

					drawList->AddRectFilledMultiColor(
						ImVec2(p1.x, (p1.y + p2.y) * 0.5f),
						ImVec2(p2.x, gOrigin.y),
						AccentU32(0.12f),
						AccentU32(0.12f),
						AccentU32(0.f),
						AccentU32(0.f)
					);
				}

				for (size_t i = 0; i < velocityHistory.size() - 1; i++) {
					float x1 = (float)i / 120.f * gWidth;
					float x2 = (float)(i + 1) / 120.f * gWidth;
					float v1 = (velocityHistory[i] / maxSpeed) * gHeight;
					float v2 = (velocityHistory[i+1] / maxSpeed) * gHeight;

					ImVec2 p1 = gOrigin + ImVec2(x1, -v1);
					ImVec2 p2 = gOrigin + ImVec2(x2, -v2);

					drawList->AddLine(p1, p2, AccentU32(0.15f), 3.f);
					drawList->AddLine(p1, p2, AccentU32(1.f), 1.2f);
				}
			}

			// Animated bind indicators
			{
				const float indicatorFontSz = 16.f;
				const float spacing = 6.f;
				const float OPACITY_SMOOTH = 10.f;
				const float POSITION_SMOOTH = 9.5f;
				const float DETECT_SMOOTH = 4.5f;
				const float DETECT_HOLD_SEC = 0.25f;
				const float DETECT_MAX_INT = 0.75f;
				const float REMOVE_OPACITY_MULT = 2.35f;

				bool keyStates[4] = {
					Vars::Movement::kEdgeBug && (GetAsyncKeyState(Vars::Movement::kEdgeBug) & 0x8000) != 0,
					Vars::Movement::kEdgeJump && (GetAsyncKeyState(Vars::Movement::kEdgeJump) & 0x8000) != 0,
					Vars::Movement::kMiniJump && (GetAsyncKeyState(Vars::Movement::kMiniJump) & 0x8000) != 0,
					Vars::Movement::kLongJump && (GetAsyncKeyState(Vars::Movement::kLongJump) & 0x8000) != 0,
				};

				bool detectStates[4] = {
					Movement::Detect_EB,
					Movement::Detect_EJ,
					Movement::Detect_MJ,
					Movement::Detect_LJ,
				};

				auto now = std::chrono::steady_clock::now();

				int visibleCount = 0;
				float labelWidths[4];
				for (int i = 0; i < 4; ++i) {
					labelWidths[i] = hudFont->CalcTextSizeA(indicatorFontSz, FLT_MAX, 0.f, g_Indicators[i].label).x;

					g_Indicators[i].active = keyStates[i];

					if (keyStates[i])
						g_Indicators[i].targetOpacity = 1.f;
					else
						g_Indicators[i].targetOpacity = 0.f;

					// Detect flash
					if (detectStates[i] && !g_Indicators[i].prevDetect) {
						g_Indicators[i].targetDetect = DETECT_MAX_INT;
						g_Indicators[i].detectTime = now;
					}
					g_Indicators[i].prevDetect = detectStates[i];

					float detectElapsed = std::chrono::duration<float>(now - g_Indicators[i].detectTime).count();
					if (detectElapsed > DETECT_HOLD_SEC)
						g_Indicators[i].targetDetect = 0.f;

					// Lerp opacity
					float opacitySmooth = OPACITY_SMOOTH;
					if (g_Indicators[i].targetOpacity < g_Indicators[i].opacity)
						opacitySmooth *= REMOVE_OPACITY_MULT;
					g_Indicators[i].opacity = LerpFloat(g_Indicators[i].opacity, g_Indicators[i].targetOpacity, opacitySmooth, dt);

					// Lerp detect intensity
					g_Indicators[i].detectIntensity = LerpFloat(g_Indicators[i].detectIntensity, g_Indicators[i].targetDetect, DETECT_SMOOTH, dt);

					if (g_Indicators[i].opacity > 0.01f)
						visibleCount++;
				}

				// Calculate layout: center visible indicators
				float totalWidth = 0.f;
				int visIdx = 0;
				for (int i = 0; i < 4; ++i) {
					if (g_Indicators[i].opacity <= 0.01f) continue;
					if (visIdx > 0) totalWidth += spacing;
					totalWidth += labelWidths[i];
					visIdx++;
				}

				float startX = (w - totalWidth) * 0.5f;
				float curX = startX;
				for (int i = 0; i < 4; ++i) {
					if (g_Indicators[i].opacity <= 0.01f) {
						g_Indicators[i].posX = curX;
						continue;
					}
					g_Indicators[i].targetX = curX;
					curX += labelWidths[i] + spacing;
				}

				// Lerp positions
				for (int i = 0; i < 4; ++i) {
					g_Indicators[i].posX = LerpFloat(g_Indicators[i].posX, g_Indicators[i].targetX, POSITION_SMOOTH, dt);
				}

				// Render indicators
				ImVec2 indicatorBase = pos + ImVec2(0.f, h + (useGraph ? 8.f : 4.f));

				for (int i = 0; i < 4; ++i) {
					if (g_Indicators[i].opacity <= 0.01f) continue;

					float alpha = g_Indicators[i].opacity;
					float detect = g_Indicators[i].detectIntensity;

					// Color: blend between idle grey and accent based on active/detect
					float r, g, b;
					if (detect > 0.01f) {
						ImVec4 ac = AccentVec4(1.f);
						r = 110.f/255.f + (ac.x - 110.f/255.f) * (detect / DETECT_MAX_INT);
						g = 110.f/255.f + (ac.y - 110.f/255.f) * (detect / DETECT_MAX_INT);
						b = 115.f/255.f + (ac.z - 115.f/255.f) * (detect / DETECT_MAX_INT);
					} else if (keyStates[i]) {
						r = 1.f; g = 1.f; b = 1.f;
					} else {
						r = 110.f/255.f; g = 110.f/255.f; b = 115.f/255.f;
					}

					ImVec2 textPos = indicatorBase + ImVec2(g_Indicators[i].posX, 0.f);

					// Shadow
					drawList->AddText(hudFont, indicatorFontSz, textPos + ImVec2(1.f, 1.f),
						ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.5f * alpha)), g_Indicators[i].label);

					// Main text
					drawList->AddText(hudFont, indicatorFontSz, textPos,
						ImGui::GetColorU32(ImVec4(r, g, b, alpha)), g_Indicators[i].label);

					// Glow on detect
					if (detect > 0.05f) {
						float glowAlpha = detect * 0.4f * alpha;
						drawList->AddText(hudFont, indicatorFontSz, textPos + ImVec2(0.f, -1.f),
							AccentU32(glowAlpha), g_Indicators[i].label);
						drawList->AddText(hudFont, indicatorFontSz, textPos + ImVec2(0.f, 1.f),
							AccentU32(glowAlpha * 0.5f), g_Indicators[i].label);
					}
				}
			}

			if (pushedFont)
				ImGui::PopFont();

			ImGui::End();
		}
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return Table.Original<fn>(42)(pDevice);
}

HRESULT __stdcall EndSceneHook::ResetHook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams)
{
	if (g_Menu.isInitialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();

	HRESULT hr = Table.Original<ResetFn>(16)(pDevice, pParams);

	if (g_Menu.isInitialized)
		ImGui_ImplDX9_CreateDeviceObjects();

	return hr;
}

void EndSceneHook::Init()
{
	XASSERT(Table.Init(g_dwDirectXDevice) == false);
	XASSERT(Table.Hook(&Func, 42) == false);
	XASSERT(Table.Hook(&ResetHook, 16) == false);
}
