#include "AppearanceTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include "../../Core/Settings.h"

namespace kx {
    namespace GUI {

        // --- Helper Functions for Rendering UI Sections ---

        // Renders all distance-related settings in a dedicated section.
        static void RenderDistanceSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("Distance Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SeparatorText("Culling Mode");
                
                int mode_int = static_cast<int>(settings.distance.mode);
                
                if (ImGui::RadioButton("Natural (Default)", &mode_int, static_cast<int>(DistanceCullingMode::Natural))) {
                    settings.distance.mode = DistanceCullingMode::Natural;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Automatically applies the game's natural culling distance. Uses 130m in WvW maps and 90m everywhere else.");
                }
                ImGui::SameLine();
                
                if (ImGui::RadioButton("Combat Focus", &mode_int, static_cast<int>(DistanceCullingMode::CombatFocus))) {
                    settings.distance.mode = DistanceCullingMode::CombatFocus;
                    if (settings.distance.renderDistanceLimit < 100.0f) {
                        settings.distance.renderDistanceLimit = 200.0f;
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Removes the distance limit for Players & NPCs for maximum awareness, while keeping objects limited to reduce clutter. Ideal for PvP and WvW.");
                }
                ImGui::SameLine();
                
                if (ImGui::RadioButton("Unlimited", &mode_int, static_cast<int>(DistanceCullingMode::Unlimited))) {
                    settings.distance.mode = DistanceCullingMode::Unlimited;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Shows all entities regardless of distance. Can increase screen clutter, but provides maximum information.");
                }
                ImGui::SameLine();
                
                if (ImGui::RadioButton("Custom", &mode_int, static_cast<int>(DistanceCullingMode::Custom))) {
                    settings.distance.mode = DistanceCullingMode::Custom;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Manually configure distance limits for each entity type.");
                }
                
                settings.distance.mode = static_cast<DistanceCullingMode>(mode_int);
                
                // Context-aware controls based on selected mode
                switch (settings.distance.mode) {
                    case DistanceCullingMode::Natural:
                        // No UI controls needed. The backend handles the 90/130m logic via GetActiveDistanceLimit.
                        break;
                        
                    case DistanceCullingMode::CombatFocus:
                        ImGui::SliderFloat("Object Distance Limit", &settings.distance.renderDistanceLimit, 10.0f, 500.0f, "%.0fm");
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Distance limit for Objects (Gadgets and Attack Targets). Players and NPCs are unlimited.");
                        }
                        break;
                        
                    case DistanceCullingMode::Unlimited:
                        break;
                        
                    case DistanceCullingMode::Custom:
                        ImGui::Indent();
                        ImGui::Checkbox("Limit Players", &settings.distance.customLimitPlayers);
                        ImGui::Checkbox("Limit NPCs", &settings.distance.customLimitNpcs);
                        ImGui::Checkbox("Limit Objects", &settings.distance.customLimitObjects);
                        
                        if (settings.distance.customLimitPlayers || settings.distance.customLimitNpcs || settings.distance.customLimitObjects) {
                            ImGui::SliderFloat("Render Distance Limit", &settings.distance.renderDistanceLimit, 10.0f, 500.0f, "%.0fm");
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Entities beyond this distance will not be rendered based on gameplay distance (player-to-target).");
                            }
                        }
                        ImGui::Unindent();
                        break;
                }

                ImGui::SeparatorText("Display Format");
                
                const char* displayModes[] = { "Meters", "GW2 Units", "Both" };
                int currentMode = static_cast<int>(settings.distance.displayMode);
                if (ImGui::Combo("Distance Format", &currentMode, displayModes, 3)) {
                    settings.distance.displayMode = static_cast<DistanceDisplayMode>(currentMode);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Choose how distances are displayed:\n\n"
                        "Meters: 30.5m (default, matches Mumble Link)\n"
                        "GW2 Units: 1200 (matches skill tooltips)\n"
                        "Both: 1200 (30.5m) (comprehensive)\n\n"
                        "Note: 1 GW2 unit = 1 inch = 0.0254 meters"
                    );
                }
            }
        }

        // Renders the global appearance settings (opacity, text styles).
        static void RenderGlobalSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("Global Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SeparatorText("General Appearance");
                
                float displayValue = settings.appearance.globalOpacity * 100.0f;
                if (ImGui::SliderFloat("Global Opacity", &displayValue, 50.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp)) {
                    settings.appearance.globalOpacity = displayValue / 100.0f;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Global opacity multiplier for ALL ESP elements.\n\n"
                        "80%% (Default): Subtle integration, matches GW2's UI style\n"
                        "100%%: Full opacity, maximum visibility\n"
                        "50-70%%: Very subtle, minimal presence\n\n"
                        "Applies to: Text, boxes, health bars, dots, and all visual elements.\n"
                        "Combines with distance fading for natural depth perception."
                    );
                }

                ImGui::Separator();
                
                ImGui::Checkbox("Enable Text Backgrounds", &settings.appearance.enableTextBackgrounds);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Add subtle dark backgrounds behind ESP text for better readability.\n"
                        "Disable for a cleaner, minimal UI appearance.\n\n"
                        "Note: Damage numbers always have no background for maximum clarity."
                    );
                }

                ImGui::Checkbox("Enable Text Shadows", &settings.appearance.enableTextShadows);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Add subtle shadows behind text for better contrast and readability.\n"
                        "Disable for maximum performance in crowded scenes or ultra-minimal UI.\n\n"
                        "Performance: Disabling shadows reduces draw calls (useful in massive zergs)."
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
                if (settings.distance.IsInDistanceLimitMode()) {
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

        // Renders menu appearance settings (UI scale, etc.)
        static void RenderMenuAppearance(Settings& settings) {
            if (ImGui::CollapsingHeader("Menu Appearance")) {
                if (ImGui::SliderFloat("UI Scale", &settings.gui.uiScale, 0.8f, 1.5f, "%.1fx")) {
                    // Scale change will be applied on next app restart
                }
                
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Scale the menu interface size\n\n"
                        "0.8x: Compact (more content visible)\n"
                        "1.0x: Default (recommended)\n"
                        "1.5x: Large (better for 4K displays)\n\n"
                        "Note: Requires app restart to take effect"
                    );
                }

                float displayOpacity = settings.gui.menuOpacity * 100.0f;
                if (ImGui::SliderFloat("Menu Opacity", &displayOpacity, 50.0f, 100.0f, "%.0f%%")) {
                    settings.gui.menuOpacity = displayOpacity / 100.0f;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "Transparency of the menu window\n\n"
                        "50%%: Subtle, see-through\n"
                        "90%% (Default): Balanced visibility\n"
                        "100%%: Fully opaque\n\n"
                        "Tip: Lower opacity during combat for less obstruction"
                    );
                }
            }
        }

        // --- Main Tab Rendering Function ---

        void RenderAppearanceTab() {
            if (ImGui::BeginTabItem("Appearance")) {
                auto& settings = AppState::Get().GetSettings();

                RenderDistanceSettings(settings);
                RenderGlobalSettings(settings);
                RenderScalingSettings(settings);
                RenderBaseSizeSettings(settings);
                RenderMenuAppearance(settings);

                ImGui::EndTabItem();
            }
        }

    } // namespace GUI
} // namespace kx
