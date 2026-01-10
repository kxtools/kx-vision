#pragma once

struct ImDrawList;

namespace kx {

/**
 * @brief Base interface for modular features in KX-Vision.
 * 
 * Features are self-contained modules that can be enabled/disabled independently.
 * Each feature manages its own state, rendering, and UI.
 */
class IFeature {
public:
    virtual ~IFeature() = default;

    /**
     * @brief Initialize the feature. Called once during startup.
     * @return true if initialization succeeded, false otherwise.
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Update feature logic. Called every frame.
     * @param deltaTime Time since last frame in seconds.
     */
    virtual void Update(float deltaTime) = 0;

    /**
     * @brief Render the feature to the ImGui background draw list.
     * @param drawList The ImGui draw list to render to (typically background).
     */
    virtual void RenderDrawList(ImDrawList* drawList) = 0;

    /**
     * @brief Render feature-specific menu items/tabs in the overlay window.
     * Called within the main ImGui window context.
     */
    virtual void OnMenuRender() = 0;

    /**
     * @brief Get the name of this feature for debugging/logging.
     */
    virtual const char* GetName() const = 0;
};

} // namespace kx
