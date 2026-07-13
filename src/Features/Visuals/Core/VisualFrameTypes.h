#pragma once

#include "../Settings/VisualsSettings.h"
#include "../../../Game/Services/Camera/Camera.h"
#include "../../../Game/Services/Combat/CombatObservation.h"
#include "../../../Game/Services/Combat/CombatObservationKey.h"
#include "../Combat/VisualCombatState.h"
#include "../../../../libs/ImGui/imgui.h"

#include <ankerl/unordered_dense.h>
#include <array>
#include <cstdint>

namespace kx {

struct VisualStyle {
    float scale = 0.0f;
    float distanceFadeAlpha = 0.0f;
    float finalAlpha = 0.0f;
    unsigned int fadedEntityColor = 0;
    float finalFontSize = 0.0f;
    float finalBoxThickness = 0.0f;
    float finalDotRadius = 0.0f;
    float finalHealthBarWidth = 0.0f;
    float finalHealthBarHeight = 0.0f;
};

struct ScreenGeometry {
    glm::vec2 screenPos{0.0f};
    ImVec2 boxMin{};
    ImVec2 boxMax{};
    ImVec2 center{};
    float circleRadius = 0.0f;
    std::array<glm::vec2, 8> projectedCorners{};
    std::array<bool, 8> cornerValidity{};
    bool isOnScreen = false;
};

struct VisualProperties {
    VisualStyle style;
    ScreenGeometry geometry;
};

struct FrameContext {
    uint64_t now;
    Camera& camera;
    const ankerl::unordered_dense::map<CombatObservationKey, CombatObservation, CombatObservationKeyHash>& combatObservations;
    const ankerl::unordered_dense::map<CombatObservationKey, VisualCombatState, CombatObservationKeyHash>& visualCombatStates;
    const VisualsConfiguration& settings;
    float adaptiveFarPlane;
    ImDrawList* drawList;
    float screenWidth;
    float screenHeight;
    bool isInWvW;
};

} // namespace kx
