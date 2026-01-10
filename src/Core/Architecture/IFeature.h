#pragma once

#include <windows.h> // For UINT, WPARAM, LPARAM

struct ImDrawList;

namespace kx {

// Forward declarations
struct FrameGameData;

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
     * @param frameData Const reference to current frame's extracted game data.
     */
    virtual void Update(float deltaTime, const FrameGameData& frameData) = 0;

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

    /**
     * @brief Handle input events before they reach ImGui or the game.
     * @param message The Windows message (WM_KEYDOWN, WM_LBUTTONDOWN, etc.)
     * @param wParam Additional message parameter
     * @param lParam Additional message parameter
     * @return true to consume the input (block propagation), false to pass through
     */
    virtual bool OnInput(UINT message, WPARAM wParam, LPARAM lParam) { return false; }

    /**
     * @brief Called every frame from the game thread.
     * Use this for safe memory writes, teleportation, or other operations
     * that must run on the game thread.
     */
    virtual void OnGameThreadUpdate() {}
};

} // namespace kx
