#include "ESPDataExtractor.h"
#include "../Core/AppState.h"
#include "../Game/AddressManager.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/SafeIterators.h"
#include "StringHelpers.h"

namespace kx {

void ESPDataExtractor::ExtractFrameData(FrameRenderData& frameData) {
    frameData.Clear();
    
    // Build character name to player name mapping
    std::map<void*, const wchar_t*> characterNameToPlayerName;
    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (pContextCollection) {
        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        
        if (charContext.data()) {
            // Safely iterate through player list
            kx::SafeAccess::PlayerList playerList(charContext);
            
            for (auto playerIt = playerList.begin(); playerIt != playerList.end(); ++playerIt) {
                if (playerIt.IsValid()) {
                    void* charDataPtr = playerIt.GetCharacterDataPtr();
                    const wchar_t* playerName = playerIt.GetName();
                    
                    if (charDataPtr && playerName) {
                        characterNameToPlayerName[charDataPtr] = playerName;
                    }
                }
            }
        }
    }
    
    // Extract data for all entity types
    ExtractPlayerData(frameData.players, characterNameToPlayerName);
    ExtractNpcData(frameData.npcs);
    ExtractGadgetData(frameData.gadgets);
}

void ESPDataExtractor::ExtractPlayerData(std::vector<RenderablePlayer>& players, const std::map<void*, const wchar_t*>& characterNameToPlayerName) {
    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection) return;
    
    kx::ReClass::ContextCollection ctxCollection(pContextCollection);
    kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
    if (!charContext.data()) return;
    
    // Safely iterate through character list for players
    kx::SafeAccess::CharacterList characterList(charContext);
    
    for (const auto& character : characterList) {
        void* characterDataPtr = const_cast<void*>(character.data());
        
        // Check if this character is a player
        if (characterNameToPlayerName.count(characterDataPtr)) {
            RenderablePlayer player;
            
            // Extract position
            kx::ReClass::ChCliCharacter& nonConstCharacter = const_cast<kx::ReClass::ChCliCharacter&>(character);
            kx::ReClass::AgChar agent = nonConstCharacter.GetAgent();
            if (!agent.data()) continue;
            
            kx::ReClass::CoChar coChar = agent.GetCoChar();
            if (!coChar.data()) continue;
            
            Coordinates3D gameWorldPos = coChar.GetVisualPosition();
            if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) continue;
            
            const float scaleFactor = 1.23f;
            player.position = glm::vec3(gameWorldPos.X / scaleFactor, gameWorldPos.Z / scaleFactor, gameWorldPos.Y / scaleFactor);
            player.isValid = true;
            
            // Extract player name
            auto playerNameIt = characterNameToPlayerName.find(characterDataPtr);
            if (playerNameIt != characterNameToPlayerName.end() && playerNameIt->second) {
                player.playerName = WStringToString(playerNameIt->second);
            }
            
            // Check if this is the local player
            void* localPlayer = AddressManager::GetLocalPlayer();
            player.isLocalPlayer = (characterDataPtr == localPlayer);
            
            // Extract health
            kx::ReClass::ChCliHealth health = nonConstCharacter.GetHealth();
            if (health.data()) {
                player.currentHealth = health.GetCurrent();
                player.maxHealth = health.GetMax();
            }
            
            // Extract energy
            kx::ReClass::ChCliEnergies energies = nonConstCharacter.GetEnergies();
            if (energies.data()) {
                player.currentEnergy = energies.GetCurrent();
                player.maxEnergy = energies.GetMax();
            }
            
            // Extract core stats
            kx::ReClass::ChCliCoreStats coreStats = nonConstCharacter.GetCoreStats();
            if (coreStats.data()) {
                player.level = coreStats.GetLevel();
                player.profession = static_cast<uint32_t>(coreStats.GetProfession());
                player.attitude = static_cast<uint32_t>(nonConstCharacter.GetAttitude());
            }
            
            players.push_back(player);
        }
    }
}

