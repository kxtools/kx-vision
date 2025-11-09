#pragma once

#include <cstdint>

/**
 * @file HavokEnums.h
 * @brief Enums for Havok Physics engine
 * 
 * This file contains enums for Havok Physics engine types used by Guild Wars 2.
 * These are low-level physics engine enums, separate from game-specific enums.
 */

namespace kx {
namespace Havok {

// Havok Physics Shape Types - hkcdShapeType enum
// Used to safely identify shape types before accessing shape-specific fields
enum class HkcdShapeType : uint8_t {
    SPHERE = 0,
    CYLINDER = 1,
    TRIANGLE = 2,
    BOX = 3,
    CAPSULE = 4,
    CONVEX_VERTICES = 5,
    TRI_SAMPLED_HEIGHT_FIELD_COLLECTION = 6,
    TRI_SAMPLED_HEIGHT_FIELD_BV_TREE = 7,
    LIST = 8,
    MOPP = 9,
    CONVEX_TRANSLATE = 10,
    CONVEX_TRANSFORM = 0x0B,
    SAMPLED_HEIGHT_FIELD = 0x0C,
    EXTENDED_MESH = 0x0D,
    TRANSFORM = 0x0E,
    COMPRESSED_MESH = 0x0F,
    STATIC_COMPOUND = 0x10,
    BV_COMPRESSED_MESH = 0x11,
    COLLECTION = 0x12,
    USER0 = 0x13,
    USER1 = 0x14,
    USER2 = 0x15,
    BV_TREE = 0x16,
    CONVEX = 0x17,
    CONVEX_PIECE = 0x18,
    MULTI_SPHERE = 0x19,
    CONVEX_LIST = 0x1A,
    TRIANGLE_COLLECTION = 0x1B,
    HEIGHT_FIELD = 0x1C,
    SPHERE_REP = 0x1D,
    BV = 0x1E,
    PLANE = 0x1F,
    PHANTOM_CALLBACK = 0x20,
    MULTI_RAY = 0x21,
    INVALID = 0x22,
    MAX_PPU_SHAPE_TYPE = 0x23,
    ALL_SHAPE_TYPES = 0xFF
};

} // namespace Havok
} // namespace kx

