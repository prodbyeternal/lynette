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
#include "../../Features/Vars.h"
#include "../../Features/Movement/Movement.h"
#include "../../SDK/GameUtil/GameUtil.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"

// EdgeBug prediction path positions (populated in EdgeBug.cpp during scanning).
extern std::vector<Vector> ebpos;

// Validated world-to-screen: returns true only if the point projects to a sane
// on/near-screen coordinate. DebugOverlay::ScreenPosition can report "success"
// for points behind the camera, producing wildly out-of-range coords; connecting
// those with AddLine draws a line straight across the whole screen (the artifact
// seen with the molotov ring / trajectory). We reject anything far outside the
// display bounds to prevent that.
static bool SafeW2S(const Vector& world, ImVec2& out)
{
	Vector s;
	if (!G::Util.W2S(world, s))
		return false;

	ImGuiIO& io = ImGui::GetIO();
	const float margin = 2000.f; // allow some off-screen slack for line continuity
	if (s.x < -margin || s.x > io.DisplaySize.x + margin ||
		s.y < -margin || s.y > io.DisplaySize.y + margin)
		return false;

	out = ImVec2(s.x, s.y);
	return true;
}

// Define helper operator overloads for ImVec2 math
inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

// EdgeBug UI state structure for success feedback tracking
struct EdgeBugUIState {
	bool successActive;
	std::chrono::steady_clock::time_point successStartTime;
	int successDurationMs;

	EdgeBugUIState() : successActive(false), successStartTime(), successDurationMs(500) {}

	void Trigger() {
		successActive = true;
		successStartTime = std::chrono::steady_clock::now();
	}

	void Reset() {
		successActive = false;
	}

	void Update() {
		if (successActive) {
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - successStartTime
			);
			if (elapsed.count() >= successDurationMs) {
				successActive = false;
			}
		}
	}
};

static std::deque<float> velocityHistory;
static EdgeBugUIState g_EdgeBugUIState;

// Local chat-style notification (visible only to us, rendered through the overlay).
// Broadcasting a real "say" command would expose the cheat to other players, so we
// draw our own chat line instead.
struct ChatNotification {
	bool        active = false;
	float       spawnTime = 0.f;   // ImGui::GetTime() when triggered
	float       lifetime = 5.0f;   // seconds visible before fade-out
};
static ChatNotification g_EdgeBugChat;

static void PushEdgeBugChatNotification()
{
	g_EdgeBugChat.active = true;
	g_EdgeBugChat.spawnTime = (float)ImGui::GetTime();
}

