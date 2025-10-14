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
            CloseHandle(m_mumbleLinkFile);
            m_mumbleLinkFile = nullptr;
            m_mumbleLink = nullptr;
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
    
    // Convert wchar_t identity to UTF-8
    std::string identityUtf8 = StringHelpers::WCharToUTF8String(m_mumbleLink->identity);
    
    // Check for conversion errors
    if (identityUtf8.empty() || identityUtf8 == "[STRING_TOO_LONG]" || identityUtf8 == "[CONVERSION_ERROR]") {
        return;
    }
    
    // Parse JSON
    auto json = nlohmann::json::parse(identityUtf8, nullptr, false);
    
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
    return (m_mumbleLink->context.uiState & UiState::IsInCombat) != 0;
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

// ====== Elite Specialization Conversion ======

EliteSpec MumbleLinkManager::ConvertAnetSpecIdToEliteSpec(uint8_t anetId) const {
    // ArenaNet uses different IDs for elite specializations than our enum
    // This mapping is based on the official API specialization IDs
    switch (anetId) {
        case 5: return EliteSpec::Druid;
        case 7: return EliteSpec::Daredevil;
        case 18: return EliteSpec::Berserker;
        case 27: return EliteSpec::Dragonhunter;
        case 34: return EliteSpec::Reaper;
        case 40: return EliteSpec::Chronomancer;
        case 43: return EliteSpec::Scrapper;
        case 48: return EliteSpec::Tempest;
        case 52: return EliteSpec::Herald;
        case 55: return EliteSpec::Soulbeast;
        case 56: return EliteSpec::Weaver;
        case 57: return EliteSpec::Holosmith;
        case 58: return EliteSpec::Deadeye;
        case 59: return EliteSpec::Mirage;
        case 60: return EliteSpec::Scourge;
        case 61: return EliteSpec::Spellbreaker;
        case 62: return EliteSpec::Firebrand;
        case 63: return EliteSpec::Renegade;
        case 64: return EliteSpec::Harbinger;
        case 65: return EliteSpec::Willbender;
        case 66: return EliteSpec::Virtuoso;
        case 67: return EliteSpec::Catalyst;
        case 68: return EliteSpec::Bladesworn;
        case 69: return EliteSpec::Vindicator;
        case 70: return EliteSpec::Mechanist;
        case 71: return EliteSpec::Specter;
        case 72: return EliteSpec::Untamed;
        default: return EliteSpec::None;
    }
}

} // namespace kx