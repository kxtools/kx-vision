/**
 * @file D3DRenderHook_GW2AL.cpp
 * @brief GW2AL addon mode specific functionality
 *
 * This file contains GW2AL-mode specific code:
 * - Simple WndProc that relies on GW2AL's input routing
 * - No Present hook (GW2AL provides the device directly)
 *
 * This file is only compiled when GW2AL_BUILD is defined.
 */

#include "D3DRenderHook.h"

#include "Config.h"
#ifdef GW2AL_BUILD // Only compile this file in GW2AL mode

#include "../Core/AppState.h"
#include "../../libs/ImGui/imgui.h"

 // Declare the external ImGui Win32 handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace kx::Hooking {

    LRESULT __stdcall D3DRenderHook::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // Give ImGui first chance at input only when our window is open
        if (m_isInit && kx::AppState::Get().IsVisionWindowOpen()) {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
                ImGuiIO& io = ImGui::GetIO();
                // Only block input when ImGui explicitly needs it
                if ((io.WantCaptureMouse && (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)) ||
                    (io.WantCaptureKeyboard && (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST))) {
                    return 1;
                }
            }
        }

        // Pass to original game window procedure
        return m_pOriginalWndProc ? CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

} // namespace kx::Hooking

#endif // GW2AL_BUILD
