#include "MumbleLinkManager.h"

#include <nlohmann/json.hpp>
#include "../Utils/StringHelpers.h"

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
        m_status = MumbleStatus::Disconnected;
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
        m_status = MumbleStatus::Disconnected;
        return false;
    }

    // On success, we just report that we have a mapped file.
    // We don't know if it's valid yet.
    m_status = MumbleStatus::Connecting;
    return true;
}

void MumbleLinkManager::Update() {
    if (!m_mumbleLink) {
        m_status = MumbleStatus::Disconnected;
        auto now = std::chrono::steady_clock::now();
        if (now - m_lastMumbleRetryTime >= MumbleRetryInterval) {
            m_lastMumbleRetryTime = now;
            Initialize();
        }
        return;
    }

    // The definitive check on every frame, as requested.
    bool isHeaderValid = (m_mumbleLink->uiVersion == 2 && std::wcscmp(m_mumbleLink->name, GW2_GAME_NAME) == 0);

    if (isHeaderValid) {
        m_status = MumbleStatus::Connected;
        if (m_mumbleLink->uiTick != m_lastTick) {
            m_lastTick = m_mumbleLink->uiTick;
            ParseIdentity();
        }
    } else {
        // Header is invalid.
        if (m_status == MumbleStatus::Connected) {
            // If we were connected, it means the game just closed. Disconnect fully.
            m_status = MumbleStatus::Disconnected;
            if (m_mumbleLink) {
                UnmapViewOfFile(m_mumbleLink);  // Properly unmap the view
                m_mumbleLink = nullptr;
            }
            if (m_mumbleLinkFile) {
                CloseHandle(m_mumbleLinkFile);
                m_mumbleLinkFile = nullptr;
            }
        } else {
            // Otherwise, we are connected to a file, but it has no valid game data.
            // This is the "Connecting" state. The GUI will handle what to do with it.
            m_status = MumbleStatus::Connecting;
        }
    }
}

// ====== Identity Parsing ======

void MumbleLinkManager::ParseIdentity() {
    if (!m_mumbleLink) {
        m_identity = Identity{}; // Reset to default
        return;
    }
    
    // Reset identity
    m_identity = Identity{};
    
    char jsonBuffer[1024];
    size_t len = StringHelpers::WriteWCharToUTF8(m_mumbleLink->identity, jsonBuffer);
    
    if (len == 0) {
        return;
    }
    
    auto json = nlohmann::json::parse(jsonBuffer, jsonBuffer + len, nullptr, false);
    
    if (json.is_discarded()) {
        return; // Parsing failed
    }
    
    // Helper to safely extract values
    auto updateIfExists = [&json](auto& value, const char* key) {
        auto f = json.find(key);
        if (f != json.end() && !f->is_null()) {
            value = f->get<std::decay_t<decltype(value)>>();
        }
    };
    
    // Extract identity fields
    updateIfExists(m_identity.commander, "commander");
    updateIfExists(m_identity.fov, "fov");
    updateIfExists(m_identity.uiScale, "uisz");
    
    // Race - convert uint8_t to Race enum
    uint8_t raceValue = 0;
    updateIfExists(raceValue, "race");
    if (raceValue <= 4) {
        m_identity.race = static_cast<Race>(raceValue);
    }
    
    updateIfExists(m_identity.specialization, "spec");
    
    // Profession - convert uint8_t to Profession enum
    uint8_t profValue = 0;
    updateIfExists(profValue, "profession");
    if (profValue <= 9) {
        m_identity.profession = static_cast<Profession>(profValue);
    }
    
    updateIfExists(m_identity.name, "name");
}

// ====== Helper Methods ======

bool MumbleLinkManager::isInCombat() const {
    if (!m_mumbleLink) return false;
    return (m_mumbleLink->context.uiState & IsInCombat) != 0;
}

bool MumbleLinkManager::isInWvW() const {
    if (!m_mumbleLink) return false;
    
    uint32_t mt = m_mumbleLink->context.mapType;
    return mt == 18 || (mt >= 9 && mt <= 15 && mt != 13);
}

MountType MumbleLinkManager::currentMount() const {
    if (!m_mumbleLink) return MountType::None;
    
    uint8_t mountIdx = m_mumbleLink->context.mountIndex;
    if (mountIdx > 10) return MountType::None;
    
    return static_cast<MountType>(mountIdx);
}

bool MumbleLinkManager::isMounted() const {
    if (!m_mumbleLink) return false;
    return m_mumbleLink->context.mountIndex != 0;
}

uint32_t MumbleLinkManager::mapId() const {
    if (!m_mumbleLink) return 0;
    return m_mumbleLink->context.mapId;
}

Profession MumbleLinkManager::characterProfession() const {
    return m_identity.profession;
}

Race MumbleLinkManager::characterRace() const {
    return m_identity.race;
}

EliteSpec MumbleLinkManager::characterSpecialization() const {
    return ConvertAnetSpecIdToEliteSpec(m_identity.specialization);
}

const std::string& MumbleLinkManager::characterName() const {
    return m_identity.name;
}

uint32_t MumbleLinkManager::uiState() const {
    if (!m_mumbleLink) return 0;
    return m_mumbleLink->context.uiState;
}

float MumbleLinkManager::GetFov() const {
    return m_identity.fov;
}

float MumbleLinkManager::GetFovOrDefault(float defaultFov) const {
    return (m_identity.fov > 0.01f) ? m_identity.fov : defaultFov;
}

// ====== Elite Specialization Conversion ======

Game::EliteSpec MumbleLinkManager::ConvertAnetSpecIdToEliteSpec(uint8_t anetId) const {
    // EliteSpec enum now uses ArenaNet API IDs directly, so we can cast directly
    // Only validate that the ID is within the known range
    if (anetId == 0 || (anetId >= 5 && anetId <= 72)) {
        return static_cast<Game::EliteSpec>(anetId);
    }
    return Game::EliteSpec::None;
}

} // namespace kx