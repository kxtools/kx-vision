#pragma once

namespace kx {

// Product-owned runtime hooks. Shared lifecycle code invokes these at fixed
// ownership boundaries without knowing which optional product is present.
class IRuntimeExtension {
public:
    virtual ~IRuntimeExtension() = default;

    virtual void OnEntryThreadStart() {}
    virtual bool Initialize() { return true; }
    virtual bool Tick() { return false; }
    virtual void OnRenderThreadStart() {}
    virtual bool RequiresExclusiveOverlay() const { return false; }
    virtual bool WantsOverlayInput() const { return false; }
    virtual void RenderOverlay() {}
    virtual void Shutdown() {}
};

} // namespace kx
