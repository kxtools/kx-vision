#include "Camera.h"
#include "D3DRenderHook.h" // For GetWindowHandle()
#include <windows.h>
#include <iostream>
#include <string>
#include <cmath> // For tanf

namespace kx {

    float ParseFov(const wchar_t* identity) {
        constexpr float defaultFov = 1.0472f; // ~60 degrees
        if (!identity) return defaultFov;
        std::wstring identityStr(identity);
        size_t fovPos = identityStr.find(L"\"fov\":");
        if (fovPos != std::wstring::npos) {
            try {
                size_t start = identityStr.find(L':', fovPos) + 1;
                size_t end = identityStr.find_first_of(L",}", start);
                float fov = std::stof(identityStr.substr(start, end - start));
                return (fov > 0.01f) ? fov : defaultFov;
            }
            catch (...) { return defaultFov; }
        }
        return defaultFov;
    }

    Camera::Camera() {
        m_viewMatrix = glm::mat4(1.0f);
        m_projectionMatrix = glm::mat4(1.0f);
        m_camPos = glm::vec3(0.0f);
        m_playerPosition = glm::vec3(0.0f);
    }

    Camera::~Camera() {
        if (m_mumbleLink) UnmapViewOfFile(m_mumbleLink);
        if (m_mumbleLinkFile) CloseHandle(m_mumbleLinkFile);
    }

    bool Camera::InitializeMumbleLink() {
        // Use CreateFileMappingW to create or open the named file mapping.
        // This is equivalent to .NET's MemoryMappedFile.CreateOrOpen.
        // If the object already exists, it opens it.
        m_mumbleLinkFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE, // Use INVALID_HANDLE_VALUE for a file-backed mapping
            NULL,                 // Default security attributes
            PAGE_READWRITE,       // Read/write access
            0,                    // High-order DWORD of maximum size
            sizeof(MumbleLinkData), // Low-order DWORD of maximum size
            L"MumbleLink"         // Name of the mapping object
        );

        if (m_mumbleLinkFile == NULL) {
            // Failed to create or open file mapping
            return false;
        }

        // Check if the file mapping object was just created (not opened existing)
        bool wasCreated = (GetLastError() == 0);

        // Map a view of the file mapping into the address space of the current process.
        m_mumbleLink = static_cast<MumbleLinkData*>(MapViewOfFile(
            m_mumbleLinkFile,     // Handle to file mapping object
            FILE_MAP_READ,        // Read access
            0,                    // High-order DWORD of a file offset
            0,                    // Low-order DWORD of a file offset
            sizeof(MumbleLinkData) // Number of bytes to map
        ));

        if (m_mumbleLink == NULL) {
            // Failed to map view of file
            CloseHandle(m_mumbleLinkFile);
            m_mumbleLinkFile = nullptr;
            return false;
        }

        m_mumbleLinkInitialized = true;
        return true;
    }

    void Camera::Update() {
        if (!m_mumbleLinkInitialized) {
            auto now = std::chrono::steady_clock::now();
            if (now - m_lastMumbleRetryTime >= MumbleRetryInterval) {
                m_lastMumbleRetryTime = now; // Update last retry time
                if (!InitializeMumbleLink()) {
                    // Initialization failed, will retry after interval
                    return;
                }
            } else {
                // Not time to retry yet, skip update for now
                return;
            }
        }

        // Check if MumbleLink data is valid and updated
        // 1. Check uiVersion (should be 2 for GW2)
        // 2. Check game name
        // 3. Check uiTick to see if data has changed
        if (m_mumbleLink->uiVersion != 2 ||
            std::wcscmp(m_mumbleLink->name, GW2_GAME_NAME) != 0 ||
            m_mumbleLink->uiTick == m_lastTick) {
            return; // Data is invalid or not updated
        }

        m_lastTick = m_mumbleLink->uiTick; // Update last tick

        // Get camera position from MumbleLink (already in Y-up)
        m_camPos = glm::vec3(
            m_mumbleLink->fCameraPosition[0],
            m_mumbleLink->fCameraPosition[1],
            m_mumbleLink->fCameraPosition[2]
        );

        // Get player position from MumbleLink (already in Y-up)
        m_playerPosition = glm::vec3(
            m_mumbleLink->fAvatarPosition[0],
            m_mumbleLink->fAvatarPosition[1],
            m_mumbleLink->fAvatarPosition[2]
        );

        // Get camera direction from MumbleLink
        glm::vec3 camFront = glm::vec3(
            m_mumbleLink->fCameraFront[0],
            m_mumbleLink->fCameraFront[1],
            m_mumbleLink->fCameraFront[2]
        );

        // Get window dimensions for aspect ratio
        RECT rect;
        HWND hWnd = Hooking::D3DRenderHook::GetWindowHandle();
        float screenWidth = 1920.0f;  // Default values in case GetClientRect fails
        float screenHeight = 1080.0f;
        if (hWnd && GetClientRect(hWnd, &rect)) {
            screenWidth = static_cast<float>(rect.right - rect.left);
            screenHeight = static_cast<float>(rect.bottom - rect.top);
        }

        // Parse FOV from MumbleLink identity data
        float fov_radians = ParseFov(m_mumbleLink->identity);

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