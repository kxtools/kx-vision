#include "ESPDataExtractor.h"
#include "../Game/AddressManager.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/SafeIterators.h"
#include "../Utils/MemorySafety.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"

namespace kx {

void ESPDataExtractor::ExtractFrameData(ObjectPool<RenderablePlayer>& playerPool,
                                        ObjectPool<RenderableNpc>& npcPool, 
                                        ObjectPool<RenderableGadget>& gadgetPool,
                                        PooledFrameRenderData& pooledData) {
    pooledData.Reset();

    // FAIL-FAST: Proactive validation of root context collection
    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection || !kx::SafeAccess::IsMemorySafe(pContextCollection)) {
        return;
    }

    // Extract all data directly into object pools
    std::map<void*, const wchar_t*> characterNameToPlayerName;
    
    // Build character name to player name mapping first
    {
        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        if (charContext.data()) {
            kx::SafeAccess::PlayerList playerList(charContext);
            for (auto playerIt = playerList.begin(); playerIt != playerList.end(); ++playerIt) {
                if (playerIt.IsValid()) {
                    characterNameToPlayerName[playerIt.GetCharacterDataPtr()] = playerIt.GetName();
                }
            }
        }
    }
    
    ExtractPlayerData(playerPool, pooledData.players, characterNameToPlayerName);
    ExtractNpcData(npcPool, pooledData.npcs);
    ExtractGadgetData(gadgetPool, pooledData.gadgets);
}

void ESPDataExtractor::ExtractPlayerData(ObjectPool<RenderablePlayer>& playerPool,
                                        std::vector<RenderablePlayer*>& players,
                                        const std::map<void*, const wchar_t*>& characterNameToPlayerName) {
    players.clear();
    players.reserve(ExtractionCapacity::PLAYERS_RESERVE); // Reserve reasonable space

    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection || !kx::SafeAccess::IsMemorySafe(pContextCollection)) return;
    
    kx::ReClass::ContextCollection ctxCollection(pContextCollection);
    kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
    if (!charContext.data()) return;
    
    // Safely iterate through character list for players
    kx::SafeAccess::CharacterList characterList(charContext);
    
    for (const auto& character : characterList) {
        void* characterDataPtr = const_cast<void*>(character.data());
        
        // Check if this character is a player
        if (characterNameToPlayerName.count(characterDataPtr)) {
            // Get an object from the pool instead of creating a new one
            RenderablePlayer* renderablePlayer = playerPool.Get();
            if (!renderablePlayer) {
                // Pool exhausted, skip remaining players
                break;
            }
            
            // Extract position
            kx::ReClass::ChCliCharacter& nonConstCharacter = const_cast<kx::ReClass::ChCliCharacter&>(character);
            kx::ReClass::AgChar agent = nonConstCharacter.GetAgent();
            if (!agent.data()) continue;
            
            kx::ReClass::CoChar coChar = agent.GetCoChar();
            if (!coChar.data()) continue;
            
            glm::vec3 gameWorldPos = coChar.GetVisualPosition();
            if (gameWorldPos.x == 0.0f && gameWorldPos.y == 0.0f && gameWorldPos.z == 0.0f) continue;

            renderablePlayer->position = glm::vec3(gameWorldPos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR, 
                                                 gameWorldPos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR, 
                                                 gameWorldPos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR);
            renderablePlayer->isValid = true;
            renderablePlayer->address = const_cast<void*>(character.data());

            // Extract player name
            auto playerNameIt = characterNameToPlayerName.find(characterDataPtr);
            if (playerNameIt != characterNameToPlayerName.end() && playerNameIt->second) {
                renderablePlayer->playerName = ESPFormatting::WStringToString(playerNameIt->second);
            }

            // Check if this is the local player
            void* localPlayer = AddressManager::GetLocalPlayer();
            renderablePlayer->isLocalPlayer = (characterDataPtr == localPlayer);

            // Extract health
            kx::ReClass::ChCliHealth health = nonConstCharacter.GetHealth();
            if (health.data()) {
                renderablePlayer->currentHealth = health.GetCurrent();
                renderablePlayer->maxHealth = health.GetMax();
            }

            // Extract energy
            kx::ReClass::ChCliEnergies energies = nonConstCharacter.GetEnergies();
            if (energies.data()) {
                renderablePlayer->currentEnergy = energies.GetCurrent();
                renderablePlayer->maxEnergy = energies.GetMax();
            }

            // Extract core stats with type-safe enum assignments
            kx::ReClass::ChCliCoreStats coreStats = nonConstCharacter.GetCoreStats();
            if (coreStats.data()) {
                renderablePlayer->level = coreStats.GetLevel();
                // Direct enum assignment instead of casting to uint32_t
                renderablePlayer->profession = coreStats.GetProfession();
                renderablePlayer->attitude = nonConstCharacter.GetAttitude();
                renderablePlayer->race = coreStats.GetRace();
            }

            players.push_back(renderablePlayer);
		}
	}
}

