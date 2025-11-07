#pragma once

#include <string>
#include <unordered_map>
#include <vec3.hpp>
#include <vec2.hpp>
#include "../../Game/GameEnums.h"
#include "../../../libs/ImGui/imgui.h"
#include "PlayerRenderData.h"
#include "ESPEntityTypes.h"

namespace kx {

// Safe data structures for the two-stage rendering pipeline
// These contain only plain data types, no pointers to game memory
// Now using proper enum types for better type safety

// Shared rendering utility structures

struct ColoredDetail {
    std::string text;
    ImU32 color = 0; // 0 means default color
};

// Base struct for all renderable entities
// Contains common fields shared by all entity types
struct RenderableEntity {
    glm::vec3 position;
    float visualDistance;            // Distance from camera (for scaling)
    float gameplayDistance;          // Distance from player (for display)
    bool isValid;
    const void* address;             // Const pointer since we only use for identification/comparison
    float currentHealth;
    float maxHealth;
    float currentBarrier = 0.0f; // Barrier overlay for health bars
    ESPEntityType entityType;
    Game::AgentType agentType;       // Agent type identifier
    int32_t agentId;                 // Agent ID
    
    // Physics-based bounding box dimensions (in Mumble Link meters)
    // HEIGHT: Extracted from HkpBoxShape, varies per entity (bosses taller, small NPCs shorter)
    // WIDTH/DEPTH: Derived from height at 35% ratio (HkpBoxShape stores capsule radii, not visual dimensions)
    float physicsWidth = 0.0f;       // Full width (X-axis) - derived from height
    float physicsDepth = 0.0f;       // Full depth (Y-axis) - derived from height  
    float physicsHeight = 0.0f;      // Full height (Z-axis) - accurate from physics
    bool hasPhysicsDimensions = false; // True if physics dimensions are available

    RenderableEntity() : position(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
                         isValid(false), address(nullptr), currentHealth(0.0f), maxHealth(0.0f), currentBarrier(0.0f),
                         entityType(ESPEntityType::Gadget), agentType(Game::AgentType::Error), agentId(0) // Default, will be overwritten
    {
    }
};

struct RenderablePlayer : public RenderableEntity {
    std::string characterName;
    std::string playerName;
    float currentEnergy;
    float maxEnergy;
    float currentSpecialEnergy;
    float maxSpecialEnergy;
    uint32_t level;
    uint32_t scaledLevel;
    Game::Profession profession;     // Type-safe enum instead of uint32_t
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::Race race;                 // Type-safe enum for character race
    bool isLocalPlayer; // Flag to identify if this is the local player

    std::unordered_map<Game::EquipmentSlot, GearSlotInfo> gear;
    
    RenderablePlayer() : RenderableEntity(),
                         currentEnergy(0.0f), maxEnergy(0.0f),
                         currentSpecialEnergy(0.0f), maxSpecialEnergy(0.0f),
                         level(0), scaledLevel(0),
                         profession(Game::Profession::None), attitude(Game::Attitude::Neutral),
                         race(Game::Race::None), isLocalPlayer(false)
    {
    }
};

struct RenderableNpc : public RenderableEntity {
    std::string name;
    uint32_t level;
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::CharacterRank rank;

    RenderableNpc() : RenderableEntity(),
                      level(0), attitude(Game::Attitude::Neutral), rank()
    {
    }
};

struct RenderableGadget : public RenderableEntity {
    std::string name;
    Game::GadgetType type;           // Type-safe enum instead of uint32_t
    Game::ResourceNodeType resourceType;
    bool isGatherable;               // Additional flag for resource nodes
    
    RenderableGadget() : RenderableEntity(),
                         type(Game::GadgetType::None), resourceType(),
                         isGatherable(false)
    {
    }
};

} // namespace kx
