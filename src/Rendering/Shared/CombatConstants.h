#pragma once

#include <cstdint>

#include "../../../libs/ImGui/imgui.h"

namespace kx {

    namespace CombatEffects {
	    // If no new damage is received for this duration, the current burst is considered "over"
	    // and the remaining accumulated damage will be flushed.
	    constexpr uint64_t BURST_INACTIVITY_TIMEOUT_MS = 1800; // 1.8 seconds

        // NEW: Force a flush if a burst lasts longer than this, ensuring feedback during constant pressure.
        constexpr uint64_t MAX_BURST_DURATION_MS = 8000; // 8 seconds

	    // The elegant fade-out animation for the chunk.
	    constexpr uint64_t DAMAGE_ACCUMULATOR_FADE_MS = 1000;

        // NEW: A very short delay after the last hit on a dead target to ensure trailing/overkill damage is captured.
        constexpr uint64_t POST_MORTEM_FLUSH_DELAY_MS = 200; // 0.2 seconds

        // NEW: Upward scroll distance for the damage number display.
        constexpr float    DAMAGE_NUMBER_MAX_Y_OFFSET  = 50.0f; // 50 pixels

        // --- Core Combat Feedback (TUNED FOR PUNCHY HITS) ---
        // A 200ms hold followed by a 400ms fade provides satisfying impact on every hit.
        constexpr uint64_t DAMAGE_FLASH_HOLD_DURATION_MS = 200;
        constexpr uint64_t DAMAGE_FLASH_FADE_DURATION_MS = 400;
        constexpr uint64_t DAMAGE_FLASH_TOTAL_DURATION_MS = DAMAGE_FLASH_HOLD_DURATION_MS + DAMAGE_FLASH_FADE_DURATION_MS;

        // --- Healing Feedback (TUNED FOR CLARITY) ---
        // A quick flash confirms the heal, and a 2s overlay shows the amount restored.
        constexpr uint64_t HEAL_FLASH_DURATION_MS = 150;
        constexpr uint64_t HEAL_OVERLAY_DURATION_MS = 2000;
        constexpr uint64_t HEAL_OVERLAY_FADE_DURATION_MS = 400;
        constexpr uint64_t BURST_HEAL_WINDOW_MS = 350; // Groups rapid heals

        // --- Death Animation (TUNED FOR A SATISFYING FINISH) ---
        // A 2.5s animation that gives a definitive and polished end-of-combat signal.
        constexpr uint64_t DEATH_BURST_DURATION_MS = 1000;
        constexpr uint64_t DEATH_FINAL_FADE_DURATION_MS = 2100;
        constexpr uint64_t DEATH_ANIMATION_TOTAL_DURATION_MS = DEATH_BURST_DURATION_MS + DEATH_FINAL_FADE_DURATION_MS;

        // --- Barrier Animation (TUNED FOR A SNAPPY POP) ---
        constexpr uint64_t BARRIER_ANIM_DURATION_MS = 250;

        // --- State Management ---
        constexpr uint64_t STATE_CLEANUP_THRESHOLD_MS = 3000;

        // --- DPS Display ---
        constexpr float DPS_FORMATTING_THRESHOLD = 1000.0f;  // Display as "1.0k" above this value
        constexpr float DPS_FONT_SIZE_MULTIPLIER = 0.9f;      // 90% of base font size
    }

    namespace ESPBarColors {
        // Base health derives from entityColor plus layout alpha, no fixed color here

        // Healing
        constexpr unsigned int HEAL_OVERLAY = IM_COL32(120, 255, 160, 200);  // soft mint, readable over HP
        constexpr unsigned int HEAL_FLASH = IM_COL32(220, 255, 255, 255);  // cold white flash

        // Damage
        constexpr unsigned int DAMAGE_ACCUM = IM_COL32(255, 170, 60, 180); // warm amber, sustained loss
        constexpr unsigned int DAMAGE_FLASH = IM_COL32(255, 255, 255, 255); // neutral white flash

        // Barrier
        constexpr unsigned int BARRIER_FILL = IM_COL32(255, 230, 180, 240); // warm cream
        constexpr unsigned int BARRIER_SEPARATOR = IM_COL32(255, 255, 255, 210); // separator on overflow

        // Death burst
        constexpr unsigned int DEATH_BURST = IM_COL32(200, 255, 255, 255);       // icy cyan

        // Energy bar
        // Keep using ESPColors::ENERGY_BAR for fill, it already fits the scheme

        // Burst DPS
        constexpr unsigned int BURST_DPS_TEXT = IM_COL32(255, 200, 50, 255); // Gold/yellow for DPS
    }

} // namespace kx
