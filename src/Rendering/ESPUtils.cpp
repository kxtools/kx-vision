#include "ESPUtils.h"

#include "gtc/type_ptr.hpp" // Required for glm::value_ptr
#include <algorithm> // For std::max, std::min

namespace kx {

    namespace ESPUtils {

        bool WorldToScreen(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight, glm::vec2& outScreenPos) {
            // Get matrices
            const glm::mat4& view = camera.GetViewMatrix();
            const glm::mat4& proj = camera.GetProjectionMatrix();

            // Define viewport manually (required for project function)
            glm::vec4 viewport = glm::vec4(0.0f, 0.0f, screenWidth, screenHeight);

            // Calculate the clip-space position
            glm::vec4 clipPos = proj * view * glm::vec4(worldPos, 1.0f);

            // Check if the point is behind the camera
            if (clipPos.w <= 0.0f) {
                return false;
            }

            // Complete the perspective division
            clipPos /= clipPos.w;

            // Check if point is within visible range [-1,1]
            if (clipPos.x < -1.0f || clipPos.x > 1.0f ||
                clipPos.y < -1.0f || clipPos.y > 1.0f ||
                clipPos.z < 0.0f || clipPos.z > 1.0f) {
                return false;
            }

            // Convert to screen coordinates
            outScreenPos.x = viewport[0] + viewport[2] * (clipPos.x + 1.0f) * 0.5f;
            outScreenPos.y = viewport[1] + viewport[3] * (1.0f - (clipPos.y + 1.0f) * 0.5f); // Flip Y for screen coordinates

            return true;
        }

        PlayerESPData GetPlayerESPData(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight) {
            PlayerESPData data;
            
            // Set 3D positions
            data.feetPos = worldPos;
            
            // Project feet position
            if (!WorldToScreen(data.feetPos, camera, screenWidth, screenHeight, data.feet)) {
                data.valid = false;
                return data;
            }
            
            // Calculate distance from camera to entity for stable sizing
            glm::vec3 cameraPos = camera.GetCameraPosition();
            float distance = glm::length(worldPos - cameraPos);
            
            // Use distance-based sizing for stability (like many professional ESP systems)
            float baseHeight = 40.0f; // Base height in pixels for players
            float rawScale = 100.0f / (distance + 10.0f);
            float minScale = (rawScale > 0.3f) ? rawScale : 0.3f;
            float scaleFactor = (minScale < 2.0f) ? minScale : 2.0f;
            data.height = baseHeight * scaleFactor;
            data.width = data.height * 0.5f; // 0.5 ratio for human-like entities
            
            // Ensure minimum size
            if (data.height < 20.0f) {
                data.height = 20.0f;
                data.width = 10.0f;
            }
            
            // Calculate bounding box for players
            // min = upper-left corner, max = lower-right corner
            data.min = glm::vec2(data.feet.x - data.width * 0.5f, data.feet.y - data.height);
            data.max = glm::vec2(data.feet.x + data.width * 0.5f, data.feet.y);
            
            data.valid = true;
            return data;
        }

        PlayerESPData GetNpcESPData(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight) {
            PlayerESPData data;
            
            // Set 3D positions
            data.feetPos = worldPos;
            
            // Project feet position
            if (!WorldToScreen(data.feetPos, camera, screenWidth, screenHeight, data.feet)) {
                data.valid = false;
                return data;
            }
            
            // Calculate distance from camera to entity for stable sizing
            glm::vec3 cameraPos = camera.GetCameraPosition();
            float distance = glm::length(worldPos - cameraPos);
            
            // Use distance-based sizing for NPCs (square boxes for various creature types)
            float baseSize = 30.0f; // Base size in pixels for NPCs (between player height and gadget size)
            float rawScale = 80.0f / (distance + 10.0f);
            float minScale = (rawScale > 0.3f) ? rawScale : 0.3f;
            float scaleFactor = (minScale < 2.0f) ? minScale : 2.0f;
            float size = baseSize * scaleFactor;
            
            // Square boxes for NPCs (works for humanoids, animals, monsters, etc.)
            data.height = size;
            data.width = size; // 1:1 ratio for NPCs
            
            // Ensure minimum size
            if (data.height < 15.0f) {
                data.height = 15.0f;
                data.width = 15.0f;
            }
            
            // Calculate bounding box for NPCs (square, anchored at feet)
            // min = upper-left corner, max = lower-right corner
            data.min = glm::vec2(data.feet.x - data.width * 0.5f, data.feet.y - data.height);
            data.max = glm::vec2(data.feet.x + data.width * 0.5f, data.feet.y);
            
            data.valid = true;
            return data;
        }

        GadgetESPData GetGadgetESPData(const glm::vec3& worldPos, const Camera& camera, float screenWidth, float screenHeight) {
            GadgetESPData data;
            
            // For gadgets, use a more stable approach
            // Instead of calculating head/feet, use distance-based sizing like many ESP systems
            data.feetPos = worldPos;
            
            // Project feet position
            if (!WorldToScreen(data.feetPos, camera, screenWidth, screenHeight, data.feet)) {
                data.valid = false;
                return data;
            }
            
            // Calculate distance from camera to entity for stable sizing
            glm::vec3 cameraPos = camera.GetCameraPosition();
            float distance = glm::length(worldPos - cameraPos);
            
            // Use distance-based sizing for stability (common in professional ESP systems)
            float baseSize = 8.0f; // Base size in pixels
            float rawScale = 50.0f / (distance + 10.0f);
            float minScale = (rawScale > 0.5f) ? rawScale : 0.5f;
            float scaleFactor = (minScale < 2.0f) ? minScale : 2.0f;
            data.height = baseSize * scaleFactor;
            data.width = data.height; // Square for gadgets
            
            // Calculate bounding box for gadgets using center-based approach
            float halfWidth = data.width * 0.5f;
            float halfHeight = data.height * 0.5f;
            data.min = glm::vec2(data.feet.x - halfWidth, data.feet.y - halfHeight);
            data.max = glm::vec2(data.feet.x + halfWidth, data.feet.y + halfHeight);
            
            data.valid = true;
            return data;
        }

        float CalculateScreenDistance(const glm::vec2& p1, const glm::vec2& p2) {
            float dx = p1.x - p2.x;
            float dy = p1.y - p2.y;
            return sqrtf(dx * dx + dy * dy);
        }

    } // namespace ESPUtils

} // namespace kx