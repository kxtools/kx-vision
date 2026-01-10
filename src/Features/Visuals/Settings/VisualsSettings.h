#pragma once

#include "../../../../libs/nlohmann/json.hpp"
#include "../../../Core/Settings/SettingsConstants.h"

namespace kx {

    // --- Category-specific settings ---
    struct AttitudeSettings {
        bool showFriendly = true;
        bool showHostile = true;
        bool showNeutral = true;
        bool showIndifferent = true;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AttitudeSettings, showFriendly, showHostile, showNeutral, showIndifferent);

    struct TrailSettings {
        bool enabled = false;
        int maxPoints = 30;
        float maxDuration = 1.0f;
        TrailDisplayMode displayMode = TrailDisplayMode::Hostile;
        TrailTeleportMode teleportMode = TrailTeleportMode::Tactical;
        float thickness = 2.0f;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TrailSettings, enabled, maxPoints, maxDuration, displayMode, teleportMode, thickness);

    struct PlayerEspSettings : AttitudeSettings {
        bool enabled = true;
        bool renderBox = false;
        bool renderWireframe = false;
        bool renderDistance = false;
        bool renderDot = false;
        bool renderDetails = false;
        bool renderHealthBar = true;
        bool renderEnergyBar = false;
        bool renderPlayerName = true;  // Show player names by default for natural identification
        bool showBurstDps = false;
        bool showDamageNumbers = true;
        bool showOnlyDamaged = false;
        bool showHealthPercentage = false;
        bool showLocalPlayer = false; // Hide local player by default
        bool enableGearDisplay = false;
        GearDisplayMode gearDisplayMode = GearDisplayMode::Compact;
        EnergyDisplayType energyDisplayType = EnergyDisplayType::Energy;
        // Detail-field filters
        bool showDetailLevel = true;
        bool showDetailHp = true;
        bool showDetailAttitude = true;
        bool showDetailEnergy = true;
        bool showDetailPosition = true;
        bool showDetailRank = true;
        bool showDetailProfession = true;
        bool showDetailRace = true;
        float hostileBoostMultiplier = 2.0f;
        TrailSettings trails;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerEspSettings, enabled, renderBox, renderWireframe, renderDistance, renderDot, renderDetails,
                                       renderHealthBar, renderEnergyBar, renderPlayerName, showBurstDps,
                                       showDamageNumbers, showOnlyDamaged, showHealthPercentage, showLocalPlayer,
                                       enableGearDisplay, gearDisplayMode, energyDisplayType, showDetailLevel, showDetailHp,
                                       showDetailAttitude, showDetailEnergy, showDetailPosition, showDetailRank,
                                       showDetailProfession, showDetailRace, hostileBoostMultiplier, showFriendly, showHostile, showNeutral,
                                       showIndifferent, trails);

    struct NpcEspSettings : AttitudeSettings {
        bool enabled = true;
        bool renderBox = false;
        bool renderWireframe = false;
        bool renderDistance = false;
        bool renderDot = false;
        bool renderDetails = false;
        bool renderHealthBar = true;
        bool showBurstDps = false;
        bool showDamageNumbers = true;
        bool showOnlyDamaged = false;
        bool showHealthPercentage = false;
        // Rank filters
        bool showLegendary = true;
        bool showChampion = true;
        bool showElite = true;
        bool showVeteran = true;
        bool showAmbient = true;
        bool showNormal = true;
        // Health-based filtering
        bool showDeadNpcs = false;  // Show NPCs with 0 HP (dead/defeated enemies)
        // Detail-field filters
        bool showDetailLevel = true;
        bool showDetailHp = true;
        bool showDetailAttitude = true;
        bool showDetailRank = true;
        bool showDetailPosition = true;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NpcEspSettings, enabled, renderBox, renderWireframe, renderDistance, renderDot, renderDetails,
                                       renderHealthBar, showBurstDps, showDamageNumbers, showOnlyDamaged,
                                       showHealthPercentage, showLegendary, showChampion, showElite, showVeteran,
                                       showAmbient, showNormal, showDeadNpcs, showDetailLevel, showDetailHp,
                                       showDetailAttitude, showDetailRank, showDetailPosition, showFriendly,
                                       showHostile, showNeutral, showIndifferent);

