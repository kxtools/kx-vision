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
struct Gw2Context {
    // Only including relevant fields for now, based on Gw2Sharp's Gw2Context.cs
    // The full struct is 256 bytes, but we only need uiState for this task.
    // We need to ensure correct offset for uiState.
    // Gw2Context.cs: [FieldOffset(48)] public UiState uiState;
    char _padding1[48]; // Padding to reach uiState offset
    UiState uiState;
    char _padding2[256 - 48 - sizeof(UiState)]; // Remaining padding to fill 256 bytes
};
#pragma pack(pop)

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
    Gw2Context context; // Use the defined Gw2Context struct
    wchar_t description[2048];
};
#pragma pack(pop)

} // namespace kx