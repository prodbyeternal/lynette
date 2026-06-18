#include "EndScene.h"
#include "../../Features/NewNewMenu/Menu.h"
#include "../../Features/NewNewMenu/babagui/imgui.h"
#include "../../Features/NewNewMenu/babagui/imgui_impl_dx9.h"
#include "../../Features/NewNewMenu/babagui/imgui_impl_win32.h"
#include <intrin.h>
#include <deque>
#include <numeric>
#include <algorithm>
#include "../../Features/Vars.h"

// Define helper operator overloads for ImVec2 math
inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

static std::deque<float> velocityHistory;
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

	// Render the velocity speedometer and keybind badges overlay
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
			
			auto drawIndicator = [&](const char* label, bool active, float xOffset) {
				ImVec4 colVec = active ? ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f) : ImVec4(80.f/255.f, 80.f/255.f, 85.f/255.f, 1.f);
				ImU32 col = ImGui::GetColorU32(colVec);
				ImVec2 lblSz = activeFont->CalcTextSizeA(12.f, FLT_MAX, 0.f, label);
				
				// Draw rounded border badge
				drawList->AddRectFilled(indicatorPos + ImVec2(xOffset, 0.f), indicatorPos + ImVec2(xOffset + 48.f, 20.f), ImGui::GetColorU32(ImVec4(15.f/255.f, 15.f/255.f, 17.f/255.f, 0.94f)), 4.f);
				drawList->AddRect(indicatorPos + ImVec2(xOffset, 0.f), indicatorPos + ImVec2(xOffset + 48.f, 20.f), col, 4.f, 0, 1.f);
				
				// Draw Text
				drawList->AddText(activeFont, 12.f, indicatorPos + ImVec2(xOffset + (48.f - lblSz.x) * 0.5f, 3.f), col, label);
			};

			// Keybind checking states
			bool ebActive = (GetAsyncKeyState(Vars::Movement::kEdgeBug) & 0x8000) != 0;
			bool ejActive = (GetAsyncKeyState(Vars::Movement::kEdgeJump) & 0x8000) != 0;
			bool mjActive = (GetAsyncKeyState(Vars::Movement::kMiniJump) & 0x8000) != 0;
			bool ljActive = (GetAsyncKeyState(Vars::Movement::kLongJump) & 0x8000) != 0;

			// Center the 4 badges inside the 280px container
			float totalIndicatorWidth = 4 * 48.f + 3 * 8.f; // 216px total
			float startingX = (w - totalIndicatorWidth) * 0.5f;

			drawIndicator("eb", ebActive, startingX);
			drawIndicator("ej", ejActive, startingX + 56.f);
			drawIndicator("mj", mjActive, startingX + 112.f);
			drawIndicator("lj", ljActive, startingX + 168.f);

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

void EndSceneHook::Init()
{
	XASSERT(Table.Init(g_dwDirectXDevice) == false);
	XASSERT(Table.Hook(&Func, 42) == false);
}