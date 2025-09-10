#define NOMINMAX

#include "ESPRenderer.h"

#include <algorithm>
#include <cstdio>                     // For snprintf
#include <gtc/type_ptr.hpp>

#include "ESP_Helpers.h"
#include "../../libs/ImGui/imgui.h"      // Required for ImGui types and functions
#include "../Core/AppState.h"
#include "../Game/AddressManager.h"
#include "../Game/GameStructs.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    // Skip ESP rendering if disabled
    if (!g_settings.espEnabled) return;
    
    // Skip if camera not initialized
    if (!s_camera) return;
    
    // Camera is updated by ImGuiManager before this is called
    
    // Check if ESP should be hidden
    if (ShouldHideESP(mumbleData)) return;
    
    // Get the ImGui background draw list
    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    // Render agents
    uintptr_t agentArrayPtr = AddressManager::GetAgentArray();
    if (agentArrayPtr) {
        AgentArray agentArray(reinterpret_cast<void*>(agentArrayPtr));
        uint32_t count = agentArray.Count();
        
        for (uint32_t i = 0; i < count; ++i) {
            Agent agent = agentArray.GetAgent(i);
            if (!agent) continue;
            
            Coordinates3D gameWorldPos = agent.GetPosition();
            if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) {
                continue;
            }
            
            RenderAgent(drawList, agent, screenWidth, screenHeight);
        }
    }
}

void ESPRenderer::RenderAgent(ImDrawList* drawList, Agent& agent, float screenWidth, float screenHeight) {
    Coordinates3D gameWorldPos = agent.GetPosition();
    const float scaleFactor = 1.23f;

    // Transform from Game (Z-up) to Camera's World (Y-up)
    glm::vec3 cameraWorldPos(
        gameWorldPos.X / scaleFactor,  // X is the same
        gameWorldPos.Z / scaleFactor,  // Game Z (height) -> Camera Y (height)
        gameWorldPos.Y / scaleFactor   // Game Y (depth) -> Camera Z (depth)
    );
    
    float distance = glm::length(cameraWorldPos - s_camera->GetPlayerPosition());

    // Apply distance limit
    if (kx::g_settings.espUseDistanceLimit && distance > kx::g_settings.espRenderDistanceLimit) {
        return; // Don't render if beyond limit
    }
    
    glm::vec2 screenPos;
    if (ESP_Helpers::WorldToScreen(cameraWorldPos, *s_camera, screenWidth, screenHeight, screenPos)) {
        ImU32 lineColor = IM_COL32(
            std::min(255, int(255 * (1.0f - distance / 200.0f))),
            100,
            std::min(255, int(255 * (distance / 200.0f))),
            200
        );

        float boxSize = std::max(4.0f, 15.0f * (50.0f / (distance + 20.0f)));

        if (g_settings.espRenderBox) {
            drawList->AddRect(
                ::ImVec2(screenPos.x - boxSize / 2, screenPos.y - boxSize / 2),
                ::ImVec2(screenPos.x + boxSize / 2, screenPos.y + boxSize / 2),
                lineColor,
                1.0f,
                ImDrawFlags_RoundCornersAll,
                1.5f
            );
        }

        if (g_settings.espRenderDistance) {
            char distText[32];
            snprintf(distText, sizeof(distText), "%.1fm", distance);

            ::ImVec2 textSize = ImGui::CalcTextSize(distText);

            drawList->AddRectFilled(
                ::ImVec2(screenPos.x - textSize.x / 2 - 2, screenPos.y - boxSize / 2 - textSize.y - 4),
                ::ImVec2(screenPos.x + textSize.x / 2 + 2, screenPos.y - boxSize / 2),
                IM_COL32(0, 0, 0, 180)
            );

            drawList->AddText(
                ::ImVec2(screenPos.x - textSize.x / 2, screenPos.y - boxSize / 2 - textSize.y - 2),
                IM_COL32(255, 255, 255, 255),
                distText
            );
        }

        if (g_settings.espRenderDot) {
            drawList->AddCircleFilled(
                ::ImVec2(screenPos.x, screenPos.y),
                2.0f,
                IM_COL32(255, 255, 255, 255)
            );
        }

        if (g_settings.espRenderDetails) {
            // Vertical position for additional text, starting below the box
            float textY = screenPos.y + boxSize / 2 + 2; // 2 pixels padding below the box

            // Display Type
            char typeText[64];
            snprintf(typeText, sizeof(typeText), "Type: %d", agent.GetType());
            ::ImVec2 typeTextSize = ImGui::CalcTextSize(typeText);

            drawList->AddRectFilled(
                ::ImVec2(screenPos.x - typeTextSize.x / 2 - 2, textY),
                ::ImVec2(screenPos.x + typeTextSize.x / 2 + 2, textY + typeTextSize.y + 4),
                IM_COL32(0, 0, 0, 180)
            );
            drawList->AddText(
                ::ImVec2(screenPos.x - typeTextSize.x / 2, textY + 2),
                IM_COL32(255, 255, 255, 255),
                typeText
            );
            textY += typeTextSize.y + 6; // Advance Y for the next line of text

            // Display Gadget Type
            char gadgetTypeText[64];
            snprintf(gadgetTypeText, sizeof(gadgetTypeText), "Gadget: %d", agent.GetGadgetType());
            ::ImVec2 gadgetTypeTextSize = ImGui::CalcTextSize(gadgetTypeText);

            drawList->AddRectFilled(
                ::ImVec2(screenPos.x - gadgetTypeTextSize.x / 2 - 2, textY),
                ::ImVec2(screenPos.x + gadgetTypeTextSize.x / 2 + 2, textY + gadgetTypeTextSize.y + 4),
                IM_COL32(0, 0, 0, 180)
            );
            drawList->AddText(
                ::ImVec2(screenPos.x - gadgetTypeTextSize.x / 2, textY + 2),
                IM_COL32(255, 255, 255, 255),
                gadgetTypeText
            );
        }
    }
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx
