#pragma once

#include "../Data/FrameData.h"

namespace kx {
    class FrameDataProcessor {
    public:
        static void Process(const FrameContext& context, 
                            const PooledFrameRenderData& filteredData,
                            PooledFrameRenderData& outData);
    };
}
