#pragma once

#include "glm.hpp"
#include <vector>

namespace kx {

// Forward declarations for objects
struct RenderablePlayer;
struct RenderableNpc;
struct RenderableGadget;

// ESP Entity Types for rendering differentiation
enum class ESPEntityType {
    Player,
    NPC,
    Gadget
};

// Universal base structure for all ESP entities
struct ESPEntityData {
    bool valid = false;
    glm::vec3 feetPos;           // 3D world position
    glm::vec2 feet;              // 2D screen position
    glm::vec2 min;               // Bounding box upper-left
    glm::vec2 max;               // Bounding box lower-right
    float height = 0.0f;         // Screen height
    float width = 0.0f;          // Screen width
};

// Player/NPC data (rectangular boxes for humanoids)
struct PlayerESPData : public ESPEntityData {
    PlayerESPData() = default;
};

// Gadget/Object data (square boxes for objects)
struct GadgetESPData : public ESPEntityData {
    GadgetESPData() = default;
};

// Pooled frame data for object pool pattern (uses pointers instead of objects)
struct PooledFrameRenderData {
    std::vector<RenderablePlayer*> players;    // Pointers to pooled players
    std::vector<RenderableNpc*> npcs;          // Pointers to pooled NPCs  
    std::vector<RenderableGadget*> gadgets;    // Pointers to pooled gadgets
    
    void Reset() {
        players.clear();
        npcs.clear();
        gadgets.clear();
    }
};

} // namespace kx
