#pragma once

#include <vec3.hpp>
#include <vec2.hpp>

#include "../../GameEnums.h"
#include "../../Havok/HavokEnums.h"
#include "../EntityTypes.h"
#include "../../../Core/Services/Combat/CombatStateKey.h"

namespace kx {

struct GameEntity {
    glm::vec3 position;
    float visualDistance;
    float gameplayDistance;
    bool isValid;
    const void* address;
    float currentHealth;
    float maxHealth;
    float currentBarrier = 0.0f;
    EntityTypes entityType;
    Game::AgentType agentType;
    int32_t agentId;
    
    float physicsWidth = 0.0f;
    float physicsDepth = 0.0f;
    float physicsHeight = 0.0f;
    bool hasPhysicsDimensions = false;
    Havok::HkcdShapeType shapeType = Havok::HkcdShapeType::INVALID;

    GameEntity() : position(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
                         isValid(false), address(nullptr), currentHealth(0.0f), maxHealth(0.0f), currentBarrier(0.0f),
                         entityType(EntityTypes::Gadget), agentType(Game::AgentType::Error), agentId(0),
                         shapeType(Havok::HkcdShapeType::INVALID)
    {
    }

    CombatStateKey GetCombatKey() const {
        return CombatStateKey(static_cast<uint32_t>(agentId), address);
    }
};

} // namespace kx

