#include "../../libs/Catch2/catch_amalgamated.hpp"

#include "../Game/AddressManager.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/SafeIterators.h"
#include <string>

// --- HELPER FUNCTIONS ---
template<typename Predicate>
kx::ReClass::ChCliCharacter findCharacter(kx::ReClass::ChCliContext& context, Predicate p) {
    kx::SafeAccess::CharacterList characterList(context);
    for (const auto& character : characterList) {
        if (p(character)) {
            return character;
        }
    }
    return kx::ReClass::ChCliCharacter(nullptr);
}

template<typename Predicate>
kx::ReClass::GdCliGadget findGadget(kx::ReClass::GdCliContext& context, Predicate p) {
    kx::SafeAccess::GadgetList gadgetList(context);
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
                CHECK(coreStats.GetLevel() == 80);
                CHECK(coreStats.GetProfession() != kx::Game::Profession::None);
                CHECK(coreStats.GetRace() != kx::Game::Race::None);

                auto health = localPlayer.GetHealth();
                CHECK(health.GetMax() > 1000.0f);
            }
        }

        // --- TEST 2: Hostile NPC Validation ---
        WHEN("Searching for a Hostile Training Golem") {
            auto golem = findCharacter(charContext, [](const auto& character) {
                return character.GetAttitude() == kx::Game::Attitude::Hostile;
                });

            THEN("The golem is found and its Attitude and Rank offsets are valid") {
                INFO("Could not find a hostile golem. Are you near the Siege Training Waypoint?");
                REQUIRE(golem.data() != nullptr);

                CHECK(golem.GetAttitude() == kx::Game::Attitude::Hostile);
                CHECK(golem.GetRank() == kx::Game::CharacterRank::Ambient);
            }
        }

        // --- TEST 3: Indifferent & Ambient NPC Validation ---
        WHEN("Searching for an Indifferent Training Golem") {
            auto golem = findCharacter(charContext, [](const auto& character) {
                // We now look for a character that is BOTH Indifferent AND Ambient.
                // This will correctly skip over any Elite guards that are also Indifferent.
                return character.GetAttitude() == kx::Game::Attitude::Indifferent
                    && character.GetRank() == kx::Game::CharacterRank::Ambient;
                });

            THEN("The golem is found and its Attitude and Rank offsets are valid") {
                INFO("Could not find an Indifferent AND Ambient golem. This test can be fragile if other NPCs are nearby.");
                REQUIRE(golem.data() != nullptr);

                CHECK(golem.GetAttitude() == kx::Game::Attitude::Indifferent);
                CHECK(golem.GetRank() == kx::Game::CharacterRank::Ambient);
            }
        }

        // --- TEST 4: PlayerCreated Gadget Validation ---
        WHEN("Searching for a Trebuchet gadget") {
            kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
            REQUIRE(gadgetContext.data() != nullptr);

            auto trebuchet = findGadget(gadgetContext, [](const auto& gadget) {
                return gadget.GetGadgetType() == kx::Game::GadgetType::PlayerCreated;
                });

            THEN("The trebuchet is found and its Type and Position offsets are valid") {
                INFO("Could not find a 'PlayerCreated' gadget (Trebuchet).");
                REQUIRE(trebuchet.data() != nullptr);
                CHECK(trebuchet.GetGadgetType() == kx::Game::GadgetType::PlayerCreated);

                glm::vec3 pos = trebuchet.GetAgKeyFramed().GetCoKeyFramed().GetPosition();
                CHECK(pos.x != 0.0f);
            }
        }

        // --- TEST 5: Waypoint Gadget Validation ---
        WHEN("Searching for the Siege Training Waypoint gadget") {
            kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
            REQUIRE(gadgetContext.data() != nullptr);

            auto waypoint = findGadget(gadgetContext, [](const auto& gadget) {
                return gadget.GetGadgetType() == kx::Game::GadgetType::Waypoint;
                });

            THEN("The waypoint is found and its Type and Position offsets are valid") {
                INFO("Could not find a 'Waypoint' gadget.");
                REQUIRE(waypoint.data() != nullptr);
                CHECK(waypoint.GetGadgetType() == kx::Game::GadgetType::Waypoint);

                glm::vec3 pos = waypoint.GetAgKeyFramed().GetCoKeyFramed().GetPosition();
                CHECK(pos.x != 0.0f);
            }
        }
    }
}