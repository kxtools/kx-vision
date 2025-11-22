#pragma once

#include <cstdint>
#include <string>
#include <windows.h>
#include "GameEnums.h"

namespace kx {

// ====== Forward Declarations ======
// Use consolidated enums from GameEnums.h
using Game::Profession;
using Game::Race;
using Game::EliteSpec;
using Game::MountType;

// ====== UI State Flags ======

// Mirroring Gw2Sharp's UiState enum
enum UiState : uint32_t {
    IsMapOpen = 1 << 0,
    IsCompassTopRight = 1 << 1,
    IsCompassRotationEnabled = 1 << 2,
    DoesGameHaveFocus = 1 << 3,
    IsCompetitiveMode = 1 << 4,
    DoesAnyInputHaveFocus = 1 << 5,
    IsInCombat = 1 << 6
};

// ====== Context Structures ======

/**
 * @brief Complete MumbleLink context structure (256 bytes)
 * Based on official GW2 MumbleLink specification
 */
#pragma pack(push, 1)
struct MumbleContext {
    uint8_t serverAddress[28];  // sockaddr_in or sockaddr_in6
    uint32_t mapId;
    uint32_t mapType;
    uint32_t shardId;
    uint32_t instance;
    uint32_t buildId;
    // Additional data beyond the 48 bytes Mumble uses for identification
    uint32_t uiState;          // Bitmask: See UiState enum
    uint16_t compassWidth;     // pixels
    uint16_t compassHeight;    // pixels
    float compassRotation;     // radians
    float playerX;             // continent coords
    float playerY;             // continent coords
    float mapCenterX;          // continent coords
    float mapCenterY;          // continent coords
    float mapScale;
    uint32_t processId;
    uint8_t mountIndex;
    uint8_t _padding[187];     // Pad to 256 bytes total
};
#pragma pack(pop)

// Legacy alias for backward compatibility
using Gw2Context = MumbleContext;

// ====== Identity Data ======

/**
 * @brief Parsed player identity information from MumbleLink
 * This data comes from the JSON-formatted identity field
 */
struct Identity {
    bool commander = false;           // Whether player is commanding in squad
    float fov = 0.0f;                // Field of view
    uint8_t uiScale = 0;             // UI scale setting
    Race race = Race::Human;         // Character race
    uint8_t specialization = 0;      // Elite specialization ID (raw value from API)
    Profession profession = Profession::None; // Character profession
    std::string name;                // Character name
};

// ====== Main MumbleLink Data Structure ======

#pragma pack(push, 1)
struct MumbleLinkData {
    UINT32 uiVersion;
    DWORD  uiTick;
    float  fAvatarPosition[3];
    float  fAvatarFront[3];
    float  fAvatarTop[3];
    wchar_t name[256];
    float  fCameraPosition[3];
    float  fCameraFront[3];
    float  fCameraTop[3];
    wchar_t identity[256];
    UINT32 context_len;
    MumbleContext context;
    wchar_t description[2048];
};
#pragma pack(pop)

} // namespace kx