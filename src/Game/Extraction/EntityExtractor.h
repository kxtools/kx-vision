#pragma once

#include "../Data/RenderableData.h"
#include "../SdkStructs.h"

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
        static bool ExtractPlayer(PlayerEntity& outPlayer,
            const ReClass::ChCliCharacter& inCharacter,
            const wchar_t* playerName,
            void* localPlayerPtr);

        /**
         * @brief Populates a RenderableNpc object from a ChCliCharacter game structure.
         * @param outNpc The RenderableNpc object to populate (from an object pool).
         * @param inCharacter The source ChCliCharacter structure from the game.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractNpc(NpcEntity& outNpc,
            const ReClass::ChCliCharacter& inCharacter);

        /**
         * @brief Populates a RenderableGadget object from a GdCliGadget game structure.
         * @param outGadget The RenderableGadget object to populate (from an object pool).
         * @param inGadget The source GdCliGadget structure from the game.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractGadget(GadgetEntity& outGadget,
            const ReClass::GdCliGadget& inGadget);

        /**
         * @brief Populates a RenderableAttackTarget object from an AgentInl game structure.
         * @param outAttackTarget The RenderableAttackTarget object to populate (from an object pool).
         * @param inAgentInl The source AgentInl structure from the attack target list.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractAttackTarget(RenderableAttackTarget& outAttackTarget,
            const ReClass::AgentInl& inAgentInl);

        /**
         * @brief Populates a RenderableItem object from an ItCliItem game structure.
         * @param outItem The RenderableItem object to populate (from an object pool).
         * @param inItem The source ItCliItem structure from the item list.
         * @return True if extraction was successful and the entity is valid, false otherwise.
         */
        static bool ExtractItem(ItemEntity& outItem,
            const ReClass::ItCliItem& inItem);

    private:
        /**
         * @brief Helper to encapsulate the detailed gear extraction logic for a player.
         * @param outPlayer The RenderablePlayer object to add gear information to.
         * @param inventory The player's inventory structure from the game.
         */
        static void ExtractGear(PlayerEntity& outPlayer, const ReClass::ChCliInventory& inventory);

        // Common extraction pattern helpers
        static bool ValidateAndExtractGamePosition(const ReClass::ChCliCharacter& character, glm::vec3& outGamePos);
        static bool ValidateAndExtractGamePosition(const ReClass::GdCliGadget& gadget, glm::vec3& outGamePos);
        static bool ValidateAndExtractGamePosition(const ReClass::AgKeyFramed& agKeyframed, glm::vec3& outGamePos);
        static glm::vec3 TransformGamePositionToMumble(const glm::vec3& gamePos);
        static void ExtractHealthData(GameEntity& entity, const ReClass::ChCliHealth& health);
        static void ExtractHealthData(GameEntity& entity, const ReClass::GdCliHealth& health);
        
        /**
         * @brief Extract physics shape dimensions from player character
         * @param entity The entity to populate with dimensions
         * @param character The character to extract dimensions from
         * @note Players use HkpRigidBody path (CoChar+0x60) which provides full shape type detection
         */
        static void ExtractPlayerShapeDimensions(GameEntity& entity, const ReClass::ChCliCharacter& character);
        
        /**
         * @brief Extract physics box shape dimensions from NPC character
         * @param entity The entity to populate with dimensions
         * @param character The character to extract dimensions from
         * @note NPCs use HkpBoxShape path (CoCharSimpleCliWrapper+0xE8) which only supports BOX shapes
         */
        static void ExtractNpcShapeDimensions(GameEntity& entity, const ReClass::ChCliCharacter& character);
        
        /**
         * @brief Extract physics shape dimensions from gadget
         * @param entity The entity to populate with dimensions
         * @param gadget The gadget to extract dimensions from
         * @note Uses unified type-safe dimension extraction (supports CYLINDER, BOX, and MOPP shapes)
         */
        static void ExtractShapeDimensions(GameEntity& entity, const ReClass::GdCliGadget& gadget);
        
        /**
         * @brief Extract physics shape dimensions from AgKeyFramed (for attack targets)
         * @param entity The entity to populate with dimensions
         * @param agKeyframed The AgKeyFramed to extract dimensions from
         * @note Uses unified type-safe dimension extraction (supports CYLINDER, BOX, and MOPP shapes)
         */
        static void ExtractBoxShapeDimensions(GameEntity& entity, const ReClass::AgKeyFramed& agKeyframed);
        
    private:
        /**
         * @brief Internal helper to extract shape dimensions from CoKeyFramed
         * @param entity The entity to populate with dimensions
         * @param coKeyframed The CoKeyFramed to extract dimensions from
         * @note Uses unified type-safe dimension extraction (supports CYLINDER, BOX, and MOPP shapes)
         *       This path is used for gadgets and attack targets. Characters use ExtractBoxShapeDimensions instead.
         *       All dimensions are returned in meters with proper coordinate conversion applied.
         */
        static void ExtractShapeDimensionsFromCoKeyframed(GameEntity& entity, const ReClass::CoKeyFramed& coKeyframed);
        
        /**
         * @brief Internal helper to extract dimensions from HkpBoxShape
         * @param entity The entity to populate with dimensions
         * @param boxShape The HkpBoxShape to extract dimensions from
         */
        static void ExtractBoxShapeDimensionsFromHkpBoxShape(GameEntity& entity, const ReClass::HkpBoxShape& boxShape);
    };

} // namespace kx