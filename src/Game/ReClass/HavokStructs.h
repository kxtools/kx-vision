#pragma once

#include "../../Utils/DebugLogger.h"
#include "../../Utils/SafeForeignClass.h"
#include "../../Rendering/Shared/LayoutConstants.h"
#include "../HavokEnums.h"
#include "../HavokOffsets.h"
#include <glm.hpp>

namespace kx {
    namespace ReClass {

        // Forward declarations
        class HkpMoppBvTreeShape;
        class HkpExtendedMeshShape;

        // Havok physics dimension validation constants
        namespace HavokValidation {
            // Raw half-extent validation (before conversion to meters)
            constexpr float MIN_HALF_EXTENT_GAME_UNITS = 0.01f;     // Minimum half-extent in game units
            constexpr float MAX_HALF_EXTENT_GAME_UNITS = 10000.0f;  // Maximum half-extent in game units
            
            // Final dimension validation (after conversion to meters)
            constexpr float MIN_DIMENSION_METERS = 0.1f;   // 10cm - minimum reasonable dimension
            constexpr float MAX_DIMENSION_METERS = 100.0f; // 100m - maximum dimension (large bosses/structures)
        }

        /**
         * @brief Havok physics box shape object - contains collision box dimensions
         */
        class HkpBoxShape : public SafeForeignClass {
        public:
            HkpBoxShape(void* ptr) : SafeForeignClass(ptr) {}

            float GetHeightHalf() const {
                if (!data()) {
                    return 0.0f;
                }
                return ReadMemberFast<float>(HavokOffsets::HkpBoxShape::HEIGHT_HALF, 0.0f);
            }

            float GetWidthHalf() const {
                if (!data()) {
                    return 0.0f;
                }
                return ReadMemberFast<float>(HavokOffsets::HkpBoxShape::WIDTH_HALF, 0.0f);
            }

            float GetDepthHalf() const {
                if (!data()) {
                    return 0.0f;
                }
                return ReadMemberFast<float>(HavokOffsets::HkpBoxShape::DEPTH_HALF, 0.0f);
            }

            float GetCollisionRadius() const {
                if (!data()) {
                    return 0.0f;
                }
                return ReadMemberFast<float>(HavokOffsets::HkpBoxShape::COLLISION_RADIUS, 0.0f);
            }

            glm::vec3 GetHalfExtents() const {
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMemberFast<glm::vec3>(HavokOffsets::HkpBoxShape::HALF_EXTENTS, { 0.0f, 0.0f, 0.0f });
            }

            // Get full dimensions (half-extents * 2)
            glm::vec3 GetFullDimensions() const {
                glm::vec3 halfExtents = GetHalfExtents();
                return halfExtents * 2.0f;
            }

            /**
             * @brief Get the primitive shape type identifier from the shape object
             * @return Primitive shape type enum value, or INVALID if read fails
             * @note This reads the single byte at shape + 0x10, which is the actual primitive type
             */
            Havok::HkcdShapeType GetShapeType() const {
                if (!data()) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Read primitive shape type from shape + 0x10 (single byte)
                uint8_t typeValue = 0xFF;
                if (!kx::Debug::SafeRead<uint8_t>(data(), HavokOffsets::HkpShapeBase::SHAPE_TYPE_PRIMITIVE, typeValue)) {
                    return Havok::HkcdShapeType::INVALID;
                }

                return static_cast<Havok::HkcdShapeType>(typeValue);
            }
        };

        /**
         * @brief Havok physics phantom object - contains physics-simulated position
         * TESTED: Physics position updates similarly to Primary - smooth and accurate
         */
        class HkpSimpleShapePhantom : public SafeForeignClass {
        public:
            HkpSimpleShapePhantom(void* ptr) : SafeForeignClass(ptr) {}

            glm::vec3 GetPhysicsPosition() const {
                // TESTED: Updates similarly to Primary position - smooth and accurate
                if (!data()) {
                    return { 0.0f, 0.0f, 0.0f };
                }
                return ReadMemberFast<glm::vec3>(HavokOffsets::HkpSimpleShapePhantom::PHYSICS_POSITION, { 0.0f, 0.0f, 0.0f });
            }
        };

        /**
         * @brief Havok physics cylinder collision shape - contains gadget dimensions
         */
        class HkpCylinderShape : public SafeForeignClass {
        public:
            HkpCylinderShape(void* ptr) : SafeForeignClass(ptr) {}
        };

        /**
         * @brief ReClass wrapper for an hkpMoppBvTreeShape
         * MOPP shapes are acceleration structures that wrap a child shape (typically hkpExtendedMeshShape)
         */
        class HkpMoppBvTreeShape : public SafeForeignClass {
        public:
            HkpMoppBvTreeShape(void* ptr) : SafeForeignClass(ptr) {}

