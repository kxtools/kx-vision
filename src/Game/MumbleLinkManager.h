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

private:
    bool Initialize();

    HANDLE m_mumbleLinkFile = nullptr;
    MumbleLinkData* m_mumbleLink = nullptr;
    bool m_mumbleLinkInitialized = false;
    std::chrono::steady_clock::time_point m_lastMumbleRetryTime;
    const std::chrono::milliseconds MumbleRetryInterval = std::chrono::seconds(5);
    uint32_t m_lastTick = 0;
    const wchar_t* GW2_GAME_NAME = L"Guild Wars 2";
};

} // namespace kx