    struct ObjectEspSettings {
        bool enabled = true;
        bool renderBox = false;         // Render 2D bounding box
        bool renderWireframe = false;    // Render 3D wireframe box
        float maxBoxHeight = 10.0f;     // Max height (meters) to show box (filters huge gadgets)
        bool renderCircle = false;      // Render a 2D circle for the object
        bool renderSphere = false;      // Render a 3D sphere for the object
        bool renderDistance = false;
        bool renderDot = true;
        bool renderDetails = false;
        bool renderHealthBar = true;
        bool showBurstDps = false;
        bool showDamageNumbers = true;
        bool showOnlyDamaged = true;
        bool showHealthPercentage = false;
        bool showDeadGadgets = true;
        
        // Gadget Type Filters
        bool showResourceNodes = true;    // Type 19
        bool showWaypoints = true;        // Type 18
        bool showVistas = true;           // Type 24
        bool showCraftingStations = true; // Type 5
        bool showAttackTargets = true;    // Type 16
        bool showPlayerCreated = true;    // Type 23
        bool showInteractables = true;    // Type 12 (covers chests, etc.)
        bool showDoors = true;            // Type 6
        bool showPortals = true;          // Type 17 (MapPortal)
        bool showDestructible = true;     // Type 1
        bool showPoints = true;           // Type 2 (PvP points)
        bool showPlayerSpecific = true;   // Type 14
        bool showProps = false;            // Type 20
        bool showBuildSites = true;       // Type 25
        bool showBountyBoards = true;     // Type 11
        bool showRifts = true;            // Type 13
        bool showGeneric = false;         // Type 3
        bool showGeneric2 = false;
        bool showUnknown = true;          // For any type not explicitly handled
        bool showAttackTargetList = true; // Attack Target List (AgentType::GadgetAttackTarget, separate from GadgetType::AttackTarget)
        bool showAttackTargetListOnlyInCombat = true; // Only show attack targets that are in combat state

        // Item Settings
        bool showItems = true;             // Main toggle for items
        // Rarity Filters
        bool showItemJunk = false;
        bool showItemCommon = false;
        bool showItemFine = true;
        bool showItemMasterwork = true;
        bool showItemRare = true;
        bool showItemExotic = true;
        bool showItemAscended = true;
        bool showItemLegendary = true;

        // Detail-field filters
        bool showDetailGadgetType = true;
        bool showDetailHealth = true;
        bool showDetailPosition = true;
        bool showDetailResourceInfo = true;
        bool showDetailGatherableStatus = true;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectEspSettings, enabled, renderBox, renderWireframe, maxBoxHeight, renderCircle, renderSphere, renderDistance,
                                       renderDot, renderDetails, renderHealthBar, showBurstDps, showDamageNumbers,
                                       showOnlyDamaged, showHealthPercentage, showDeadGadgets, showResourceNodes,
                                       showWaypoints, showVistas, showCraftingStations, showAttackTargets,
                                       showPlayerCreated, showInteractables, showDoors, showPortals, showDestructible,
                                       showPoints, showPlayerSpecific, showProps, showBuildSites, showBountyBoards,
                                       showRifts, showGeneric, showGeneric2, showUnknown, showAttackTargetList,
                                       showAttackTargetListOnlyInCombat, showItems, showItemJunk, showItemCommon, showItemFine, 
                                       showItemMasterwork, showItemRare, showItemExotic, showItemAscended, showItemLegendary,
                                       showDetailGadgetType, showDetailHealth, showDetailPosition, showDetailResourceInfo,
                                       showDetailGatherableStatus);

} // namespace kx
