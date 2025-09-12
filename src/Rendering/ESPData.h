#pragma once

#include "glm.hpp"

namespace kx {

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

} // namespace kx
