#include "../../libs/Catch2/catch_amalgamated.hpp"

#include "../Memory/AddressManager.h"
#include "../Game/SdkStructs.h"
#include "../Memory/SafeGameArray.h"
#include <string>
#include <sstream>
#include <map>
#include <vector>

// --- HELPER FUNCTIONS ---

struct OffsetDiagnostic {
    bool is_valid;
    std::string failed_at;
    uintptr_t failed_offset;
    void* failed_address;
    
    OffsetDiagnostic() : is_valid(true), failed_at(""), failed_offset(0), failed_address(nullptr) {}
};

std::string DiagnoseOffset(
    void* basePtr,
    const std::string& structName,
    const std::string& offsetName,
    uintptr_t offsetValue,
    size_t valueSize = sizeof(uint32_t)
) {
    if (!basePtr) {
        return structName + " base pointer is NULL";
    }
    
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(basePtr);
    uintptr_t targetAddr = baseAddr + offsetValue;
    
    std::stringstream ss;
    ss << "\n   → Sample " << structName << " @0x" << std::hex << baseAddr;
    ss << ", " << offsetName << "(0x" << std::hex << offsetValue << ") → addr:0x" << std::hex << targetAddr;
    
    // Read value at offset
    if (valueSize == sizeof(uint32_t)) {
        uint32_t value = 0;
        kx::Debug::SafeRead(basePtr, offsetValue, value);
        ss << " = 0x" << std::hex << value;
    } else if (valueSize == sizeof(uint64_t)) {
        uint64_t value = 0;
        kx::Debug::SafeRead(basePtr, offsetValue, value);
        ss << " = 0x" << std::hex << value;
    } else if (valueSize == sizeof(uint16_t)) {
        uint16_t value = 0;
        kx::Debug::SafeRead(basePtr, offsetValue, value);
        ss << " = 0x" << std::hex << value;
    } else if (valueSize == sizeof(uint8_t)) {
        uint8_t value = 0;
        kx::Debug::SafeRead(basePtr, offsetValue, value);
        ss << " = 0x" << std::hex << static_cast<uint32_t>(value);
    }
    
    // Generate nearby offset suggestions (±8, ±16 byte increments)
    ss << "\n   → Inspect in Cheat Engine, try nearby offsets: ";
    std::vector<uintptr_t> suggestions;
    for (int delta : {-16, -8, 8, 16, 24}) {
        if (static_cast<int>(offsetValue) + delta >= 0) {
            suggestions.push_back(offsetValue + delta);
        }
    }
    
    for (size_t i = 0; i < suggestions.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "0x" << std::hex << suggestions[i];
    }
    
    return ss.str();
}

std::string DiagnoseGadgetAccess(const kx::ReClass::GdCliGadget& gadget) {
    if (!gadget.data()) {
        return "Gadget base pointer is NULL";
    }
    
    std::stringstream result;
    result << "Gadget @0x" << std::hex << reinterpret_cast<uintptr_t>(gadget.data());
    
    auto agKeyFramed = gadget.GetAgKeyFramed();
    if (!agKeyFramed.data()) {
        result << " → AG_KEYFRAMED(0x38) FAILED";
        return result.str();
    }
    result << " → AgKeyFramed @0x" << std::hex << reinterpret_cast<uintptr_t>(agKeyFramed.data());
    
    auto coKeyFramed = agKeyFramed.GetCoKeyFramed();
    if (!coKeyFramed.data()) {
        result << " → CO_KEYFRAMED(0x50) FAILED";
        return result.str();
    }
    result << " → CoKeyFramed @0x" << std::hex << reinterpret_cast<uintptr_t>(coKeyFramed.data());
    
    glm::vec3 pos = coKeyFramed.GetPosition();
    if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f) {
        result << " → Position(0x30) returns (0,0,0)";
        return result.str();
    }
    result << " ✓";
    return result.str();
}

std::string DiagnoseCharacterAccess(const kx::ReClass::ChCliCharacter& character) {
    if (!character.data()) {
        return "Character base pointer is NULL";
    }
    
    std::stringstream result;
    result << "Character @0x" << std::hex << reinterpret_cast<uintptr_t>(character.data());
    
    auto agent = character.GetAgent();
    if (!agent.data()) {
        result << " → Agent(0x98) FAILED";
        return result.str();
    }
    result << " → Agent @0x" << std::hex << reinterpret_cast<uintptr_t>(agent.data());
    
    auto coChar = agent.GetCoChar();
    if (!coChar.data()) {
        result << " → CO_CHAR(0x50) FAILED";
        return result.str();
    }
    result << " → CoChar @0x" << std::hex << reinterpret_cast<uintptr_t>(coChar.data());
    
    glm::vec3 pos = coChar.GetVisualPosition();
    if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f) {
        result << " → Position(0x30) returns (0,0,0)";
        return result.str();
    }
    result << " ✓";
    return result.str();
}

