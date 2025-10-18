#include "Camera.h"
#include "MumbleLinkManager.h"

#include <iostream>
#include <string>
#include <windows.h>

namespace kx {

    Camera::Camera() {
        m_viewMatrix = glm::mat4(1.0f);
        m_projectionMatrix = glm::mat4(1.0f);
        m_camPos = glm::vec3(0.0f);
        m_playerPosition = glm::vec3(0.0f);
    }

    Camera::~Camera() {
        // No longer responsible for MumbleLink cleanup
    }

    void Camera::Update(const MumbleLinkManager& mumbleManager, HWND hWnd) {
        const MumbleLinkData* mumbleData = mumbleManager.GetData();
        if (!mumbleData) {
            // If there's no data, don't update the matrices.
            // They will retain their last valid state.
            return;
        }

        // Get camera position from MumbleLink (already in Y-up)
        m_camPos = glm::vec3(
            mumbleData->fCameraPosition[0],
            mumbleData->fCameraPosition[1],
            mumbleData->fCameraPosition[2]
        );

        // Get player position from MumbleLink (already in Y-up)
        m_playerPosition = glm::vec3(
            mumbleData->fAvatarPosition[0],
            mumbleData->fAvatarPosition[1],
            mumbleData->fAvatarPosition[2]
        );

        // Get camera direction from MumbleLink
        glm::vec3 camFront = glm::vec3(
            mumbleData->fCameraFront[0],
            mumbleData->fCameraFront[1],
            mumbleData->fCameraFront[2]
        );

        // Get window dimensions for aspect ratio
        RECT rect;
        float screenWidth = 1920.0f;  // Default values in case GetClientRect fails
        float screenHeight = 1080.0f;
        if (hWnd && GetClientRect(hWnd, &rect)) {
            screenWidth = static_cast<float>(rect.right - rect.left);
            screenHeight = static_cast<float>(rect.bottom - rect.top);
        }

        // Get FOV from MumbleLinkManager (already parsed from identity data)
        float fov_radians = mumbleManager.GetFovOrDefault();

        // Calculate view matrix - manually implementing a left-handed lookAt
        glm::vec3 target = m_camPos + camFront;
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Y is up in GW2's world space

        // Create view vectors
        glm::vec3 zaxis = glm::normalize(target - m_camPos); // Forward (positive Z in left-handed)
        glm::vec3 xaxis = glm::normalize(glm::cross(worldUp, zaxis)); // Right
        glm::vec3 yaxis = glm::cross(zaxis, xaxis); // Up

        // Build view matrix explicitly for left-handed system
        m_viewMatrix = glm::mat4(1.0f);
        m_viewMatrix[0][0] = xaxis.x;
        m_viewMatrix[0][1] = yaxis.x;
        m_viewMatrix[0][2] = zaxis.x;
        m_viewMatrix[1][0] = xaxis.y;
        m_viewMatrix[1][1] = yaxis.y;
        m_viewMatrix[1][2] = zaxis.y;
        m_viewMatrix[2][0] = xaxis.z;
        m_viewMatrix[2][1] = yaxis.z;
        m_viewMatrix[2][2] = zaxis.z;
        m_viewMatrix[3][0] = -glm::dot(xaxis, m_camPos);
        m_viewMatrix[3][1] = -glm::dot(yaxis, m_camPos);
        m_viewMatrix[3][2] = -glm::dot(zaxis, m_camPos);

        // Define perspective projection parameters
        float zNear = 0.1f;
        float zFar = 30000.0f;
        float aspect = screenWidth / screenHeight;

        // Build projection matrix explicitly for DirectX-style left-handed projection
        m_projectionMatrix = glm::mat4(0.0f);
        m_projectionMatrix[0][0] = 1.0f / (aspect * tanf(fov_radians / 2.0f));
        m_projectionMatrix[1][1] = 1.0f / tanf(fov_radians / 2.0f);
        m_projectionMatrix[2][2] = zFar / (zFar - zNear);
        m_projectionMatrix[2][3] = 1.0f;
        m_projectionMatrix[3][2] = -(zFar * zNear) / (zFar - zNear);
    }

} // namespace kx