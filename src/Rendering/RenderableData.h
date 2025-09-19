#pragma once

#include <string>
#include <vec3.hpp>
#include <vec2.hpp>
#include "../Game/GameEnums.h"

namespace kx {

// Safe data structures for the two-stage rendering pipeline
// These contain only plain data types, no pointers to game memory
// Now using proper enum types for better type safety

struct RenderablePlayer {
    glm::vec3 position;
    glm::vec2 screenPos;             // Pre-calculated screen position
    float distance;                  // Pre-calculated distance to camera
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
    
        RenderablePlayer() : position(0.0f), screenPos(0.0f), distance(0.0f),
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
    float distance;                  // Pre-calculated distance to camera
    std::string name;
    float currentHealth;
    float maxHealth;
    uint32_t level;
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::CharacterRank rank;
    bool isValid;
    void* address;

    RenderableNpc() : position(0.0f), screenPos(0.0f), distance(0.0f),
                      currentHealth(0.0f), maxHealth(0.0f),
                      level(0), attitude(Game::Attitude::Neutral), rank(), isValid(false), address(nullptr)
    {
    }
};

struct RenderableGadget {
    glm::vec3 position;
    glm::vec2 screenPos;             // Pre-calculated screen position
    float distance;                  // Pre-calculated distance to camera
    std::string name;
    Game::GadgetType type;           // Type-safe enum instead of uint32_t
    Game::ResourceNodeType resourceType;
    bool isGatherable;               // Additional flag for resource nodes
    bool isValid;
    void* address;
    
    RenderableGadget() : position(0.0f), screenPos(0.0f), distance(0.0f),
                         type(Game::GadgetType::None), resourceType(),
                         isGatherable(false), isValid(false), address(nullptr)
    {
    }
};

} // namespace kx