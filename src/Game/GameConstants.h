#pragma once

namespace kx {
    namespace Game {
        
        // Maximum number of gadget types supported by the game
        constexpr int MAX_GADGET_TYPES = 32;

        // Combat and interaction range constants (in game units)
        namespace CombatRanges {
            constexpr float MELEE_RANGE = 130.0f;
            constexpr float RANGED_COMBAT_RANGE = 300.0f;
            constexpr float LONG_RANGE = 900.0f;
        }

        namespace InteractionRanges {
            constexpr float STANDARD_INTERACTION_RANGE = 300.0f;
            constexpr float MEDIUM_INTERACTION_RANGE = 600.0f;
        }

    } // namespace Game
} // namespace kx