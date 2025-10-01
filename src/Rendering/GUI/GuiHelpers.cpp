#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include <string>

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
                ImGui::Text("Visual Elements");

                std::string boxLabel = "Show Box##" + std::string(categoryName);
                std::string distLabel = "Show Distance##" + std::string(categoryName);
                std::string dotLabel = "Show Dot##" + std::string(categoryName);

                ImGui::Checkbox(boxLabel.c_str(), &renderBox);
                ImGui::SameLine();
                ImGui::Checkbox(distLabel.c_str(), &renderDistance);
                ImGui::SameLine();
                ImGui::Checkbox(dotLabel.c_str(), &renderDot);

                if (renderHealthBar) {
                    std::string healthLabel = "Show Health Bar##" + std::string(categoryName);
                    ImGui::Checkbox(healthLabel.c_str(), renderHealthBar);
                    if (renderDetails) ImGui::SameLine();
                }

                if (renderDetails) {
                    std::string detailsLabel = "Show Details##" + std::string(categoryName);
                    ImGui::Checkbox(detailsLabel.c_str(), renderDetails);
                }

                if (renderPlayerName) {
                    std::string nameLabel = "Show Player Name##" + std::string(categoryName);
                    ImGui::Checkbox(nameLabel.c_str(), renderPlayerName);
                }
            }
        }

    } // namespace GUI
} // namespace kx
