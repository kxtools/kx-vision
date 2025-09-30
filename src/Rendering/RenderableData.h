#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vec3.hpp>
#include <vec2.hpp>
#include "../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"

namespace kx {

// Safe data structures for the two-stage rendering pipeline
// These contain only plain data types, no pointers to game memory
// Now using proper enum types for better type safety

struct GearSlotInfo {
    uint32_t itemId = 0;
    uint32_t statId = 0;
    Game::ItemRarity rarity = Game::ItemRarity::None;
    // We can add fields for upgrades, rarity, etc. here later.
};

struct CompactStatInfo {
    std::string statName;
    int count = 0;
    Game::ItemRarity highestRarity = Game::ItemRarity::None;
};

struct DominantStat {
    std::string name;
    float percentage;
};

struct ColoredDetail {
    std::string text;
    ImU32 color = 0; // 0 means default color
};

struct RenderablePlayer {
    glm::vec3 position;
    glm::vec2 screenPos;             // Pre-calculated screen position
    float visualDistance;            // Distance from camera (for scaling)
    float gameplayDistance;          // Distance from player (for display)
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
    bool isValid;
    bool isLocalPlayer; // Flag to identify if this is the local player
    void* address;

    std::unordered_map<Game::EquipmentSlot, GearSlotInfo> gear;
    
        RenderablePlayer() : position(0.0f), screenPos(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
                             currentHealth(0.0f), maxHealth(0.0f),
                             currentEnergy(0.0f), maxEnergy(0.0f), level(0), scaledLevel(0),
                             profession(Game::Profession::None), attitude(Game::Attitude::Neutral),
                             race(Game::Race::None), isValid(false), isLocalPlayer(false), address(nullptr)
    {
    }
};

struct RenderableNpc {
    glm::vec3 position;
    glm::vec2 screenPos;             // Pre-calculated screen position
    float visualDistance;            // Distance from camera (for scaling)
    float gameplayDistance;          // Distance from player (for display)
    std::string name;
    float currentHealth;
    float maxHealth;
    uint32_t level;
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::CharacterRank rank;
    bool isValid;
    void* address;

    RenderableNpc() : position(0.0f), screenPos(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
                      currentHealth(0.0f), maxHealth(0.0f),
                      level(0), attitude(Game::Attitude::Neutral), rank(), isValid(false), address(nullptr)
    {
    }
};

struct RenderableGadget {
    glm::vec3 position;
    glm::vec2 screenPos;             // Pre-calculated screen position
    float visualDistance;            // Distance from camera (for scaling)
    float gameplayDistance;          // Distance from player (for display)
    std::string name;
    Game::GadgetType type;           // Type-safe enum instead of uint32_t
    Game::ResourceNodeType resourceType;
    bool isGatherable;               // Additional flag for resource nodes
    bool isValid;
    void* address;
    
    RenderableGadget() : position(0.0f), screenPos(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
                         type(Game::GadgetType::None), resourceType(),
                         isGatherable(false), isValid(false), address(nullptr)
    {
    }
};

} // namespace kx