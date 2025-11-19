#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"

namespace kx {
    namespace GUI {

        void RenderPlayerStyleSettings(PlayerEspSettings& settings) {
            ImGui::SeparatorText("Core Visuals");
            ImGui::Checkbox("Show Box##Player", &settings.renderBox); ImGui::SameLine();
            ImGui::Checkbox("3D Wireframe##Player", &settings.renderWireframe);
            ImGui::Checkbox("Show Dot##Player", &settings.renderDot); ImGui::SameLine();
            ImGui::Checkbox("Show Distance##Player", &settings.renderDistance);

            ImGui::SeparatorText("Status Bars");
            ImGui::Checkbox("Show Health Bar##Player", &settings.renderHealthBar);
            if (settings.renderHealthBar) {
                ImGui::SameLine();
                ImGui::Checkbox("Show %##Player", &settings.showHealthPercentage); ImGui::SameLine();
                ImGui::Checkbox("Only show damaged##Player", &settings.showOnlyDamaged);
            }
            ImGui::Checkbox("Show Energy Bar##Player", &settings.renderEnergyBar);

            ImGui::SeparatorText("Text & Labels");
            ImGui::Checkbox("Show Player Name##Player", &settings.renderPlayerName);

            ImGui::SeparatorText("Floating Combat Text");
            ImGui::Checkbox("Show Damage Numbers##Player", &settings.showDamageNumbers); ImGui::SameLine();
            ImGui::Checkbox("Show Burst DPS##Player", &settings.showBurstDps);
        }

        void RenderNpcStyleSettings(NpcEspSettings& settings) {
            ImGui::SeparatorText("Core Visuals");
            ImGui::Checkbox("Show Box##NPC", &settings.renderBox); ImGui::SameLine();
            ImGui::Checkbox("3D Wireframe##NPC", &settings.renderWireframe);
            ImGui::Checkbox("Show Dot##NPC", &settings.renderDot); ImGui::SameLine();
            ImGui::Checkbox("Show Distance##NPC", &settings.renderDistance);

            ImGui::SeparatorText("Status Bars");
            ImGui::Checkbox("Show Health Bar##NPC", &settings.renderHealthBar);
            if (settings.renderHealthBar) {
                ImGui::SameLine();
                ImGui::Checkbox("Show %##NPC", &settings.showHealthPercentage); ImGui::SameLine();
                ImGui::Checkbox("Only show damaged##NPC", &settings.showOnlyDamaged);
            }

            ImGui::SeparatorText("Floating Combat Text");
            ImGui::Checkbox("Show Damage Numbers##NPC", &settings.showDamageNumbers); ImGui::SameLine();
            ImGui::Checkbox("Show Burst DPS##NPC", &settings.showBurstDps);
        }

        void RenderObjectStyleSettings(ObjectEspSettings& settings) {
            ImGui::SeparatorText("Core Visuals");
            ImGui::Checkbox("Show Box##Object", &settings.renderBox); ImGui::SameLine();
            ImGui::Checkbox("3D Wireframe##Object", &settings.renderWireframe);
            if (settings.renderBox || settings.renderWireframe) {
                ImGui::Indent();
                ImGui::SliderFloat("Max Height##ObjectBox", &settings.maxBoxHeight, 1.0f, 100.0f, "%.1f m");
                ImGui::TextDisabled("Hide boxes and wireframes for gadgets taller than this");
                ImGui::Unindent();
            }
            ImGui::Checkbox("2D Circle##Object", &settings.renderCircle); ImGui::SameLine();
            ImGui::Checkbox("3D Sphere##Object", &settings.renderSphere);
            ImGui::Checkbox("Show Dot##Object", &settings.renderDot); ImGui::SameLine();
            ImGui::Checkbox("Show Distance##Object", &settings.renderDistance);

            ImGui::SeparatorText("Status Bars");
            ImGui::Checkbox("Show Health Bar##Object", &settings.renderHealthBar);
            if (settings.renderHealthBar) {
                ImGui::SameLine();
                ImGui::Checkbox("Show %##Object", &settings.showHealthPercentage); ImGui::SameLine();
                ImGui::Checkbox("Only show damaged##Object", &settings.showOnlyDamaged);
            }

            ImGui::SeparatorText("Floating Combat Text");
            ImGui::Checkbox("Show Damage Numbers##Object", &settings.showDamageNumbers); ImGui::SameLine();
            ImGui::Checkbox("Show Burst DPS##Object", &settings.showBurstDps);
        }

    } // namespace GUI
} // namespace kx
