#include "MumbleLinkManager.h"

#include <iostream>

namespace kx {

MumbleLinkManager::MumbleLinkManager() {
    // Initialization is handled lazily in the first Update call
}

MumbleLinkManager::~MumbleLinkManager() {
    if (m_mumbleLink) UnmapViewOfFile(m_mumbleLink);
    if (m_mumbleLinkFile) CloseHandle(m_mumbleLinkFile);
}

bool MumbleLinkManager::Initialize() {
    m_mumbleLinkFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(MumbleLinkData),
        L"MumbleLink"
    );

    if (m_mumbleLinkFile == NULL) {
        return false;
    }

    m_mumbleLink = static_cast<MumbleLinkData*>(MapViewOfFile(
        m_mumbleLinkFile,
        FILE_MAP_READ,
        0,
        0,
        sizeof(MumbleLinkData)
    ));

    if (m_mumbleLink == NULL) {
        CloseHandle(m_mumbleLinkFile);
        m_mumbleLinkFile = nullptr;
        return false;
    }

    m_mumbleLinkInitialized = true;
    return true;
}

void MumbleLinkManager::Update() {
    if (!m_mumbleLinkInitialized) {
        auto now = std::chrono::steady_clock::now();
        if (now - m_lastMumbleRetryTime >= MumbleRetryInterval) {
            m_lastMumbleRetryTime = now;
            if (!Initialize()) {
                return;
            }
        } else {
            return;
        }
    }

    if (m_mumbleLink->uiVersion != 2 ||
        std::wcscmp(m_mumbleLink->name, GW2_GAME_NAME) != 0 ||
        m_mumbleLink->uiTick == m_lastTick) {
        return; // Data is invalid or not updated
    }

    m_lastTick = m_mumbleLink->uiTick;
}

} // namespace kx
