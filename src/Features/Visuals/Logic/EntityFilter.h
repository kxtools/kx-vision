#pragma once

namespace kx {

struct FrameGameData;
struct FrameContext;
struct VisualsConfiguration;

class EntityFilter {
public:
    /**
     * @brief OPTIMIZED filter method - filters already pooled data (no object allocations)
     * @param extractedData Input pooled data from extraction
     * @param context Frame context containing camera, settings, game state, etc.
     * @param visualsConfig Feature-specific visuals configuration
     * @param filteredData Output filtered pooled data
     */
    static void FilterPooledData(const FrameGameData& extractedData, const FrameContext& context,
                                 const VisualsConfiguration& visualsConfig, FrameGameData& filteredData);

};

} // namespace kx
