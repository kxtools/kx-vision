#include "ESPDataExtractor.h"
#include "Extractors/EntityExtractor.h"
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

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection || !kx::SafeAccess::IsMemorySafe(pContextCollection)) {
            return;
        }

        // build the map of character pointers to player names.
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

        // The extraction calls now pass the map.
        ExtractPlayerData(playerPool, pooledData.players, characterToPlayerNameMap);
        ExtractNpcData(npcPool, pooledData.npcs, characterToPlayerNameMap); // Pass map to exclude players
        ExtractGadgetData(gadgetPool, pooledData.gadgets);
    }

    void ESPDataExtractor::ExtractPlayerData(ObjectPool<RenderablePlayer>& playerPool,
        std::vector<RenderablePlayer*>& players,
        const std::unordered_map<void*, const wchar_t*>& characterToPlayerNameMap) {
        players.clear();
        players.reserve(ExtractionCapacity::PLAYERS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        if (!charContext.data()) return;

        void* localPlayerPtr = AddressManager::GetLocalPlayer();

        kx::SafeAccess::CharacterList characterList(charContext);
        for (const auto& character : characterList) {
            // Find if this character is in our player map
            auto it = characterToPlayerNameMap.find(const_cast<void*>(character.data()));
            if (it != characterToPlayerNameMap.end()) {
                RenderablePlayer* renderablePlayer = playerPool.Get();
                if (!renderablePlayer) break; // Pool exhausted

                // Delegate all extraction logic to the helper class
                if (EntityExtractor::ExtractPlayer(*renderablePlayer, character, it->second, localPlayerPtr)) {
                    players.push_back(renderablePlayer);
                }
            }
        }
    }

    void ESPDataExtractor::ExtractNpcData(ObjectPool<RenderableNpc>& npcPool,
        std::vector<RenderableNpc*>& npcs,
        const std::unordered_map<void*, const wchar_t*>& characterToPlayerNameMap) {
        npcs.clear();
        npcs.reserve(ExtractionCapacity::NPCS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
        if (!charContext.data()) return;

        kx::SafeAccess::CharacterList characterList(charContext);
        for (const auto& character : characterList) {
            // If the character is NOT in the player map, it's an NPC
            if (characterToPlayerNameMap.find(const_cast<void*>(character.data())) == characterToPlayerNameMap.end()) {
                RenderableNpc* renderableNpc = npcPool.Get();
                if (!renderableNpc) break; // Pool exhausted

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

        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        kx::SafeAccess::GadgetList gadgetList(gadgetContext);
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