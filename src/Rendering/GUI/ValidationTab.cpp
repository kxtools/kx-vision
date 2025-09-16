#include "ValidationTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../../libs/ImGui/imgui_stdlib.h"
#include <string>
#include <sstream>

extern void RunAllTests();
extern std::stringstream g_testResults;

namespace kx {
    namespace GUI {
        void RenderValidationTab() {
            if (ImGui::BeginTabItem("Validation")) {
                // We use a static bool to track the state across multiple frames.
                // It will only be false until the button is clicked for the first time.
                static bool testsHaveBeenRun = false;

                ImGui::TextWrapped("Run these tests in the PvP Lobby for best results.");
                ImGui::Separator();

                // If the tests have been run, we want to disable the button.
                if (testsHaveBeenRun) {
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Run Core Pointer Test");
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::SetTooltip("Tests can only be run once per session.\nPlease re-inject the DLL to run the suite again.");
                    }
                }
                else {
                    // This is the normal, clickable button.
                    if (ImGui::Button("Run Core Pointer Test")) {
                        RunAllTests();
                        // Once clicked, we set the flag to true to disable future clicks.
                        testsHaveBeenRun = true;
                    }
                }

                ImGui::Separator();
                ImGui::Text("Results:");

                // The results display logic remains the same, but we need to use a static string
                // to ensure the results persist after the test run.
                static std::string resultsStr;
                if (testsHaveBeenRun && resultsStr.empty()) {
                    resultsStr = g_testResults.str();
                }

                ImGui::InputTextMultiline("##Results", &resultsStr,
                    ImVec2(-1.0f, -1.0f),
                    ImGuiInputTextFlags_ReadOnly);

                ImGui::EndTabItem();
            }
        }
    }
}