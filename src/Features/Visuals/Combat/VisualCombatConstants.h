#pragma once

#include "../../../../libs/ImGui/imgui.h"

#include <cstdint>

namespace kx::VisualCombatEffects {

constexpr uint64_t BURST_INACTIVITY_TIMEOUT_MS = 1800;
constexpr uint64_t MAX_BURST_DURATION_MS = 8000;
constexpr uint64_t DAMAGE_ACCUMULATOR_FADE_MS = 1000;
constexpr uint64_t POST_MORTEM_FLUSH_DELAY_MS = 200;
constexpr float DAMAGE_NUMBER_MAX_Y_OFFSET = 50.0f;

constexpr uint64_t DAMAGE_FLASH_HOLD_DURATION_MS = 200;
constexpr uint64_t DAMAGE_FLASH_FADE_DURATION_MS = 400;
constexpr uint64_t DAMAGE_FLASH_TOTAL_DURATION_MS =
    DAMAGE_FLASH_HOLD_DURATION_MS + DAMAGE_FLASH_FADE_DURATION_MS;

constexpr uint64_t HEAL_FLASH_DURATION_MS = 150;
constexpr uint64_t HEAL_OVERLAY_DURATION_MS = 2000;
constexpr uint64_t HEAL_OVERLAY_FADE_DURATION_MS = 400;
constexpr uint64_t BURST_HEAL_WINDOW_MS = 350;

constexpr uint64_t DEATH_BURST_DURATION_MS = 1000;
constexpr uint64_t DEATH_FINAL_FADE_DURATION_MS = 2100;
constexpr uint64_t DEATH_ANIMATION_TOTAL_DURATION_MS =
    DEATH_BURST_DURATION_MS + DEATH_FINAL_FADE_DURATION_MS;

constexpr uint64_t BARRIER_ANIM_DURATION_MS = 250;

constexpr float DPS_FORMATTING_THRESHOLD = 1000.0f;
constexpr float DPS_FONT_SIZE_MULTIPLIER = 0.9f;

} // namespace kx::VisualCombatEffects

namespace kx::VisualCombatColors {

constexpr unsigned int HEAL_OVERLAY = IM_COL32(120, 255, 160, 200);
constexpr unsigned int HEAL_FLASH = IM_COL32(220, 255, 255, 255);
constexpr unsigned int DAMAGE_ACCUM = IM_COL32(255, 170, 60, 180);
constexpr unsigned int DAMAGE_FLASH = IM_COL32(255, 255, 255, 255);
constexpr unsigned int BARRIER_FILL = IM_COL32(255, 230, 180, 240);
constexpr unsigned int BARRIER_SEPARATOR = IM_COL32(255, 255, 255, 210);
constexpr unsigned int DEATH_BURST = IM_COL32(200, 255, 255, 255);
constexpr unsigned int BURST_DPS_TEXT = IM_COL32(255, 200, 50, 255);

} // namespace kx::VisualCombatColors
