#pragma once

namespace kx {
    namespace GUI {

        /**
         * @brief Renders a collapsible section with common ESP visual element checkboxes
         * 
         * This helper function provides a consistent UI for toggling ESP visual elements
         * across different entity categories (Players, NPCs, Objects).
         * 
         * @param categoryName Display name for the collapsible header
         * @param renderBox Reference to box rendering toggle
         * @param renderDistance Reference to distance text toggle
         * @param renderDot Reference to center dot toggle
         * @param renderHealthBar Optional pointer to health bar toggle (e.g., for Players/NPCs)
         * @param renderDetails Optional pointer to details text toggle
         * @param renderPlayerName Optional pointer to player name toggle (Players only)
         */
        void RenderCategoryStyleSettings(const char* categoryName, 
                                        bool& renderBox, 
                                        bool& renderDistance, 
                                        bool& renderDot, 
                                        bool* renderHealthBar = nullptr, 
                                        bool* renderEnergyBar = nullptr,
                                        bool* renderDetails = nullptr, 
                                        bool* renderPlayerName = nullptr,
                                        bool* showBurstDps = nullptr);

    } // namespace GUI
} // namespace kx
