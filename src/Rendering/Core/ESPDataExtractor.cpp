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
                
                // Initialize on first encounter
                if (persistent.address == nullptr) {
                    persistent.address = charPtr;
                }
                
                // 1. SHIFT HISTORY: The most recent data from last tick becomes the "previous" data for this tick.
                persistent.previousPosition = persistent.currentPosition;
                persistent.previousUpdateTime = persistent.lastUpdateTime;
                
                // Get pooled object for this frame's rendering
                RenderablePlayer* renderablePlayer = playerPool.Get();
                if (!renderablePlayer) continue; // Pool exhausted, skip this entity

                // Extract fresh data from game memory
                if (EntityExtractor::ExtractPlayer(*renderablePlayer, character, it->second, localPlayerPtr)) {
                    // 2. UPDATE PERSISTENT: Store the brand new data.
                    persistent.currentPosition = renderablePlayer->position;
                    persistent.lastUpdateTime = currentTime;
                    
                    // 3. HANDLE SPAWN: Prevent lerping from (0,0,0) on the first frame an entity is seen.
                    if (persistent.previousUpdateTime == 0.0) {
                        persistent.previousPosition = persistent.currentPosition;
                        persistent.previousUpdateTime = persistent.lastUpdateTime;
                    }
                    
                    // 4. CALCULATE ADAPTIVE SMOOTHED VELOCITY: (distance / time)
                    double deltaTime = persistent.lastUpdateTime - persistent.previousUpdateTime;
                    if (deltaTime > 0.001) { // Avoid division by zero
                        glm::vec3 instantVelocity = (persistent.currentPosition - persistent.previousPosition) / static_cast<float>(deltaTime);
                        
                        if (glm::length(persistent.smoothedVelocity) < 0.001f) {
                            // If there's no previous velocity, start with the new one.
                            persistent.smoothedVelocity = instantVelocity;
                        } else {
                            // --- ADAPTIVE SMOOTHING LOGIC ---
                            // Compare the direction of the new velocity with the old smoothed velocity.
                            glm::vec3 normInstant = glm::normalize(instantVelocity);
                            glm::vec3 normSmoothed = glm::normalize(persistent.smoothedVelocity);
                            float directionSimilarity = glm::dot(normInstant, normSmoothed);

                            // Remap the dot product [-1, 1] to a [0, 1] range.
                            // 1.0 = same direction, 0.0 = opposite direction.
                            float responsiveness = (1.0f - directionSimilarity) / 2.0f; // Inverted: 0 for same dir, 1 for opposite

                            // Mix between min and max smoothing factors based on responsiveness.
                            // If direction is the same, use MIN factor. If opposite, use MAX factor.
                            float dynamicSmoothingFactor = glm::mix(
                                RenderingEffects::MIN_VELOCITY_SMOOTHING_FACTOR,
                                RenderingEffects::MAX_VELOCITY_SMOOTHING_FACTOR,
                                responsiveness
                            );

                            // Apply the dynamic smoothing factor to the EMA.
                            persistent.smoothedVelocity = glm::mix(
                                persistent.smoothedVelocity,
                                instantVelocity,
                                dynamicSmoothingFactor
                            );
                        }
                    }
                    
                    // 5. COPY ALL DATA to the frame object for this frame's render pass.
                    renderablePlayer->currentPosition = persistent.currentPosition;
                    renderablePlayer->previousPosition = persistent.previousPosition;
                    renderablePlayer->smoothedVelocity = persistent.smoothedVelocity;
                    renderablePlayer->lastUpdateTime = persistent.lastUpdateTime;
                    renderablePlayer->previousUpdateTime = persistent.previousUpdateTime;
                    
                    players.push_back(renderablePlayer);
                }
            } else {
                // This is an NPC - get or create persistent state
                RenderableNpc& persistent = persistentNpcs[charPtr];
                
                // Initialize on first encounter
                if (persistent.address == nullptr) {
                    persistent.address = charPtr;
                }
                
                // 1. SHIFT HISTORY: The most recent data from last tick becomes the "previous" data for this tick.
                persistent.previousPosition = persistent.currentPosition;
                persistent.previousUpdateTime = persistent.lastUpdateTime;
                
                // Get pooled object for this frame's rendering
                RenderableNpc* renderableNpc = npcPool.Get();
                if (!renderableNpc) continue; // Pool exhausted, skip this entity

                // Extract fresh data from game memory
                if (EntityExtractor::ExtractNpc(*renderableNpc, character)) {
                    // 2. UPDATE PERSISTENT: Store the brand new data.
                    persistent.currentPosition = renderableNpc->position;
                    persistent.lastUpdateTime = currentTime;
                    
                    // 3. HANDLE SPAWN: Prevent lerping from (0,0,0) on the first frame an entity is seen.
                    if (persistent.previousUpdateTime == 0.0) {
                        persistent.previousPosition = persistent.currentPosition;
                        persistent.previousUpdateTime = persistent.lastUpdateTime;
                    }
                    
                    // 4. CALCULATE ADAPTIVE SMOOTHED VELOCITY: (distance / time)
                    double deltaTime = persistent.lastUpdateTime - persistent.previousUpdateTime;
                    if (deltaTime > 0.001) { // Avoid division by zero
                        glm::vec3 instantVelocity = (persistent.currentPosition - persistent.previousPosition) / static_cast<float>(deltaTime);
                        
                        if (glm::length(persistent.smoothedVelocity) < 0.001f) {
                            // If there's no previous velocity, start with the new one.
                            persistent.smoothedVelocity = instantVelocity;
                        } else {
                            // --- ADAPTIVE SMOOTHING LOGIC ---
                            // Compare the direction of the new velocity with the old smoothed velocity.
                            glm::vec3 normInstant = glm::normalize(instantVelocity);
                            glm::vec3 normSmoothed = glm::normalize(persistent.smoothedVelocity);
                            float directionSimilarity = glm::dot(normInstant, normSmoothed);

                            // Remap the dot product [-1, 1] to a [0, 1] range.
                            // 1.0 = same direction, 0.0 = opposite direction.
                            float responsiveness = (1.0f - directionSimilarity) / 2.0f; // Inverted: 0 for same dir, 1 for opposite

                            // Mix between min and max smoothing factors based on responsiveness.
                            // If direction is the same, use MIN factor. If opposite, use MAX factor.
                            float dynamicSmoothingFactor = glm::mix(
                                RenderingEffects::MIN_VELOCITY_SMOOTHING_FACTOR,
                                RenderingEffects::MAX_VELOCITY_SMOOTHING_FACTOR,
                                responsiveness
                            );

                            // Apply the dynamic smoothing factor to the EMA.
                            persistent.smoothedVelocity = glm::mix(
                                persistent.smoothedVelocity,
                                instantVelocity,
                                dynamicSmoothingFactor
                            );
                        }
                    }
                    
                    // 5. COPY ALL DATA to the frame object for this frame's render pass.
                    renderableNpc->currentPosition = persistent.currentPosition;
                    renderableNpc->previousPosition = persistent.previousPosition;
                    renderableNpc->smoothedVelocity = persistent.smoothedVelocity;
                    renderableNpc->lastUpdateTime = persistent.lastUpdateTime;
                    renderableNpc->previousUpdateTime = persistent.previousUpdateTime;
                    
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
            
            // Initialize on first encounter
            if (persistent.address == nullptr) {
                persistent.address = gadgetPtr;
            }
            
            // 1. SHIFT HISTORY: The most recent data from last tick becomes the "previous" data for this tick.
            persistent.previousPosition = persistent.currentPosition;
            persistent.previousUpdateTime = persistent.lastUpdateTime;
            
            // Get pooled object for this frame's rendering
            RenderableGadget* renderableGadget = gadgetPool.Get();
            if (!renderableGadget) break; // Pool exhausted

            // Extract fresh data from game memory
            if (EntityExtractor::ExtractGadget(*renderableGadget, gadget)) {
                // 2. UPDATE PERSISTENT: Store the brand new data.
                persistent.currentPosition = renderableGadget->position;
                persistent.lastUpdateTime = currentTime;
                
                // 3. HANDLE SPAWN: Prevent lerping from (0,0,0) on the first frame an entity is seen.
                if (persistent.previousUpdateTime == 0.0) {
                    persistent.previousPosition = persistent.currentPosition;
                    persistent.previousUpdateTime = persistent.lastUpdateTime;
                }
                
                // 4. CALCULATE ADAPTIVE SMOOTHED VELOCITY: (distance / time)
                double deltaTime = persistent.lastUpdateTime - persistent.previousUpdateTime;
                if (deltaTime > 0.001) { // Avoid division by zero
                    glm::vec3 instantVelocity = (persistent.currentPosition - persistent.previousPosition) / static_cast<float>(deltaTime);
                    
                    if (glm::length(persistent.smoothedVelocity) < 0.001f) {
                        // If there's no previous velocity, start with the new one.
                        persistent.smoothedVelocity = instantVelocity;
                    } else {
                        // --- ADAPTIVE SMOOTHING LOGIC ---
                        // Compare the direction of the new velocity with the old smoothed velocity.
                        glm::vec3 normInstant = glm::normalize(instantVelocity);
                        glm::vec3 normSmoothed = glm::normalize(persistent.smoothedVelocity);
                        float directionSimilarity = glm::dot(normInstant, normSmoothed);

                        // Remap the dot product [-1, 1] to a [0, 1] range.
                        // 1.0 = same direction, 0.0 = opposite direction.
                        float responsiveness = (1.0f - directionSimilarity) / 2.0f; // Inverted: 0 for same dir, 1 for opposite

                        // Mix between min and max smoothing factors based on responsiveness.
                        // If direction is the same, use MIN factor. If opposite, use MAX factor.
                        float dynamicSmoothingFactor = glm::mix(
                            RenderingEffects::MIN_VELOCITY_SMOOTHING_FACTOR,
                            RenderingEffects::MAX_VELOCITY_SMOOTHING_FACTOR,
                            responsiveness
                        );

                        // Apply the dynamic smoothing factor to the EMA.
                        persistent.smoothedVelocity = glm::mix(
                            persistent.smoothedVelocity,
                            instantVelocity,
                            dynamicSmoothingFactor
                        );
                    }
                }
                
                // 5. COPY ALL DATA to the frame object for this frame's render pass.
                renderableGadget->currentPosition = persistent.currentPosition;
                renderableGadget->previousPosition = persistent.previousPosition;
                renderableGadget->smoothedVelocity = persistent.smoothedVelocity;
                renderableGadget->lastUpdateTime = persistent.lastUpdateTime;
                renderableGadget->previousUpdateTime = persistent.previousUpdateTime;
                
                gadgets.push_back(renderableGadget);
            }
        }
    }

} // namespace kx