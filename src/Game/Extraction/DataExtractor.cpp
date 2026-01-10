#include "DataExtractor.h"
#include <ankerl/unordered_dense.h>
#include "../../Memory/AddressManager.h"
#include "../SdkStructs.h"
#include "../../Memory/SafeGameArray.h"
#include "../../Memory/Safety.h"
#include "EntityExtractor.h"
#include "../../Rendering/Shared/LayoutConstants.h"

namespace kx {

    void DataExtractor::ExtractFrameData(ObjectPool<PlayerEntity>& playerPool,
        ObjectPool<NpcEntity>& npcPool,
        ObjectPool<GadgetEntity>& gadgetPool,
        ObjectPool<AttackTargetEntity>& attackTargetPool,
        ObjectPool<ItemEntity>& itemPool,
        FrameGameData& pooledData) {
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

    void DataExtractor::ExtractCharacterData(ObjectPool<PlayerEntity>& playerPool,
        ObjectPool<NpcEntity>& npcPool,
        std::vector<PlayerEntity*>& players,
        std::vector<NpcEntity*>& npcs,
        const ankerl::unordered_dense::map<void*, const wchar_t*>& characterToPlayerNameMap) {
        players.clear();
        npcs.clear();
        players.reserve(EntityLimits::MAX_PLAYERS);
        npcs.reserve(EntityLimits::MAX_NPCS);

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
                PlayerEntity* renderablePlayer = playerPool.Get();
                if (!renderablePlayer) continue; // Pool exhausted, skip this entity

                // Delegate all extraction logic to the helper class
                if (EntityExtractor::ExtractPlayer(*renderablePlayer, character, it->second, localPlayerPtr)) {
                    players.push_back(renderablePlayer);
                }
            } else {
                // This is an NPC
                NpcEntity* renderableNpc = npcPool.Get();
                if (!renderableNpc) continue; // Pool exhausted, skip this entity

                // Delegate all extraction logic to the helper class
                if (EntityExtractor::ExtractNpc(*renderableNpc, character)) {
                    npcs.push_back(renderableNpc);
                }
            }
        }
    }

    void DataExtractor::ExtractGadgetData(ObjectPool<GadgetEntity>& gadgetPool,
        std::vector<GadgetEntity*>& gadgets) {
        gadgets.clear();
        gadgets.reserve(EntityLimits::MAX_GADGETS);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        auto gadgetList = gadgetContext.GetGadgets();
        for (const auto& gadget : gadgetList) {
            GadgetEntity* renderableGadget = gadgetPool.Get();
            if (!renderableGadget) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            if (EntityExtractor::ExtractGadget(*renderableGadget, gadget)) {
                gadgets.push_back(renderableGadget);
            }
        }
    }

    void DataExtractor::ExtractAttackTargetData(ObjectPool<AttackTargetEntity>& attackTargetPool,
        std::vector<AttackTargetEntity*>& attackTargets) {
        attackTargets.clear();
        attackTargets.reserve(EntityLimits::MAX_ATTACK_TARGETS);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::GdCliContext gadgetContext = ctxCollection.GetGdCliContext();
        if (!gadgetContext.data()) return;

        auto attackTargetList = gadgetContext.GetAttackTargets();
        for (const auto& agentInl : attackTargetList) {
            AttackTargetEntity* renderableAttackTarget = attackTargetPool.Get();
            if (!renderableAttackTarget) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            if (EntityExtractor::ExtractAttackTarget(*renderableAttackTarget, agentInl)) {
                attackTargets.push_back(renderableAttackTarget);
            }
        }
    }

    void DataExtractor::ExtractItemData(ObjectPool<ItemEntity>& itemPool,
        std::vector<ItemEntity*>& items) {
        items.clear();
        items.reserve(EntityLimits::MAX_ITEMS);

        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (!pContextCollection) return;

        ReClass::ContextCollection ctxCollection(pContextCollection);
        ReClass::ItCliContext itemContext = ctxCollection.GetItCliContext();
        if (!itemContext.data()) return;

        auto itemList = itemContext.GetItems();
        for (const auto& item : itemList) {
            // Pre-filter: Don't waste a pool slot on equipment items
            // Only process items with LocationType == Agent (ground loot)
            if (item.GetLocationType() != Game::ItemLocation::Agent) {
                continue;
            }

            ItemEntity* renderableItem = itemPool.Get();
            if (!renderableItem) break; // Pool exhausted

            // Delegate all extraction logic to the helper class
            // Note: ExtractItem also checks LocationType as a double-safety check
            if (EntityExtractor::ExtractItem(*renderableItem, item)) {
                items.push_back(renderableItem);
            }
        }
    }

} // namespace kx