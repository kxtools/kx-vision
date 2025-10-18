#include "EntityExtractor.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPFormatting.h"
#include "../../Game/GameEnums.h"
#include "../../Utils/StringHelpers.h"
#include <vector>

namespace kx {

    bool EntityExtractor::ExtractPlayer(RenderablePlayer& outPlayer,
        const ReClass::ChCliCharacter& inCharacter,
        const wchar_t* playerName,
        void* localPlayerPtr) {

        // --- Validation and Position ---
        ReClass::AgChar agent = inCharacter.GetAgent();
        if (!agent) return false;

        ReClass::CoChar coChar = agent.GetCoChar();
        if (!coChar) return false;

        glm::vec3 gameWorldPos = coChar.GetVisualPosition();
        if (gameWorldPos.x == 0.0f && gameWorldPos.y == 0.0f && gameWorldPos.z == 0.0f) return false;

        // --- Populate Core Data ---
        outPlayer.position = glm::vec3(
            gameWorldPos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
            gameWorldPos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
            gameWorldPos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR
        );
        outPlayer.isValid = true;
        outPlayer.entityType = ESPEntityType::Player;
        outPlayer.address = inCharacter.data();
        outPlayer.isLocalPlayer = (outPlayer.address == localPlayerPtr);
        if (playerName) {
            outPlayer.playerName = StringHelpers::WCharToUTF8String(playerName);
        }

        // --- Health & Energy ---
        ReClass::ChCliHealth health = inCharacter.GetHealth();
        if (health) {
            outPlayer.currentHealth = health.GetCurrent();
            outPlayer.maxHealth = health.GetMax();
            outPlayer.currentBarrier = health.GetBarrier();
        }

        // Dodge Energy
        ReClass::ChCliEnergies energies = inCharacter.GetEnergies();
        if (energies) {
            outPlayer.currentEnergy = energies.GetCurrent();
            outPlayer.maxEnergy = energies.GetMax();
        }

        // Special Energy
        ReClass::ChCliSpecialEnergies specialEnergies = inCharacter.GetSpecialEnergies();
        if (specialEnergies) {
            outPlayer.currentSpecialEnergy = specialEnergies.GetCurrent();
            outPlayer.maxSpecialEnergy = specialEnergies.GetMax();
        }

        // --- Core Stats ---
        ReClass::ChCliCoreStats coreStats = inCharacter.GetCoreStats();
        if (coreStats) {
            outPlayer.level = coreStats.GetLevel();
            outPlayer.scaledLevel = coreStats.GetScaledLevel();
            outPlayer.profession = coreStats.GetProfession();
            outPlayer.attitude = inCharacter.GetAttitude();
            outPlayer.race = coreStats.GetRace();
        }

        // --- Gear ---
        ReClass::Inventory inventory = inCharacter.GetInventory();
        if (inventory) {
            ExtractGear(outPlayer, inventory);
        }

        return true;
    }

    bool EntityExtractor::ExtractNpc(RenderableNpc& outNpc, const ReClass::ChCliCharacter& inCharacter) {

        // --- Validation and Position ---
        ReClass::AgChar agent = inCharacter.GetAgent();
        if (!agent) return false;

        ReClass::CoChar coChar = agent.GetCoChar();
        if (!coChar) return false;

        glm::vec3 gameWorldPos = coChar.GetVisualPosition();
        if (gameWorldPos.x == 0.0f && gameWorldPos.y == 0.0f && gameWorldPos.z == 0.0f) return false;

        // --- Populate Core Data ---
        outNpc.position = glm::vec3(
            gameWorldPos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
            gameWorldPos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
            gameWorldPos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR
        );
        outNpc.isValid = true;
        outNpc.entityType = ESPEntityType::NPC;
        outNpc.address = inCharacter.data();

        // --- Health ---
        ReClass::ChCliHealth health = inCharacter.GetHealth();
        if (health) {
            outNpc.currentHealth = health.GetCurrent();
            outNpc.maxHealth = health.GetMax();
            outNpc.currentBarrier = health.GetBarrier();
        }

        // --- Stats ---
        ReClass::ChCliCoreStats coreStats = inCharacter.GetCoreStats();
        if (coreStats) {
            outNpc.level = coreStats.GetLevel();
        }
        outNpc.attitude = inCharacter.GetAttitude();
        outNpc.rank = inCharacter.GetRank();

        return true;
    }

