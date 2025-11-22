#pragma once

#include <chrono>
#include <windows.h>

#include "MumbleLink.h"

namespace kx {

class MumbleLinkManager {
public:
    enum class MumbleStatus {
        Disconnected,
        Connecting,  // File is mapped, but header is invalid.
        Connected    // File is mapped, and header is valid.
    };

    MumbleLinkManager();
    ~MumbleLinkManager();

    void Update();
    const MumbleLinkData* GetData() const { return m_mumbleLink; }
    bool IsInitialized() const { return m_status == MumbleStatus::Connected; }
    MumbleStatus GetStatus() const { return m_status; }

    // ====== Helper Methods ======
    
    /**
     * @brief Check if player is currently in combat
     */
    bool isInCombat() const;
    
    /**
     * @brief Check if player is in World vs World
     */
    bool isInWvW() const;
    
    /**
     * @brief Get the currently active mount
     */
    MountType currentMount() const;
    
    /**
     * @brief Check if player is mounted on any mount
     */
    bool isMounted() const;
    
    /**
     * @brief Get current map ID
     */
    uint32_t mapId() const;
    
    /**
     * @brief Get character profession
     */
    Profession characterProfession() const;
    
    /**
     * @brief Get character race
     */
    Race characterRace() const;
    
    /**
     * @brief Get character elite specialization (converted from raw API ID)
     */
    EliteSpec characterSpecialization() const;
    
    /**
     * @brief Get character name
     */
    const std::string& characterName() const;
    
    /**
     * @brief Get current UI state flags
     */
    uint32_t uiState() const;
    
    /**
     * @brief Get field of view from parsed identity data
     * @return FOV in radians, or 0.0f if not available
     */
    float GetFov() const;
    
    /**
     * @brief Get field of view with fallback to default value
     * @param defaultFov Default FOV to use if not available (default: 1.0472f ~60 degrees)
     * @return FOV in radians
     */
    float GetFovOrDefault(float defaultFov = 1.0472f) const;

private:
    bool Initialize();
    void ParseIdentity();
    EliteSpec ConvertAnetSpecIdToEliteSpec(uint8_t anetId) const;

    HANDLE m_mumbleLinkFile = nullptr;
    MumbleLinkData* m_mumbleLink = nullptr;
    
    MumbleStatus m_status = MumbleStatus::Disconnected;
    
    std::chrono::steady_clock::time_point m_lastMumbleRetryTime;
    const std::chrono::milliseconds MumbleRetryInterval = std::chrono::seconds(5);
    uint32_t m_lastTick = 0;
    const wchar_t* GW2_GAME_NAME = L"Guild Wars 2";
    
    // Parsed identity data
    Identity m_identity;
};

} // namespace kx
