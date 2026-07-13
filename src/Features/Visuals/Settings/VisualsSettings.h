#pragma once

#include "../../../Core/Settings/SettingsConstants.h"
#include "../../../Core/Settings/SettingsFields.h"
#include "VisualRenderSettings.h"

namespace kx {

#define KX_ATTITUDE_SETTINGS(X) \
    X(bool, showFriendly, true) \
    X(bool, showHostile, true) \
    X(bool, showNeutral, true) \
    X(bool, showIndifferent, true)
KX_DEFINE_SETTINGS_TYPE(AttitudeSettings, KX_ATTITUDE_SETTINGS)

#define KX_TRAIL_SETTINGS(X) \
    X(bool, enabled, false) \
    X(int, maxPoints, 30) \
    X(float, maxDuration, 1.0f) \
    X(TrailDisplayMode, displayMode, TrailDisplayMode::Hostile) \
    X(TrailTeleportMode, teleportMode, TrailTeleportMode::Tactical) \
    X(float, thickness, 2.0f)
KX_DEFINE_SETTINGS_TYPE(TrailSettings, KX_TRAIL_SETTINGS)

#define KX_PLAYER_ESP_SETTINGS(X) \
    X(bool, enabled, true) X(bool, renderBox, false) X(bool, renderWireframe, false) \
    X(bool, renderDistance, false) X(bool, renderDot, false) X(bool, renderDetails, false) \
    X(bool, renderHealthBar, true) X(bool, renderEnergyBar, false) X(bool, renderPlayerName, true) \
    X(bool, showBurstDps, false) X(bool, showDamageNumbers, true) X(bool, showOnlyDamaged, false) \
    X(bool, showHealthPercentage, false) X(bool, showLocalPlayer, false) X(bool, enableGearDisplay, false) \
    X(GearDisplayMode, gearDisplayMode, GearDisplayMode::Compact) \
    X(EnergyDisplayType, energyDisplayType, EnergyDisplayType::Energy) \
    X(bool, showDetailLevel, true) X(bool, showDetailHp, true) X(bool, showDetailAttitude, true) \
    X(bool, showDetailEnergy, true) X(bool, showDetailPosition, true) X(bool, showDetailRank, true) \
    X(bool, showDetailProfession, true) X(bool, showDetailRace, true) X(bool, showDetailPhysics, false) \
    X(float, hostileBoostMultiplier, 2.0f) X(TrailSettings, trails, {})
struct PlayerEspSettings : AttitudeSettings { KX_PLAYER_ESP_SETTINGS(KX_SETTING_DECLARE) };
inline void to_json(nlohmann::json& j, const PlayerEspSettings& value) {
    j = nlohmann::json::object(); KX_ATTITUDE_SETTINGS(KX_SETTING_SAVE) KX_PLAYER_ESP_SETTINGS(KX_SETTING_SAVE)
}
inline void from_json(const nlohmann::json& j, PlayerEspSettings& value) {
    value = PlayerEspSettings{}; if (!j.is_object()) return;
    KX_ATTITUDE_SETTINGS(KX_SETTING_LOAD) KX_PLAYER_ESP_SETTINGS(KX_SETTING_LOAD)
}

#define KX_NPC_ESP_SETTINGS(X) \
    X(bool, enabled, true) X(bool, renderBox, false) X(bool, renderWireframe, false) \
    X(bool, renderDistance, false) X(bool, renderDot, false) X(bool, renderDetails, false) \
    X(bool, renderHealthBar, true) X(bool, showBurstDps, false) X(bool, showDamageNumbers, true) \
    X(bool, showOnlyDamaged, false) X(bool, showHealthPercentage, false) \
    X(bool, showLegendary, true) X(bool, showChampion, true) X(bool, showElite, true) \
    X(bool, showVeteran, true) X(bool, showAmbient, true) X(bool, showNormal, true) X(bool, showDeadNpcs, false) \
    X(bool, showDetailLevel, true) X(bool, showDetailHp, true) X(bool, showDetailAttitude, true) \
    X(bool, showDetailRank, true) X(bool, showDetailPosition, true)
struct NpcEspSettings : AttitudeSettings { KX_NPC_ESP_SETTINGS(KX_SETTING_DECLARE) };
inline void to_json(nlohmann::json& j, const NpcEspSettings& value) {
    j = nlohmann::json::object(); KX_ATTITUDE_SETTINGS(KX_SETTING_SAVE) KX_NPC_ESP_SETTINGS(KX_SETTING_SAVE)
}
inline void from_json(const nlohmann::json& j, NpcEspSettings& value) {
    value = NpcEspSettings{}; if (!j.is_object()) return;
    KX_ATTITUDE_SETTINGS(KX_SETTING_LOAD) KX_NPC_ESP_SETTINGS(KX_SETTING_LOAD)
}

#define KX_OBJECT_ESP_SETTINGS(X) \
    X(bool, enabled, true) X(bool, renderBox, false) X(bool, renderWireframe, false) X(float, maxBoxHeight, 10.0f) \
    X(bool, renderCircle, false) X(bool, renderSphere, false) X(bool, renderDistance, false) X(bool, renderDot, true) \
    X(bool, renderDetails, false) X(bool, renderHealthBar, true) X(bool, showBurstDps, false) \
    X(bool, showDamageNumbers, true) X(bool, showOnlyDamaged, true) X(bool, showHealthPercentage, false) \
    X(bool, showDeadGadgets, true) X(bool, showResourceNodes, true) X(bool, showWaypoints, true) \
    X(bool, showVistas, true) X(bool, showCraftingStations, true) X(bool, showAttackTargets, true) \
    X(bool, showPlayerCreated, true) X(bool, showInteractables, true) X(bool, showDoors, true) \
    X(bool, showPortals, true) X(bool, showDestructible, true) X(bool, showPoints, true) \
    X(bool, showPlayerSpecific, true) X(bool, showProps, false) X(bool, showBuildSites, true) \
    X(bool, showBountyBoards, true) X(bool, showRifts, true) X(bool, showGeneric, false) \
    X(bool, showGeneric2, false) X(bool, showUnknown, true) X(bool, showAttackTargetList, true) \
    X(bool, showAttackTargetListOnlyInCombat, true) X(bool, showItems, true) X(bool, showItemJunk, false) \
    X(bool, showItemCommon, false) X(bool, showItemFine, true) X(bool, showItemMasterwork, true) \
    X(bool, showItemRare, true) X(bool, showItemExotic, true) X(bool, showItemAscended, true) \
    X(bool, showItemLegendary, true) X(bool, showDetailGadgetType, true) X(bool, showDetailHealth, true) \
    X(bool, showDetailPosition, true) X(bool, showDetailResourceInfo, true) X(bool, showDetailGatherableStatus, true)
KX_DEFINE_SETTINGS_TYPE(ObjectEspSettings, KX_OBJECT_ESP_SETTINGS)

#ifdef _DEBUG
#define KX_VISUAL_DEBUG_DEFAULT true
#else
#define KX_VISUAL_DEBUG_DEFAULT false
#endif
#define KX_VISUALS_CONFIGURATION(X) \
    X(PlayerEspSettings, playerESP, {}) X(NpcEspSettings, npcESP, {}) X(ObjectEspSettings, objectESP, {}) \
    X(DistanceSettings, distance, {}) X(ScalingSettings, scaling, {}) X(ElementSizeSettings, sizes, {}) \
    X(AppearanceSettings, appearance, {}) X(float, espUpdateRate, 60.0f) \
    X(bool, hideDepletedNodes, true) X(bool, showDebugAddresses, KX_VISUAL_DEBUG_DEFAULT)
KX_DEFINE_SETTINGS_TYPE(VisualsConfiguration, KX_VISUALS_CONFIGURATION)

} // namespace kx
