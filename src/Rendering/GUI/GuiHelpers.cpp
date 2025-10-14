#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include <string>

// A local helper to encapsulate the ImGui ID generation, cleaning up the main function.
static void CheckboxWithId(const char* label, const char* categoryName, bool* value, const char* tooltip = nullptr) {
    // This creates a unique ID like "Show Box##Players" to prevent ImGui ID collisions.
    std::string id_label = std::string(label) + "##" + std::string(categoryName);
    ImGui::Checkbox(id_label.c_str(), value);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip(tooltip);
    }
}

namespace kx {
    namespace GUI {

        void RenderCategoryStyleSettings(const char* categoryName,
            bool& renderBox,
            bool& renderDistance,
            bool& renderDot,
            bool* renderHealthBar,
            bool* renderEnergyBar,
            bool* renderDetails,
            bool* renderPlayerName,
            bool* showBurstDps,
            bool* showDamageNumbers) {
            if (ImGui::CollapsingHeader(categoryName, ImGuiTreeNodeFlags_DefaultOpen)) {
                // Group 1: Core geometric visuals. These are fundamental.
                ImGui::SeparatorText("Core Visuals");

                CheckboxWithId("Show Box", categoryName, &renderBox, nullptr);
                ImGui::SameLine(150); // Use a fixed position for clean alignment
                CheckboxWithId("Show Distance", categoryName, &renderDistance, nullptr);
                ImGui::SameLine(300); // Use a fixed position for clean alignment
                CheckboxWithId("Show Dot", categoryName, &renderDot, nullptr);

                // Check if there are any informational overlays to show.
                // If not, we don't even render the separator, keeping the UI clean.
                bool hasInfoOverlays = (renderHealthBar || renderDetails || renderPlayerName || showBurstDps || showDamageNumbers);

                if (hasInfoOverlays) {
                    // Group 2: Informational text and data overlays.
                    ImGui::SeparatorText("Informational Overlays");

                    if (renderHealthBar) {
                        CheckboxWithId("Show Health Bar", categoryName, renderHealthBar, nullptr);
                    }
                    if (showDamageNumbers) {
                        CheckboxWithId("Show Damage Numbers", categoryName, showDamageNumbers, "Displays floating combat text for incoming damage.");
                    }
                    if (showBurstDps) {
                        ImGui::SameLine();
                        CheckboxWithId("Show Burst DPS", categoryName, showBurstDps, "Displays the real-time burst DPS a target is taking from all sources. Ideal for tracking burn phases and overall damage pressure.");
                    }

                    if (renderEnergyBar) {
                        CheckboxWithId("Show Energy Bar", categoryName, renderEnergyBar, nullptr);
                    }
                    if (renderPlayerName) {
                        CheckboxWithId("Show Player Name", categoryName, renderPlayerName, nullptr);
                    }
                    if (renderDetails) {
                        CheckboxWithId("Show Details", categoryName, renderDetails, nullptr);
                    }
                }
            }
        }

    } // namespace GUI
} // namespace kx