#include "AppearanceTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"

namespace kx {
    namespace GUI {
        void RenderAppearanceTab() {
            if (ImGui::BeginTabItem("Appearance")) {
                auto& settings = kx::AppState::Get().GetSettings();
                
                ImGui::Text("Visual Style Settings");
                ImGui::Separator();
                
                // Global Style Settings
                if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Use Distance Limit", &settings.espUseDistanceLimit);
                    if (settings.espUseDistanceLimit) {
                        ImGui::SliderFloat("Render Distance Limit", &settings.espRenderDistanceLimit, 10.0f, 2000.0f, "%.0fm");
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Entities beyond this distance will not be rendered.\nEntities fade out in the last 11% of this distance.");
                        }
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("ESP Scaling Configuration");
                    
                    ImGui::SliderFloat("Min Scale", &settings.espMinScale, 0.1f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Minimum scale factor for very distant entities.\nLower values = smaller distant entities.");
                    }
                    
                    ImGui::SliderFloat("Max Scale", &settings.espMaxScale, 0.1f, 5.0f, "%.2f"); 
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Maximum scale factor for very close entities.\n1.0 = base size, >1.0 = larger, <1.0 = smaller.");
                    }
                    
                    ImGui::SliderFloat("Distance Factor", &settings.espDistanceFactor, 20.0f, 500.0f, "%.0f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Controls the scaling curve. This is the distance (in meters) at which elements will shrink to 50% of their size.\nHigher values = scaling happens over a longer distance.");
                    }

                    ImGui::SliderFloat("Scaling Exponent", &settings.espScalingExponent, 0.5f, 2.5f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Controls the curve shape. > 1.0 makes scaling more aggressive at long distances. < 1.0 makes it more aggressive at close distances.");
                    }

                    ImGui::SliderFloat("Min Font Size", &settings.espMinFontSize, 1.0f, 10.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The absolute minimum pixel size for fonts at maximum distance.\nPrevents text from becoming completely unreadable.");
                    }
                }

                if (ImGui::CollapsingHeader("Element Base Sizes", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderFloat("Font Size", &settings.espBaseFontSize, 6.0f, 30.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Base font size before distance scaling is applied.");
                    }
                    ImGui::SliderFloat("Dot Radius", &settings.espBaseDotRadius, 1.0f, 10.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Base radius for the center dot before distance scaling.");
                    }
                    ImGui::SliderFloat("Box Thickness", &settings.espBaseBoxThickness, 0.5f, 5.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Base thickness for bounding boxes before distance scaling.");
                    }
                    ImGui::SliderFloat("Health Bar Width", &settings.espBaseHealthBarWidth, 20.0f, 100.0f, "%.0fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Base width for standalone health bars before distance scaling.");
                    }
                    ImGui::SliderFloat("Health Bar Height", &settings.espBaseHealthBarHeight, 2.0f, 20.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Base height for standalone health bars before distance scaling.");
                    }
                }
                
                // Player Style Settings
                if (ImGui::CollapsingHeader("Player Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Visual Elements");
                    ImGui::Checkbox("Show Box##Player", &settings.playerESP.renderBox);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Distance##Player", &settings.playerESP.renderDistance);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Dot##Player", &settings.playerESP.renderDot);
                    
                    ImGui::Checkbox("Show Health Bar##Player", &settings.playerESP.renderHealthBar);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Details##Player", &settings.playerESP.renderDetails);
                    ImGui::Checkbox("Show Player Name##Player", &settings.playerESP.renderPlayerName);
                }
                
                // NPC Style Settings
                if (ImGui::CollapsingHeader("NPC Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Visual Elements");
                    ImGui::Checkbox("Show Box##NPC", &settings.npcESP.renderBox);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Distance##NPC", &settings.npcESP.renderDistance);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Dot##NPC", &settings.npcESP.renderDot);
                    
                    ImGui::Checkbox("Show Health Bar##NPC", &settings.npcESP.renderHealthBar);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Details##NPC", &settings.npcESP.renderDetails);
                }
                
                // Object Style Settings
                if (ImGui::CollapsingHeader("Object Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Visual Elements");
                    ImGui::Checkbox("Show Box##Object", &settings.objectESP.renderBox);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Distance##Object", &settings.objectESP.renderDistance);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Dot##Object", &settings.objectESP.renderDot);
                    
                    ImGui::Checkbox("Show Details##Object", &settings.objectESP.renderDetails);
                }
                ImGui::EndTabItem();
            }
        }
    }
}
