#pragma once

#include <chrono>
#include <windows.h>

#include "MumbleLink.h"

namespace kx {

class MumbleLinkManager {
public:
    MumbleLinkManager();
    ~MumbleLinkManager();

    void Update();
    const MumbleLinkData* GetData() const { return m_mumbleLink; }
    bool IsInitialized() const { return m_mumbleLinkInitialized; }

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

private:
    bool Initialize();
    void ParseIdentity();
    EliteSpec ConvertAnetSpecIdToEliteSpec(uint8_t anetId) const;

    HANDLE m_mumbleLinkFile = nullptr;
    MumbleLinkData* m_mumbleLink = nullptr;
    bool m_mumbleLinkInitialized = false;
    std::chrono::steady_clock::time_point m_lastMumbleRetryTime;
    const std::chrono::milliseconds MumbleRetryInterval = std::chrono::seconds(5);
    uint32_t m_lastTick = 0;
    const wchar_t* GW2_GAME_NAME = L"Guild Wars 2";
    
    // Parsed identity data
    Identity m_identity;
};

} // namespace kx