void ESPDataExtractor::ExtractNpcData(ObjectPool<RenderableNpc>& npcPool,
                                     std::vector<RenderableNpc*>& npcs) {
    npcs.clear();
    npcs.reserve(ExtractionCapacity::NPCS_RESERVE); // Reserve reasonable space

    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection || !kx::SafeAccess::IsMemorySafe(pContextCollection)) return;
    
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
            // Get an object from the pool instead of creating a new one
            RenderableNpc* renderableNpc = npcPool.Get();
            if (!renderableNpc) {
                // Pool exhausted, skip remaining NPCs
                break;
            }
            
            // Extract position
            kx::ReClass::ChCliCharacter& nonConstCharacter = const_cast<kx::ReClass::ChCliCharacter&>(character);
            kx::ReClass::AgChar agent = nonConstCharacter.GetAgent();
            if (!agent.data()) continue;
            
            kx::ReClass::CoChar coChar = agent.GetCoChar();
            if (!coChar.data()) continue;
            
            glm::vec3 gameWorldPos = coChar.GetVisualPosition();
            if (gameWorldPos.x == 0.0f && gameWorldPos.y == 0.0f && gameWorldPos.z == 0.0f) continue;
            
            renderableNpc->position = glm::vec3(gameWorldPos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR, 
                                              gameWorldPos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR, 
                                              gameWorldPos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR);
            renderableNpc->isValid = true;
        	renderableNpc->address = const_cast<void*>(character.data());
            
            // Extract health
            kx::ReClass::ChCliHealth health = nonConstCharacter.GetHealth();
            if (health.data()) {
                renderableNpc->currentHealth = health.GetCurrent();
                renderableNpc->maxHealth = health.GetMax();
            }
            
            // Extract basic stats with type-safe enum assignment
            kx::ReClass::ChCliCoreStats coreStats = nonConstCharacter.GetCoreStats();
            if (coreStats.data()) {
                renderableNpc->level = coreStats.GetLevel();
            }
            // Direct enum assignment instead of casting to uint32_t
            renderableNpc->attitude = nonConstCharacter.GetAttitude();
            renderableNpc->rank = nonConstCharacter.GetRank();
            
            npcs.push_back(renderableNpc);
		}
	}
}

void ESPDataExtractor::ExtractGadgetData(ObjectPool<RenderableGadget>& gadgetPool,
                                        std::vector<RenderableGadget*>& gadgets) {
    gadgets.clear();
    gadgets.reserve(ExtractionCapacity::GADGETS_RESERVE); // Reserve reasonable space

    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection || !kx::SafeAccess::IsMemorySafe(pContextCollection)) return;
    
    kx::ReClass::ContextCollection ctxCollection(pContextCollection);
    kx::ReClass::GdCliContext gadgetCtx = ctxCollection.GetGdCliContext();
    if (!gadgetCtx.data()) return;
    
    // Safely iterate through gadget list
    kx::SafeAccess::GadgetList gadgetList(gadgetCtx);
    for (const auto& gadget : gadgetList) {
        // Get an object from the pool instead of creating a new one
        RenderableGadget* renderableGadget = gadgetPool.Get();
        if (!renderableGadget) {
            // Pool exhausted, skip remaining gadgets
            break;
        }
        
        // Extract position
        kx::ReClass::GdCliGadget& nonConstGadget = const_cast<kx::ReClass::GdCliGadget&>(gadget);
        kx::ReClass::AgKeyFramed agKeyFramed = nonConstGadget.GetAgKeyFramed();
        kx::ReClass::CoKeyFramed coKeyFramed = agKeyFramed.GetCoKeyFramed();
        if (!coKeyFramed.data()) continue;
        
        glm::vec3 gameWorldPos = coKeyFramed.GetPosition();
        if (gameWorldPos.x == 0.0f && gameWorldPos.y == 0.0f && gameWorldPos.z == 0.0f) continue;
        
        renderableGadget->position = glm::vec3(gameWorldPos.x / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR, 
                                             gameWorldPos.z / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR, 
                                             gameWorldPos.y / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR);
        renderableGadget->isValid = true;
        renderableGadget->address = const_cast<void*>(gadget.data());

        // Extract type with type-safe enum assignment
        renderableGadget->type = nonConstGadget.GetGadgetType();
        
        // Extract gatherable status
        renderableGadget->isGatherable = nonConstGadget.IsGatherable();

        gadgets.push_back(renderableGadget);
    }
}

} // namespace kx