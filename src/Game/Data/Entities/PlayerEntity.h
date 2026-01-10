#pragma once

#include "GameEntity.h"
#include "../PlayerGearData.h"
#include <array>

namespace kx {

struct PlayerEntity : public GameEntity {
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

    struct GearItem {
        Game::EquipmentSlot slot;
        GearSlotInfo info;
    };

    static constexpr size_t MAX_GEAR_ITEMS = 32;
    std::array<GearItem, MAX_GEAR_ITEMS> gear{};
    size_t gearCount = 0;

    const GearSlotInfo* GetGearInfo(Game::EquipmentSlot slot) const {
        for (size_t i = 0; i < gearCount; ++i) {
            if (gear[i].slot == slot) return &gear[i].info;
        }
        return nullptr;
    }

    void AddGear(Game::EquipmentSlot slot, const GearSlotInfo& info) {
        if (gearCount < MAX_GEAR_ITEMS) {
            gear[gearCount++] = { slot, info };
        }
    }
    
    PlayerEntity() : GameEntity(),
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

