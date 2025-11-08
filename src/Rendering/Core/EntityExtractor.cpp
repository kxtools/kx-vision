#include "EntityExtractor.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPFormatting.h"
#include "../../Game/GameEnums.h"
#include "../../Utils/StringHelpers.h"
#include <vector>

namespace kx {

// Physics dimension extraction validation constants
namespace PhysicsValidation {
    // Height validation range in meters (applies to both characters and gadgets after conversion)
    constexpr float MIN_HEIGHT_METERS = 0.1f;   // 10cm - minimum reasonable entity height
    constexpr float MAX_HEIGHT_METERS = 30.0f;  // 30m - maximum height (allows large bosses)
    
    // Gadget height validation in centimeters (before conversion)
    constexpr int32_t MIN_HEIGHT_CM = 10;       // 10cm minimum
    constexpr int32_t MAX_HEIGHT_CM = 3000;     // 30m maximum
    
    // Width-to-height ratio for ESP bounding boxes (proportional approach)
    constexpr float WIDTH_TO_HEIGHT_RATIO = 0.35f;  // 35% - typical humanoid/object proportions
}

    bool EntityExtractor::ExtractPlayer(RenderablePlayer& outPlayer,
        const ReClass::ChCliCharacter& inCharacter,
        const wchar_t* playerName,
        void* localPlayerPtr) {

        // --- Validation and Position ---
        glm::vec3 gamePos;
        if (!ValidateAndExtractGamePosition(inCharacter, gamePos)) return false;

        // --- Populate Core Data ---
        outPlayer.position = TransformGamePositionToMumble(gamePos);
        outPlayer.isValid = true;
        outPlayer.entityType = ESPEntityType::Player;
        outPlayer.address = inCharacter.data();
        outPlayer.isLocalPlayer = (outPlayer.address == localPlayerPtr);
        if (playerName) {
            outPlayer.playerName = StringHelpers::WCharToUTF8String(playerName);
        }

        // --- Agent Info ---
        ReClass::AgChar agent = inCharacter.GetAgent();
        if (agent) {
            outPlayer.agentType = agent.GetType();
            outPlayer.agentId = agent.GetId();
        }

        // --- Health & Energy ---
        ReClass::ChCliHealth health = inCharacter.GetHealth();
        ExtractHealthData(outPlayer, health);

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
        
        // --- Physics Box Shape Dimensions ---
        ExtractBoxShapeDimensions(outPlayer, inCharacter);

        return true;
    }

    bool EntityExtractor::ExtractNpc(RenderableNpc& outNpc, const ReClass::ChCliCharacter& inCharacter) {

        // --- Validation and Position ---
        glm::vec3 gamePos;
        if (!ValidateAndExtractGamePosition(inCharacter, gamePos)) return false;

        // --- Populate Core Data ---
        outNpc.position = TransformGamePositionToMumble(gamePos);
        outNpc.isValid = true;
        outNpc.entityType = ESPEntityType::NPC;
        outNpc.address = inCharacter.data();

        // --- Agent Info ---
        ReClass::AgChar agent = inCharacter.GetAgent();
        if (agent) {
            outNpc.agentType = agent.GetType();
            outNpc.agentId = agent.GetId();
        }

        // --- Health ---
        ReClass::ChCliHealth health = inCharacter.GetHealth();
        ExtractHealthData(outNpc, health);

        // --- Stats ---
        ReClass::ChCliCoreStats coreStats = inCharacter.GetCoreStats();
        if (coreStats) {
            outNpc.level = coreStats.GetLevel();
        }
        outNpc.attitude = inCharacter.GetAttitude();
        outNpc.rank = inCharacter.GetRank();
        
        // --- Physics Box Shape Dimensions ---
        ExtractBoxShapeDimensions(outNpc, inCharacter);

        return true;
    }

    bool EntityExtractor::ExtractGadget(RenderableGadget& outGadget, const ReClass::GdCliGadget& inGadget) {

        // --- Validation and Position ---
        glm::vec3 gamePos;
        if (!ValidateAndExtractGamePosition(inGadget, gamePos)) return false;

        // --- Populate Core Data ---
        outGadget.position = TransformGamePositionToMumble(gamePos);
        outGadget.isValid = true;
        outGadget.entityType = ESPEntityType::Gadget;
        outGadget.address = inGadget.data();
        outGadget.type = inGadget.GetGadgetType();
        outGadget.isGatherable = inGadget.IsGatherable();

        // --- Agent Info ---
        ReClass::AgKeyFramed agent = inGadget.GetAgKeyFramed();
        if (agent) {
            outGadget.agentType = agent.GetType();
            outGadget.agentId = agent.GetId();
        }

        // --- Health ---
        ReClass::ChCliHealth health = inGadget.GetHealth();
        ExtractHealthData(outGadget, health);

        if (outGadget.type == Game::GadgetType::ResourceNode) {
            outGadget.resourceType = inGadget.GetResourceNodeType();
        }
        
        // --- Physics Cylinder Shape Dimensions ---
        ExtractCylinderShapeDimensions(outGadget, inGadget);

        return true;
    }

