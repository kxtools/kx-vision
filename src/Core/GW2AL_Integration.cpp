/**
 * @file GW2AL_Integration.cpp
 * @brief GW2AL addon loader integration layer
 * 
 * This file provides the entry points and event handlers for GW2AL (Guild Wars 2 Addon Loader) mode.
 * It is only compiled when GW2AL_BUILD is defined in Config.h.
 * 
 * Key responsibilities:
 * - Provide GW2AL addon descriptor and lifecycle functions (load/unload)
 * - Handle D3D11 rendering events (Present, Resize) via d3d9_wrapper
 * - Coordinate with AppLifecycleManager for initialization and updates
 */

#include "Config.h"
#ifdef GW2AL_BUILD // This entire file will only be compiled when GW2AL_BUILD is defined

#include <cstdio> // For fclose in console cleanup
#include "AppState.h"
#include "AppLifecycleManager.h"
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "../Rendering/D3DState.h"
#include "../../libs/gw2al/gw2al_api.h"
#include "../../libs/gw2al/gw2al_d3d9_wrapper.h" 
#include "../Hooking/GW2AL/d3d9_wrapper_structs.h"

#include "Hooks.h"
#include "../Rendering/ImGuiManager.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/Console.h"
#include "../Game/AddressManager.h"

// Global pointer to the core API, needed for callbacks
gw2al_core_vtable* g_al_api = nullptr;

// External reference to the global AppLifecycleManager instance
namespace kx {
    extern AppLifecycleManager g_App;
}

/**
 * @brief Main rendering callback triggered by d3d9_wrapper every frame
 * 
 * This is called on EVERY frame, before the game's Present call.
 * We render our ImGui overlay here.
 */
void OnPresent(D3D9_wrapper_event_data* evd) {
    // Early exit checks
    if (!evd || !evd->stackPtr) return;
    if (!kx::Hooking::D3DRenderHook::IsInitialized()) return;
    if (kx::AppState::Get().IsShuttingDown()) return;

    // Extract the swap chain from the event data
    IDXGISwapChain* pSwapChain = (IDXGISwapChain*)((void**)evd->stackPtr)[0];
    if (!pSwapChain) return;

    auto* device = kx::Hooking::D3DRenderHook::GetDevice();
    auto* context = kx::Hooking::D3DRenderHook::GetContext();
    if (!device || !context) return;

    // Get the back buffer and create a temporary render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) return;

    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    if (FAILED(device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView))) {
        pBackBuffer->Release();
        return;
    }
    pBackBuffer->Release();

    // CRITICAL: Backup D3D state before rendering (shared pipeline with game and other addons)
    kx::StateBackupD3D11 d3dstate;
    kx::BackupD3D11State(context, d3dstate);

    // === Update game state every frame ===
    auto& camera = kx::g_App.GetCamera();
    auto& mumbleLinkManager = kx::g_App.GetMumbleLinkManager();
    
    // Update MumbleLink every frame
    mumbleLinkManager.Update();
    const kx::MumbleLinkData* mumbleLinkData = mumbleLinkManager.GetData();
    
    // Check and initialize game services if ready (MumbleLink connected + player in-game)
    kx::g_App.CheckAndInitializeServices();
    
    // Update camera with latest MumbleLink data
    HWND windowHandle = kx::Hooking::D3DRenderHook::GetWindowHandle();
    camera.Update(mumbleLinkData, windowHandle);
    
    // === Handle input (INSERT key toggle) ===
    static bool lastToggleKeyState = false;
    bool currentToggleKeyState = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
    
    if (currentToggleKeyState && !lastToggleKeyState) {
        // Toggle the window visibility flag (same one ImGui X button uses)
        bool isOpen = kx::AppState::Get().IsVisionWindowOpen();
        kx::AppState::Get().SetVisionWindowOpen(!isOpen);
    }
    
    lastToggleKeyState = currentToggleKeyState;
    
    // Get display size from swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);
    float displayWidth = static_cast<float>(sd.BufferDesc.Width);
    float displayHeight = static_cast<float>(sd.BufferDesc.Height);

    // Render our ImGui overlay
    ImGuiManager::NewFrame();
    ImGuiManager::RenderUI(camera, mumbleLinkManager, mumbleLinkData, windowHandle, displayWidth, displayHeight);
    ImGuiManager::Render(context, mainRenderTargetView);

    // CRITICAL: Restore D3D state after rendering
    kx::RestoreD3D11State(context, d3dstate);

    // Clean up temporary render target view
    mainRenderTargetView->Release();
}

/**
 * @brief Callback for swap chain resize events
 * 
 * Called when the game window is resized. We need to update our render target.
 */
void OnResize(D3D9_wrapper_event_data* evd) {
    if (!evd || !evd->stackPtr) return;
    if (!kx::Hooking::D3DRenderHook::IsInitialized()) return;

    // Get the swap chain pointer from the event data
    struct swc_ResizeBuffers_cp {
        IDXGISwapChain* swc;
        // ... other params we don't need
    };
    swc_ResizeBuffers_cp* params = (swc_ResizeBuffers_cp*)evd->stackPtr;
    if (!params || !params->swc) return;

    kx::Hooking::D3DRenderHook::OnResize(params->swc);
}

