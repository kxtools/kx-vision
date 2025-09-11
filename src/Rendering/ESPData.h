#pragma once

#include "glm.hpp"

namespace kx {

// Base structure for ESP entity data
struct ESPEntityData {
    bool valid = false;
    glm::vec3 feetPos;
    glm::vec3 headPos;
    glm::vec2 feet;      // Screen position of feet
    glm::vec2 head;      // Screen position of head
    glm::vec2 min;       // Upper-left corner of bounding box
    glm::vec2 max;       // Lower-right corner of bounding box
    float height = 0.0f; // Screen height
    float width = 0.0f;  // Screen width
    float appxHeight = 1.8f; // World height approximation
    float appxWidth = 0.6f;  // World width approximation
};

// Player/NPC specific data
struct PlayerESPData : public ESPEntityData {
    PlayerESPData() {
        appxHeight = 0.9f; // More reasonable height for GW2 (will be * 2.0f = 1.8m)
        appxWidth = 0.6f;  // Human width
    }
};

// Gadget/Object specific data  
struct GadgetESPData : public ESPEntityData {
    GadgetESPData() {
        appxHeight = 0.25f; // Smaller objects (will be * 2.0f = 0.5m)
        appxWidth = 0.25f;  // Square objects
    }
};

} // namespace kx
