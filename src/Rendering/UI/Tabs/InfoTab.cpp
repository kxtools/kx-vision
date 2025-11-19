#include "InfoTab.h"
#include <windows.h>
#include "ImGui/imgui.h"

namespace kx {
    namespace GUI {
        void RenderInfoTab() {
            if (ImGui::BeginTabItem("Info")) {
                ImGui::Text("About KX Vision");
                ImGui::Separator();
                
                // Credits
                ImGui::Text("KX Vision by Krixx");
                ImGui::Text("Visit kxtools.xyz for more tools!");
                ImGui::Separator();

                // External links section with consistent layout
                const char* links[][3] = {
                    {"GitHub:", "Repository", "https://github.com/kxtools/kx-vision"},
                    {"Website:", "kxtools.xyz", "https://kxtools.xyz"},
                    {"Discord:", "Join Server", "https://discord.gg/z92rnB4kHm"}
                };

                // Render all links with consistent formatting
                for (const auto& link : links) {
                    ImGui::Text("%s", link[0]);
                    ImGui::SameLine();
                    if (ImGui::Button(link[1])) {
                        ShellExecuteA(NULL, "open", link[2], NULL, NULL, SW_SHOWNORMAL);
                    }
                }
                
                ImGui::Spacing();
                ImGui::EndTabItem();
            }
        }
    }
}