            /**
             * @brief Gets a pointer to the child shape that this MOPP tree wraps
             * @return A void pointer to the child shape (e.g., an hkpExtendedMeshShape), or nullptr if invalid
             */
            void* GetChildShape() const {
                if (!data()) {
                    return nullptr;
                }
                return ReadMemberFast<void*>(HavokOffsets::HkpMoppBvTreeShape::CHILD_SHAPE_POINTER, nullptr);
            }
        };

        /**
         * @brief ReClass wrapper for an hkpExtendedMeshShape
         * These complex mesh shapes cache their own AABB for performance
         */
        class HkpExtendedMeshShape : public SafeForeignClass {
        public:
            HkpExtendedMeshShape(void* ptr) : SafeForeignClass(ptr) {}

            /**
             * @brief Reads the cached AABB half-extents from the shape
             * @return A vec3 containing the half-dimensions in game coordinates (width/2, depth/2, height/2), or glm::vec3(0.0f) if invalid
             * 
             * Reads individual components from the AABB structure:
             * - 0xC0: Width (X component)
             * - 0xC4: Depth (Y component in Havok, maps to Y/depth in game)
             * - 0xC8: Height (Z component in Havok, confirmed by user as height)
             * 
             * Returns in game coordinate system: (width, depth, height) = (X, Y, Z)
             */
            glm::vec3 GetAabbHalfExtents() const {
                if (!data()) {
                    return glm::vec3(0.0f);
                }
                
                // Read individual components from AABB structure
                float widthHalf = ReadMemberFast<float>(HavokOffsets::HkpExtendedMeshShape::AABB_WIDTH_HALF, 0.0f);
                float depthHalf = ReadMemberFast<float>(HavokOffsets::HkpExtendedMeshShape::AABB_DEPTH_HALF, 0.0f);
                float heightHalf = ReadMemberFast<float>(HavokOffsets::HkpExtendedMeshShape::AABB_HEIGHT_HALF, 0.0f);
                
                // Return in game coordinate system: (width, depth, height) = (X, Y, Z)
                return glm::vec3(widthHalf, depthHalf, heightHalf);
            }
        };

        /**
         * @brief Havok physics rigid body - contains physics shape reference
         * 
         * Type-safe shape dimension extraction using primitive shape type byte at shape+0x10.
         * This prevents unsafe casts and reading incorrect fields from wrong shape types.
         */
        class HkpRigidBody : public SafeForeignClass {
        public:
            HkpRigidBody(void* ptr) : SafeForeignClass(ptr) {}

            /**
             * @brief Get the wrapper shape type from the rigid body (for future filtering/early-out)
             * @return Wrapper type enum value, or INVALID if read fails
             * @note This reads from HkpRigidBody + 0x4C and can be used for early filtering
             */
            Havok::HkcdShapeType GetShapeTypeWrapper() const {
                if (!data()) {
                    return Havok::HkcdShapeType::INVALID;
                }
                uint8_t typeValue = ReadMemberFast<uint8_t>(HavokOffsets::HkpRigidBody::SHAPE_TYPE_WRAPPER, 0xFF);
                return static_cast<Havok::HkcdShapeType>(typeValue);
            }

            /**
             * @brief Get the primitive shape type identifier from the shape object
             * @return Primitive shape type enum value, or INVALID if read fails
             * @note This reads the single byte at shape + 0x10, which is the actual primitive type
             */
            Havok::HkcdShapeType GetShapeType() const {
                if (!data()) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Read shape pointer at +0x20
                void* shapePtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(data(), HavokOffsets::HkpRigidBody::SHAPE, shapePtr)) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Validate shape pointer (null check only - SafeRead handles memory safety)
                if (!shapePtr) {
                    return Havok::HkcdShapeType::INVALID;
                }

                // Read primitive shape type from shape + 0x10 (single byte)
                uint8_t typeValue = 0xFF;
                if (!kx::Debug::SafeRead<uint8_t>(shapePtr, HavokOffsets::HkpShapeBase::SHAPE_TYPE_PRIMITIVE, typeValue)) {
                    return Havok::HkcdShapeType::INVALID;
                }

                return static_cast<Havok::HkcdShapeType>(typeValue);
            }

        private:
            // Helper: Read int32 height in centimeters, convert to meters
            float ReadInt32HeightCm(void* shapePtr, uintptr_t offset, int32_t minCm, int32_t maxCm) const {
                int32_t heightCm = 0;
                if (!kx::Debug::SafeRead<int32_t>(shapePtr, offset, heightCm)) {
                    return -1.0f;
                }
                
                if (heightCm < minCm || heightCm > maxCm) {
                    return -1.0f;
                }
                
                return heightCm / 100.0f;
            }

