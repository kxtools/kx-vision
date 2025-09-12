#pragma once

#include <string>
#include <windows.h> // For WideCharToMultiByte
#include "../Game/GameEnums.h"

namespace kx {
namespace ESPFormatting {

// Helper to convert wide-character string to UTF-8 string with enhanced safety
inline std::string WStringToString(const wchar_t* wstr) {
    if (!wstr) return "";
    
    // Check for empty string
    if (wstr[0] == L'\0') return "";
    
    // Add length limit to prevent malformed strings from causing issues
    const size_t MAX_STRING_LENGTH = 8192; // Reasonable limit
    size_t wstr_len = wcsnlen(wstr, MAX_STRING_LENGTH);
    if (wstr_len == 0) return "";
    if (wstr_len >= MAX_STRING_LENGTH) {
        // String too long, might be corrupted
        return "[STRING_TOO_LONG]";
    }

    const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(wstr_len), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return "";

    std::string strTo(size_needed, 0);
    
    // Safe access to string data
    char* data_ptr = strTo.empty() ? nullptr : &strTo[0];
    if (!data_ptr) return "";
    
    const int result = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(wstr_len), data_ptr, size_needed, NULL, NULL);
    
    if (result <= 0) {
        return "[CONVERSION_ERROR]";
    }
    
    // Resize to actual converted length (no null terminator to remove since we didn't include it)
    strTo.resize(result);
    return strTo;
}

// Legacy helpers for backward compatibility - these now use the new enums internally

// Helper to convert profession ID to string (legacy uint32_t version)
inline std::string ProfessionToString(uint32_t profId) {
    // Validate reasonable range to prevent display of garbage values
    if (profId > 1000) {
        return "Invalid Prof ID: " + std::to_string(profId);
    }
    
    Game::Profession profession = static_cast<Game::Profession>(profId);
    const char* name = Game::EnumHelpers::GetProfessionName(profession);
    if (name != nullptr) {
        return std::string(name);
    }
    return "Prof ID: " + std::to_string(profId);
}

// Helper to convert profession enum to string (new type-safe version)
inline std::string ProfessionToString(Game::Profession profession) {
    const char* name = Game::EnumHelpers::GetProfessionName(profession);
    if (name != nullptr) {
        return std::string(name);
    }
    return "Prof ID: " + std::to_string(static_cast<uint32_t>(profession));
}

// Helper to convert race enum to string (new type-safe version)
inline std::string RaceToString(Game::Race race) {
    const char* name = Game::EnumHelpers::GetRaceName(race);
    if (name != nullptr) {
        return std::string(name);
    }
    return "Race ID: " + std::to_string(static_cast<uint8_t>(race));
}

// Helper to convert race ID to string (legacy uint8_t version)
inline std::string RaceToString(uint8_t raceId) {
    return RaceToString(static_cast<Game::Race>(raceId));
}

// New helpers for the enhanced enum system

// Helper to convert attitude to string
inline std::string AttitudeToString(Game::Attitude attitude) {
    const char* name = Game::EnumHelpers::GetAttitudeName(attitude);
    if (name != nullptr) {
        return std::string(name);
    }
    return "Attitude ID: " + std::to_string(static_cast<uint32_t>(attitude));
}

// Helper to convert gadget type to string
inline std::string GadgetTypeToString(Game::GadgetType type) {
    const char* name = Game::EnumHelpers::GetGadgetTypeName(type);
    if (name != nullptr) {
        return std::string(name);
    }
    return "Gadget ID: " + std::to_string(static_cast<uint32_t>(type));
}

// Helper to get full character description
inline std::string GetCharacterDescription(Game::Profession profession, Game::Race race, uint32_t level) {
    // Validate level to prevent display of garbage values
    if (level > 100000) {
        return "Invalid Level: " + std::to_string(level);
    }
    
    std::string prof = ProfessionToString(profession);
    std::string raceStr = RaceToString(race);
    
    const char* armorName = Game::EnumHelpers::GetArmorWeight(profession);
    std::string armor = (armorName != nullptr) ? std::string(armorName) : "Unknown Armor";
    
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

} // namespace ESPFormatting
} // namespace kx
