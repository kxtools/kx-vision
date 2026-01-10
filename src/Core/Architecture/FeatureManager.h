#pragma once

#include <memory>
#include <vector>

namespace spdlog { class logger; }
struct ImDrawList;

namespace kx {

class IFeature;

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
     */
    void UpdateAll(float deltaTime);

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

private:
    std::vector<std::unique_ptr<IFeature>> m_features;
};

} // namespace kx
