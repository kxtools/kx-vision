#pragma once

#include <string>
#include "../../Game/GameEnums.h"
#include "../../Game/HavokEnums.h"
#include "../../Game/Generated/EnumsAndStructs.h"

namespace kx::Formatting {

    const char* GetProfessionName(Game::Profession prof);
    const char* GetRaceName(Game::Race race);
    const char* GetGadgetTypeName(Game::GadgetType type);
    const char* GetRankName(Game::CharacterRank rank);
    const char* GetAttitudeName(Game::Attitude attitude);
    std::string GetAgentTypeName(Game::AgentType type);
    
    bool IsWeaponSlot(Game::EquipmentSlot slot);
    
    std::string ResourceNodeTypeToString(Game::ResourceNodeType type);
    const char* EquipmentSlotToString(Game::EquipmentSlot slot);
    
    std::string GetShapeTypeName(Havok::HkcdShapeType type);
    const char* GetAttributeShortName(data::ApiAttribute attribute);

} // namespace kx::Formatting

