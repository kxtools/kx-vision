#include "ESPDataExtractor.h"
#include "EntityExtractor.h"
#include "../../Game/AddressManager.h"
#include "../../Game/ReClassStructs.h"
#include "../../Utils/SafeIterators.h"
#include "../../Utils/MemorySafety.h"
#include "../Utils/ESPConstants.h"

namespace kx {

    void ESPDataExtractor::ExtractFrameData(ObjectPool<RenderablePlayer>& playerPool,
        ObjectPool<RenderableNpc>& npcPool,
        ObjectPool<RenderableGadget>& gadgetPool,
        PooledFrameRenderData& pooledData) {
        pooledData.Reset();

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection || !SafeAccess::IsMemorySafe(pContextCollection)) {
            return;
        }

        // Build the map of character pointers to player names
        std::unordered_map<void*, const wchar_t*> characterToPlayerNameMap;
        {
            ReClass::ContextCollection ctxCollection(pContextCollection);
            ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
            if (charContext.data()) {
                SafeAccess::PlayerList playerList(charContext);
                for (auto playerIt = playerList.begin(); playerIt != playerList.end(); ++playerIt) {
                    if (playerIt.IsValid()) {
                        characterToPlayerNameMap[playerIt.GetCharacterDataPtr()] = playerIt.GetName();
                    }
                }
            }
        }

        // Single pass extraction for both players and NPCs
        ExtractCharacterData(playerPool, npcPool, pooledData.players, pooledData.npcs, characterToPlayerNameMap);
        ExtractGadgetData(gadgetPool, pooledData.gadgets);
    }

    void ESPDataExtractor::ExtractCharacterData(ObjectPool<RenderablePlayer>& playerPool,
        ObjectPool<RenderableNpc>& npcPool,
        std::vector<RenderablePlayer*>& players,
        std::vector<RenderableNpc*>& npcs,
        const std::unordered_map<void*, const wchar_t*>& characterToPlayerNameMap) {
        players.clear();
        npcs.clear();
        players.reserve(ExtractionCapacity::PLAYERS_RESERVE);
        npcs.reserve(ExtractionCapacity::NPCS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        if (!charContext.data()) return;

        void* localPlayerPtr = AddressManager::GetLocalPlayer();

        // Single pass over the character list - process both players and NPCs
        SafeAccess::CharacterList characterList(charContext);
        for (const auto& character : characterList) {
            void* charPtr = const_cast<void*>(character.data());
            
            // Check if this character is a player
            auto it = characterToPlayerNameMap.find(charPtr);
            if (it != characterToPlayerNameMap.end()) {
                // This is a player
                RenderablePlayer* renderablePlayer = playerPool.Get();
                if (!renderablePlayer) continue; // Pool exhausted, skip this entity

                // Delegate all extraction logic to the helper class
                if (EntityExtractor::ExtractPlayer(*renderablePlayer, character, it->second, localPlayerPtr)) {
                    players.push_back(renderablePlayer);
                }
            } else {
                // This is an NPC
                RenderableNpc* renderableNpc = npcPool.Get();
                if (!renderableNpc) continue; // Pool exhausted, skip this entity

                // Delegate all extraction logic to the helper class
                if (EntityExtractor::ExtractNpc(*renderableNpc, character)) {
                    npcs.push_back(renderableNpc);
                }
            }
        }
    }

    void ESPDataExtractor::ExtractGadgetData(ObjectPool<RenderableGadget>& gadgetPool,
        std::vector<RenderableGadget*>& gadgets) {
        gadgets.clear();
        gadgets.reserve(ExtractionCapacity::GADGETS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        SafeAccess::GadgetList gadgetList(gadgetContext);
        for (const auto& gadget : gadgetList) {
            RenderableGadget* renderableGadget = gadgetPool.Get();
            if (!renderableGadget) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            if (EntityExtractor::ExtractGadget(*renderableGadget, gadget)) {
                gadgets.push_back(renderableGadget);
            }
        }
    }

} // namespace kx