    bool EntityExtractor::ExtractGadget(RenderableGadget& outGadget, const ReClass::GdCliGadget& inGadget) {

        // --- Validation and Position ---
        ReClass::AgKeyFramed agKeyFramed = inGadget.GetAgKeyFramed();
        if (!agKeyFramed) return false;

        ReClass::CoKeyFramed coKeyFramed = agKeyFramed.GetCoKeyFramed();
        if (!coKeyFramed) return false;

        glm::vec3 gameWorldPos = coKeyFramed.GetPosition();
        if (gameWorldPos.x == 0.0f && gameWorldPos.y == 0.0f && gameWorldPos.z == 0.0f) return false;

        // --- Populate Core Data ---
        outGadget.position = glm::vec3(
            gameWorldPos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
            gameWorldPos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
            gameWorldPos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR
        );
        outGadget.isValid = true;
        outGadget.entityType = ESPEntityType::Gadget;
        outGadget.address = inGadget.data();
        outGadget.type = inGadget.GetGadgetType();
        outGadget.isGatherable = inGadget.IsGatherable();

        // --- Health ---
        ReClass::ChCliHealth health = inGadget.GetHealth();
        if (health) {
            outGadget.currentHealth = health.GetCurrent();
            outGadget.maxHealth = health.GetMax();
        }

        if (outGadget.type == Game::GadgetType::ResourceNode) {
            outGadget.resourceType = inGadget.GetResourceNodeType();
        }

        return true;
    }

    void EntityExtractor::ExtractGear(RenderablePlayer& outPlayer, const ReClass::Inventory& inventory) {
        outPlayer.gear.clear();

        const std::vector<Game::EquipmentSlot> slotsToCheck = {
            Game::EquipmentSlot::Helm, Game::EquipmentSlot::Shoulders, Game::EquipmentSlot::Chest,
            Game::EquipmentSlot::Gloves, Game::EquipmentSlot::Pants, Game::EquipmentSlot::Boots,
            Game::EquipmentSlot::Back, Game::EquipmentSlot::Amulet, Game::EquipmentSlot::Accessory1,
            Game::EquipmentSlot::Accessory2, Game::EquipmentSlot::Ring1, Game::EquipmentSlot::Ring2,
            Game::EquipmentSlot::MainhandWeapon1, Game::EquipmentSlot::OffhandWeapon1,
            Game::EquipmentSlot::MainhandWeapon2, Game::EquipmentSlot::OffhandWeapon2
        };

        for (const auto& slotEnum : slotsToCheck) {
            ReClass::EquipSlot slot = inventory.GetEquipSlot(static_cast<int>(slotEnum));
            if (!slot) continue;

            ReClass::ItemDef itemDef = slot.GetItemDefinition();
            if (!itemDef || itemDef.GetId() == 0) continue;

            GearSlotInfo slotInfo;
            slotInfo.itemId = itemDef.GetId();
            slotInfo.rarity = itemDef.GetRarity();

            if (ESPFormatting::IsWeaponSlot(slotEnum)) {
                ReClass::Stat stat = slot.GetStatWeapon();
                if (stat) slotInfo.statId = stat.GetId();
            }
            else {
                ReClass::Stat stat = slot.GetStatGear();
                if (stat) slotInfo.statId = stat.GetId();
            }

            outPlayer.gear[slotEnum] = slotInfo;
        }
    }

} // namespace kx