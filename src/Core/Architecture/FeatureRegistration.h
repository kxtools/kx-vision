#pragma once

namespace kx {

class FeatureManager;

// Neutral lifecycle callbacks supplied by the product composition root.
// Core owns invocation timing, while modules own concrete feature construction.
struct FeatureRegistration {
    using RegisterFn = void (*)(FeatureManager&);
    using SyncFn = bool (*)(FeatureManager&);

    RegisterFn registerAll = nullptr;
    SyncFn sync = nullptr;
};

} // namespace kx
