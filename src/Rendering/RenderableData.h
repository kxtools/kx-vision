#pragma once

#include <string>
#include <vector>
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
    Game::Profession profession;     // Type-safe enum instead of uint32_t
    Game::Attitude attitude;         // Type-safe enum instead of uint32_t
    Game::Race race;                 // Type-safe enum for character race
    bool isValid;
    bool isLocalPlayer; // Flag to identify if this is the local player
    
    RenderablePlayer() : position(0.0f), screenPos(0.0f), distance(0.0f), 
                        currentHealth(0.0f), maxHealth(0.0f), 
                        currentEnergy(0.0f), maxEnergy(0.0f), level(0), 
                        profession(Game::Profession::None), attitude(Game::Attitude::Neutral),
                        race(Game::Race::None), isValid(false), isLocalPlayer(false) {}
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
    bool isValid;
    
    RenderableNpc() : position(0.0f), screenPos(0.0f), distance(0.0f), 
                     currentHealth(0.0f), maxHealth(0.0f), 
                     level(0), attitude(Game::Attitude::Neutral), isValid(false) {}
};

struct RenderableGadget {
    glm::vec3 position;
    glm::vec2 screenPos;             // Pre-calculated screen position
    float distance;                  // Pre-calculated distance to camera
    std::string name;
    Game::GadgetType type;           // Type-safe enum instead of uint32_t
    bool isGatherable;               // Additional flag for resource nodes
    bool isValid;
    
    RenderableGadget() : position(0.0f), screenPos(0.0f), distance(0.0f), 
                        type(Game::GadgetType::None), 
                        isGatherable(false), isValid(false) {}
};

// Container for all renderable data for one frame
struct FrameRenderData {
    std::vector<RenderablePlayer> players;
    std::vector<RenderableNpc> npcs;
    std::vector<RenderableGadget> gadgets;
    
    void Clear() {
        players.clear();
        npcs.clear();
        gadgets.clear();
    }
};

} // namespace kx