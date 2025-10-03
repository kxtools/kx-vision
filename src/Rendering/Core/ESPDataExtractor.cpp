#include "ESPDataExtractor.h"
#include "ESPRenderer.h"
#include "../Extractors/EntityExtractor.h"
#include "../../Game/AddressManager.h"
#include "../../Game/ReClassStructs.h"
#include "../../Utils/SafeIterators.h"
#include "../../Utils/MemorySafety.h"
#include "../Utils/ESPConstants.h"
#include <Windows.h>

namespace kx {

    void ESPDataExtractor::ExtractFrameData(ObjectPool<RenderablePlayer>& playerPool,
        ObjectPool<RenderableNpc>& npcPool,
        ObjectPool<RenderableGadget>& gadgetPool,
        PooledFrameRenderData& pooledData) {
        pooledData.Reset();

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection || !kx::SafeAccess::IsMemorySafe(pContextCollection)) {
            return;
        }

        // Build the map of character pointers to player names
        std::unordered_map<void*, const wchar_t*> characterToPlayerNameMap;
        {
            kx::ReClass::ContextCollection ctxCollection(pContextCollection);
            kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
            if (charContext.data()) {
                kx::SafeAccess::PlayerList playerList(charContext);
                for (auto playerIt = playerList.begin(); playerIt != playerList.end(); ++playerIt) {
                    if (playerIt.IsValid()) {
                        characterToPlayerNameMap[playerIt.GetCharacterDataPtr()] = playerIt.GetName();
                    }
                }
            }
        }

        // Get persistent maps from ESPRenderer for interpolation
        auto& persistentPlayers = ESPRenderer::GetPlayerData();
        auto& persistentNpcs = ESPRenderer::GetNpcData();
        auto& persistentGadgets = ESPRenderer::GetGadgetData();

