#pragma once

#include <cstdint>
#include <windows.h>

namespace kx {

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

// Mirroring Gw2Sharp's Gw2Context struct
#pragma pack(push, 1)
struct MumbleContext {
    std::byte serverAddress[28]; // Server address (sockaddr_in or sockaddr_in6)
    uint32_t mapId;              // Current map ID. CRITICAL: This is 0 until a character is loaded into a map.
    uint32_t mapType;
    uint32_t shardId;
    uint32_t instance;
    uint32_t buildId;
    uint32_t uiState;            // Bitmask for UI state (map open, in combat, etc.)
    uint16_t compassWidth;       // In pixels
    uint16_t compassHeight;      // In pixels
    float compassRotation;       // In radians
    float playerX;               // Player coordinates on the continent
    float playerY;
    float mapCenterX;            // Map center coordinates on the continent
    float mapCenterY;
    float mapScale;
    uint32_t processId;
    uint8_t mountIndex;
    // ... remaining bytes are reserved/unused
};
static_assert(sizeof(MumbleContext) >= 56, "MumbleContext struct size is incorrect!"); // Basic check

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
    unsigned char context[256];
    wchar_t description[2048];
};
static_assert(sizeof(MumbleLinkData) >= 2972, "MumbleLinkData struct size is incorrect!");
#pragma pack(pop)

} // namespace kx