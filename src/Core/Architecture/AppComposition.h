#pragma once

#include <memory>

#include "FeatureRegistration.h"
#include "IRuntimeExtension.h"

namespace kx {

struct AppComposition {
    FeatureRegistration features;
    std::unique_ptr<IRuntimeExtension> runtimeExtension;
};

} // namespace kx
