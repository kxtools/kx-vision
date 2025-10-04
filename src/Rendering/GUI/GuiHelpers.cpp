#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include <string>

// A local helper to encapsulate the ImGui ID generation, cleaning up the main function.
static void CheckboxWithId(const char* label, const char* categoryName, bool* value) {
    // This creates a unique ID like "Show Box##Players" to prevent ImGui ID collisions.
    std::string id_label = std::string(label) + "##" + std::string(categoryName);
    ImGui::Checkbox(id_label.c_str(), value);
}

namespace kx {
    namespace GUI {

        void RenderCategoryStyleSettings(const char* categoryName,
            bool& renderBox,
            bool& renderDistance,
            bool& renderDot,
            bool* renderHealthBar,
            bool* renderDetails,
            bool* renderPlayerName) {
            if (ImGui::CollapsingHeader(categoryName, ImGuiTreeNodeFlags_DefaultOpen)) {
                // Group 1: Core geometric visuals. These are fundamental.
                ImGui::SeparatorText("Core Visuals");

                CheckboxWithId("Show Box", categoryName, &renderBox);
                ImGui::SameLine(150); // Use a fixed position for clean alignment
                CheckboxWithId("Show Distance", categoryName, &renderDistance);
                ImGui::SameLine(300); // Use a fixed position for clean alignment
                CheckboxWithId("Show Dot", categoryName, &renderDot);

                // Check if there are any informational overlays to show.
                // If not, we don't even render the separator, keeping the UI clean.
                bool hasInfoOverlays = (renderHealthBar || renderDetails || renderPlayerName);

                if (hasInfoOverlays) {
                    // Group 2: Informational text and data overlays.
                    ImGui::SeparatorText("Informational Overlays");

                    if (renderHealthBar) {
                        CheckboxWithId("Show Health Bar", categoryName, renderHealthBar);
                    }
                    if (renderPlayerName) {
                        CheckboxWithId("Show Player Name", categoryName, renderPlayerName);
                    }
                    if (renderDetails) {
                        CheckboxWithId("Show Details", categoryName, renderDetails);
                    }
                }
            }
        }

    } // namespace GUI
} // namespace kx