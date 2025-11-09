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
        static constexpr uintptr_t WORLD_POINTER = 0x10;  // hkpWorld* pointer to the physics world this rigid body belongs to
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
        static constexpr uintptr_t RADIUS = 0x28;               // float: The cylinder's radius
        static constexpr uintptr_t HEIGHT_HALF_FLOAT = 0x3C;    // float: Half-height in meters. For primitive cylinders (ID 0x01)
    };

    /**
     * @brief hkpMoppBvTreeShape - Havok MOPP shape (BvTree)
     * Identified by SHAPE_TYPE_PRIMITIVE == 0x09.
     * This is an acceleration structure that wraps a child shape (e.g., a mesh).
     * To get its dimensions, you must get the child shape and find its AABB.
     */
    struct HkpMoppBvTreeShape {
        static constexpr uintptr_t CODE = 0x28;                    // hkpMoppCode*: Pointer to the compressed tree data
        static constexpr uintptr_t CHILD_SHAPE_POINTER = 0x58;    // hkpShape*: Pointer to the child shape (typically hkpExtendedMeshShape)
    };

    /**
     * @brief hkpExtendedMeshShape - A complex mesh shape, often the child of a MOPP
     * Identified by SHAPE_TYPE_PRIMITIVE == 0x0D.
     * These shapes cache their own Axis-Aligned Bounding Box (AABB) for performance.
     * The AABB is stored as an hkVector4 starting at 0xC0.
     */
    struct HkpExtendedMeshShape {
        static constexpr uintptr_t AABB_HALF_EXTENTS = 0xC0;  // hkVector4: Cached AABB half-extents (width/2, height/2, depth/2, padding)
        static constexpr uintptr_t AABB_WIDTH_HALF = 0xC0;    // float: X component (width/2)
        static constexpr uintptr_t AABB_DEPTH_HALF = 0xC4;   // float: Y component (depth/2 in Havok system)
        static constexpr uintptr_t AABB_HEIGHT_HALF = 0xC8;   // float: Z component (height/2 - confirmed this is height)
    };

    /**
     * @brief hkpSimpleShapePhantom - Havok physics phantom object
     * Contains physics-driven position data. Not a collision shape.
     */
    struct HkpSimpleShapePhantom {
        static constexpr uintptr_t PHYSICS_POSITION = 0x120;  // glm::vec3: Physics position
    };

    // ============================================================================
    // HAVOK PHYSICS WORLD AND BROADPHASE
    // ============================================================================

    /**
     * @brief hkpWorld - The main physics world object
     * Contains the broadphase border that manages world boundary phantoms
     */
    struct HkpWorld {
        static constexpr uintptr_t BROAD_PHASE_BORDER = 0x188;  // hkpBroadPhaseBorder* pointer to the object managing world boundary phantoms
    };

    /**
     * @brief hkpBroadPhaseBorder - Manages the 6 "wall" phantoms that define world boundaries
     * Contains an array of 6 hkpAabbPhantom pointers representing the world walls
     */
    struct HkpBroadPhaseBorder {
        static constexpr uintptr_t PHANTOM_ARRAY = 0x0;  // hkpPhantom*[6]: Array of 6 phantom pointers (world boundary walls)
    };

    /**
     * @brief hkpAabbPhantom - A phantom shape defined by a floating-point AABB
     * Used for world boundary walls and other non-colliding phantom objects
     */
    struct HkpAabbPhantom {
        static constexpr uintptr_t AABB_MIN = 0xF0;   // hkVector4: The minimum corner of the AABB
        static constexpr uintptr_t AABB_MAX = 0x100;  // hkVector4: The maximum corner of the AABB
    };

} // namespace HavokOffsets

