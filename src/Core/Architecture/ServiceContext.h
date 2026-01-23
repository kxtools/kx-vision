#pragma once

#include <d3d11.h>

namespace kx {

// Forward declarations
class EntityManager;
class Camera;
class MumbleLinkManager;
class SettingsManager;
struct Settings;

/**
 * @brief Service Context - Dependency Injection container for features
 * 
 * This struct contains pointers to all core services that features may need.
 * It is passed to features during initialization, update, and rendering,
 * eliminating the need for global singletons and making features testable.
 * 
 * All pointers in this context are non-owning and remain valid for the
 * lifetime of the feature system.
 */
struct ServiceContext {
    EntityManager* entities = nullptr;
    Camera* camera = nullptr;
    MumbleLinkManager* mumble = nullptr;
    Settings* settings = nullptr;
    ID3D11Device* device = nullptr;
};

} // namespace kx
