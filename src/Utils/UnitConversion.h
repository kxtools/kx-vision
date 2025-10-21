#pragma once

#include <cmath>

/**
 * @file UnitConversion.h
 * @brief Unit conversion utilities for GW2 distance measurements
 * 
 * This module provides conversions between real meters and GW2 units.
 * 
 * IMPORTANT: Distance Units Used in This Project
 * ===============================================
 * 
 * Meters:
 *   - Real meters as provided by Mumble Link
 *   - Standard international unit (SI)
 *   - Used for all distance calculations
 * 
 * GW2 Units:
 *   - In-game skill range units (as shown in skill tooltips)
 *   - 1 GW2 unit = 1 inch = 0.0254 meters
 *   - Example: 1200 range skill = 30.48 meters
 * 
 * Conversion Constants:
 *   - M_PER_UNIT = 0.0254 (meters per GW2 unit)
 *   - UNITS_PER_M = 39.37007874 (GW2 units per meter)
 * 
 * Verification:
 *   - Skill range 1200 = 30.48m ✓
 *   - Skill range 900 = 22.86m ✓
 *   - Tested against in-game skill ranges
 */

namespace kx {

/**
 * @brief GW2 Unit Conversion Utilities
 * 
 * Guild Wars 2 uses a proprietary unit system where:
 * - 1 GW2 unit = 1 inch = 0.0254 meters
 * - This allows direct comparison with real-world measurements via Mumble Link
 */
namespace UnitConversion {

    // Conversion constants
    constexpr double M_PER_UNIT = 0.0254;           // meters per GW2 unit
    constexpr double UNITS_PER_M = 39.37007874;     // GW2 units per meter (1 / M_PER_UNIT)

    /**
     * @brief Convert meters to GW2 units
     * @param meters Distance in meters
     * @return Distance in GW2 units
     */
    inline float MetersToGW2Units(float meters) {
        return static_cast<float>(meters * UNITS_PER_M);
    }

    /**
     * @brief Convert GW2 units to meters
     * @param units Distance in GW2 units
     * @return Distance in meters
     */
    inline float GW2UnitsToMeters(float units) {
        return static_cast<float>(units * M_PER_UNIT);
    }

} // namespace UnitConversion

} // namespace kx