        // Single pass extraction for both players and NPCs (with persistent state)
        ExtractCharacterData(playerPool, npcPool, pooledData.players, pooledData.npcs, 
                           characterToPlayerNameMap, persistentPlayers, persistentNpcs);
        ExtractGadgetData(gadgetPool, pooledData.gadgets, persistentGadgets);
    }

    void ESPDataExtractor::ExtractCharacterData(ObjectPool<RenderablePlayer>& playerPool,
        ObjectPool<RenderableNpc>& npcPool,
        std::vector<RenderablePlayer*>& players,
        std::vector<RenderableNpc*>& npcs,
        const std::unordered_map<void*, const wchar_t*>& characterToPlayerNameMap,
        std::unordered_map<const void*, RenderablePlayer>& persistentPlayers,
        std::unordered_map<const void*, RenderableNpc>& persistentNpcs) {
        players.clear();
        npcs.clear();
        players.reserve(ExtractionCapacity::PLAYERS_RESERVE);
        npcs.reserve(ExtractionCapacity::NPCS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        if (!charContext.data()) return;

        void* localPlayerPtr = AddressManager::GetLocalPlayer();
        double currentTime = GetTickCount64() / 1000.0; // Current time in seconds

        // Single pass over the character list - process both players and NPCs
        kx::SafeAccess::CharacterList characterList(charContext);
        for (const auto& character : characterList) {
            void* charPtr = const_cast<void*>(character.data());
            
            // Check if this character is a player
            auto it = characterToPlayerNameMap.find(charPtr);
            if (it != characterToPlayerNameMap.end()) {
                // This is a player - get or create persistent state
                RenderablePlayer& persistent = persistentPlayers[charPtr];
                
                // Store previous position before updating
                if (persistent.address != nullptr) {
                    persistent.previousPosition = persistent.currentPosition;
                } else {
                    // First time seeing this entity - initialize
                    persistent.address = charPtr;
                }
                
                // Get pooled object for this frame's rendering
                RenderablePlayer* renderablePlayer = playerPool.Get();
                if (!renderablePlayer) continue; // Pool exhausted, skip this entity

                // Extract fresh data from game memory
                if (EntityExtractor::ExtractPlayer(*renderablePlayer, character, it->second, localPlayerPtr)) {
                    // Update persistent state with new position
                    persistent.currentPosition = renderablePlayer->position;
                    persistent.lastUpdateTime = currentTime;
                    
                    // Initialize previousPosition on first spawn to prevent jumping
                    if (persistent.previousPosition == glm::vec3(0.0f)) {
                        persistent.previousPosition = persistent.currentPosition;
                    }
                    
                    // Calculate and smooth velocity using exponential moving average
                    glm::vec3 instantVelocity = persistent.currentPosition - persistent.previousPosition;
                    if (persistent.smoothedVelocity == glm::vec3(0.0f)) {
                        // First velocity calculation - use instant velocity
                        persistent.smoothedVelocity = instantVelocity;
                    } else {
                        // Smooth velocity: EMA = α * new + (1-α) * old
                        persistent.smoothedVelocity = RenderingEffects::VELOCITY_SMOOTHING_FACTOR * instantVelocity +
                                                     (1.0f - RenderingEffects::VELOCITY_SMOOTHING_FACTOR) * persistent.smoothedVelocity;
                    }
                    
                    // Copy persistent interpolation data to frame object
                    renderablePlayer->currentPosition = persistent.currentPosition;
                    renderablePlayer->previousPosition = persistent.previousPosition;
                    renderablePlayer->smoothedVelocity = persistent.smoothedVelocity;
                    renderablePlayer->lastUpdateTime = persistent.lastUpdateTime;
                    
                    players.push_back(renderablePlayer);
                }
            } else {
                // This is an NPC - get or create persistent state
                RenderableNpc& persistent = persistentNpcs[charPtr];
                
                // Store previous position before updating
                if (persistent.address != nullptr) {
                    persistent.previousPosition = persistent.currentPosition;
                } else {
                    // First time seeing this entity - initialize
                    persistent.address = charPtr;
                }
                
                // Get pooled object for this frame's rendering
                RenderableNpc* renderableNpc = npcPool.Get();
                if (!renderableNpc) continue; // Pool exhausted, skip this entity

                // Extract fresh data from game memory
                if (EntityExtractor::ExtractNpc(*renderableNpc, character)) {
                    // Update persistent state with new position
                    persistent.currentPosition = renderableNpc->position;
                    persistent.lastUpdateTime = currentTime;
                    
                    // Initialize previousPosition on first spawn to prevent jumping
                    if (persistent.previousPosition == glm::vec3(0.0f)) {
                        persistent.previousPosition = persistent.currentPosition;
                    }
                    
                    // Calculate and smooth velocity using exponential moving average
                    glm::vec3 instantVelocity = persistent.currentPosition - persistent.previousPosition;
                    if (persistent.smoothedVelocity == glm::vec3(0.0f)) {
                        // First velocity calculation - use instant velocity
                        persistent.smoothedVelocity = instantVelocity;
                    } else {
                        // Smooth velocity: EMA = α * new + (1-α) * old
                        persistent.smoothedVelocity = RenderingEffects::VELOCITY_SMOOTHING_FACTOR * instantVelocity +
                                                     (1.0f - RenderingEffects::VELOCITY_SMOOTHING_FACTOR) * persistent.smoothedVelocity;
                    }
                    
                    // Copy persistent interpolation data to frame object
                    renderableNpc->currentPosition = persistent.currentPosition;
                    renderableNpc->previousPosition = persistent.previousPosition;
                    renderableNpc->smoothedVelocity = persistent.smoothedVelocity;
                    renderableNpc->lastUpdateTime = persistent.lastUpdateTime;
                    
                    npcs.push_back(renderableNpc);
                }
            }
        }
    }

    void ESPDataExtractor::ExtractGadgetData(ObjectPool<RenderableGadget>& gadgetPool,
        std::vector<RenderableGadget*>& gadgets,
        std::unordered_map<const void*, RenderableGadget>& persistentGadgets) {
        gadgets.clear();
        gadgets.reserve(ExtractionCapacity::GADGETS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        double currentTime = GetTickCount64() / 1000.0; // Current time in seconds

        kx::SafeAccess::GadgetList gadgetList(gadgetContext);
        for (const auto& gadget : gadgetList) {
            void* gadgetPtr = const_cast<void*>(gadget.data());
            
            // Get or create persistent state
            RenderableGadget& persistent = persistentGadgets[gadgetPtr];
            
            // Store previous position before updating
            if (persistent.address != nullptr) {
                persistent.previousPosition = persistent.currentPosition;
            } else {
                // First time seeing this entity - initialize
                persistent.address = gadgetPtr;
            }
            
            // Get pooled object for this frame's rendering
            RenderableGadget* renderableGadget = gadgetPool.Get();
            if (!renderableGadget) break; // Pool exhausted

            // Extract fresh data from game memory
            if (EntityExtractor::ExtractGadget(*renderableGadget, gadget)) {
                // Update persistent state with new position
                persistent.currentPosition = renderableGadget->position;
                persistent.lastUpdateTime = currentTime;
                
                // Initialize previousPosition on first spawn to prevent jumping
                if (persistent.previousPosition == glm::vec3(0.0f)) {
                    persistent.previousPosition = persistent.currentPosition;
                }
                
                // Calculate and smooth velocity using exponential moving average
                glm::vec3 instantVelocity = persistent.currentPosition - persistent.previousPosition;
                if (persistent.smoothedVelocity == glm::vec3(0.0f)) {
                    // First velocity calculation - use instant velocity
                    persistent.smoothedVelocity = instantVelocity;
                } else {
                    // Smooth velocity: EMA = α * new + (1-α) * old
                    persistent.smoothedVelocity = RenderingEffects::VELOCITY_SMOOTHING_FACTOR * instantVelocity +
                                                 (1.0f - RenderingEffects::VELOCITY_SMOOTHING_FACTOR) * persistent.smoothedVelocity;
                }
                
                // Copy persistent interpolation data to frame object
                renderableGadget->currentPosition = persistent.currentPosition;
                renderableGadget->previousPosition = persistent.previousPosition;
                renderableGadget->smoothedVelocity = persistent.smoothedVelocity;
                renderableGadget->lastUpdateTime = persistent.lastUpdateTime;
                
                gadgets.push_back(renderableGadget);
            }
        }
    }

} // namespace kx