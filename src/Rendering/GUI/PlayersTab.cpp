#include "PlayersTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"

namespace kx {
    namespace GUI {
        void RenderPlayersTab() {
            if (ImGui::BeginTabItem("Players")) {
                auto& settings = kx::AppState::Get().GetSettings();
                
                ImGui::Checkbox("Enable Player ESP", &settings.playerESP.enabled);
                
                if (settings.playerESP.enabled) {
                    ImGui::Separator();
                    ImGui::Text("Player Filter Options");
                    ImGui::Checkbox("Show Local Player", &settings.playerESP.showLocalPlayer);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Show your own character in the ESP overlay.");
                    }

                    const char* gearModes[] = { "Off", "Compact (Stat Names)", "Compact (Top 3 Attributes)", "Detailed" };
                    ImGui::PushItemWidth(250.0f);
                    ImGui::Combo("Gear Display", &settings.playerESP.gearDisplayMode, gearModes, IM_ARRAYSIZE(gearModes));
                    ImGui::PopItemWidth();
                }
                ImGui::EndTabItem();
            }
        }
    }
}
