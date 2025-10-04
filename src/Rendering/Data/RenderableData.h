#pragma once

#include <string>
#include <unordered_map>
#include <vec3.hpp>
#include <vec2.hpp>
#include "../../Game/GameEnums.h"
#include "../../../libs/ImGui/imgui.h"
#include "PlayerRenderData.h"

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
    // --- Position Data for Interpolation/Extrapolation ---
    glm::vec3 currentPosition;       // The newest position from the game (interpolation target)
    glm::vec3 previousPosition;      // The position from the previous update (interpolation start)
    glm::vec3 smoothedVelocity;      // Smoothed velocity in meters/second for extrapolation
    double lastUpdateTime;           // Timestamp when currentPosition was last updated (in seconds)
    double previousUpdateTime;       // Timestamp of the previous update (for true velocity calculation)
    
    // --- Legacy field (kept for compatibility, use interpolated position for rendering) ---
    glm::vec3 position;              // Deprecated: extractor still populates this
    
    glm::vec2 screenPos;             // Pre-calculated screen position
    float visualDistance;            // Distance from camera (for scaling)
    float gameplayDistance;          // Distance from player (for display)
    bool isValid;
    const void* address;             // Const pointer since we only use for identification/comparison

    RenderableEntity() : currentPosition(0.0f), previousPosition(0.0f), 
                         smoothedVelocity(0.0f), lastUpdateTime(0.0), previousUpdateTime(0.0),
                         position(0.0f), screenPos(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
                         isValid(false), address(nullptr)
    {
    }
};

struct RenderablePlayer : public RenderableEntity {
    std::string characterName;
    std::string playerName;
    float currentHealth;
    float maxHealth;
    float currentEnergy;
    float maxEnergy;
    uint32_t level;
    uint32_t scaledLevel;
    Game::Profession profession;     // Type-safe enum instead of uint32_t
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::Race race;                 // Type-safe enum for character race
    bool isLocalPlayer; // Flag to identify if this is the local player

    std::unordered_map<Game::EquipmentSlot, GearSlotInfo> gear;
    
    RenderablePlayer() : RenderableEntity(),
                         currentHealth(0.0f), maxHealth(0.0f),
                         currentEnergy(0.0f), maxEnergy(0.0f), level(0), scaledLevel(0),
                         profession(Game::Profession::None), attitude(Game::Attitude::Neutral),
                         race(Game::Race::None), isLocalPlayer(false)
    {
    }
};

struct RenderableNpc : public RenderableEntity {
    std::string name;
    float currentHealth;
    float maxHealth;
    uint32_t level;
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::CharacterRank rank;

    RenderableNpc() : RenderableEntity(),
                      currentHealth(0.0f), maxHealth(0.0f),
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