#define NOMINMAX

#include "ESPRenderer.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <gtc/type_ptr.hpp>

#include "ESP_Helpers.h"
#include "../../libs/ImGui/imgui.h"
#include "../Core/AppState.h"
#include "../Game/AddressManager.h"
#include "../Game/GameStructs.h"
#include "../Game/ReClassStructs.h"

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

// The new unified rendering function
void ESPRenderer::RenderEntity(ImDrawList* drawList, const glm::vec3& worldPos, float distance, float screenWidth, float screenHeight, unsigned int color, const std::vector<std::string>& details) {
    if (g_settings.espUseDistanceLimit && distance > g_settings.espRenderDistanceLimit) {
        return;
    }

    glm::vec2 screenPos;
    if (ESP_Helpers::WorldToScreen(worldPos, *s_camera, screenWidth, screenHeight, screenPos)) {
        float boxSize = std::max(4.0f, 15.0f * (50.0f / (distance + 20.0f)));

        if (g_settings.espRenderBox) {
            drawList->AddRect(
                ::ImVec2(screenPos.x - boxSize / 2, screenPos.y - boxSize / 2),
                ::ImVec2(screenPos.x + boxSize / 2, screenPos.y + boxSize / 2),
                color, 1.0f, ImDrawFlags_RoundCornersAll, 1.5f);
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
            drawList->AddText(::ImVec2(screenPos.x - textSize.x / 2, screenPos.y - boxSize / 2 - textSize.y - 2),
                IM_COL32(255, 255, 255, 255), distText);
        }

        if (g_settings.espRenderDot) {
            drawList->AddCircleFilled(::ImVec2(screenPos.x, screenPos.y), 2.0f, IM_COL32(255, 255, 255, 255));
        }

        if (g_settings.espRenderDetails && !details.empty()) {
            float textY = screenPos.y + boxSize / 2 + 2;
            for (const auto& detailText : details) {
                ::ImVec2 textSize = ImGui::CalcTextSize(detailText.c_str());
                drawList->AddRectFilled(
                    ::ImVec2(screenPos.x - textSize.x / 2 - 2, textY),
                    ::ImVec2(screenPos.x + textSize.x / 2 + 2, textY + textSize.y + 4),
                    IM_COL32(0, 0, 0, 180)
                );
                drawList->AddText(
                    ::ImVec2(screenPos.x - textSize.x / 2, textY + 2),
                    IM_COL32(255, 255, 255, 255),
                    detailText.c_str()
                );
                textY += textSize.y + 6;
            }
        }
    }
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    if (!s_camera || ShouldHideESP(mumbleData)) {
        return;
    }

    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    const float scaleFactor = 1.23f;

    // --- Render Agents (Legacy Method) ---
    if (g_settings.espRenderAgents) {
        try {
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

                    glm::vec3 cameraWorldPos(
                        gameWorldPos.X / scaleFactor,
                        gameWorldPos.Z / scaleFactor,
                        gameWorldPos.Y / scaleFactor
                    );

                    float distance = glm::length(cameraWorldPos - s_camera->GetPlayerPosition());

                    unsigned int color = IM_COL32(
                        std::min(255, int(255 * (1.0f - distance / 200.0f))),
                        100,
                        std::min(255, int(255 * (distance / 200.0f))),
                        200
                    );

                    std::vector<std::string> details;
                    if (g_settings.espRenderDetails) {
                        char typeText[64], gadgetText[64];
                        snprintf(typeText, sizeof(typeText), "Type: %d", agent.GetType());
                        snprintf(gadgetText, sizeof(gadgetText), "Gadget: %d", agent.GetGadgetType());
                        details.push_back(typeText);
                        details.push_back(gadgetText);
                    }

                    RenderEntity(drawList, cameraWorldPos, distance, screenWidth, screenHeight, color, details);
                }
            }
        }
        catch (...) {
            // Prevent crash
        }
    }

    // --- Render Characters (Modern Method) ---
    if (g_settings.espRenderCharacters) {
        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (pContextCollection) {
            try {
                kx::ReClass::ContextCollection ctxCollection(pContextCollection);
                if (!ctxCollection) return;

                kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
                if (!charContext) return;

                kx::ReClass::ChCliCharacter** characterList = charContext.GetCharacterList();
                uint32_t characterCapacity = charContext.GetCharacterListCapacity();

                if (!characterList || characterCapacity > 8000) return;

                const uintptr_t MIN_VALID_POINTER = 0x10000;
                const uintptr_t MAX_VALID_POINTER = 0x00007FFFFFFFFFFF;

                for (uint32_t i = 0; i < characterCapacity; ++i) {
                    void* rawCharacterPtr = characterList[i];
                    if (!rawCharacterPtr || (uintptr_t)rawCharacterPtr < MIN_VALID_POINTER || (uintptr_t)rawCharacterPtr > MAX_VALID_POINTER) {
                        continue;
                    }

                    kx::ReClass::ChCliCharacter character(rawCharacterPtr);
                    kx::ReClass::AgChar agent = character.GetAgent();
                    if (!agent) continue;
                    kx::ReClass::CoChar coChar = agent.GetCoChar();
                    if (!coChar) continue;

                    Coordinates3D gameWorldPos = coChar.GetVisualPosition();
                    if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) {
                        continue;
                    }

                    glm::vec3 cameraWorldPos(
                        gameWorldPos.X / scaleFactor,
                        gameWorldPos.Z / scaleFactor,
                        gameWorldPos.Y / scaleFactor
                    );

                    float distance = glm::length(cameraWorldPos - s_camera->GetPlayerPosition());
                    unsigned int color = IM_COL32(255, 0, 0, 200);

                    RenderEntity(drawList, cameraWorldPos, distance, screenWidth, screenHeight, color, {});
                }
            }
            catch (...) {
                // Prevent crash
            }
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