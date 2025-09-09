#define NOMINMAX

#include "ESPRenderer.h"
#include "../libs/ImGui/imgui.h"      // Required for ImGui types and functions
#include "AddressManager.h"
#include "GameStructs.h"
#include "ESP_Helpers.h"
#include "AppState.h"
#include <algorithm>
#include <gtc/type_ptr.hpp>
#include <cstdio>                     // For snprintf

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::Render(float screenWidth, float screenHeight) {
    // Skip ESP rendering if disabled
    if (!g_espEnabled) return;
    
    // Skip if camera not initialized
    if (!s_camera) return;
    
    // Update camera data once per frame
    s_camera->Update();
    
    // Check if ESP should be hidden
    if (ShouldHideESP()) return;
    
    // Get the ImGui background draw list (ImDrawList is not in the kx namespace)
    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    // Render agents
    uintptr_t agentArrayPtr = AddressManager::GetAgentArray();
    if (agentArrayPtr) {
        AgentArray agentArray(reinterpret_cast<void*>(agentArrayPtr));
        uint32_t count = agentArray.Count();
        
        for (uint32_t i = 0; i < count; ++i) {
            Agent agent = agentArray.GetAgent(i);
            if (!agent) continue;
            
            // Check for valid position
            Coordinates3D gameWorldPos = agent.GetPosition();
            if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) {
                continue;
            }
            
            // Render the agent
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
    
    // Calculate distance to agent for scaling and coloring
    float distance = glm::length(cameraWorldPos - s_camera->GetPlayerPosition());
    
    glm::vec2 screenPos;
    if (ESP_Helpers::WorldToScreen(cameraWorldPos, *s_camera, screenWidth, screenHeight, screenPos)) {
        // Set color based on distance (closer = more red, further = more blue)
        ImU32 lineColor = IM_COL32(
            std::min(255, int(255 * (1.0f - distance / 200.0f))), // R (red for close)
            100,                                                  // G (constant)
            std::min(255, int(255 * (distance / 200.0f))),        // B (blue for far)
            200                                                   // Alpha
        );
        
        // Calculate box size
        float boxSize = std::max(4.0f, 15.0f * (50.0f / (distance + 20.0f)));
        
        // Draw box
        if (g_espRenderBox) {
            drawList->AddRect(
                ::ImVec2(screenPos.x - boxSize / 2, screenPos.y - boxSize / 2),
                ::ImVec2(screenPos.x + boxSize / 2, screenPos.y + boxSize / 2),
                lineColor,
                1.0f,
                ImDrawFlags_RoundCornersAll,
                1.5f
            );
        }
        
        // Draw distance text
        if (g_espRenderDistance) {
            char distText[32];
            snprintf(distText, sizeof(distText), "%.1fm", distance);
            
            // Calculate text dimensions
            ::ImVec2 textSize = ImGui::CalcTextSize(distText);
            
            // Draw background for text
            drawList->AddRectFilled(
                ::ImVec2(screenPos.x - textSize.x / 2 - 2, screenPos.y - boxSize / 2 - textSize.y - 4),
                ::ImVec2(screenPos.x + textSize.x / 2 + 2, screenPos.y - boxSize / 2),
                IM_COL32(0, 0, 0, 180)
            );
            
            // Draw text
            drawList->AddText(
                ::ImVec2(screenPos.x - textSize.x / 2, screenPos.y - boxSize / 2 - textSize.y - 2),
                IM_COL32(255, 255, 255, 255),
                distText
            );
        }
        
        // Draw center dot
        if (g_espRenderDot) {
            drawList->AddCircleFilled(
                ::ImVec2(screenPos.x, screenPos.y),
                2.0f,
                IM_COL32(255, 255, 255, 255)
            );
        }
    }
}

bool ESPRenderer::ShouldHideESP() {
    // Hide ESP if map is open
    if (s_camera->IsMumbleLinkInitialized() && 
        (s_camera->GetMumbleLinkData()->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx
