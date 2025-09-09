#include "ESP_Helpers.h"
#include "gtc/matrix_transform.hpp" 
#include "gtc/type_ptr.hpp" // Required for glm::value_ptr

namespace kx {

    namespace ESP_Helpers {

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

    } // namespace ESP_Helpers

} // namespace kx