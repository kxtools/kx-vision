#pragma once

#include "../RenderableData.h"
#include "../../Game/ReClassStructs.h"

namespace kx {

    /**
     * @brief A static helper class that encapsulates the logic for extracting data
     *        for a single entity from game memory structures into a safe Renderable object.
     */
    class EntityExtractor {
    public:
        /**
         * @brief Populates a RenderablePlayer object from a ChCliCharacter game structure.
         * @param outPlayer The RenderablePlayer object to populate (from an object pool).
         * @param inCharacter The source ChCliCharacter structure from the game.
         * @param playerName The player's name, obtained from the player list.
         * @param localPlayerPtr A pointer to the local player's character object for comparison.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractPlayer(RenderablePlayer& outPlayer,
            const ReClass::ChCliCharacter& inCharacter,
            const wchar_t* playerName,
            void* localPlayerPtr);

        /**
         * @brief Populates a RenderableNpc object from a ChCliCharacter game structure.
         * @param outNpc The RenderableNpc object to populate (from an object pool).
         * @param inCharacter The source ChCliCharacter structure from the game.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractNpc(RenderableNpc& outNpc,
            const ReClass::ChCliCharacter& inCharacter);

        /**
         * @brief Populates a RenderableGadget object from a GdCliGadget game structure.
         * @param outGadget The RenderableGadget object to populate (from an object pool).
         * @param inGadget The source GdCliGadget structure from the game.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractGadget(RenderableGadget& outGadget,
            const ReClass::GdCliGadget& inGadget);

    private:
        /**
         * @brief Helper to encapsulate the detailed gear extraction logic for a player.
         * @param outPlayer The RenderablePlayer object to add gear information to.
         * @param inventory The player's inventory structure from the game.
         */
        static void ExtractGear(RenderablePlayer& outPlayer, const ReClass::Inventory& inventory);
    };

} // namespace kx