            // Helper: Read float half-extent in game coordinates, convert to full height in meters
            float ReadFloatHeightHalfExtent(void* shapePtr, uintptr_t offset) const {
                float heightHalf = 0.0f;
                if (!kx::Debug::SafeRead<float>(shapePtr, offset, heightHalf)) {
                    return -1.0f;
                }
                
                if (!std::isfinite(heightHalf) || heightHalf <= 0.0f || heightHalf > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS) {
                    return -1.0f;
                }
                
                float fullHeightMeters = (heightHalf * 2.0f) / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR;
                
                if (fullHeightMeters < HavokValidation::MIN_DIMENSION_METERS || fullHeightMeters > HavokValidation::MAX_DIMENSION_METERS) {
                    return -1.0f;
                }
                
                return fullHeightMeters;
            }

            // Helper: Read float half-height in meters, convert to full height (no coordinate conversion)
            float ReadFloatHeightHalfMeters(void* shapePtr, uintptr_t offset) const {
                float heightHalf = 0.0f;
                if (!kx::Debug::SafeRead<float>(shapePtr, offset, heightHalf)) {
                    return -1.0f;
                }
                
                if (!std::isfinite(heightHalf) || heightHalf <= 0.0f || heightHalf > HavokValidation::MAX_DIMENSION_METERS / 2.0f) {
                    return -1.0f;
                }
                
                float fullHeightMeters = heightHalf * 2.0f;
                
                if (fullHeightMeters < HavokValidation::MIN_DIMENSION_METERS || fullHeightMeters > HavokValidation::MAX_DIMENSION_METERS) {
                    return -1.0f;
                }
                
                return fullHeightMeters;
            }

            // Helper: Read int32 height directly (no conversion)
            float ReadInt32HeightDirect(void* shapePtr, uintptr_t offset, int32_t min, int32_t max) const {
                int32_t height = 0;
                if (!kx::Debug::SafeRead<int32_t>(shapePtr, offset, height)) {
                    return -1.0f;
                }
                
                if (height < min || height > max) {
                    return -1.0f;
                }
                
                return static_cast<float>(height);
            }

            // Helper: Read vec3 half-extents, validate, and return full extents
            glm::vec3 ReadBoxHalfExtents(void* shapePtr, uintptr_t offset) const {
                glm::vec3 halfExtents(0.0f);
                if (!kx::Debug::SafeRead<glm::vec3>(shapePtr, offset, halfExtents)) {
                    return glm::vec3(0.0f);
                }
                
                if (!std::isfinite(halfExtents.x) || !std::isfinite(halfExtents.y) || !std::isfinite(halfExtents.z)) {
                    return glm::vec3(0.0f);
                }
                
                if (halfExtents.x <= 0.0f || halfExtents.y <= 0.0f || halfExtents.z <= 0.0f) {
                    return glm::vec3(0.0f);
                }
                
                if (halfExtents.x > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS || 
                    halfExtents.y > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS || 
                    halfExtents.z > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS) {
                    return glm::vec3(0.0f);
                }
                
                glm::vec3 fullExtents = halfExtents * 2.0f;
                fullExtents = fullExtents / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR;
                
                if (fullExtents.x < HavokValidation::MIN_DIMENSION_METERS || fullExtents.x > HavokValidation::MAX_DIMENSION_METERS ||
                    fullExtents.y < HavokValidation::MIN_DIMENSION_METERS || fullExtents.y > HavokValidation::MAX_DIMENSION_METERS ||
                    fullExtents.z < HavokValidation::MIN_DIMENSION_METERS || fullExtents.z > HavokValidation::MAX_DIMENSION_METERS) {
                    return glm::vec3(0.0f);
                }
                
                return fullExtents;
            }

            // Helper: Read cylinder half-height, return height only (width/depth should be derived using WIDTH_TO_HEIGHT_RATIO)
            // Note: GW2 uses the same generic cylinder object everywhere, so all cylinders will be the same size.
            // This means only height information is available from the shape, and width/depth must be derived proportionally.
            glm::vec3 ReadCylinderDimensions(void* shapePtr, uintptr_t heightOffset) const {
                float halfHeight = 0.0f;
                if (!kx::Debug::SafeRead<float>(shapePtr, heightOffset, halfHeight)) {
                    return glm::vec3(0.0f);
                }
                
                if (!std::isfinite(halfHeight) || halfHeight <= 0.0f || halfHeight > HavokValidation::MAX_DIMENSION_METERS / 2.0f) {
                    return glm::vec3(0.0f);
                }
                
                float fullHeight = halfHeight * 2.0f;
                
                if (fullHeight < HavokValidation::MIN_DIMENSION_METERS || fullHeight > HavokValidation::MAX_DIMENSION_METERS) {
                    return glm::vec3(0.0f);
                }
                
                // Return height only - width and depth should be derived using WIDTH_TO_HEIGHT_RATIO in the extractor
                return glm::vec3(0.0f, fullHeight, 0.0f);
            }

