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
#include "Bootstrap.h"
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "../../libs/gw2al/gw2al_api.h"
#include "../../libs/gw2al/gw2al_d3d9_wrapper.h" 
#include "../Hooking/GW2AL/d3d9_wrapper_structs.h"

#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/Console.h"

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

    // Correctly get the swapchain from the wrapped object
    wrapped_com_obj* wrapObj = reinterpret_cast<wrapped_com_obj*>(evd->stackPtr[0]);
    if (!wrapObj) return;

    IDXGISwapChain* pSwapChain = wrapObj->orig_swc;
    if (!pSwapChain) return;

    auto* device = kx::Hooking::D3DRenderHook::GetDevice();
    auto* context = kx::Hooking::D3DRenderHook::GetContext();
    if (!device || !context) return;

    // Create a render target view from the current back buffer for robustness
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) return;

    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    HRESULT hr = device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr) || !mainRenderTargetView) {
        return;
    }

    // Get display size from swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);
    float displayWidth = static_cast<float>(sd.BufferDesc.Width);
    float displayHeight = static_cast<float>(sd.BufferDesc.Height);
    HWND windowHandle = kx::Hooking::D3DRenderHook::GetWindowHandle();

    // Centralized per-frame tick (update + render)
    kx::g_App.RenderTick(windowHandle, displayWidth, displayHeight, context, mainRenderTargetView);

    // Clean up the per-frame render target view
    mainRenderTargetView->Release();
}

void OnDXGIPostCreateSwapChain(D3D9_wrapper_event_data* evd) {
    if (!evd || !evd->stackPtr) {
        LOG_ERROR("[GW2AL] Invalid event data in OnDXGIPostCreateSwapChain");
        return;
    }

    dxgi_CreateSwapChain_cp* params = reinterpret_cast<dxgi_CreateSwapChain_cp*>(evd->stackPtr);
    if (!params->inDevice || !params->ppSwapchain || !*params->ppSwapchain) {
        LOG_ERROR("[GW2AL] Null parameter in CreateSwapChain event");
        return;
    }

    // Safely get the D3D11 device interface using QueryInterface
    ID3D11Device* device = nullptr;
    HRESULT hr = params->inDevice->QueryInterface(__uuidof(ID3D11Device), (void**)&device);

    if (FAILED(hr) || !device) {
        LOG_ERROR("[GW2AL] Failed to query D3D11Device interface: 0x%08X", hr);
        return;
    }

    // Initialize with device and swap chain
    bool initResult = kx::Hooking::D3DRenderHook::InitializeFromDevice(
        device,
        *params->ppSwapchain
    );

    // Always release our reference to the device
    device->Release();

    if (initResult) {
        LOG_INFO("[GW2AL] D3DRenderHook initialized successfully");
        kx::g_App.OnRendererInitialized();
    }
    else {
        LOG_ERROR("[GW2AL] Failed to initialize D3DRenderHook - device interface query or initialization failed");
    }
}

/**
 * @brief Callback for swap chain resize events
 *
 * Called when the game window is resized. We need to update our render target.
 */
void OnResize(D3D9_wrapper_event_data* evd) {
    if (!evd || !evd->stackPtr) {
        return;
    }
    if (!kx::Hooking::D3DRenderHook::IsInitialized()) {
        return;
    }

    // Get the swap chain pointer from the event data
    // Note: In a POST event, the parameters reflect the state AFTER the call.
    swc_ResizeBuffers_cp* params = reinterpret_cast<swc_ResizeBuffers_cp*>(evd->stackPtr);
    if (!params || !params->swc) {
        return;
    }

    // Delegate to the D3DRenderHook to handle the resource cleanup.
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

    kx::Bootstrap::Initialize("GW2AL");

    if (!kx::g_App.InitializeForGW2AL()) {
        LOG_ERROR("Failed to initialize AppLifecycleManager for GW2AL mode - HookManager initialization failed");
        return GW2AL_FAIL;
    }

    pD3D9_wrapper_enable_event enable_event = (pD3D9_wrapper_enable_event)g_al_api->query_function(
        g_al_api->hash_name((wchar_t*)D3D9_WRAPPER_ENABLE_EVENT_FNAME)
    );

    enable_event(METH_DXGI_CreateSwapChain, WRAP_CB_POST);
    enable_event(METH_SWC_Present, WRAP_CB_PRE);
    enable_event(METH_SWC_ResizeBuffers, WRAP_CB_PRE);

    // Watch for swap chain creation. This is our main initialization point.
    // We DO NOT unwatch this event, so we can re-initialize if needed.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")),
        g_al_api->hash_name(L"kxvision_init"),
        (gw2al_api_event_handler)&OnDXGIPostCreateSwapChain,
        -1 // High priority
    );

    // Watch for the Present call every frame. This is our render loop.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_Present")),
        g_al_api->hash_name(L"kxvision_present"),
        (gw2al_api_event_handler)&OnPresent,
        10
    );

    // Watch for the ResizeBuffers event to handle window size changes.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_ResizeBuffers")),
        g_al_api->hash_name(L"kxvision_resize"),
        (gw2al_api_event_handler)&OnResize,
        10
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
            g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_ResizeBuffers")),
            g_al_api->hash_name(L"kxvision_resize")
        );
    }

    // Perform cleanup via the lifecycle manager
    kx::g_App.Shutdown();
    
    LOG_INFO("KXVision shut down successfully in GW2AL mode");
    kx::Bootstrap::Cleanup();
    
    return GW2AL_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Let gw2addon_load handle all initialization. Do nothing here.
        break;

    case DLL_PROCESS_DETACH:
        // Exit flow explanation for GW2AL mode:
        // 1. When game closes normally -> gw2addon_unload() is called -> Shutdown() -> done
        // 2. If unload fails or game crashes -> Windows calls DllMain(..., DLL_PROCESS_DETACH)
        // This is the fallback path that ensures donation prompt and settings save happen even
        // if the clean gw2addon_unload path isn't taken.
        //
        // The atomic flag in SaveSettingsOnExit() prevents duplicate saves if Shutdown()
        // was already called.
        kx::g_App.ShowDonationPromptIfNeeded();
        kx::g_App.SaveSettingsOnExit();
        break;
    }
    return TRUE;
}

#endif // GW2AL_BUILD
