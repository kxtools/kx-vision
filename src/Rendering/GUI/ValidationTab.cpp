#include "ValidationTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../../libs/ImGui/imgui_stdlib.h"
#include <string>
#include <sstream>

// These tell the compiler that these exist in another file (TestRunner.cpp).
extern void RunAllTests();
extern std::stringstream g_testResults;

namespace kx {
    namespace GUI {
        void RenderValidationTab() {
            if (ImGui::BeginTabItem("Validation")) {
                ImGui::TextWrapped("Run these tests in the PvP Lobby for best results.");
                ImGui::Separator();

                static std::string resultsStr;

                if (ImGui::Button("Run Core Pointer Test")) {
                    RunAllTests();
                    resultsStr = g_testResults.str();
                }

                ImGui::Separator();
                ImGui::Text("Results:");

                ImGui::InputTextMultiline("##Results",
                    &resultsStr,
                    ImVec2(-1.0f, -1.0f),
                    ImGuiInputTextFlags_ReadOnly);

                ImGui::EndTabItem();
            }
        }
    }
}