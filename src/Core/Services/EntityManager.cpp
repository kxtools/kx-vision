#include "EntityManager.h"
#include "../../Game/Extraction/DataExtractor.h"
#include <algorithm>

namespace kx {

EntityManager::EntityManager() {
    for (auto& slot : m_slots) slot = std::make_shared<SnapshotSlot>();
}

void EntityManager::Update(uint64_t now) {
    if (!ShouldExtract(now)) {
        return;
    }

    std::shared_ptr<SnapshotSlot> writeSlot = FindWritableSlot();
    // Guardrail: never recycle storage visible to render/automation. Dropping one
    // extraction is safer than blocking game thread or allocating a fourth slot.
    if (!writeSlot) {
        m_lastGameDataUpdateMs = now;
        return;
    }

    if (ExtractInto(*writeSlot)) {
        UpdateCombatObservations(*writeSlot, now);
        PublishSnapshot(writeSlot, now);
    }

    m_lastGameDataUpdateMs = now;
}

bool EntityManager::ShouldExtract(uint64_t now) const {
    const float updateRateHz = m_requestedUpdateRateHz.load(std::memory_order_relaxed);
    const uint64_t updateIntervalMs = static_cast<uint64_t>(1000.0f / (std::max)(1.0f, updateRateHz));
    // Guardrail: keep throttle math in monotonic integer milliseconds. Converting the
    // large tick epoch to float seconds loses precision and eventually changes cadence.
    return now - m_lastGameDataUpdateMs >= updateIntervalMs;
}

std::shared_ptr<EntityManager::SnapshotSlot> EntityManager::FindWritableSlot() {
    for (auto& slot : m_slots) {
        if (slot.use_count() == 1) {
            return slot;
        }
    }
    return {};
}

bool EntityManager::ExtractInto(SnapshotSlot& slot) {
    slot.ResetStorage();
    m_charToNameMap.clear();
    return DataExtractor::ExtractFrameData(
        slot.players, slot.npcs, slot.gadgets,
        slot.attackTargets, slot.items, slot.frameData,
        m_charToNameMap
    );
}

void EntityManager::UpdateCombatObservations(SnapshotSlot& slot, uint64_t now) {
    const auto& data = slot.frameData;
    const size_t totalCount = data.players.size() + data.npcs.size() + data.gadgets.size() +
        data.attackTargets.size() + data.items.size();

    m_activeCombatKeys.clear();
    m_activeCombatKeys.reserve(totalCount);

    const auto collectKeys = [&](const auto& collection) {
        for (const auto* entity : collection) {
            const CombatObservationKey key = entity->GetCombatKey();
            if (key.IsValid()) {
                m_activeCombatKeys.insert(key);
            }
        }
    };

    collectKeys(data.players);
    collectKeys(data.npcs);
    collectKeys(data.gadgets);
    collectKeys(data.attackTargets);
    collectKeys(data.items);

    m_combatObservationTracker.Prune(m_activeCombatKeys);

    m_allEntitiesBuffer.clear();
    if (m_allEntitiesBuffer.capacity() < totalCount) {
        m_allEntitiesBuffer.reserve(totalCount);
    }

    m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), data.players.begin(), data.players.end());
    m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), data.npcs.begin(), data.npcs.end());
    m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), data.gadgets.begin(), data.gadgets.end());
    m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), data.attackTargets.begin(), data.attackTargets.end());
    m_allEntitiesBuffer.insert(m_allEntitiesBuffer.end(), data.items.begin(), data.items.end());

    m_combatObservationTracker.Update(m_allEntitiesBuffer, now);
}

void EntityManager::PublishSnapshot(const std::shared_ptr<SnapshotSlot>& slot, uint64_t now) {
    // Guardrail: finish every field before release-store. Readers treat publication as
    // immutable and perform no second readiness check.
    m_combatObservationTracker.CopyObservationsTo(slot->combatObservations);
    slot->capturedAtMs = now;

    FrameSnapshotHandle published(slot, static_cast<const EntityFrameSnapshot*>(slot.get()));
    m_publishedSnapshot.store(std::move(published), std::memory_order_release);
}

void EntityManager::Reset() {
    m_publishedSnapshot.store({}, std::memory_order_release);
    for (auto& slot : m_slots) if (slot.use_count() == 1) slot->ResetStorage();
    // Guardrail: agent IDs can be reused across maps. Keeping combat state here
    // would attach stale health/death/trail effects to unrelated new entities.
    m_combatObservationTracker.Clear();
    m_activeCombatKeys.clear();
    m_allEntitiesBuffer.clear();
    m_lastGameDataUpdateMs = 0;
}

} // namespace kx
