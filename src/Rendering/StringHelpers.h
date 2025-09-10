#pragma once

#include <string>
#include <windows.h> // For WideCharToMultiByte

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

// Helper to convert profession ID to string
inline std::string ProfessionToString(uint32_t profId) {
    switch (profId) {
        case 1: return "Guardian";
        case 2: return "Warrior";
        case 3: return "Engineer";
        case 4: return "Ranger";
        case 5: return "Thief";
        case 6: return "Elementalist";
        case 7: return "Mesmer";
        case 8: return "Necromancer";
        case 9: return "Revenant";
        default: return "Prof ID: " + std::to_string(profId);
    }
}

// Helper to convert race ID to string
inline std::string RaceToString(uint8_t raceId) {
    switch (raceId) {
        case 0: return "Asura";
        case 1: return "Charr";
        case 2: return "Human";
        case 3: return "Norn";
        case 4: return "Sylvari";
        default: return "Race ID: " + std::to_string(raceId);
    }
}

} // namespace kx
