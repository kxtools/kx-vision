#pragma once

#include <string>
#include <vector>
#include <vec3.hpp>

namespace kx {

// Safe data structures for the two-stage rendering pipeline
// These contain only plain data types, no pointers to game memory

struct RenderablePlayer {
    glm::vec3 position;
    std::string characterName;
    std::string playerName;
    float currentHealth;
    float maxHealth;
    float currentEnergy;
    float maxEnergy;
    uint32_t level;
    uint32_t profession;
    uint32_t attitude;
    bool isValid;
    bool isLocalPlayer; // Flag to identify if this is the local player
    
    RenderablePlayer() : position(0.0f), currentHealth(0.0f), maxHealth(0.0f), 
                        currentEnergy(0.0f), maxEnergy(0.0f), level(0), 
                        profession(0), attitude(0), isValid(false), isLocalPlayer(false) {}
};

struct RenderableNpc {
    glm::vec3 position;
    std::string name;
    float currentHealth;
    float maxHealth;
    uint32_t level;
    uint32_t attitude;
    bool isValid;
    
    RenderableNpc() : position(0.0f), currentHealth(0.0f), maxHealth(0.0f), 
                     level(0), attitude(0), isValid(false) {}
};

struct RenderableGadget {
    glm::vec3 position;
    std::string name;
    uint32_t type;
    bool isValid;
    
    RenderableGadget() : position(0.0f), type(0), isValid(false) {}
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