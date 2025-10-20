#include "AppearanceTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include "../../Core/Settings.h"

namespace kx {
    namespace GUI {

        // --- Helper Functions for Rendering UI Sections ---

        // Renders the global settings like distance limit.
        static void RenderGlobalSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Use Distance Limit", &settings.distance.useDistanceLimit);
                if (settings.distance.useDistanceLimit) {
                    ImGui::SliderFloat("Render Distance Limit", &settings.distance.renderDistanceLimit, 10.0f, 500.0f, "%.0fm");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Entities beyond this distance will not be rendered based on gameplay distance (player-to-target).");
                    }
                }

                ImGui::Separator();
                ImGui::Text("Text Display");
                ImGui::Checkbox("Enable Text Backgrounds", &settings.sizes.enableTextBackgrounds);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Add subtle dark backgrounds behind ESP text for better readability.\n"
                        "Disable for a cleaner, minimal UI appearance.\n\n"
                        "Note: Damage numbers always have no background for maximum clarity.\n"
                        "Text shadows remain in both modes for basic readability."
                    );
                }
            }
        }

        // Renders all sliders related to the ESP scaling curve.
        static void RenderScalingSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("ESP Scaling Configuration")) { // Collapsed by default

                // --- PRIMARY, ALWAYS-VISIBLE CONTROLS ---
                ImGui::SliderFloat("Scaling Start Distance", &settings.scaling.scalingStartDistance, 0.0f, 150.0f, "%.1fm");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The distance at which elements begin to scale down.\nSet to 0.0 for a continuous curve.");
                }

                // --- MODE-SPECIFIC CONTROLS ---
                if (settings.distance.useDistanceLimit) {
                    // --- SETTINGS FOR "LIMIT MODE" ---
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Render Limit Mode Scaling");
                    
                    ImGui::SliderFloat("Distance Factor##Limit", &settings.scaling.limitDistanceFactor, 50.0f, 500.0f, "%.0f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The main control for the curve's steepness.\nHigher values = gentler slope (slower scaling).");
                    }
                    
                    ImGui::SliderFloat("Scaling Exponent##Limit", &settings.scaling.limitScalingExponent, 0.5f, 2.5f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Controls the mathematical shape of the curve.\nDefault is 1.2 for natural falloff.");
                    }
                } else {
                    // --- SETTINGS FOR "NO LIMIT MODE" ---
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Unlimited Mode Scaling (Adaptive)");
                    ImGui::TextWrapped("Distance Factor is automatic based on scene.");
                    
                    // Display current adaptive far plane value
                    float adaptiveFarPlane = AppState::Get().GetAdaptiveFarPlane();
                    ImGui::Text("Adaptive Far Plane: %.1fm", adaptiveFarPlane);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(
                            "Auto-calculated from the 95th percentile of entity distances.\n"
                            "Updated once per second. Shows the maximum render range.\n"
                            "Distance Factor = %.1fm (50%% scale at this distance)",
                            adaptiveFarPlane / 2.0f
                        );
                    }
                    
                    ImGui::SliderFloat("Curve Shape (Exponent)##NoLimit", &settings.scaling.noLimitScalingExponent, 0.5f, 2.0f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(
                            "Controls how aggressively entities shrink with distance.\n\n"
                            "0.5-1.0: Gentle scaling, maximum readability\n"
                            "1.2 (Recommended): Balanced scaling for most scenarios\n"
                            "1.5-2.0: Aggressive scaling, reduces clutter"
                        );
                    }
                }

                ImGui::Separator();

                // --- ADVANCED OPTIONS TOGGLE ---
                static bool showAdvancedScaling = false;
                ImGui::Checkbox("Show Advanced Scaling Options", &showAdvancedScaling);

                if (showAdvancedScaling) {
                    ImGui::Indent(); // Indent the advanced options for clarity

                    ImGui::SliderFloat("Min Scale", &settings.scaling.minScale, 0.1f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The minimum size an element can shrink to (as a percentage).\nPrevents elements from becoming invisibly small.");
                    }

                    ImGui::SliderFloat("Max Scale", &settings.scaling.maxScale, 0.5f, 2.0f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The maximum size an element can magnify to up close.\nSet to 1.0 to disable any magnification.");
                    }

                    ImGui::SliderFloat("Min Font Size", &settings.sizes.minFontSize, 6.0f, 12.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The absolute minimum pixel size for fonts at maximum distance.");
                    }

                    ImGui::Unindent();
                }
            }
        }

        // Renders all sliders related to the base size of ESP elements.
        static void RenderBaseSizeSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("Element Base Sizes")) { // Collapsed by default
                ImGui::SliderFloat("Font Size", &settings.sizes.baseFontSize, 6.0f, 30.0f, "%.1fpx");
                ImGui::SliderFloat("Dot Radius", &settings.sizes.baseDotRadius, 1.0f, 10.0f, "%.1fpx");
                ImGui::SliderFloat("Box Thickness", &settings.sizes.baseBoxThickness, 0.5f, 5.0f, "%.1fpx");

                ImGui::Separator();

                ImGui::SliderFloat("Health Bar Width", &settings.sizes.baseHealthBarWidth, 20.0f, 100.0f, "%.0fpx");
                ImGui::SliderFloat("Health Bar Height", &settings.sizes.baseHealthBarHeight, 2.0f, 20.0f, "%.1fpx");

                ImGui::Separator();

                ImGui::SliderFloat("Base Box Height", &settings.sizes.baseBoxHeight, 20.0f, 150.0f, "%.0fpx");
                ImGui::SliderFloat("Base Box Width", &settings.sizes.baseBoxWidth, 10.0f, 100.0f, "%.0fpx");
            }
        }

        // --- Main Tab Rendering Function ---

        void RenderAppearanceTab() {
            if (ImGui::BeginTabItem("Appearance")) {
                auto& settings = AppState::Get().GetSettings();

                ImGui::Text("Global Visual Style Settings");
                ImGui::Separator();

                RenderGlobalSettings(settings);
                RenderScalingSettings(settings);
                RenderBaseSizeSettings(settings);

                // --- Advanced Fading Section ---
                if (ImGui::CollapsingHeader("Advanced Fading (Unlimited Mode)")) {
                    if (settings.distance.useDistanceLimit) {
                        ImGui::TextDisabled("These settings only apply when 'Use Distance Limit' is OFF.");
                    } else {
                        ImGui::Checkbox("Enable Player & NPC Fade", &settings.distance.enablePlayerNpcFade);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip(
                                "Apply a subtle fade to players and NPCs at long range (90m to 300m).\n"
                                "Provides depth perception without compromising combat clarity."
                            );
                        }

                        if (settings.distance.enablePlayerNpcFade) {
                            ImGui::SliderFloat("Player/NPC Min Alpha", &settings.distance.playerNpcMinAlpha, 0.5f, 1.0f, "%.2f");
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip(
                                    "The minimum opacity for players/NPCs at their maximum render distance.\n"
                                    "Default: 0.50 (50%%) - maintains high visibility while adding subtle depth.\n"
                                    "Lower values increase fade intensity, higher values reduce it."
                                );
                            }
                        }
                    }
                }

                ImGui::EndTabItem();
            }
        }

    } // namespace GUI
} // namespace kx
