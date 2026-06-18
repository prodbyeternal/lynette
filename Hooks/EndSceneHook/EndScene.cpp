#include "EndScene.h"
#include "../../Features/NewNewMenu/Menu.h"
#include "../../Features/NewNewMenu/babagui/imgui.h"
#include <intrin.h>
#include <deque>
#include <numeric>
#include <algorithm>
#include "../../Features/Vars.h"

static std::deque<float> velocityHistory;
HRESULT __stdcall EndSceneHook::Func(IDirect3DDevice9* pDevice)
{
	static void* firstAddress = _ReturnAddress();
	if (firstAddress != _ReturnAddress())
		return Table.Original<fn>(42)(pDevice);

	g_Menu.Render(pDevice);

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
			drawList->AddRectFilled(pos + ImVec2(2.f, 2.f), pos + ImVec2(w + 2.f, h + 2.f), ImColor(0, 0, 0, 45), 8.f);
			drawList->AddRectFilled(pos, pos + ImVec2(w, h), ImColor(10, 10, 12, 215), 8.f);
			drawList->AddRect(pos, pos + ImVec2(w, h), ImColor(255, 255, 255, 12), 8.f, 0, 1.f);

			// Render Speed Text (Pink Accent)
			char speedStr[32];
			sprintf(speedStr, "%d", static_cast<int>(std::round(currentSpeed)));
			ImVec2 textSz = ImGui::GetFont()->CalcTextSizeA(26.f, FLT_MAX, 0.f, speedStr);
			
			drawList->AddText(ImGui::GetFont(), 26.f, pos + ImVec2((w - textSz.x) * 0.5f, 10.f), ImColor(210, 100, 185, 255), speedStr);
			drawList->AddText(ImGui::GetFont(), 10.f, pos + ImVec2((w - textSz.x) * 0.5f + textSz.x + 3.f, 22.f), ImColor(160, 160, 160, 255), "UPS");

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
						ImColor(210, 100, 185, 30), ImColor(210, 100, 185, 30),
						ImColor(210, 100, 185, 0), ImColor(210, 100, 185, 0)
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

					drawList->AddLine(p1, p2, ImColor(210, 100, 185, 40), 3.f);
					drawList->AddLine(p1, p2, ImColor(210, 100, 185, 255), 1.2f);
				}
			}

			// Render Bind Indicators (eb, ej, mj, lj) positioned cleanly below the graph/speedometer
			ImVec2 indicatorPos = pos + ImVec2(0.f, h + 8.f);
			
			auto drawIndicator = [&](const char* label, bool active, float xOffset) {
				ImColor col = active ? ImColor(210, 100, 185, 255) : ImColor(80, 80, 85, 255);
				ImVec2 lblSz = ImGui::GetFont()->CalcTextSizeA(12.f, FLT_MAX, 0.f, label);
				
				// Draw rounded border badge
				drawList->AddRectFilled(indicatorPos + ImVec2(xOffset, 0.f), indicatorPos + ImVec2(xOffset + 48.f, 20.f), ImColor(15, 15, 17, 240), 4.f);
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

	return Table.Original<fn>(42)(pDevice);
}

void EndSceneHook::Init()
{
	XASSERT(Table.Init(g_dwDirectXDevice) == false);
	XASSERT(Table.Hook(&Func, 42) == false);
}