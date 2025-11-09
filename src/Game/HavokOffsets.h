#pragma once
#include <cstdint>

/**
 * @file HavokOffsets.h
 * @brief Memory offsets for Havok Physics engine structures
 * 
 * This file contains offsets for Havok Physics engine objects used by Guild Wars 2.
 * These are low-level physics engine structures, separate from game-specific structures.
 */

namespace HavokOffsets {
    
    // ============================================================================
    // HAVOK PHYSICS RIGID BODY
    // ============================================================================

    /**
     * @brief hkpRigidBody - Havok physics rigid body object
     * Contains physics simulation data and shape reference for dynamic objects (gadgets)
     */
    struct HkpRigidBody {
        static constexpr uintptr_t SHAPE = 0x0020;  // hkpShape* pointer to collision shape (e.g., hkpCylinderShape)
        static constexpr uintptr_t SHAPE_TYPE_WRAPPER = 0x4C;  // uint8_t hkcdShapeType - wrapper type (6=Terrain, 11=Transform)
    };

    /**
     * @brief Common offsets for all Havok shape types
     * 
     * NOTE: This is an offset struct only, NOT a ReClass wrapper class.
     * We read the primitive shape type byte directly from shape pointers using these offsets.
     * Unlike HkpBoxShape, HkpCylinderShape, etc., we don't need a ReClass wrapper class
     * for HkpShapeBase since we only access the primitive type field directly.
     * 
     * The primitive shape type is stored as a single byte at offset 0x10 in the shape object.
     */
    struct HkpShapeBase {
        static constexpr uintptr_t SHAPE_TYPE_PRIMITIVE = 0x10;  // uint8_t hkcdShapeType - primitive shape type (1=Cylinder, 3=Box, 4=Capsule, etc.)
    };

    // ============================================================================
    // HAVOK PHYSICS SHAPES
    // ============================================================================

    /**
     * @brief hkpBoxShape - Havok physics box shape object
     * Identified by SHAPE_TYPE_PRIMITIVE == 0x03.
     */
    struct HkpBoxShape {
        static constexpr uintptr_t COLLISION_RADIUS = 0x20;  // float: Base collision radius or padding
        static constexpr uintptr_t HALF_EXTENTS = 0x30;      // hkVector4: half-extents (width/2, depth/2, height/2, padding)
        static constexpr uintptr_t WIDTH_HALF = 0x30;        // float: X half-extent
        static constexpr uintptr_t DEPTH_HALF = 0x34;        // float: Y half-extent
        static constexpr uintptr_t HEIGHT_HALF = 0x38;       // float: Z half-extent
    };

    /**
     * @brief hkpCylinderShape - Havok physics cylinder collision shape
     * Identified by SHAPE_TYPE_PRIMITIVE == 0x01.
     */
    struct HkpCylinderShape {
        static constexpr uintptr_t COLLISION_RADIUS = 0x20;      // float: Base collision radius (often 0.05)
        static constexpr uintptr_t RADIUS = 0x28;               // float: The cylinder's radius
        static constexpr uintptr_t HEIGHT_HALF_FLOAT = 0x3C;    // float: Half-height in meters. For primitive cylinders (ID 0x01)
    };

    /**
     * @brief hkpCapsuleShape - Havok physics capsule collision shape
     * Identified by SHAPE_TYPE_PRIMITIVE == 0x04.
     * Note: A capsule's height is derived from its two endpoint vertices.
     * Further research is needed to find the vertex offsets.
     * The previously noted HEIGHT at 0x3C might be an engine-specific property.
     */
    struct HkpCapsuleShape {
        static constexpr uintptr_t HEIGHT = 0x003C;   // int32_t: Height property (observed values: 4, 5, etc.) - exact meaning/units TBD
    };

    /**
     * @brief hkpSimpleShapePhantom - Havok physics phantom object
     * Contains physics-driven position data. Not a collision shape.
     */
    struct HkpSimpleShapePhantom {
        static constexpr uintptr_t PHYSICS_POSITION = 0x120;  // glm::vec3: Physics position
    };

} // namespace HavokOffsets

