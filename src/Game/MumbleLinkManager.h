#pragma once
#include <chrono>
#include <windows.h>
#include "MumbleLink.h"

namespace kx {

    class MumbleLinkManager {
    public:
        MumbleLinkManager();
        ~MumbleLinkManager();

        // Updates the internal state by reading from shared memory. Call this once per frame.
        void Update();

        // Returns a pointer to the current MumbleLink data. Can be nullptr if not connected.
        const MumbleLinkData* GetData() const { return m_mumbleLink; }

        // Returns true if the MumbleLink connection is established and valid.
        bool IsInitialized() const { return m_mumbleLinkInitialized; }

    private:
        // Attempts to connect to the MumbleLink shared memory file.
        bool Initialize();

        HANDLE m_mumbleLinkFile = nullptr;
        MumbleLinkData* m_mumbleLink = nullptr;
        bool m_mumbleLinkInitialized = false;
        uint32_t m_lastTick = 0;

        // Retry logic for when the game starts before the MumbleLink is created.
        std::chrono::steady_clock::time_point m_lastMumbleRetryTime;
        const std::chrono::milliseconds MumbleRetryInterval = std::chrono::seconds(5);

        const wchar_t* GW2_MUMBLE_LINK_NAME = L"MumbleLink";
        const wchar_t* GW2_GAME_NAME = L"Guild Wars 2";
    };

} // namespace kx