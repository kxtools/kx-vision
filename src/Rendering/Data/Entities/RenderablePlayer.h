#pragma once

#include "RenderableEntity.h"
#include "../PlayerRenderData.h"
#include <ankerl/unordered_dense.h>

namespace kx {

struct RenderablePlayer : public RenderableEntity {
    char playerName[64] = { 0 };
    float currentEndurance;
    float maxEndurance;
    float currentEnergy;
    float maxEnergy;
    uint32_t level;
    uint32_t scaledLevel;
    Game::Profession profession;
    Game::Attitude attitude;
    Game::Race race;
    bool isLocalPlayer;

    ankerl::unordered_dense::map<Game::EquipmentSlot, GearSlotInfo> gear;
    
    RenderablePlayer() : RenderableEntity(),
                         currentEndurance(0.0f), maxEndurance(0.0f),
                         currentEnergy(0.0f), maxEnergy(0.0f),
                         level(0), scaledLevel(0),
                         profession(Game::Profession::None), attitude(Game::Attitude::Neutral),
                         race(Game::Race::None), isLocalPlayer(false)
    {
        entityType = EntityTypes::Player;
    }
};

} // namespace kx