HRESULT __stdcall EndSceneHook::Func(IDirect3DDevice9* pDevice)
{
	static void* firstAddress = _ReturnAddress();
	if (firstAddress != _ReturnAddress())
		return Table.Original<fn>(42)(pDevice);

	// Ensure ImGui is initialized
	if (!g_Menu.isInitialized)
	{
		g_Menu.Init(pDevice);
	}

	// Toggle menu visibility on VK_INSERT
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		g_Menu.isOpen = !g_Menu.isOpen;
		I::VGuiSurface->SetCursorAlwaysVisible(g_Menu.isOpen);
	}

	// Start a single, unified ImGui frame for this scene render pass
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Render the cheat menu if open
	g_Menu.Render(pDevice);

	// EdgeBug chat notification (independent of the velocity HUD toggle).
	// Detect the Detect_EB false->true edge here so it fires even when the HUD is off.
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
				// Fade out over the last second
				float alpha = 1.f;
				if (elapsed > g_EdgeBugChat.lifetime - 1.f)
					alpha = g_EdgeBugChat.lifetime - elapsed;
				alpha = alpha < 0.f ? 0.f : (alpha > 1.f ? 1.f : alpha);

				ImGuiIO& io = ImGui::GetIO();
				ImDrawList* fg = ImGui::GetForegroundDrawList();
				ImFont* font = verdanaFont ? verdanaFont : ImGui::GetFont();
				float fontSz = 16.f;

				// Bottom-left, chat-style position
				ImVec2 base = ImVec2(20.f, io.DisplaySize.y - 140.f);

				const char* name = "lynette";
				const char* msg = " // edgebugged";

				ImU32 pink = ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, alpha));
				ImU32 white = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, alpha));
				ImU32 shadow = ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.6f * alpha));

				ImVec2 nameSz = font->CalcTextSizeA(fontSz, FLT_MAX, 0.f, name);

				// Shadow
				fg->AddText(font, fontSz, ImVec2(base.x + 1.f, base.y + 1.f), shadow, name);
				fg->AddText(font, fontSz, ImVec2(base.x + nameSz.x + 1.f, base.y + 1.f), shadow, msg);
				// Colored text
				fg->AddText(font, fontSz, base, pink, name);
				fg->AddText(font, fontSz, ImVec2(base.x + nameSz.x, base.y), white, msg);
			}
		}
	}

	// Grenade trajectory prediction: simulate the throw arc of the currently held
	// throwable (molotov, pipebomb, bile jar) with gravity + wall bounces and draw it.
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

				// Faithful L4D2 throw: CBaseCSGrenade releases the projectile from
				// roughly eye height + a small forward offset, with a velocity of
				// ~1100 u/s along the aim vector plus a +200 u/s upward bias, then
				// the projectile is governed by sv_gravity. (These match the values
				// used by the engine's ThrowGrenade for survivor throwables.)
				Vector pos = pLocal->Weapon_ShootPosition() + fwd * 16.f;
				const float throwSpeed = 1100.f;
				Vector vel = fwd * throwSpeed + up * 200.f;

				static ConVar* sv_gravity = I::Cvars->FindVar("sv_gravity");
				float gravity = sv_gravity ? sv_gravity->GetFloat() : 800.f;
				const float dt = 1.f / 30.f; // simulation step

				ImDrawList* fg = ImGui::GetForegroundDrawList();
				const Color tc = Vars::Grenade::TrajectoryColor;
				ImU32 lineCol = ImGui::GetColorU32(ImVec4(tc.r()/255.f, tc.g()/255.f, tc.b()/255.f, 0.9f));

				Vector prevScreen; bool havePrev = false;
				bool haveLand = false; Vector landWorld;

				for (int step = 0; step < 120; ++step)
				{
					Vector next = pos + vel * dt;
					vel.z -= gravity * dt;

					// Trace this segment against the world.
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
						// Bounce: reflect velocity about the surface normal with damping.
						Vector n = tr.plane.normal;
						float dot = vel.x*n.x + vel.y*n.y + vel.z*n.z;
						vel.x = (vel.x - 2.f*dot*n.x) * 0.45f;
						vel.y = (vel.y - 2.f*dot*n.y) * 0.45f;
						vel.z = (vel.z - 2.f*dot*n.z) * 0.45f;
						pos = tr.endpos + n * 0.5f;

						// If it has lost most of its energy, treat this as the landing point.
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

	// Molotov fire range visual: draw a ground circle around every active inferno
	// (the actual fire entity) for its entire lifetime, sized to the real fire
	// spread distance from the inferno_max_range cvar. This keeps the ring on the
	// fire until it burns out, rather than only while aiming.
	if (Vars::Grenade::MolotovRangeVisual &&
		I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible())
	{
		// Approximate fire footprint radius. inferno_max_range (default 172) is the
		// theoretical max spread, which draws larger than the actual visible burn,
		// so we use a user-tunable radius defaulting to a smaller, realistic value.
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

	// EdgeBug debug visual: draw the predicted scan path in world space so you can
	// see exactly where the edgebug routine is projecting your movement.
	if (Vars::Movement::bEdgeBug && Vars::Movement::EdgeBugDebugVisual &&
		I::EngineClient->IsInGame() && !I::EngineVGui->IsGameUIVisible())
	{
		// ebpos is written/cleared on the game thread (CreateMove) while we read it
		// here on the render thread. Snapshot it into a local copy first so a
		// concurrent clear() can't cause a subscript-out-of-range crash.
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

				// Node marker at each predicted tick position.
				fg->AddCircleFilled(screen, 2.5f, nodeCol);

				// Connect consecutive on-screen nodes with a line.
				if (havePrev)
					fg->AddLine(prevScreen, screen, pathCol, 2.0f);

				prevScreen = screen;
				havePrev = true;
			}

			// Highlight the final/landing node when an edgebug was actually detected.
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
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * Vars::ESP::VelocityX, io.DisplaySize.y * Vars::ESP::VelocityY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(300, useGraph ? 160 : 80));
			ImGui::Begin("##SpeedometerHUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings);

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			float w = 280.f;
			float h = useGraph ? 100.f : 35.f;

			// Push font if it's initialized
			bool pushedFont = false;
			if (verdanaFont)
			{
				ImGui::PushFont(verdanaFont);
				pushedFont = true;
			}

			// Draw Background container ONLY if graph is turned on
			if (useGraph)
			{
				drawList->AddRectFilled(pos + ImVec2(2.f, 2.f), pos + ImVec2(w + 2.f, h + 2.f), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.17f), 8.f);
				drawList->AddRectFilled(pos, pos + ImVec2(w, h), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.84f), 8.f);
				drawList->AddRect(pos, pos + ImVec2(w, h), ImGui::GetColorU32(ImGuiCol_Border, 0.05f), 8.f, 0, 1.f);
			}

			// Render Speed Text (Pink Accent)
			char speedStr[32];
			sprintf(speedStr, "%d", static_cast<int>(std::round(currentSpeed)));
			
			ImFont* activeFont = pushedFont ? verdanaFont : ImGui::GetFont();
			ImVec2 textSz = activeFont->CalcTextSizeA(26.f, FLT_MAX, 0.f, speedStr);
			float textYOffset = useGraph ? 10.f : 0.f;
			
			drawList->AddText(activeFont, 26.f, pos + ImVec2((w - textSz.x) * 0.5f, textYOffset), ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f)), speedStr);
			drawList->AddText(activeFont, 10.f, pos + ImVec2((w - textSz.x) * 0.5f + textSz.x + 3.f, textYOffset + 12.f), ImGui::GetColorU32(ImVec4(160.f/255.f, 160.f/255.f, 160.f/255.f, 1.f)), "UPS");

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

				// Render neon pink gradient fill
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
						ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 0.12f)),
						ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 0.12f)),
						ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 0.f)),
						ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 0.f))
					);
				}

				// Draw Neon Line Overlay
				for (size_t i = 0; i < velocityHistory.size() - 1; i++) {
					float x1 = (float)i / 120.f * gWidth;
					float x2 = (float)(i + 1) / 120.f * gWidth;
					float v1 = (velocityHistory[i] / maxSpeed) * gHeight;
					float v2 = (velocityHistory[i+1] / maxSpeed) * gHeight;

					ImVec2 p1 = gOrigin + ImVec2(x1, -v1);
					ImVec2 p2 = gOrigin + ImVec2(x2, -v2);

					drawList->AddLine(p1, p2, ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 0.15f)), 3.f);
					drawList->AddLine(p1, p2, ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f)), 1.2f);
				}
			}

			// Render Bind Indicators (eb, ej, mj, lj) positioned cleanly below the graph/speedometer
			ImVec2 indicatorPos = pos + ImVec2(0.f, h + (useGraph ? 8.f : 4.f));
			
			auto drawIndicator = [&](const char* label, bool active, bool success, float xOffset) {
				ImVec4 colVec;
				if (success)
					colVec = ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f); // pink on successful edgebug
				else if (active)
					colVec = ImVec4(1.f, 1.f, 1.f, 1.f);                          // white while bind held
				else
					colVec = ImVec4(110.f/255.f, 110.f/255.f, 115.f/255.f, 0.65f); // dim when idle

				ImU32 col = ImGui::GetColorU32(colVec);
				ImVec2 lblSz = activeFont->CalcTextSizeA(13.f, FLT_MAX, 0.f, label);

				// Text only (no background box). Subtle shadow first for legibility.
				ImVec2 textPos = indicatorPos + ImVec2(xOffset + (48.f - lblSz.x) * 0.5f, 0.f);
				drawList->AddText(activeFont, 13.f, textPos + ImVec2(1.f, 1.f), ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.5f)), label);
				drawList->AddText(activeFont, 13.f, textPos, col, label);
			};

			// Keybind checking states
			bool ebActive = (GetAsyncKeyState(Vars::Movement::kEdgeBug) & 0x8000) != 0;
			bool ejActive = (GetAsyncKeyState(Vars::Movement::kEdgeJump) & 0x8000) != 0;
			bool mjActive = (GetAsyncKeyState(Vars::Movement::kMiniJump) & 0x8000) != 0;
			bool ljActive = (GetAsyncKeyState(Vars::Movement::kLongJump) & 0x8000) != 0;

			// EdgeBug success state management
			static bool prevDetectEB = false;
			if (Movement::Detect_EB && !prevDetectEB) {
				// Detect false→true transition
				g_EdgeBugUIState.Trigger();
			}
			prevDetectEB = Movement::Detect_EB;

			// Update success state timer
			g_EdgeBugUIState.Update();

			// Reset success state when EdgeBug keybind is released
			if (!ebActive) {
				g_EdgeBugUIState.Reset();
			}

			// Center the 4 badges inside the 280px container
			float totalIndicatorWidth = 4 * 48.f + 3 * 8.f; // 216px total
			float startingX = (w - totalIndicatorWidth) * 0.5f;

			drawIndicator("eb", ebActive, g_EdgeBugUIState.successActive, startingX);
			drawIndicator("ej", ejActive, false, startingX + 56.f);
			drawIndicator("mj", mjActive, false, startingX + 112.f);
			drawIndicator("lj", ljActive, false, startingX + 168.f);

			if (pushedFont)
			{
				ImGui::PopFont();
			}

			ImGui::End();
		}
	}

	// End and present the unified ImGui frame
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return Table.Original<fn>(42)(pDevice);
}

HRESULT __stdcall EndSceneHook::ResetHook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams)
{
	// Release ImGui's DX9 device objects (font texture, vertex/index buffers)
	// BEFORE the device is reset, otherwise the reset fails / leaves stale GPU
	// resources and rendering freezes after alt-tab out of fullscreen.
	if (g_Menu.isInitialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();

	HRESULT hr = Table.Original<ResetFn>(16)(pDevice, pParams);

	// Recreate them after the device has been reset.
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