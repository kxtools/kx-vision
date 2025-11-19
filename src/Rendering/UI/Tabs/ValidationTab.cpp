#include "ValidationTab.h"
#include <string>
#include <sstream>

#include "ImGui/imgui.h"

// Forward declare from TestRunner.cpp
extern void RunAllTests();
extern std::stringstream g_testResults;

namespace kx {
    namespace GUI {
        void RenderValidationTab() {
            if (ImGui::BeginTabItem("Validation")) {
                static bool testsHaveBeenRun = false;
                static std::string resultsStr; // Keep results stored

                ImGui::TextWrapped("Run these tests in the PvP Lobby for best results.");
                ImGui::Separator();

                // --- Test Runner Controls ---
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
                        resultsStr = g_testResults.str(); // Capture the results
                        testsHaveBeenRun = true;
                    }
                }

                // Add the "Copy to Clipboard" button
                // It is disabled until the tests have been run.
                if (!testsHaveBeenRun) {
                    ImGui::SameLine();
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Copy Results");
                    ImGui::PopStyleVar();
                }
                else {
                    ImGui::SameLine();
                    if (ImGui::Button("Copy Results")) {
                        ImGui::SetClipboardText(resultsStr.c_str());
                    }
                }

                ImGui::Separator();
                ImGui::Text("Results:");

                // --- Results Display ---
                // A simple scrollable child window is perfect for displaying colored text.
                ImGui::BeginChild("ResultsRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

                if (!resultsStr.empty()) {
                    const ImVec4 greenColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
                    const ImVec4 redColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

                    std::stringstream ss(resultsStr);
                    std::string line;

                    while (std::getline(ss, line)) {
                        // Check for keywords to determine the color
                        if (line.find("passed:") != std::string::npos || line.find("All tests passed") != std::string::npos) {
                            ImGui::TextColored(greenColor, "%s", line.c_str());
                        }
                        else if (line.find("failed:") != std::string::npos || line.find("FAILED") != std::string::npos) {
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