// In Core/ESPVisualsProcessor.h

#pragma once

#include "../Data/ESPData.h"

namespace kx {
    class ESPVisualsProcessor {
    public:
        static void Process(const FrameContext& context, 
                            const PooledFrameRenderData& filteredData,
                            PooledFrameRenderData& outData);
    };
}