            // Helper: Read MOPP shape dimensions by extracting AABB from child shape
            glm::vec3 ReadMoppDimensions(void* moppShapePtr) const {
                // Get the child shape from the MOPP
                HkpMoppBvTreeShape moppShape(moppShapePtr);
                void* childShapePtr = moppShape.GetChildShape();
                if (!childShapePtr) {
                    return glm::vec3(0.0f);
                }
                
                // Read AABB half-extents from the child shape (typically hkpExtendedMeshShape)
                // GetAabbHalfExtents() returns (width, depth, height) in game coordinates
                HkpExtendedMeshShape childShape(childShapePtr);
                glm::vec3 halfExtents = childShape.GetAabbHalfExtents();
                
                // Validate half-extents
                if (halfExtents.x == 0.0f && halfExtents.y == 0.0f && halfExtents.z == 0.0f) {
                    return glm::vec3(0.0f);
                }
                
                if (!std::isfinite(halfExtents.x) || !std::isfinite(halfExtents.y) || !std::isfinite(halfExtents.z)) {
                    return glm::vec3(0.0f);
                }
                
                if (halfExtents.x <= 0.0f || halfExtents.y <= 0.0f || halfExtents.z <= 0.0f) {
                    return glm::vec3(0.0f);
                }
                
                if (halfExtents.x > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS || 
                    halfExtents.y > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS || 
                    halfExtents.z > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS) {
                    return glm::vec3(0.0f);
                }
                
                // Convert half-extents to full extents
                glm::vec3 fullExtents = halfExtents * 2.0f;
                
                // Convert from game coordinates to meters (same as ReadBoxHalfExtents)
                fullExtents = fullExtents / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR;
                
                if (fullExtents.x < HavokValidation::MIN_DIMENSION_METERS || fullExtents.x > HavokValidation::MAX_DIMENSION_METERS ||
                    fullExtents.y < HavokValidation::MIN_DIMENSION_METERS || fullExtents.y > HavokValidation::MAX_DIMENSION_METERS ||
                    fullExtents.z < HavokValidation::MIN_DIMENSION_METERS || fullExtents.z > HavokValidation::MAX_DIMENSION_METERS) {
                    return glm::vec3(0.0f);
                }
                
                // Map from (width, depth, height) to (width, height, depth) for TryGetDimensions() API
                // halfExtents is (width, depth, height), so fullExtents is also (width, depth, height)
                // Return (width, height, depth) by swapping Y and Z components
                return glm::vec3(fullExtents.x, fullExtents.z, fullExtents.y);
            }

            // Helper: Read list shape dimensions from its cached bounding box half-extents
            glm::vec3 ReadListShapeDimensions(void* shapePtr) const {
                // Read width and depth from vec3 at 0x50
                float widthHalf = 0.0f;
                float depthHalf = 0.0f;
                if (!kx::Debug::SafeRead<float>(shapePtr, HavokOffsets::HkpListShape::WIDTH_HALF, widthHalf) ||
                    !kx::Debug::SafeRead<float>(shapePtr, HavokOffsets::HkpListShape::DEPTH_HALF, depthHalf)) {
                    return glm::vec3(0.0f);
                }
                
                // Read primary height from 0x58
                float heightHalf = 0.0f;
                bool heightValid = kx::Debug::SafeRead<float>(shapePtr, HavokOffsets::HkpListShape::HEIGHT_HALF, heightHalf);
                
                // If primary height is invalid, try backup height from 0x68
                if (!heightValid || !std::isfinite(heightHalf) || heightHalf <= 0.0f || heightHalf > 10000.0f) {
                    if (!kx::Debug::SafeRead<float>(shapePtr, HavokOffsets::HkpListShape::HEIGHT_HALF_BACKUP, heightHalf)) {
                        return glm::vec3(0.0f);
                    }
                }
                
                // Validate all components
                if (!std::isfinite(widthHalf) || !std::isfinite(depthHalf) || !std::isfinite(heightHalf)) {
                    return glm::vec3(0.0f);
                }
                
                if (widthHalf <= 0.0f || depthHalf <= 0.0f || heightHalf <= 0.0f) {
                    return glm::vec3(0.0f);
                }
                
                if (widthHalf > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS || 
                    depthHalf > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS || 
                    heightHalf > HavokValidation::MAX_HALF_EXTENT_GAME_UNITS) {
                    return glm::vec3(0.0f);
                }
                
                glm::vec3 halfExtents(widthHalf, depthHalf, heightHalf);
                
                // Convert to full dimensions and scale to meters
                glm::vec3 fullExtents = halfExtents * 2.0f;
                fullExtents = fullExtents / CoordinateTransform::GAME_TO_MUMBLE_SCALE_FACTOR;
                
                // Final validation on scaled values
                if (fullExtents.x < HavokValidation::MIN_DIMENSION_METERS || fullExtents.x > HavokValidation::MAX_DIMENSION_METERS ||
                    fullExtents.y < HavokValidation::MIN_DIMENSION_METERS || fullExtents.y > HavokValidation::MAX_DIMENSION_METERS ||
                    fullExtents.z < HavokValidation::MIN_DIMENSION_METERS || fullExtents.z > HavokValidation::MAX_DIMENSION_METERS) {
                    return glm::vec3(0.0f);
                }
                
                // Map from Havok (width, depth, height) to API (width, height, depth) by swapping Y and Z
                return glm::vec3(fullExtents.x, fullExtents.z, fullExtents.y);
            }

