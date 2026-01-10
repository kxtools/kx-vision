#pragma once

#include <windows.h> // For UINT, WPARAM, LPARAM
#include <memory>
#include <vector>

namespace spdlog { class logger; }
struct ImDrawList;

namespace kx {

class IFeature;
struct FrameGameData;

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

    /**
     * @brief Initialize all registered features.
     * @return true if all features initialized successfully, false otherwise.
     */
    bool InitializeAll();

    /**
     * @brief Update all features.
     * @param deltaTime Time since last frame in seconds.
     * @param frameData Const reference to current frame's extracted game data.
     */
    void UpdateAll(float deltaTime, const FrameGameData& frameData);

    /**
     * @brief Render all features to the draw list.
     * @param drawList The ImGui draw list to render to.
     */
    void RenderAllDrawLists(ImDrawList* drawList);

    /**
     * @brief Render all feature menu items/tabs.
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

    /**
     * @brief Get read-only access to all registered features.
     * Used for settings management and other introspection needs.
     */
    const std::vector<std::unique_ptr<IFeature>>& GetFeatures() const { return m_features; }

private:
    std::vector<std::unique_ptr<IFeature>> m_features;
};

} // namespace kx
