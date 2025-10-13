#include "AppState.h"
#include "../Rendering/Data/ESPData.h"
#include "../Rendering/Data/RenderableData.h"
#include "../Rendering/Utils/ESPConstants.h"
#include "../Utils/DebugLogger.h"
#include <vector>
#include <algorithm>
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <nlohmann/json.hpp>

namespace kx {

    AppState::AppState() {
        m_lastFarPlaneRecalc = std::chrono::steady_clock::now();
        LoadSettings();
    }

    AppState& AppState::Get() {
        static AppState instance;
        return instance;
    }

    // ===== Settings Persistence =====

    std::string AppState::GetSettingsFilePath() {
        char path[MAX_PATH] = {0};
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
            std::string dir = std::string(path) + "\\KX-Vision";
            CreateDirectoryA(dir.c_str(), NULL);
            return dir + "\\settings.json";
        }
        return "settings.json"; // fallback to working dir
    }

    static void SetIfExistsBool(const nlohmann::json& j, const char* key, bool& out) {
        auto it = j.find(key);
        if (it != j.end() && it->is_boolean()) out = *it;
    }
    static void SetIfExistsFloat(const nlohmann::json& j, const char* key, float& out) {
        auto it = j.find(key);
        if (it != j.end() && it->is_number()) out = it->get<float>();
    }
    static void SetIfExistsEnum(const nlohmann::json& j, const char* key, int& out) {
        auto it = j.find(key);
        if (it != j.end() && it->is_number_integer()) out = it->get<int>();
    }

    bool AppState::LoadSettings() {
        std::ifstream f(GetSettingsFilePath());
        if (!f.is_open()) return false;
        nlohmann::json j;
        try { f >> j; } catch (...) { return false; }

        try {
            // Performance
            SetIfExistsFloat(j, "espUpdateRate", m_settings.espUpdateRate);
            SetIfExistsBool(j, "hideDepletedNodes", m_settings.hideDepletedNodes);

            // Debug
            SetIfExistsBool(j, "enableDebugLogging", m_settings.enableDebugLogging);
#ifdef _DEBUG
            SetIfExistsBool(j, "showDebugAddresses", m_settings.showDebugAddresses);
#endif

            // Global
            if (j.contains("distance")) {
                const auto& d = j["distance"]; 
                SetIfExistsBool(d, "useDistanceLimit", m_settings.distance.useDistanceLimit);
                SetIfExistsFloat(d, "renderDistanceLimit", m_settings.distance.renderDistanceLimit);
                SetIfExistsBool(d, "enablePlayerNpcFade", m_settings.distance.enablePlayerNpcFade);
                SetIfExistsFloat(d, "playerNpcMinAlpha", m_settings.distance.playerNpcMinAlpha);
            }
            if (j.contains("scaling")) {
                const auto& s = j["scaling"]; 
                SetIfExistsFloat(s, "scalingStartDistance", m_settings.scaling.scalingStartDistance);
                SetIfExistsFloat(s, "minScale", m_settings.scaling.minScale);
                SetIfExistsFloat(s, "maxScale", m_settings.scaling.maxScale);
                SetIfExistsFloat(s, "limitDistanceFactor", m_settings.scaling.limitDistanceFactor);
                SetIfExistsFloat(s, "limitScalingExponent", m_settings.scaling.limitScalingExponent);
                SetIfExistsFloat(s, "noLimitScalingExponent", m_settings.scaling.noLimitScalingExponent);
            }
            if (j.contains("sizes")) {
                const auto& s = j["sizes"]; 
                SetIfExistsFloat(s, "baseFontSize", m_settings.sizes.baseFontSize);
                SetIfExistsFloat(s, "minFontSize", m_settings.sizes.minFontSize);
                SetIfExistsFloat(s, "baseDotRadius", m_settings.sizes.baseDotRadius);
                SetIfExistsFloat(s, "baseBoxThickness", m_settings.sizes.baseBoxThickness);
                SetIfExistsFloat(s, "baseBoxHeight", m_settings.sizes.baseBoxHeight);
                SetIfExistsFloat(s, "baseBoxWidth", m_settings.sizes.baseBoxWidth);
                SetIfExistsFloat(s, "baseHealthBarWidth", m_settings.sizes.baseHealthBarWidth);
                SetIfExistsFloat(s, "baseHealthBarHeight", m_settings.sizes.baseHealthBarHeight);
            }

            // Category: Player
            if (j.contains("playerESP")) {
                const auto& p = j["playerESP"]; 
                SetIfExistsBool(p, "enabled", m_settings.playerESP.enabled);
                SetIfExistsBool(p, "renderBox", m_settings.playerESP.renderBox);
                SetIfExistsBool(p, "renderDistance", m_settings.playerESP.renderDistance);
                SetIfExistsBool(p, "renderDot", m_settings.playerESP.renderDot);
                SetIfExistsBool(p, "renderDetails", m_settings.playerESP.renderDetails);
                SetIfExistsBool(p, "renderHealthBar", m_settings.playerESP.renderHealthBar);
                SetIfExistsBool(p, "renderEnergyBar", m_settings.playerESP.renderEnergyBar);
                SetIfExistsBool(p, "renderPlayerName", m_settings.playerESP.renderPlayerName);
                SetIfExistsBool(p, "showLocalPlayer", m_settings.playerESP.showLocalPlayer);
                int gearMode = (int)m_settings.playerESP.gearDisplayMode;
                SetIfExistsEnum(p, "gearDisplayMode", gearMode);
                m_settings.playerESP.gearDisplayMode = (GearDisplayMode)gearMode;
                int energyType = (int)m_settings.playerESP.energyDisplayType;
                SetIfExistsEnum(p, "energyDisplayType", energyType);
                m_settings.playerESP.energyDisplayType = (EnergyDisplayType)energyType;
                SetIfExistsBool(p, "showFriendly", m_settings.playerESP.showFriendly);
                SetIfExistsBool(p, "showHostile", m_settings.playerESP.showHostile);
                SetIfExistsBool(p, "showNeutral", m_settings.playerESP.showNeutral);
                SetIfExistsBool(p, "showIndifferent", m_settings.playerESP.showIndifferent);
                SetIfExistsBool(p, "showDetailLevel", m_settings.playerESP.showDetailLevel);
                SetIfExistsBool(p, "showDetailHp", m_settings.playerESP.showDetailHp);
                SetIfExistsBool(p, "showDetailAttitude", m_settings.playerESP.showDetailAttitude);
                SetIfExistsBool(p, "showDetailEnergy", m_settings.playerESP.showDetailEnergy);
                SetIfExistsBool(p, "showDetailPosition", m_settings.playerESP.showDetailPosition);
                SetIfExistsBool(p, "showDetailRank", m_settings.playerESP.showDetailRank);
                SetIfExistsBool(p, "showDetailProfession", m_settings.playerESP.showDetailProfession);
                SetIfExistsBool(p, "showDetailRace", m_settings.playerESP.showDetailRace);
                SetIfExistsBool(p, "showDetailName", m_settings.playerESP.showDetailName);
            }

            // Category: NPC
            if (j.contains("npcESP")) {
                const auto& n = j["npcESP"]; 
                SetIfExistsBool(n, "enabled", m_settings.npcESP.enabled);
                SetIfExistsBool(n, "renderBox", m_settings.npcESP.renderBox);
                SetIfExistsBool(n, "renderDistance", m_settings.npcESP.renderDistance);
                SetIfExistsBool(n, "renderDot", m_settings.npcESP.renderDot);
                SetIfExistsBool(n, "renderDetails", m_settings.npcESP.renderDetails);
                SetIfExistsBool(n, "renderHealthBar", m_settings.npcESP.renderHealthBar);
                SetIfExistsBool(n, "showFriendly", m_settings.npcESP.showFriendly);
                SetIfExistsBool(n, "showHostile", m_settings.npcESP.showHostile);
                SetIfExistsBool(n, "showNeutral", m_settings.npcESP.showNeutral);
                SetIfExistsBool(n, "showIndifferent", m_settings.npcESP.showIndifferent);
                SetIfExistsBool(n, "showLegendary", m_settings.npcESP.showLegendary);
                SetIfExistsBool(n, "showChampion", m_settings.npcESP.showChampion);
                SetIfExistsBool(n, "showElite", m_settings.npcESP.showElite);
                SetIfExistsBool(n, "showVeteran", m_settings.npcESP.showVeteran);
                SetIfExistsBool(n, "showAmbient", m_settings.npcESP.showAmbient);
                SetIfExistsBool(n, "showNormal", m_settings.npcESP.showNormal);
                SetIfExistsBool(n, "showDeadNpcs", m_settings.npcESP.showDeadNpcs);
                SetIfExistsBool(n, "showDetailLevel", m_settings.npcESP.showDetailLevel);
                SetIfExistsBool(n, "showDetailHp", m_settings.npcESP.showDetailHp);
                SetIfExistsBool(n, "showDetailAttitude", m_settings.npcESP.showDetailAttitude);
                SetIfExistsBool(n, "showDetailRank", m_settings.npcESP.showDetailRank);
                SetIfExistsBool(n, "showDetailPosition", m_settings.npcESP.showDetailPosition);
            }

            // Category: Objects
            if (j.contains("objectESP")) {
                const auto& o = j["objectESP"]; 
                SetIfExistsBool(o, "enabled", m_settings.objectESP.enabled);
                SetIfExistsBool(o, "renderCircle", m_settings.objectESP.renderCircle);
                SetIfExistsBool(o, "renderSphere", m_settings.objectESP.renderSphere);
                SetIfExistsBool(o, "renderDistance", m_settings.objectESP.renderDistance);
                SetIfExistsBool(o, "renderDot", m_settings.objectESP.renderDot);
                SetIfExistsBool(o, "renderDetails", m_settings.objectESP.renderDetails);
                SetIfExistsBool(o, "renderHealthBar", m_settings.objectESP.renderHealthBar);
                SetIfExistsBool(o, "showOnlyDamagedGadgets", m_settings.objectESP.showOnlyDamagedGadgets);
                SetIfExistsBool(o, "showDeadGadgets", m_settings.objectESP.showDeadGadgets);
                SetIfExistsBool(o, "showResourceNodes", m_settings.objectESP.showResourceNodes);
                SetIfExistsBool(o, "showWaypoints", m_settings.objectESP.showWaypoints);
                SetIfExistsBool(o, "showVistas", m_settings.objectESP.showVistas);
                SetIfExistsBool(o, "showCraftingStations", m_settings.objectESP.showCraftingStations);
                SetIfExistsBool(o, "showAttackTargets", m_settings.objectESP.showAttackTargets);
                SetIfExistsBool(o, "showPlayerCreated", m_settings.objectESP.showPlayerCreated);
                SetIfExistsBool(o, "showInteractables", m_settings.objectESP.showInteractables);
                SetIfExistsBool(o, "showDoors", m_settings.objectESP.showDoors);
                SetIfExistsBool(o, "showPortals", m_settings.objectESP.showPortals);
                SetIfExistsBool(o, "showDestructible", m_settings.objectESP.showDestructible);
                SetIfExistsBool(o, "showPoints", m_settings.objectESP.showPoints);
                SetIfExistsBool(o, "showPlayerSpecific", m_settings.objectESP.showPlayerSpecific);
                SetIfExistsBool(o, "showProps", m_settings.objectESP.showProps);
                SetIfExistsBool(o, "showBuildSites", m_settings.objectESP.showBuildSites);
                SetIfExistsBool(o, "showBountyBoards", m_settings.objectESP.showBountyBoards);
                SetIfExistsBool(o, "showRifts", m_settings.objectESP.showRifts);
                SetIfExistsBool(o, "showGeneric", m_settings.objectESP.showGeneric);
                SetIfExistsBool(o, "showUnknown", m_settings.objectESP.showUnknown);
                SetIfExistsBool(o, "showDetailGadgetType", m_settings.objectESP.showDetailGadgetType);
                SetIfExistsBool(o, "showDetailHealth", m_settings.objectESP.showDetailHealth);
                SetIfExistsBool(o, "showDetailPosition", m_settings.objectESP.showDetailPosition);
                SetIfExistsBool(o, "showDetailResourceInfo", m_settings.objectESP.showDetailResourceInfo);
                SetIfExistsBool(o, "showDetailGatherableStatus", m_settings.objectESP.showDetailGatherableStatus);
            }
        } catch (...) {
            return false;
        }
        return true;
    }

    bool AppState::SaveSettings() const {
        nlohmann::json j;
        // Performance and debug
        j["espUpdateRate"] = m_settings.espUpdateRate;
        j["hideDepletedNodes"] = m_settings.hideDepletedNodes;
        j["enableDebugLogging"] = m_settings.enableDebugLogging;
#ifdef _DEBUG
        j["showDebugAddresses"] = m_settings.showDebugAddresses;
#endif

        // Groups
        j["distance"] = {
            {"useDistanceLimit", m_settings.distance.useDistanceLimit},
            {"renderDistanceLimit", m_settings.distance.renderDistanceLimit},
            {"enablePlayerNpcFade", m_settings.distance.enablePlayerNpcFade},
            {"playerNpcMinAlpha", m_settings.distance.playerNpcMinAlpha}
        };
        j["scaling"] = {
            {"scalingStartDistance", m_settings.scaling.scalingStartDistance},
            {"minScale", m_settings.scaling.minScale},
            {"maxScale", m_settings.scaling.maxScale},
            {"limitDistanceFactor", m_settings.scaling.limitDistanceFactor},
            {"limitScalingExponent", m_settings.scaling.limitScalingExponent},
            {"noLimitScalingExponent", m_settings.scaling.noLimitScalingExponent}
        };
        j["sizes"] = {
            {"baseFontSize", m_settings.sizes.baseFontSize},
            {"minFontSize", m_settings.sizes.minFontSize},
            {"baseDotRadius", m_settings.sizes.baseDotRadius},
            {"baseBoxThickness", m_settings.sizes.baseBoxThickness},
            {"baseBoxHeight", m_settings.sizes.baseBoxHeight},
            {"baseBoxWidth", m_settings.sizes.baseBoxWidth},
            {"baseHealthBarWidth", m_settings.sizes.baseHealthBarWidth},
            {"baseHealthBarHeight", m_settings.sizes.baseHealthBarHeight}
        };

        // Player
        j["playerESP"] = {
            {"enabled", m_settings.playerESP.enabled},
            {"renderBox", m_settings.playerESP.renderBox},
            {"renderDistance", m_settings.playerESP.renderDistance},
            {"renderDot", m_settings.playerESP.renderDot},
            {"renderDetails", m_settings.playerESP.renderDetails},
            {"renderHealthBar", m_settings.playerESP.renderHealthBar},
            {"renderEnergyBar", m_settings.playerESP.renderEnergyBar},
            {"renderPlayerName", m_settings.playerESP.renderPlayerName},
            {"showLocalPlayer", m_settings.playerESP.showLocalPlayer},
            {"gearDisplayMode", (int)m_settings.playerESP.gearDisplayMode},
            {"energyDisplayType", (int)m_settings.playerESP.energyDisplayType},
            {"showFriendly", m_settings.playerESP.showFriendly},
            {"showHostile", m_settings.playerESP.showHostile},
            {"showNeutral", m_settings.playerESP.showNeutral},
            {"showIndifferent", m_settings.playerESP.showIndifferent},
            {"showDetailLevel", m_settings.playerESP.showDetailLevel},
            {"showDetailHp", m_settings.playerESP.showDetailHp},
            {"showDetailAttitude", m_settings.playerESP.showDetailAttitude},
            {"showDetailEnergy", m_settings.playerESP.showDetailEnergy},
            {"showDetailPosition", m_settings.playerESP.showDetailPosition},
            {"showDetailRank", m_settings.playerESP.showDetailRank},
            {"showDetailProfession", m_settings.playerESP.showDetailProfession},
            {"showDetailRace", m_settings.playerESP.showDetailRace},
            {"showDetailName", m_settings.playerESP.showDetailName}
        };

        // NPC
        j["npcESP"] = {
            {"enabled", m_settings.npcESP.enabled},
            {"renderBox", m_settings.npcESP.renderBox},
            {"renderDistance", m_settings.npcESP.renderDistance},
            {"renderDot", m_settings.npcESP.renderDot},
            {"renderDetails", m_settings.npcESP.renderDetails},
            {"renderHealthBar", m_settings.npcESP.renderHealthBar},
            {"showFriendly", m_settings.npcESP.showFriendly},
            {"showHostile", m_settings.npcESP.showHostile},
            {"showNeutral", m_settings.npcESP.showNeutral},
            {"showIndifferent", m_settings.npcESP.showIndifferent},
            {"showLegendary", m_settings.npcESP.showLegendary},
            {"showChampion", m_settings.npcESP.showChampion},
            {"showElite", m_settings.npcESP.showElite},
            {"showVeteran", m_settings.npcESP.showVeteran},
            {"showAmbient", m_settings.npcESP.showAmbient},
            {"showNormal", m_settings.npcESP.showNormal},
            {"showDeadNpcs", m_settings.npcESP.showDeadNpcs},
            {"showDetailLevel", m_settings.npcESP.showDetailLevel},
            {"showDetailHp", m_settings.npcESP.showDetailHp},
            {"showDetailAttitude", m_settings.npcESP.showDetailAttitude},
            {"showDetailRank", m_settings.npcESP.showDetailRank},
            {"showDetailPosition", m_settings.npcESP.showDetailPosition}
        };

        // Objects
        j["objectESP"] = {
            {"enabled", m_settings.objectESP.enabled},
            {"renderCircle", m_settings.objectESP.renderCircle},
            {"renderSphere", m_settings.objectESP.renderSphere},
            {"renderDistance", m_settings.objectESP.renderDistance},
            {"renderDot", m_settings.objectESP.renderDot},
            {"renderDetails", m_settings.objectESP.renderDetails},
            {"renderHealthBar", m_settings.objectESP.renderHealthBar},
            {"showOnlyDamagedGadgets", m_settings.objectESP.showOnlyDamagedGadgets},
            {"showDeadGadgets", m_settings.objectESP.showDeadGadgets},
            {"showResourceNodes", m_settings.objectESP.showResourceNodes},
            {"showWaypoints", m_settings.objectESP.showWaypoints},
            {"showVistas", m_settings.objectESP.showVistas},
            {"showCraftingStations", m_settings.objectESP.showCraftingStations},
            {"showAttackTargets", m_settings.objectESP.showAttackTargets},
            {"showPlayerCreated", m_settings.objectESP.showPlayerCreated},
            {"showInteractables", m_settings.objectESP.showInteractables},
            {"showDoors", m_settings.objectESP.showDoors},
            {"showPortals", m_settings.objectESP.showPortals},
            {"showDestructible", m_settings.objectESP.showDestructible},
            {"showPoints", m_settings.objectESP.showPoints},
            {"showPlayerSpecific", m_settings.objectESP.showPlayerSpecific},
            {"showProps", m_settings.objectESP.showProps},
            {"showBuildSites", m_settings.objectESP.showBuildSites},
            {"showBountyBoards", m_settings.objectESP.showBountyBoards},
            {"showRifts", m_settings.objectESP.showRifts},
            {"showGeneric", m_settings.objectESP.showGeneric},
            {"showUnknown", m_settings.objectESP.showUnknown},
            {"showDetailGadgetType", m_settings.objectESP.showDetailGadgetType},
            {"showDetailHealth", m_settings.objectESP.showDetailHealth},
            {"showDetailPosition", m_settings.objectESP.showDetailPosition},
            {"showDetailResourceInfo", m_settings.objectESP.showDetailResourceInfo},
            {"showDetailGatherableStatus", m_settings.objectESP.showDetailGatherableStatus}
        };

        std::ofstream f(GetSettingsFilePath());
        if (!f.is_open()) return false;
        try { f << j.dump(2); } catch (...) { return false; }
        return true;
    }

    void AppState::UpdateAdaptiveFarPlane(const PooledFrameRenderData& frameData) {
        // Only recalculate once per second
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastFarPlaneRecalc).count() < 1) {
            return;
        }
        m_lastFarPlaneRecalc = now;

        // 1. Collect distances from gadgets/objects only
        // Rationale: Players and NPCs are limited to ~200m by game mechanics,
        // but objects (waypoints, vistas, resource nodes) can be 1000m+ away.
        // Using only object distances gives us the true scene depth for intelligent scaling.
        std::vector<float> distances;
        distances.reserve(frameData.gadgets.size()); // Pre-allocate memory
        
        for (const auto* g : frameData.gadgets) {
            if (g) distances.push_back(g->gameplayDistance);
        }

        // 2. Handle edge cases - too few objects for reliable percentile statistics
        if (distances.size() < AdaptiveScaling::MIN_ENTITIES_FOR_PERCENTILE) {
            // Use average distance of available objects with reasonable bounds
            if (distances.empty()) {
                m_adaptiveFarPlane = AdaptiveScaling::FAR_PLANE_DEFAULT; // No objects - use conservative mid-range default
                return;
            }
            
            // Calculate average distance from the few objects we have
            float sum = 0.0f;
            for (float d : distances) {
                sum += d;
            }
            float avgDistance = sum / distances.size();
            
            // Clamp and smooth the result
            float targetFarPlane = (std::clamp)(avgDistance, AdaptiveScaling::FAR_PLANE_MIN, AdaptiveScaling::FAR_PLANE_MAX);
            float oldFarPlane = m_adaptiveFarPlane;
            m_adaptiveFarPlane = m_adaptiveFarPlane + (targetFarPlane - m_adaptiveFarPlane) * 0.5f; // LERP
            
            LOG_DEBUG("[AdaptiveFarPlane] Few objects (%zu), using average: %.1fm (was %.1fm)", 
                      distances.size(), m_adaptiveFarPlane, oldFarPlane);
            return;
        }

        // 3. Find the 95th percentile. std::nth_element is faster than a full sort.
        size_t percentile_index = static_cast<size_t>(distances.size() * 0.95);
        std::nth_element(distances.begin(), distances.begin() + percentile_index, distances.end());
        float newFarPlane = distances[percentile_index];
        
        // Clamp the value to a reasonable range to prevent extreme outliers
        newFarPlane = (std::clamp)(newFarPlane, AdaptiveScaling::FAR_PLANE_MIN, AdaptiveScaling::FAR_PLANE_MAX);

        // 4. Smoothly interpolate to the new value to prevent visual "snapping"
        // This makes the transition invisible to the user if the range changes
        float oldFarPlane = m_adaptiveFarPlane;
        m_adaptiveFarPlane = m_adaptiveFarPlane + (newFarPlane - m_adaptiveFarPlane) * 0.5f; // LERP
        
        // Log the adaptive far plane calculation for debugging (only visible when debug logging enabled)
        LOG_DEBUG("[AdaptiveFarPlane] Entities: %zu | 95th percentile: %.1fm | Smoothed: %.1fm (was %.1fm)", 
                  distances.size(), newFarPlane, m_adaptiveFarPlane, oldFarPlane);
    }

} // namespace kx