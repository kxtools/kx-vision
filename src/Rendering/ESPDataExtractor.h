#pragma once

#include <vector>
#include <map>
#include "RenderableData.h"

namespace kx {

/**
 * @brief Handles data extraction from game memory (Stage 1 of rendering pipeline)
 * 
 * This class encapsulates all unsafe memory operations that read from game structures.
 * It extracts data into safe local data structures that can be rendered without
 * risk of memory access violations.
 */
class ESPDataExtractor {
public:
    /**
     * @brief Main extraction method - extracts all entity data for one frame
     * @param frameData Output container for all extracted data
     */
    static void ExtractFrameData(FrameRenderData& frameData);

private:
    /**
     * @brief Extract player data from character list
     * @param players Output vector for player data
     * @param characterNameToPlayerName Mapping of character pointers to player names
     */
    static void ExtractPlayerData(std::vector<RenderablePlayer>& players, 
                                 const std::map<void*, const wchar_t*>& characterNameToPlayerName);

    /**
     * @brief Extract NPC data from character list (excludes players)
     * @param npcs Output vector for NPC data
     */
    static void ExtractNpcData(std::vector<RenderableNpc>& npcs);

    /**
     * @brief Extract gadget/object data from gadget list
     * @param gadgets Output vector for gadget data
     */
    static void ExtractGadgetData(std::vector<RenderableGadget>& gadgets);
};

} // namespace kx