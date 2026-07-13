#pragma once

#include "../GameEnums.h"
#include "../Havok/HavokEnums.h"
#include "../Generated/EnumsAndStructs.h"

namespace kx::Formatting {

    const char* GetProfessionName(Game::Profession prof);
    const char* GetRaceName(Game::Race race);
    const char* GetGadgetTypeName(Game::GadgetType type);
    const char* GetRankName(Game::CharacterRank rank);
    const char* GetAttitudeName(Game::Attitude attitude);
    const char* GetAgentTypeName(Game::AgentType type);
    
    const char* ResourceNodeTypeToString(Game::ResourceNodeType type);
    const char* EquipmentSlotToString(Game::EquipmentSlot slot);
    
    const char* GetShapeTypeName(Havok::HkcdShapeType type);
    const char* GetAttributeShortName(data::ApiAttribute attribute);
    const char* GetItemLocationName(Game::ItemLocation location);

} // namespace kx::Formatting