template<typename Predicate>
std::string FindCharacterWithDiagnostics(kx::ReClass::ChCliContext& context, Predicate p, 
                                         std::string& diagnosticInfo) {
    auto characterList = context.GetCharacters();
    size_t totalCount = 0;
    std::map<int, int> typeCounts;
    kx::ReClass::ChCliCharacter firstCharacter(nullptr);
    
    for (const auto& character : characterList) {
        totalCount++;
        if (totalCount == 1) {
            firstCharacter = character;
        }
        
        if (p(character)) {
            diagnosticInfo = DiagnoseCharacterAccess(character);
            return "Found";
        }
        auto attitude = character.GetAttitude();
        typeCounts[static_cast<int>(attitude)]++;
    }
    
    std::stringstream ss;
    ss << "No matching character found in " << totalCount << " total characters. ";
    ss << "Attitudes found: ";
    for (auto& pair : typeCounts) {
        ss << pair.first << "(" << pair.second << ") ";
    }
    
    // If suspiciously few attitude types, show memory address details
    if (totalCount > 5 && typeCounts.size() <= 2 && firstCharacter.data()) {
        ss << DiagnoseOffset(firstCharacter.data(), "ChCliCharacter", "ATTITUDE", 0x00C0);
    }
    
    diagnosticInfo = ss.str();
    return "NotFound";
}

template<typename Predicate>
std::string FindGadgetWithDiagnostics(kx::ReClass::GdCliContext& context, Predicate p,
                                      std::string& diagnosticInfo) {
    auto gadgetList = context.GetGadgets();
    size_t totalCount = 0;
    std::map<int, int> typeCounts;
    kx::ReClass::GdCliGadget firstGadget(nullptr);
    
    for (const auto& gadget : gadgetList) {
        totalCount++;
        if (totalCount == 1) {
            firstGadget = gadget;
        }
        
        auto type = gadget.GetGadgetType();
        typeCounts[static_cast<int>(type)]++;
        
        if (p(gadget)) {
            diagnosticInfo = DiagnoseGadgetAccess(gadget);
            return "Found";
        }
    }
    
    std::stringstream ss;
    ss << "No matching gadget found in " << totalCount << " total gadgets. ";
    ss << "Types found: ";
    for (auto& pair : typeCounts) {
        ss << pair.first << "(" << pair.second << ") ";
    }
    
    // If all gadgets have type 0, show memory address details for Cheat Engine inspection
    if (totalCount > 0 && typeCounts.size() == 1 && typeCounts.count(0) > 0 && firstGadget.data()) {
        ss << DiagnoseOffset(firstGadget.data(), "GdCliGadget", "TYPE", 0x0200);
    }
    
    diagnosticInfo = ss.str();
    return "NotFound";
}

template<typename Predicate>
kx::ReClass::ChCliCharacter findCharacter(kx::ReClass::ChCliContext& context, Predicate p) {
    auto characterList = context.GetCharacters();
    for (const auto& character : characterList) {
        if (p(character)) {
            return character;
        }
    }
    return kx::ReClass::ChCliCharacter(nullptr);
}

template<typename Predicate>
kx::ReClass::GdCliGadget findGadget(kx::ReClass::GdCliContext& context, Predicate p) {
    auto gadgetList = context.GetGadgets();
    for (const auto& gadget : gadgetList) {
        if (p(gadget)) {
            return gadget;
        }
    }
    return kx::ReClass::GdCliGadget(nullptr);
}


// --- TEST SCENARIOS ---

