#include "ValidationTab.h"
#include "../../../libs/ImGui/imgui.h"
#include <string>
#include <sstream>

extern void RunAllTests();
extern std::stringstream g_testResults;

namespace kx {
    namespace GUI {
        void RenderValidationTab() {
            if (ImGui::BeginTabItem("Validation")) {
                static bool testsHaveBeenRun = false;

                ImGui::TextWrapped("Run these tests in the PvP Lobby for best results.");
                ImGui::Separator();

                if (testsHaveBeenRun) {
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Run Core Pointer Test");
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::SetTooltip("Tests can only be run once per session.\nPlease re-inject the DLL to run the suite again.");
                    }
                }
                else {
                    if (ImGui::Button("Run Core Pointer Test")) {
                        RunAllTests();
                        testsHaveBeenRun = true;
                    }
                }

                ImGui::Separator();
                ImGui::Text("Results:");

                // Create a scrollable child window to contain the results
                // This mimics the behavior of the InputTextMultiline
                ImGui::BeginChild("ResultsRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

                static std::string resultsStr;
                if (testsHaveBeenRun && resultsStr.empty()) {
                    resultsStr = g_testResults.str();
                }

                if (!resultsStr.empty()) {
                    // Define our colors for pass and fail
                    const ImVec4 greenColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
                    const ImVec4 redColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

                    // Use a stringstream to easily read the results line by line
                    std::stringstream ss(resultsStr);
                    std::string line;

                    while (std::getline(ss, line)) {
                        // Check for keywords to determine the color
                        if (line.find("passed:") != std::string::npos || line.find("All tests passed") != std::string::npos) {
                            ImGui::TextColored(greenColor, "%s", line.c_str());
                        }
                        else if (line.find("failed:") != std::string::npos || line.find("FAILED") != std::string::npos || line.find("fail") != std::string::npos) {
                            ImGui::TextColored(redColor, "%s", line.c_str());
                        }
                        else {
                            // Default color for headers and other info
                            ImGui::TextUnformatted(line.c_str());
                        }
                    }
                }

                ImGui::EndChild(); // End the scrollable region

                ImGui::EndTabItem();
            }
        }
    }
}