#include "../../libs/Catch2/catch_amalgamated.hpp"

#include "../Game/AddressManager.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/SafeIterators.h"

SCENARIO("Core Pointer Validation", "[Core]")
{
    GIVEN("The DLL is injected and AddressManager is initialized")
    {
        WHEN("The ContextCollection pointer is retrieved")
        {
            void* pContext = kx::AddressManager::GetContextCollectionPtr();
            THEN("The pointer should not be null")
            {
                REQUIRE(pContext != nullptr);
            }
        }
    }
}

SCENARIO("Live Offset Validation at Siege Training Waypoint", "[Offsets]")
{
    INFO("--- INSTRUCTIONS ---");
    INFO("For this test to pass, please stand at the 'Siege Training Waypoint' in the PvP Lobby.");

    GIVEN("A valid ContextCollection pointer") {
        void* pContextCollection = kx::AddressManager::GetContextCollectionPtr();
        INFO("Could not get ContextCollection. Are patterns outdated?");
        REQUIRE(pContextCollection != nullptr);

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        REQUIRE(charContext.data() != nullptr);

        // --- LOCAL PLAYER & POSITION TEST ---
        WHEN("Accessing the local player's data") {
            void* pLocalPlayer = kx::AddressManager::GetLocalPlayer();
            REQUIRE(pLocalPlayer != nullptr);

            kx::ReClass::ChCliCharacter localPlayer(pLocalPlayer);
            kx::ReClass::AgChar agent = localPlayer.GetAgent();
            REQUIRE(agent.data() != nullptr);

            glm::vec3 pos = agent.GetCoChar().GetVisualPosition();

            THEN("The player's position is sane and core stats are valid") {
                INFO("Position was (0,0,0). Are you fully loaded into the map?");
                REQUIRE(pos.x != 0.0f); // Simple check to ensure we're not at the world origin
                REQUIRE(localPlayer.GetHealth().GetMax() > 1000.0f);
            }
        }

        // --- HOSTILE NPC TEST ---
        WHEN("Searching for the nearby Hostile Training Golem") {
            kx::ReClass::ChCliCharacter golem(nullptr);
            kx::SafeAccess::CharacterList characterList(charContext);

            for (auto character : characterList) {
                if (character.GetAttitude() == kx::Game::Attitude::Hostile) {
                    golem.reset(character.data());
                    break;
                }
            }

            THEN("A golem is found and its offsets are valid") {
                INFO("Could not find a hostile golem. Are you at the Siege Training Waypoint?");
                REQUIRE(golem.data() != nullptr);
                REQUIRE(golem.GetAttitude() == kx::Game::Attitude::Hostile);
                REQUIRE(golem.GetRank() != kx::Game::CharacterRank::Normal);
            }
        }

        // --- GADGET TEST ---
        WHEN("Searching for a Trebuchet gadget") {
            kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
            REQUIRE(gadgetContext.data() != nullptr);

            kx::ReClass::GdCliGadget trebuchet(nullptr);
            kx::SafeAccess::GadgetList gadgetList(gadgetContext);

            for (auto gadget : gadgetList) {
                if (gadget.GetGadgetType() == kx::Game::GadgetType::PlayerCreated) {
                    trebuchet.reset(gadget.data());
                    break;
                }
            }

            THEN("The trebuchet is found and its type offset is valid") {
                INFO("Could not find a 'PlayerCreated' gadget (Trebuchet). Are you at the Siege Training Waypoint?");
                REQUIRE(trebuchet.data() != nullptr);
                REQUIRE(trebuchet.GetGadgetType() == kx::Game::GadgetType::PlayerCreated);
            }
        }
    }
}