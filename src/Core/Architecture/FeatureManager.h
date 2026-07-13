#pragma once

#include <windows.h> // For UINT, WPARAM, LPARAM
#include <memory>
#include <shared_mutex>
#include <vector>
#include "../../../libs/nlohmann/json_fwd.hpp"

namespace spdlog { class logger; }
struct ImDrawList;

namespace kx {

class IFeature;
struct EntityFrameSnapshot;
using FrameSnapshotHandle = std::shared_ptr<const EntityFrameSnapshot>;
struct ServiceContext;

/**
 * @brief Manages the lifecycle of all registered features.
 * 
 * The FeatureManager acts as a central orchestrator for modular features,
 * handling initialization, updates, rendering, and menu operations.
 */
class FeatureManager {
public:
    FeatureManager();
    ~FeatureManager();

    // Prevent copying
    FeatureManager(const FeatureManager&) = delete;
    FeatureManager& operator=(const FeatureManager&) = delete;

    /**
     * @brief Register a new feature.
     * @param feature Unique pointer to the feature to register.
     */
    void RegisterFeature(std::unique_ptr<IFeature> feature);
    bool HasFeature(const char* featureName) const;
    bool RemoveFeature(const char* featureName);

    /**
     * @brief Initialize all registered features.
     * @param ctx Service context containing pointers to core services.
     * @return true if all features initialized successfully, false otherwise.
     */
    bool InitializeAll(const ServiceContext& ctx);

    /**
     * @brief Shut down all features in reverse order of registration, then clear the list.
     */
    void ShutdownAll();

    /**
     * @brief Update all features.
     * @param deltaTime Time since last frame in seconds.
     * @param frameData Const reference to current frame's extracted game data.
     * @param ctx Service context containing pointers to core services.
     */
    void UpdateAll(float deltaTime, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx);

    /**
     * @brief Render all features to the draw list.
     * @param drawList The ImGui draw list to render to.
     * @param ctx Service context containing pointers to core services.
     */
    void RenderAllDrawLists(ImDrawList* drawList, const FrameSnapshotHandle& snapshot, const ServiceContext& ctx);

    /**
     * @brief Render all feature tab content for the main hub.
     */
    void RenderAllTabs();

    /**
     * @brief Render all feature-owned standalone windows.
     */
    void RenderAllStandaloneWindows();

    /**
     * @brief Legacy compatibility wrapper for feature menu rendering.
     */
    void RenderAllMenus();

    /**
     * @brief Get the number of registered features.
     */
    size_t GetFeatureCount() const;

    /**
     * @brief Broadcast input event to all features.
     * @param message The Windows message (WM_KEYDOWN, WM_LBUTTONDOWN, etc.)
     * @param wParam Additional message parameter
     * @param lParam Additional message parameter
     * @return true if any feature consumed the input, false otherwise
     */
    bool BroadcastInput(UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @brief Run game thread updates for all features.
     * Called from the game thread hook.
     */
    void RunGameThreadUpdates();
    void LoadPendingSettings(const nlohmann::json& root);
    void LoadAllSettings(const nlohmann::json& root);
    void ResetAllSettings();

    /**
     * @brief Get read-only access to all registered features.
     * Used for settings management and other introspection needs.
     */
    template <typename Fn>
    void ForEachFeature(Fn&& fn) {
        // Guardrail: callbacks may inspect/mutate feature-owned settings, but registration
        // cannot invalidate feature addresses until visit completes.
        std::shared_lock lock(m_featureMutex);
        for (auto& feature : m_features) {
            if (feature) fn(*feature);
        }
    }

private:
    std::vector<std::unique_ptr<IFeature>> m_features;
    mutable std::shared_mutex m_featureMutex;
    size_t m_initializedFeatureCount = 0;
};

} // namespace kx