SCENARIO("Live Offset Validation in PvP Lobby", "[Offsets]")
{
    INFO("--- INSTRUCTIONS ---");
    INFO("For these tests to pass, stand near the Siege Training Waypoint in the PvP Lobby.");
    INFO("You should see two types of golems: hostile red ones, and neutral indifferent ones.");

    GIVEN("A valid ContextCollection pointer") {
        void* pContextCollection = kx::AddressManager::GetContextCollectionPtr();
        INFO("Could not get ContextCollection. Are patterns outdated?");
        REQUIRE(pContextCollection != nullptr);

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        REQUIRE(charContext.data() != nullptr);

        // --- TEST 1: Local Player Validation ---
        WHEN("Accessing the local player's data") {
            void* pLocalPlayer = kx::AddressManager::GetLocalPlayer();
            INFO("Could not get Local Player pointer from ChCliContext.");
            REQUIRE(pLocalPlayer != nullptr);

            kx::ReClass::ChCliCharacter localPlayer(pLocalPlayer);

            THEN("The player's position and core stats offsets are valid") {
                glm::vec3 pos = localPlayer.GetAgent().GetCoChar().GetVisualPosition();
                INFO("Position was (0,0,0). Are you fully loaded into the map?");
                CHECK(pos.x != 0.0f);

                auto coreStats = localPlayer.GetCoreStats();
                CHECK(coreStats.GetScaledLevel() == 80);
                CHECK(coreStats.GetProfession() != kx::Game::Profession::None);
                CHECK(coreStats.GetRace() != kx::Game::Race::None);

                auto health = localPlayer.GetHealth();
                CHECK(health.GetMax() > 1000.0f);
            }
        }

        // --- TEST 2: Hostile NPC Validation ---
        WHEN("Searching for a Hostile Training Golem") {
            std::string diagnosticInfo;
            std::string result = FindCharacterWithDiagnostics(charContext,
                [](const auto& character) {
                    return character.GetAttitude() == kx::Game::Attitude::Hostile;
                }, diagnosticInfo);

            THEN("The golem is found and its Attitude and Rank offsets are valid") {
                INFO(diagnosticInfo);
                REQUIRE(result == "Found");
                
                auto golem = findCharacter(charContext, [](const auto& character) {
                    return character.GetAttitude() == kx::Game::Attitude::Hostile;
                });

                CHECK(golem.GetAttitude() == kx::Game::Attitude::Hostile);
                CHECK(golem.GetRank() == kx::Game::CharacterRank::Ambient);
            }
        }

        // --- TEST 3: Indifferent & Ambient NPC Validation ---
        WHEN("Searching for an Indifferent Training Golem") {
            std::string diagnosticInfo;
            std::string result = FindCharacterWithDiagnostics(charContext,
                [](const auto& character) {
                    // We now look for a character that is BOTH Indifferent AND Ambient.
                    // This will correctly skip over any Elite guards that are also Indifferent.
                    return character.GetAttitude() == kx::Game::Attitude::Indifferent
                        && character.GetRank() == kx::Game::CharacterRank::Ambient;
                }, diagnosticInfo);

            THEN("The golem is found and its Attitude and Rank offsets are valid") {
                INFO(diagnosticInfo);
                REQUIRE(result == "Found");
                
                auto golem = findCharacter(charContext, [](const auto& character) {
                    return character.GetAttitude() == kx::Game::Attitude::Indifferent
                        && character.GetRank() == kx::Game::CharacterRank::Ambient;
                });

                CHECK(golem.GetAttitude() == kx::Game::Attitude::Indifferent);
                CHECK(golem.GetRank() == kx::Game::CharacterRank::Ambient);
            }
        }

        // --- TEST 4: PlayerCreated Gadget Validation ---
        WHEN("Searching for a Trebuchet gadget") {
            kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
            REQUIRE(gadgetContext.data() != nullptr);

            std::string diagnosticInfo;
            std::string result = FindGadgetWithDiagnostics(gadgetContext, 
                [](const auto& gadget) {
                    return gadget.GetGadgetType() == kx::Game::GadgetType::PlayerCreated;
                }, diagnosticInfo);

            THEN("The trebuchet is found and its Type and Position offsets are valid") {
                INFO(diagnosticInfo);
                REQUIRE(result == "Found");
                
                auto trebuchet = findGadget(gadgetContext, [](const auto& gadget) {
                    return gadget.GetGadgetType() == kx::Game::GadgetType::PlayerCreated;
                });
                
                CHECK(trebuchet.GetGadgetType() == kx::Game::GadgetType::PlayerCreated);

                glm::vec3 pos = trebuchet.GetAgKeyFramed().GetCoKeyFramed().GetPosition();
                CHECK(pos.x != 0.0f);
            }
        }

        // --- TEST 5: Waypoint Gadget Validation ---
        WHEN("Searching for the Siege Training Waypoint gadget") {
            kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
            REQUIRE(gadgetContext.data() != nullptr);

            std::string diagnosticInfo;
            std::string result = FindGadgetWithDiagnostics(gadgetContext,
                [](const auto& gadget) {
                    return gadget.GetGadgetType() == kx::Game::GadgetType::Waypoint;
                }, diagnosticInfo);

            THEN("The waypoint is found and its Type and Position offsets are valid") {
                INFO(diagnosticInfo);
                REQUIRE(result == "Found");
                
                auto waypoint = findGadget(gadgetContext, [](const auto& gadget) {
                    return gadget.GetGadgetType() == kx::Game::GadgetType::Waypoint;
                });
                
                CHECK(waypoint.GetGadgetType() == kx::Game::GadgetType::Waypoint);

                glm::vec3 pos = waypoint.GetAgKeyFramed().GetCoKeyFramed().GetPosition();
                CHECK(pos.x != 0.0f);
            }
        }
    }
}