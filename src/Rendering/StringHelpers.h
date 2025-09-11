#pragma once

#include <string>
#include <windows.h> // For WideCharToMultiByte
#include "../Game/GameEnums.h"

namespace kx {

// Helper to convert wide-character string to UTF-8 string
inline std::string WStringToString(const wchar_t* wstr) {
    if (!wstr || wstr[0] == L'\0') return "";

    const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (size_needed == 0) return "";

    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, NULL, NULL);

    strTo.pop_back(); // Remove the null terminator from the string's size
    return strTo;
}

// Legacy helpers for backward compatibility - these now use the new enums internally

// Helper to convert profession ID to string (legacy uint32_t version)
inline std::string ProfessionToString(uint32_t profId) {
    Game::Profession profession = static_cast<Game::Profession>(profId);
    const char* name = Game::EnumHelpers::GetProfessionName(profession);
    if (strcmp(name, "Unknown") == 0) {
        return "Prof ID: " + std::to_string(profId);
    }
    return std::string(name);
}

// Helper to convert profession enum to string (new type-safe version)
inline std::string ProfessionToString(Game::Profession profession) {
    return std::string(Game::EnumHelpers::GetProfessionName(profession));
}

// Helper to convert race ID to string (legacy uint8_t version)
inline std::string RaceToString(uint8_t raceId) {
    Game::Race race = static_cast<Game::Race>(raceId);
    const char* name = Game::EnumHelpers::GetRaceName(race);
    if (strcmp(name, "Unknown") == 0) {
        return "Race ID: " + std::to_string(raceId);
    }
    return std::string(name);
}

// Helper to convert race enum to string (new type-safe version)
inline std::string RaceToString(Game::Race race) {
    return std::string(Game::EnumHelpers::GetRaceName(race));
}

// New helpers for the enhanced enum system

// Helper to convert attitude to string
inline std::string AttitudeToString(Game::Attitude attitude) {
    return std::string(Game::EnumHelpers::GetAttitudeName(attitude));
}

// Helper to convert gadget type to string
inline std::string GadgetTypeToString(Game::GadgetType type) {
    return std::string(Game::EnumHelpers::GetGadgetTypeName(type));
}

// Helper to get full character description
inline std::string GetCharacterDescription(Game::Profession profession, Game::Race race, uint32_t level) {
    std::string prof = ProfessionToString(profession);
    std::string raceStr = RaceToString(race);
    std::string armor = Game::EnumHelpers::GetArmorWeight(profession);
    
    return "Lvl " + std::to_string(level) + " " + raceStr + " " + prof + " (" + armor + ")";
}

// Helper to get gadget description with context
inline std::string GetGadgetDescription(Game::GadgetType type, bool isGatherable = true) {
    std::string typeName = GadgetTypeToString(type);
    
    if (type == Game::GadgetType::ResourceNode) {
        typeName += isGatherable ? " (Gatherable)" : " (Depleted)";
    }
    
    return typeName;
}

} // namespace kx
