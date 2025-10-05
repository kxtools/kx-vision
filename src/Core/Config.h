#pragma once

#include <string_view> // For std::string_view

// ===== Build Configuration =====
// Uncomment the line below to build for GW2AL (addon loader) mode.
// Comment it out to build as a standalone DLL for manual injection.
//#define GW2AL_BUILD
// ==============================

namespace kx {
    constexpr std::string_view APP_VERSION = "0.7";

    // Configuration for the target process and function signature
    constexpr std::string_view TARGET_PROCESS_NAME = "Gw2-64.exe";

    // Patterns for address scanning
    constexpr std::string_view AGENT_VIEW_CONTEXT_PATTERN = "40 53 48 83 EC 20 F6 05 ?? ?? ?? ?? 01 48 8D 05";
    constexpr std::string_view AGENT_ARRAY_LEA_PATTERN = "48 8D 0D ?? ?? ?? ?? 48 89 1D ?? ?? ?? ?? 48 89 1D ?? ?? ?? ?? 48 83 C4 20";
    constexpr std::string_view WORLD_VIEW_CONTEXT_PATTERN = "48 85 C0 75 20 41 B8 2E 04 00 00";
    constexpr std::string_view BGFX_CONTEXT_FUNC_PATTERN = "BA 10 00 00 00 48 8B 04 C8 81 3C 02 62 67 66 78"; // backup: "57 ? ? ? ? 48 8B 35 2D CD" (Offset: 9)
    constexpr std::string_view CONTEXT_COLLECTION_FUNC_PATTERN = "8B ? ? ? ? ? 65 ? ? ? ? ? ? ? ? BA ? ? ? ? 48 ? ? ? 48 ? ? ? C3";
    constexpr std::string_view ALERT_CONTEXT_LOCATOR_PATTERN = "48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 41 0F 28 CA 48 8B 08 48 8B 51 58"; // "ViewAdvanceAlert"

} // namespace kx