/**
 * @brief Returns the addon description for GW2AL
 * 
 * This is the first function GW2AL calls to identify our addon.
 */
extern "C" __declspec(dllexport) gw2al_addon_dsc* gw2addon_get_description() {
    static gw2al_addon_dsc desc = {};
    desc.name = L"KXVision";
    desc.description = L"KXVision universal addon";
    desc.majorVer = 1;
    desc.minorVer = 0;
    desc.revision = 1;

    // IMPORTANT: Dependencies are the core loader and the d3d9_wrapper.
    // We are NOT using lib_imgui, so it should not be listed as a dependency.
    static gw2al_addon_dsc deps[] = {
        GW2AL_CORE_DEP_ENTRY,
        D3D_WRAPPER_DEP_ENTRY,
        {0,0,0,0,0,0} // Null terminator for the dependency list
    };
    desc.dependList = deps;

    return &desc;
}

/**
 * @brief Called by GW2AL when the addon is loaded
 * 
 * This is our entry point for GW2AL mode. We set up event watchers
 * and initialize the application lifecycle manager.
 */
extern "C" __declspec(dllexport) gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api) {
    g_al_api = core_api;

    LOG_INIT();
    
    // Setup debug console in debug builds
#ifdef _DEBUG
    kx::SetupConsole();
#endif
    
    LOG_INFO("KXVision starting up in GW2AL mode...");

    // Initialize the application lifecycle manager for GW2AL mode
    if (!kx::g_App.InitializeForGW2AL()) {
        LOG_ERROR("Failed to initialize AppLifecycleManager for GW2AL mode");
        return GW2AL_FAIL;
    }

    // Get the function pointer from d3d9_wrapper to enable events
    pD3D9_wrapper_enable_event enable_event = (pD3D9_wrapper_enable_event)g_al_api->query_function(
        g_al_api->hash_name((wchar_t*)D3D9_WRAPPER_ENABLE_EVENT_FNAME)
    );

    // Enable ONLY the events we need to kickstart our initialization and rendering
    enable_event(METH_DXGI_CreateSwapChain, WRAP_CB_POST);
    enable_event(METH_SWC_Present, WRAP_CB_PRE);
    enable_event(METH_SWC_ResizeBuffers, WRAP_CB_POST);

    // Watch for the swap chain creation event. This is our main initialization point.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")),
        g_al_api->hash_name(L"kxvision_init"),
        [](void* data) { // Lambda function for our callback
            D3D9_wrapper_event_data* evd = (D3D9_wrapper_event_data*)data;
            dxgi_CreateSwapChain_cp* params = (dxgi_CreateSwapChain_cp*)evd->stackPtr;

            // This is the FIRST point where we have a D3D device. Initialize our renderer.
            if (kx::Hooking::D3DRenderHook::InitializeFromDevice(
                (ID3D11Device*)params->inDevice,
                *params->ppSwapchain
            )) {
                LOG_INFO("D3DRenderHook initialized successfully from GW2AL");
                
                // Notify the lifecycle manager that the renderer is ready
                kx::g_App.OnRendererInitialized();
                
                // Now that the renderer is initialized, we can unwatch this event.
                g_al_api->unwatch_event(
                    g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")), 
                    g_al_api->hash_name(L"kxvision_init")
                );
            } else {
                LOG_ERROR("Failed to initialize D3DRenderHook from GW2AL");
            }
        },
        -1 // High priority to initialize as early as possible
    );

    // Watch for the Present call every frame. This is our render loop.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_Present")),
        g_al_api->hash_name(L"kxvision_present"),
        (gw2al_api_event_handler)&OnPresent,
        0
    );

    // Watch for the ResizeBuffers event to handle window size changes.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_SWC_ResizeBuffers")),
        g_al_api->hash_name(L"kxvision_resize"),
        (gw2al_api_event_handler)&OnResize,
        0
    );

    LOG_INFO("KXVision GW2AL event handlers registered successfully");
    return GW2AL_OK;
}

/**
 * @brief Called by GW2AL when the addon is unloaded (e.g., game exit)
 * 
 * We must unsubscribe from events and perform cleanup.
 */
extern "C" __declspec(dllexport) gw2al_api_ret gw2addon_unload(int game_exiting) {
    LOG_INFO("KXVision unloading in GW2AL mode...");
    
    // Unsubscribe from events to prevent callbacks to an unloaded module
    if (g_al_api) {
        g_al_api->unwatch_event(
            g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_Present")), 
            g_al_api->hash_name(L"kxvision_present")
        );
        g_al_api->unwatch_event(
            g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_SWC_ResizeBuffers")), 
            g_al_api->hash_name(L"kxvision_resize")
        );
    }

    // Perform cleanup via the lifecycle manager
    kx::g_App.Shutdown();

    LOG_INFO("KXVision shut down successfully in GW2AL mode");
    LOG_CLEANUP();

#ifdef _DEBUG
    // Clean up debug console in debug builds
    if (stdout) fclose(stdout);
    if (stderr) fclose(stderr);
    if (stdin) fclose(stdin);
    FreeConsole();
#endif

    return GW2AL_OK;
}

#endif // GW2AL_BUILD