        public:
            /**
             * @brief Type-safe dimension extraction from rigid body shape
             * @return Full dimensions as glm::vec3 (width, height, depth) in meters, or glm::vec3(0.0f) if data is invalid
             * 
             * Supports CYLINDER, BOX, MOPP, and LIST shapes.
             * - CYLINDER: GW2 uses the same generic cylinder object everywhere, so all cylinders will be the same size.
             *   Only height information is available from the shape; width/depth are derived using WIDTH_TO_HEIGHT_RATIO.
             * - BOX: Extracts dimensions directly from the box shape's half-extents.
             * - MOPP: Extracts dimensions from the child shape's cached AABB.
             * - LIST: Extracts dimensions from the list shape's cached bounding box half-extents. Uses backup height at 0x68 if primary height at 0x58 is invalid.
             * - Unknown/unsupported shape types: Returns a small default box (0.5m × 0.5m × 0.5m) to ensure visibility.
             * 
             * Returns glm::vec3(0.0f) only when data is invalid (null pointers, read failures, or validation failures).
             * Unknown shape types return a small default box instead of zero to ensure they remain visible.
             */
            glm::vec3 TryGetDimensions() const {
                if (!data()) {
                    return glm::vec3(0.0f);
                }

                // Read shape pointer at +0x20
                void* shapePtr = nullptr;
                if (!kx::Debug::SafeRead<void*>(data(), HavokOffsets::HkpRigidBody::SHAPE, shapePtr)) {
                    return glm::vec3(0.0f);
                }

                // Validate shape pointer (null check only - SafeRead handles memory safety)
                if (!shapePtr) {
                    return glm::vec3(0.0f);
                }

                // Read primitive shape type from shape + 0x10 (single byte)
                Havok::HkcdShapeType shapeType = GetShapeType();
                if (shapeType == Havok::HkcdShapeType::INVALID) {
                    return glm::vec3(0.0f);
                }

                // Switch on primitive shape type and extract dimensions from appropriate fields
                switch (shapeType) {
                    case Havok::HkcdShapeType::CYLINDER:
                        return ReadCylinderDimensions(shapePtr, 
                            HavokOffsets::HkpCylinderShape::HEIGHT_HALF_FLOAT);
                    
                    case Havok::HkcdShapeType::BOX:
                        return ReadBoxHalfExtents(shapePtr, HavokOffsets::HkpBoxShape::HALF_EXTENTS);
                    
                    case Havok::HkcdShapeType::MOPP:
                        return ReadMoppDimensions(shapePtr);
                    
                    case Havok::HkcdShapeType::LIST:
                        return ReadListShapeDimensions(shapePtr);
                    
                    default:
                        // Unknown/unsupported shape types: return a small default box (0.5m cube)
                        // This ensures unknown shapes are still visible with a reasonable bounding box
                        // Matches the gadget fallback size for visual consistency
                        return glm::vec3(
                            EntityWorldBounds::GADGET_WORLD_WIDTH,
                            EntityWorldBounds::GADGET_WORLD_HEIGHT,
                            EntityWorldBounds::GADGET_WORLD_DEPTH
                        );
                }
            }

        };

    } // namespace ReClass
} // namespace kx