void ESPDataExtractor::ExtractNpcData(std::vector<RenderableNpc>& npcs) {
    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection) return;
    
    kx::ReClass::ContextCollection ctxCollection(pContextCollection);
    kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
    if (!charContext.data()) return;
    
    // Build character to player name mapping to exclude players
    std::map<void*, const wchar_t*> characterNameToPlayerName;
    kx::SafeAccess::PlayerList playerList(charContext);
    for (auto playerIt = playerList.begin(); playerIt != playerList.end(); ++playerIt) {
        if (playerIt.IsValid()) {
            characterNameToPlayerName[playerIt.GetCharacterDataPtr()] = playerIt.GetName();
        }
    }
    
    // Safely iterate through character list for NPCs
    kx::SafeAccess::CharacterList characterList(charContext);
    for (const auto& character : characterList) {
        void* characterDataPtr = const_cast<void*>(character.data());
        
        // Check if this character is NOT a player (i.e., it's an NPC)
        if (!characterNameToPlayerName.count(characterDataPtr)) {
            RenderableNpc npc;
            
            // Extract position
            kx::ReClass::ChCliCharacter& nonConstCharacter = const_cast<kx::ReClass::ChCliCharacter&>(character);
            kx::ReClass::AgChar agent = nonConstCharacter.GetAgent();
            if (!agent.data()) continue;
            
            kx::ReClass::CoChar coChar = agent.GetCoChar();
            if (!coChar.data()) continue;
            
            Coordinates3D gameWorldPos = coChar.GetVisualPosition();
            if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) continue;
            
            const float scaleFactor = 1.23f;
            npc.position = glm::vec3(gameWorldPos.X / scaleFactor, gameWorldPos.Z / scaleFactor, gameWorldPos.Y / scaleFactor);
            npc.isValid = true;
            
            // Extract health
            kx::ReClass::ChCliHealth health = nonConstCharacter.GetHealth();
            if (health.data()) {
                npc.currentHealth = health.GetCurrent();
                npc.maxHealth = health.GetMax();
            }
            
            // Extract basic stats
            kx::ReClass::ChCliCoreStats coreStats = nonConstCharacter.GetCoreStats();
            if (coreStats.data()) {
                npc.level = coreStats.GetLevel();
            }
            npc.attitude = static_cast<uint32_t>(nonConstCharacter.GetAttitude());
            
            npcs.push_back(npc);
        }
    }
}

void ESPDataExtractor::ExtractGadgetData(std::vector<RenderableGadget>& gadgets) {
    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection) return;
    
    kx::ReClass::ContextCollection ctxCollection(pContextCollection);
    kx::ReClass::GdCliContext gadgetCtx = ctxCollection.GetGdCliContext();
    if (!gadgetCtx.data()) return;
    
    // Safely iterate through gadget list
    kx::SafeAccess::GadgetList gadgetList(gadgetCtx);
    for (const auto& gadget : gadgetList) {
        RenderableGadget renderableGadget;
        
        // Extract position
        kx::ReClass::GdCliGadget& nonConstGadget = const_cast<kx::ReClass::GdCliGadget&>(gadget);
        kx::ReClass::AgKeyFramed agKeyFramed = nonConstGadget.GetAgKeyFramed();
        kx::ReClass::CoKeyFramed coKeyFramed = agKeyFramed.GetCoKeyFramed();
        if (!coKeyFramed.data()) continue;
        
        Coordinates3D gameWorldPos = coKeyFramed.GetPosition();
        if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) continue;
        
        const float scaleFactor = 1.23f;
        renderableGadget.position = glm::vec3(gameWorldPos.X / scaleFactor, gameWorldPos.Z / scaleFactor, gameWorldPos.Y / scaleFactor);
        renderableGadget.isValid = true;
        
        // Extract type
        renderableGadget.type = static_cast<uint32_t>(nonConstGadget.GetGadgetType());
        
        gadgets.push_back(renderableGadget);
    }
}

} // namespace kx