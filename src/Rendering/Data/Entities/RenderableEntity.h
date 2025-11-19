#pragma once

#include <string_view>
#include <vector>
#include <algorithm>
#include <cstring>
#include <vec3.hpp>
#include <vec2.hpp>

#include "../../../Game/GameEnums.h"
#include "../../../Game/HavokEnums.h"
#include "../../../../libs/ImGui/imgui.h"
#include "../EntityTypes.h"
#include "../../Combat/CombatStateKey.h"

namespace kx {

constexpr size_t kMaxTextBufferSize = 128;

struct FixedText {
    char buffer[kMaxTextBufferSize];

    FixedText() { buffer[0] = '\0'; }
    
    FixedText(std::string_view sv) {
        size_t length = std::min(sv.length(), kMaxTextBufferSize - 1);
        if (length > 0) {
            std::memcpy(buffer, sv.data(), length);
        }
        buffer[length] = '\0';
    }

    const char* c_str() const { return buffer; }
};

struct ColoredDetail {
    FixedText text;
    ImU32 color = 0;
    
    template <typename T>
    ColoredDetail(const T& t, ImU32 c) : text(t), color(c) {}
};

struct RenderableEntity {
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

    RenderableEntity() : position(0.0f), visualDistance(0.0f), gameplayDistance(0.0f),
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