    bool EntityExtractor::ExtractAttackTarget(RenderableAttackTarget& outAttackTarget,
        const ReClass::AgentInl& inAgentInl) {

        if (!inAgentInl) return false;

        // --- Get AgKeyFramed for agent type/ID, position, and physics dimensions ---
        ReClass::AgKeyFramed agKeyframed = inAgentInl.GetAgKeyFramed();
        if (!agKeyframed) return false;

        // --- Position ---
        // Use position from AgKeyFramed->CoKeyFramed (AgentInl position appears to be in wrong coordinate system)
        glm::vec3 gamePos;
        if (!ValidateAndExtractGamePosition(agKeyframed, gamePos)) return false;

        // --- Populate Core Data ---
        outAttackTarget.position = TransformGamePositionToMumble(gamePos);
        outAttackTarget.isValid = true;
        outAttackTarget.entityType = ESPEntityType::AttackTarget;
        outAttackTarget.address = inAgentInl.data();
        outAttackTarget.agentType = agKeyframed.GetType();
        outAttackTarget.agentId = agKeyframed.GetId();

        // Health data not available - AgentInl health pointer not confirmed/working
        outAttackTarget.currentHealth = 0.0f;
        outAttackTarget.maxHealth = 0.0f;
        outAttackTarget.currentBarrier = 0.0f;

        // --- Physics Box Shape Dimensions ---
        ExtractBoxShapeDimensions(outAttackTarget, agKeyframed);

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

// Helper method implementations
bool EntityExtractor::ValidateAndExtractGamePosition(const ReClass::ChCliCharacter& character, glm::vec3& outGamePos) {
    ReClass::AgChar agent = character.GetAgent();
    if (!agent) return false;

    ReClass::CoChar coChar = agent.GetCoChar();
    if (!coChar) return false;

    outGamePos = coChar.GetVisualPosition();
    if (outGamePos.x == 0.0f && outGamePos.y == 0.0f && outGamePos.z == 0.0f) return false;

    return true;
}

bool EntityExtractor::ValidateAndExtractGamePosition(const ReClass::GdCliGadget& gadget, glm::vec3& outGamePos) {
    ReClass::AgKeyFramed agKeyFramed = gadget.GetAgKeyFramed();
    if (!agKeyFramed) return false;

    ReClass::CoKeyFramed coKeyFramed = agKeyFramed.GetCoKeyFramed();
    if (!coKeyFramed) return false;

    outGamePos = coKeyFramed.GetPosition();
    if (outGamePos.x == 0.0f && outGamePos.y == 0.0f && outGamePos.z == 0.0f) return false;

    return true;
}

bool EntityExtractor::ValidateAndExtractGamePosition(const ReClass::AgKeyFramed& agKeyframed, glm::vec3& outGamePos) {
    if (!agKeyframed) return false;

    ReClass::CoKeyFramed coKeyFramed = agKeyframed.GetCoKeyFramed();
    if (!coKeyFramed) return false;

    outGamePos = coKeyFramed.GetPosition();
    if (outGamePos.x == 0.0f && outGamePos.y == 0.0f && outGamePos.z == 0.0f) return false;

    return true;
}

glm::vec3 EntityExtractor::TransformGamePositionToMumble(const glm::vec3& gamePos) {
    return glm::vec3(
        gamePos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
        gamePos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR,
        gamePos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR
    );
}

void EntityExtractor::ExtractHealthData(RenderableEntity& entity, const ReClass::ChCliHealth& health) {
    if (health) {
        entity.currentHealth = health.GetCurrent();
        entity.maxHealth = health.GetMax();
        entity.currentBarrier = health.GetBarrier();
    }
}

void EntityExtractor::ExtractBoxShapeDimensions(RenderableEntity& entity, const ReClass::ChCliCharacter& character) {
    // Navigate: ChCliCharacter -> AgChar -> CoChar -> CoCharSimpleCliWrapper -> HkpBoxShape
    ReClass::AgChar agent = character.GetAgent();
    if (!agent) return;
    
    ReClass::CoChar coChar = agent.GetCoChar();
    if (!coChar) return;
    
    ReClass::CoCharSimpleCliWrapper wrapper = coChar.GetSimpleCliWrapper();
    if (!wrapper) return;
    
    ReClass::HkpBoxShape boxShape = wrapper.GetBoxShape();
    if (!boxShape) return;
    
    ExtractBoxShapeDimensionsFromHkpBoxShape(entity, boxShape);
}

void EntityExtractor::ExtractCylinderShapeDimensions(RenderableEntity& entity, const ReClass::GdCliGadget& gadget) {
    // Navigate: GdCliGadget -> AgKeyFramed -> CoKeyFramed
    ReClass::AgKeyFramed agent = gadget.GetAgKeyFramed();
    if (!agent) return;
    
    ReClass::CoKeyFramed coKeyframed = agent.GetCoKeyFramed();
    if (!coKeyframed) return;
    
    ExtractCylinderShapeDimensionsFromCoKeyframed(entity, coKeyframed);
}

void EntityExtractor::ExtractBoxShapeDimensions(RenderableEntity& entity, const ReClass::AgKeyFramed& agKeyframed) {
    // Navigate: AgKeyFramed -> CoKeyFramed
    ReClass::CoKeyFramed coKeyframed = agKeyframed.GetCoKeyFramed();
    if (!coKeyframed) return;
    
    ExtractBoxShapeDimensionsFromCoKeyframed(entity, coKeyframed);
}

void EntityExtractor::ExtractCylinderShapeDimensionsFromCoKeyframed(RenderableEntity& entity, const ReClass::CoKeyFramed& coKeyframed) {
    // Navigate: CoKeyFramed -> HkpRigidBody -> HkpCylinderShape
    ReClass::HkpRigidBody rigidBody = coKeyframed.GetRigidBody();
    if (!rigidBody) return;
    
    ReClass::HkpCylinderShape cylinderShape = rigidBody.GetCylinderShape();
    if (!cylinderShape) return;
    
    // Read height from integer offset at 0x38 (centimeters)
    // This is the most reliable offset - stores height in centimeters as int32_t
    // Evidence: 229 = 2.29m (catapult), 500 = 5m (control point)
    int32_t heightCm = cylinderShape.GetHeightCentimeters();
    
    // Validate integer height range in centimeters
    if (heightCm < PhysicsValidation::MIN_HEIGHT_CM || 
        heightCm > PhysicsValidation::MAX_HEIGHT_CM) {
        return; // Invalid or out of range - use fallback dimensions
    }
    
    // === HEIGHT: Accurate per-entity dimension from physics ===
    // Convert from centimeters to meters
    entity.physicsHeight = heightCm / 100.0f;
    
    // === WIDTH/DEPTH: Derived from height for ESP visualization ===
    // Note: HkpCylinderShape radius values are too small for ESP visualization (~0.05 game units).
    // For ESP boxes, we derive proportional dimensions (35% width-to-height).
    entity.physicsWidth = entity.physicsHeight * PhysicsValidation::WIDTH_TO_HEIGHT_RATIO;
    entity.physicsDepth = entity.physicsHeight * PhysicsValidation::WIDTH_TO_HEIGHT_RATIO;
    
    entity.hasPhysicsDimensions = true;
}

void EntityExtractor::ExtractBoxShapeDimensionsFromCoKeyframed(RenderableEntity& entity, const ReClass::CoKeyFramed& coKeyframed) {
    // Navigate: CoKeyFramed -> HkpRigidBody -> HkpBoxShape
    ReClass::HkpRigidBody rigidBody = coKeyframed.GetRigidBody();
    if (!rigidBody) return;
    
    ReClass::HkpBoxShape boxShape = rigidBody.GetBoxShape();
    if (!boxShape) return;
    
    ExtractBoxShapeDimensionsFromHkpBoxShape(entity, boxShape);
}

void EntityExtractor::ExtractBoxShapeDimensionsFromHkpBoxShape(RenderableEntity& entity, const ReClass::HkpBoxShape& boxShape) {
    // Read half-extents from Havok physics box shape (in game coordinate space)
    float heightHalf = boxShape.GetHeightHalf();
    
    // Reject obviously invalid values before conversion
    if (!std::isfinite(heightHalf) || heightHalf <= 0.0f) {
        return; // NaN, infinity, negative, or zero - use fallback dimensions
    }
    
    // === HEIGHT: Accurate per-entity dimension from physics ===
    // HkpBoxShape stores dimensions in game coordinate space (Havok Physics System)
    // Evidence: heightHalf ~0.75 game units → 1.5 full → ÷1.23 → ~1.22m (typical character)
    // Conversion follows unit-systems.md: Game coordinates require ÷GAME_TO_MUMBLE_SCALE_FACTOR
    float fullHeightMeters = (heightHalf * 2.0f) / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR;
    
    // Validate converted height in meters
    // This rejects corrupted data while allowing normal entities (~1.2m) and large structures (~8m+)
    if (fullHeightMeters < PhysicsValidation::MIN_HEIGHT_METERS || 
        fullHeightMeters > PhysicsValidation::MAX_HEIGHT_METERS) {
        return; // Out of reasonable range - use fallback dimensions
    }
    
    entity.physicsHeight = fullHeightMeters;
    
    // === WIDTH/DEPTH: Derived from height for ESP visualization ===
    // Note: HkpBoxShape width/depth values represent capsule collision radii (~0.035 game units),
    // not visual dimensions. For ESP boxes, we derive proportional dimensions (35% width-to-height).
    entity.physicsWidth = entity.physicsHeight * PhysicsValidation::WIDTH_TO_HEIGHT_RATIO;
    entity.physicsDepth = entity.physicsHeight * PhysicsValidation::WIDTH_TO_HEIGHT_RATIO;
    
    entity.hasPhysicsDimensions = true;
}

} // namespace kx