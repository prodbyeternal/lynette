#include "EndScene.h"
#include "../../Features/NewNewMenu/Menu.h"
#include "../../Features/NewNewMenu/babagui/imgui.h"
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

	// We must render the menu if it's open, which creates/starts the ImGui frame.
	if (g_Menu.isOpen)
	{
		g_Menu.Render(pDevice);
	}
	else
	{
		// If the menu is closed, we need to manually start a new frame to draw our speedometer
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	// Velocity HUD & Keybind Indicators Overlay
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

			ImGuiIO& io = ImGui::GetIO();
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * Vars::ESP::VelocityX, io.DisplaySize.y * Vars::ESP::VelocityY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(300, 160));
			ImGui::Begin("##SpeedometerHUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings);

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			float w = 280.f;
			float h = 100.f;

			// Draw Background container if configured (matching premium design with subtle pink shadow)
			drawList->AddRectFilled(pos + ImVec2(2.f, 2.f), pos + ImVec2(w + 2.f, h + 2.f), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.17f), 8.f);
			drawList->AddRectFilled(pos, pos + ImVec2(w, h), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.84f), 8.f);
			drawList->AddRect(pos, pos + ImVec2(w, h), ImGui::GetColorU32(ImGuiCol_Border, 0.05f), 8.f, 0, 1.f);

			// Render Speed Text (Pink Accent)
			char speedStr[32];
			sprintf(speedStr, "%d", static_cast<int>(std::round(currentSpeed)));
			ImVec2 textSz = ImGui::GetFont()->CalcTextSizeA(26.f, FLT_MAX, 0.f, speedStr);
			
			drawList->AddText(ImGui::GetFont(), 26.f, pos + ImVec2((w - textSz.x) * 0.5f, 10.f), ImGui::GetColorU32(ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f)), speedStr);
			drawList->AddText(ImGui::GetFont(), 10.f, pos + ImVec2((w - textSz.x) * 0.5f + textSz.x + 3.f, 22.f), ImGui::GetColorU32(ImVec4(160.f/255.f, 160.f/255.f, 160.f/255.f, 1.f)), "UPS");

			// Draw Line Graph
			if (Vars::ESP::VelocityGraph && velocityHistory.size() > 2)
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
			ImVec2 indicatorPos = pos + ImVec2(0.f, h + 8.f);
			
			auto drawIndicator = [&](const char* label, bool active, float xOffset) {
				ImVec4 colVec = active ? ImVec4(210.f/255.f, 100.f/255.f, 185.f/255.f, 1.f) : ImVec4(80.f/255.f, 80.f/255.f, 85.f/255.f, 1.f);
				ImU32 col = ImGui::GetColorU32(colVec);
				ImVec2 lblSz = ImGui::GetFont()->CalcTextSizeA(12.f, FLT_MAX, 0.f, label);
				
				// Draw rounded border badge
				drawList->AddRectFilled(indicatorPos + ImVec2(xOffset, 0.f), indicatorPos + ImVec2(xOffset + 48.f, 20.f), ImGui::GetColorU32(ImVec4(15.f/255.f, 15.f/255.f, 17.f/255.f, 0.94f)), 4.f);
				drawList->AddRect(indicatorPos + ImVec2(xOffset, 0.f), indicatorPos + ImVec2(xOffset + 48.f, 20.f), col, 4.f, 0, 1.f);
				
				// Draw Text
				drawList->AddText(ImGui::GetFont(), 12.f, indicatorPos + ImVec2(xOffset + (48.f - lblSz.x) * 0.5f, 3.f), col, label);
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

			ImGui::End();
		}
	}

	// End the ImGui frame only if we manually started it (menu is closed)
	if (!g_Menu.isOpen)
	{
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return Table.Original<fn>(42)(pDevice);
}

void EndSceneHook::Init()
{
	XASSERT(Table.Init(g_dwDirectXDevice) == false);
	XASSERT(Table.Hook(&Func, 42) == false);
}