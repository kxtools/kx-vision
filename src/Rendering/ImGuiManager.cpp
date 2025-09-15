#define NOMINMAX

#include "ImGuiManager.h"

#include <string>
#include <windows.h>

#include "ESPRenderer.h"
#include "GuiStyle.h"
#include "Hooks.h"
#include "../../libs/ImGui/imgui.h"
#include "../../libs/ImGui/imgui_impl_dx11.h"
#include "../../libs/ImGui/imgui_impl_win32.h"
#include "../Core/AppState.h"
#include "../Core/Config.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"

#include "GUI/AppearanceTab.h"
#include "GUI/InfoTab.h"
#include "GUI/NpcsTab.h"
#include "GUI/ObjectsTab.h"
#include "GUI/PlayersTab.h"
#include "GUI/SettingsTab.h"

// Define static members
kx::Camera ImGuiManager::m_camera;
kx::MumbleLinkManager ImGuiManager::m_mumbleLinkManager;

bool ImGuiManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd, bool is_gw2al_mode) {
    IMGUI_CHECKVERSION();
    
    // In your case, you ALWAYS want your own ImGui, so we don't need to check the mode here.
    // If you wanted to support lib_imgui, you would check `is_gw2al_mode` here.
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    GUIStyle::LoadAppFont();
    GUIStyle::ApplyCustomStyle();
    if (!ImGui_ImplWin32_Init(hwnd)) return false;
    if (!ImGui_ImplDX11_Init(device, context)) return false;

    kx::ESPRenderer::Initialize(m_camera);
    return true;
}

void ImGuiManager::NewFrame() {
    // Prepare ImGui for a new frame of rendering
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRenderTargetView) {
    ImGui::EndFrame();
    ImGui::Render();

    ID3D11RenderTargetView* rtv = mainRenderTargetView;
    // If no RTV is provided (like from the GW2AL event), get it from D3DRenderHook's state
    if (!rtv && kx::Hooking::D3DRenderHook::IsInitialized()) {
        rtv = kx::Hooking::D3DRenderHook::GetRenderTargetView(); // You'll need to add this getter
    }

    if (rtv) {
        context->OMSetRenderTargets(1, &rtv, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

// Add this to your RenderUI function
void ImGuiManager::RenderUI() {
    // This static flag ensures we only run our core initialization logic ONCE.
    static bool is_fully_initialized = false;

    // Always update MumbleLink first to get the latest game state.
    m_mumbleLinkManager.Update();

    // --- DEFINITIVE FIX for Initialization Timing ---
    if (!is_fully_initialized && m_mumbleLinkManager.IsInitialized()) {
        // We are connected to MumbleLink, now get the data.
        const kx::MumbleLinkData* mumbleData = m_mumbleLinkManager.GetData();

        // Cast the raw context block to our MumbleContext struct.
        const kx::MumbleContext* context = reinterpret_cast<const kx::MumbleContext*>(mumbleData->context);

        // Now, we can safely check mapId. If it's not 0, the player is in-game.
        if (context && context->mapId != 0) {
            LOG_INFO("[ImGuiManager] MumbleLink is active and in-map (Map ID: %u). Initializing core components...", context->mapId);
            kx::AddressManager::Initialize();
            kx::InitializeHooks();
            is_fully_initialized = true; // Set the flag so this block never runs again.
        }
    }
    // --- END FIX ---

    // The rest of the function handles frame-by-frame logic.

    // Handle the window toggle hotkey.
    static bool lastToggleKeyState = false;
    bool currentToggleKeyState = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
    if (currentToggleKeyState && !lastToggleKeyState) {
        auto& settings = kx::AppState::Get().GetSettings();
        settings.showVisionWindow = !settings.showVisionWindow;
    }
    lastToggleKeyState = currentToggleKeyState;

    ImGuiIO& io = ImGui::GetIO();
    const kx::MumbleLinkData* mumbleData = m_mumbleLinkManager.GetData();

    // Update the camera.
    m_camera.Update(mumbleData, kx::Hooking::D3DRenderHook::GetWindowHandle());

    // Render the ESP overlay.
    kx::ESPRenderer::Render(io.DisplaySize.x, io.DisplaySize.y, mumbleData);

    // Render the UI window if it's shown.
    if (kx::AppState::Get().GetSettings().showVisionWindow) {
        RenderESPWindow();
    }
}

void ImGuiManager::RenderESPWindow() {
    // Use AppState singleton instead of global variable
    if (!kx::AppState::Get().IsVisionWindowOpen()) return;

    // Get settings reference once for the entire function
    auto& settings = kx::AppState::Get().GetSettings();

    std::string windowTitle = "KX Vision v";
    windowTitle += kx::APP_VERSION.data();

    // Set larger initial window size and center it on first use
    ImVec2 initialSize(600, 450);
    ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
    
    // Center the window on first spawn
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(ImVec2(center.x - initialSize.x * 0.5f, center.y - initialSize.y * 0.5f), ImGuiCond_FirstUseEver);
    
    // Pass a direct pointer to the singleton's vision window state
    ImGui::Begin(windowTitle.c_str(), kx::AppState::Get().GetVisionWindowOpenRef());

    RenderHints();

    // Connection status
    ImGui::Text("MumbleLink Status: %s", 
        m_mumbleLinkManager.IsInitialized() ? "Connected" : "Disconnected");
    ImGui::Separator(); // Add a separator for visual clarity

    if (ImGui::BeginTabBar("##ESPCategories")) {
	    kx::GUI::RenderPlayersTab();
	    kx::GUI::RenderNPCsTab();
	    kx::GUI::RenderObjectsTab();
	    kx::GUI::RenderAppearanceTab();
	    kx::GUI::RenderSettingsTab();
	    kx::GUI::RenderInfoTab();
        ImGui::EndTabBar();
    }

    ImGui::End();
}



void ImGuiManager::RenderHints() {
    // Display keyboard shortcuts with consistent styling
    const char* hints[] = {
        "Press INSERT to show/hide window.",
        "Press DELETE to unload DLL."
    };
    
    for (const auto& hint : hints) {
        ImGui::TextDisabled("Hint: %s", hint);
    }
    
    ImGui::Separator();
}

void ImGuiManager::Shutdown(bool is_gw2al_mode) {
    // Again, you always manage your own context, so the mode doesn't matter here.
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
