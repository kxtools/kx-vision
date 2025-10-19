/**
 * @file D3DRenderHook_WndProc.cpp
 * @brief Window procedure hook for handling input in both DLL and GW2AL modes
 * 
 * This WndProc implementation provides:
 * - Production-ready input handling
 * - Camera rotation support (works even when ImGui menu is open)
 * - Focus loss handling (clears stuck keys on alt-tab)
 * - Selective ImGui handler calling (performance optimization)
 * - All edge cases handled (horizontal wheel, bounds checking, etc.)
 */

#include "D3DRenderHook.h"
#include <windowsx.h>
#include "../Core/AppState.h"
#include "../Utils/DebugLogger.h"
#include "../../libs/ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace kx::Hooking {

    LRESULT __stdcall D3DRenderHook::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // Track mouse button states
        if (uMsg == WM_RBUTTONDOWN) m_rightMouseDown = true;
        else if (uMsg == WM_RBUTTONUP) m_rightMouseDown = false;
        else if (uMsg == WM_LBUTTONDOWN) m_leftMouseDown = true;
        else if (uMsg == WM_LBUTTONUP) m_leftMouseDown = false;
        
        // Handle focus loss - clear all input states (like Nexus/GW2Common)
        if (uMsg == WM_KILLFOCUS || uMsg == WM_ACTIVATEAPP) {
            if (m_isInit) {
                ImGuiIO& io = ImGui::GetIO();
                // Clear all mouse buttons
                for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
                    io.MouseDown[i] = false;
                }
                // Clear all keyboard keys (newer ImGui versions use KeysData)
                for (int i = 0; i < IM_ARRAYSIZE(io.KeysData); i++) {
                    io.KeysData[i].Down = false;
                }
                ImGui::ClearActiveID();
            }
            m_rightMouseDown = false;
            m_leftMouseDown = false;
        }

        // Only process ImGui input when initialized and window is open
        if (m_isInit && AppState::Get().IsVisionWindowOpen()) {
            // For mouse button events, only call ImGui handler if we're over ImGui or an item is active
            bool isMouseButtonEvent = (uMsg >= WM_LBUTTONDOWN && uMsg <= WM_XBUTTONDBLCLK);
            bool shouldCallImGuiHandler = !isMouseButtonEvent || 
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) ||
                ImGui::IsAnyItemActive();
            
            if (shouldCallImGuiHandler) {
                ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
            }
            
            // Get ImGui IO state AFTER calling handler (if we called it)
            ImGuiIO& io = ImGui::GetIO();
            
            // Update hover tracking for mouse move
            if (uMsg == WM_MOUSEMOVE) {
                m_wasOverImGuiWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            }
            
            // For mouse button events, use selective blocking
            switch (uMsg) {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_XBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDBLCLK:
                case WM_XBUTTONDBLCLK:
                    // Only block if we called ImGui handler AND ImGui wants the mouse
                    if (shouldCallImGuiHandler && io.WantCaptureMouse) {
                        return 1;
                    }
                    else {
                        // CRITICAL: Clear any active ImGui items when clicking outside
                        ImGui::ClearActiveID();
                    }
                    break;
                    
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_XBUTTONUP:
                    // NEVER block mouse UP - always let them pass through (like Nexus does)
                    break;
                    
                case WM_MOUSEWHEEL:
                case WM_MOUSEHWHEEL:
                    // Always call handler for mouse wheel, then check if ImGui wants it
                    if (!shouldCallImGuiHandler) {
                        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    }
                    if (io.WantCaptureMouse) {
                        return 1;
                    }
                    break;
                    
                case WM_MOUSEMOVE:
                    // Never block mouse move
                    break;
                    
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                    // Bounds check wParam (like Nexus does)
                    if (wParam >= 256) {
                        break;
                    }
                    // Always call handler for keyboard events
                    if (!shouldCallImGuiHandler) {
                        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    }
                    if (io.WantCaptureKeyboard) {
                        return 1;
                    }
                    break;
                    
                case WM_CHAR:
                    // Always call handler for char events
                    if (!shouldCallImGuiHandler) {
                        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    }
                    if (io.WantTextInput) {
                        return 1;
                    }
                    break;
            }
        }

        // Pass to original game window procedure
        return m_pOriginalWndProc ? CallWindowProc(m_pOriginalWndProc, hWnd, uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

} // namespace kx::Hooking
