#include "DataExtractor.h"
#include <ankerl/unordered_dense.h>
#include "../../Memory/AddressManager.h"
#include "../../Game/SdkStructs.h"
#include "../../Memory/SafeIterators.h"
#include "../../Memory/MemorySafety.h"
#include "Extraction/EntityExtractor.h"

namespace kx {

    void DataExtractor::ExtractFrameData(ObjectPool<RenderablePlayer>& playerPool,
        ObjectPool<RenderableNpc>& npcPool,
        ObjectPool<RenderableGadget>& gadgetPool,
        ObjectPool<RenderableAttackTarget>& attackTargetPool,
        ObjectPool<RenderableItem>& itemPool,
        PooledFrameRenderData& pooledData) {
        pooledData.Reset();

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection || !SafeAccess::IsMemorySafe(pContextCollection)) {
            return;
        }

        // Build the map of character pointers to player names
        ankerl::unordered_dense::map<void*, const wchar_t*> characterToPlayerNameMap;
        characterToPlayerNameMap.reserve(150);
        {
            ReClass::ContextCollection ctxCollection(pContextCollection);
            ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
            if (charContext.data()) {
                auto playerList = charContext.GetPlayers();
                for (const auto& player : playerList) {
                    auto character = player.GetCharacter();
                    if (character.data()) {
                        characterToPlayerNameMap[character.data()] = player.GetName();
                    }
                }
            }
        }

        // Single pass extraction for both players and NPCs
        ExtractCharacterData(playerPool, npcPool, pooledData.players, pooledData.npcs, characterToPlayerNameMap);
        ExtractGadgetData(gadgetPool, pooledData.gadgets);
        ExtractAttackTargetData(attackTargetPool, pooledData.attackTargets);
        ExtractItemData(itemPool, pooledData.items);
    }

    void DataExtractor::ExtractCharacterData(ObjectPool<RenderablePlayer>& playerPool,
        ObjectPool<RenderableNpc>& npcPool,
        std::vector<RenderablePlayer*>& players,
        std::vector<RenderableNpc*>& npcs,
        const ankerl::unordered_dense::map<void*, const wchar_t*>& characterToPlayerNameMap) {
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
        auto characterList = charContext.GetCharacters();
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

    void DataExtractor::ExtractGadgetData(ObjectPool<RenderableGadget>& gadgetPool,
        std::vector<RenderableGadget*>& gadgets) {
        gadgets.clear();
        gadgets.reserve(ExtractionCapacity::GADGETS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        auto gadgetList = gadgetContext.GetGadgets();
        for (const auto& gadget : gadgetList) {
            RenderableGadget* renderableGadget = gadgetPool.Get();
            if (!renderableGadget) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            if (EntityExtractor::ExtractGadget(*renderableGadget, gadget)) {
                gadgets.push_back(renderableGadget);
            }
        }
    }

    void DataExtractor::ExtractAttackTargetData(ObjectPool<RenderableAttackTarget>& attackTargetPool,
        std::vector<RenderableAttackTarget*>& attackTargets) {
        attackTargets.clear();
        attackTargets.reserve(ExtractionCapacity::ATTACK_TARGETS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        auto attackTargetList = gadgetContext.GetAttackTargets();
        for (const auto& agentInl : attackTargetList) {
            RenderableAttackTarget* renderableAttackTarget = attackTargetPool.Get();
            if (!renderableAttackTarget) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            if (EntityExtractor::ExtractAttackTarget(*renderableAttackTarget, agentInl)) {
                attackTargets.push_back(renderableAttackTarget);
            }
        }
    }

    void DataExtractor::ExtractItemData(ObjectPool<RenderableItem>& itemPool,
        std::vector<RenderableItem*>& items) {
        items.clear();
        items.reserve(ExtractionCapacity::ITEMS_RESERVE);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::ItCliContext itemContext = ctxCollection.GetItCliContext();
        if (!itemContext.data()) return;

        auto itemList = itemContext.GetItems();
        for (const auto& item : itemList) {
            RenderableItem* renderableItem = itemPool.Get();
            if (!renderableItem) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            if (EntityExtractor::ExtractItem(*renderableItem, item)) {
                items.push_back(renderableItem);
            }
        }
    }

} // namespace kx