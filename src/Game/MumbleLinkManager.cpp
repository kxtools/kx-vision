#include "MumbleLinkManager.h"
#include <string> // Required for std::wcscmp

namespace kx {

    MumbleLinkManager::MumbleLinkManager() {
        // Initialization is deferred to the first Update() call.
        // This makes the startup process more resilient to timing issues.
    }

    MumbleLinkManager::~MumbleLinkManager() {
        // Clean up Windows handles on destruction.
        if (m_mumbleLink) {
            UnmapViewOfFile(m_mumbleLink);
            m_mumbleLink = nullptr;
        }
        if (m_mumbleLinkFile) {
            CloseHandle(m_mumbleLinkFile);
            m_mumbleLinkFile = nullptr;
        }
    }

    bool MumbleLinkManager::Initialize() {
        // Try to open the shared memory file created by Guild Wars 2.
        m_mumbleLinkFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(MumbleLinkData),
            GW2_MUMBLE_LINK_NAME
        );

        if (m_mumbleLinkFile == NULL) {
            // This is not an error; it just means the game hasn't created the MumbleLink yet.
            return false;
        }

        // Map the shared memory file into our process's address space.
        m_mumbleLink = static_cast<MumbleLinkData*>(MapViewOfFile(
            m_mumbleLinkFile,
            FILE_MAP_READ,
            0,
            0,
            sizeof(MumbleLinkData)
        ));

        if (m_mumbleLink == NULL) {
            // If mapping fails, clean up and report failure.
            CloseHandle(m_mumbleLinkFile);
            m_mumbleLinkFile = nullptr;
            return false;
        }

        m_mumbleLinkInitialized = true;
        return true;
    }

    void MumbleLinkManager::Update() {
        // If we are not yet connected, periodically try to initialize.
        if (!m_mumbleLinkInitialized) {
            auto now = std::chrono::steady_clock::now();
            if (now - m_lastMumbleRetryTime >= MumbleRetryInterval) {
                m_lastMumbleRetryTime = now;
                Initialize(); // Attempt to connect.
            }
            // Return early if we are still not connected.
            return;
        }

        // --- Data Validation ---
        // Once connected, we must continuously validate the data. If the game closes
        // or another application creates a MumbleLink, the data will become invalid.
        if (m_mumbleLink->uiVersion != 2 ||
            std::wcscmp(m_mumbleLink->name, GW2_GAME_NAME) != 0) {

            // Data is invalid. Mark as uninitialized to trigger reconnect attempts.
            //m_mumbleLinkInitialized = false;

            //// Clean up resources.
            //UnmapViewOfFile(m_mumbleLink);
            //m_mumbleLink = nullptr;
            //CloseHandle(m_mumbleLinkFile);
            //m_mumbleLinkFile = nullptr;
            return;
        }

        // If data is valid, update our internal tick counter.
        // The data in m_mumbleLink is always the latest from shared memory.
        if (m_mumbleLink->uiTick != m_lastTick) {
            m_lastTick = m_mumbleLink->uiTick;
        }
    }

} // namespace kx