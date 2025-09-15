#include "AppState.h"
#ifdef GW2AL_BUILD // This entire file will only be compiled when GW2AL_BUILD is defined in your project settings.

#include "Integration.h"
#include <windows.h>

#include "D3DState.h"
#include "../../libs/gw2al/gw2al_api.h"
#include "../../libs/gw2al/gw2al_d3d9_wrapper.h" 
#include "../Hooking/d3d9_wrapper_structs.h"

#include "../Game/AddressManager.h"
#include "../Hooking/Hooks.h"
#include "../Rendering/ImGuiManager.h"
#include "../Hooking/D3DRenderHook.h"
#include "../Utils/DebugLogger.h"

// Global pointer to the core API, needed for callbacks
gw2al_core_vtable* g_al_api = nullptr;

// This is our main rendering callback, which will be triggered by d3d9_wrapper every frame.
// In Core/GW2AL_Integration.cpp
void OnPresent(d3d9_wrapper_event_data* evd) {
    if (!kx::Hooking::D3DRenderHook::IsInitialized()) return;

    IDXGISwapChain* pSwapChain = (IDXGISwapChain*)((void**)evd->stackPtr)[0];
    if (!pSwapChain) return;

    auto* device = kx::Hooking::D3DRenderHook::GetDevice();
    auto* context = kx::Hooking::D3DRenderHook::GetContext();
    if (!device || !context) return;

    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) return;

    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    if (FAILED(device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView))) {
        pBackBuffer->Release();
        return;
    }
    pBackBuffer->Release();

    // Backup the game's D3D state. THIS IS THE FIX.
    kx::StateBackupD3D11 d3dstate;
    kx::BackupD3D11State(context, d3dstate);

    ImGuiManager::NewFrame();
    ImGuiManager::RenderUI();
    ImGuiManager::Render(context, mainRenderTargetView);

    // Restore the game's D3D state. THIS IS THE FIX.
    kx::RestoreD3D11State(context, d3dstate);

    mainRenderTargetView->Release();
}

void OnResize(d3d9_wrapper_event_data* evd) {
    if (!kx::Hooking::D3DRenderHook::IsInitialized()) return;

    // Get the swap chain pointer from the event data
    struct swc_ResizeBuffers_cp {
        IDXGISwapChain* swc;
        // ... other params we don't need
    };
    swc_ResizeBuffers_cp* params = (swc_ResizeBuffers_cp*)evd->stackPtr;

    kx::Hooking::D3DRenderHook::OnResize(params->swc);
}

// This is the addon's entry point for GW2AL.
extern "C" __declspec(dllexport) gw2al_addon_dsc* gw2addon_get_description() {
    static gw2al_addon_dsc desc = {};
    desc.name = L"KXVision";
    desc.description = L"KXVision universal addon";
    desc.majorVer = 1;
    desc.minorVer = 0;
    desc.revision = 1;

    // IMPORTANT: Your dependencies are the core loader and the d3d9_wrapper.
    // You are NOT using lib_imgui, so it should not be listed as a dependency.
    static gw2al_addon_dsc deps[] = {
        GW2AL_CORE_DEP_ENTRY,
        D3D_WRAPPER_DEP_ENTRY,
        {0,0,0,0,0,0} // Null terminator for the dependency list
    };
    desc.dependList = deps;

    return &desc;
}

// This is called by GW2AL when the addon is loaded.
extern "C" __declspec(dllexport) gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api) {
    g_al_api = core_api;
    g_addon_loader_present = true;

    // CRITICAL FIX: Initialize your logger. This is why logs weren't appearing.
    LOG_INIT();
    LOG_INFO("KXVision starting up in GW2AL mode...");

    // Initialize your core components that don't depend on DirectX yet.
    //kx::AddressManager::Initialize();
    kx::InitializeHooks(); // This now only hooks the game thread, not Present.

    // Get the function pointer from d3d9_wrapper that allows us to subscribe to events.
    pD3D9_wrapper_enable_event enable_event = (pD3D9_wrapper_enable_event)g_al_api->query_function(
        g_al_api->hash_name((wchar_t*)D3D9_WRAPPER_ENABLE_EVENT_FNAME)
    );

    // Tell d3d9_wrapper that we are interested in two events:
    // 1. When the swap chain is created (so we can get the D3D device).
    // 2. Before the frame is presented (so we can draw our UI).
    enable_event(METH_DXGI_CreateSwapChain, WRAP_CB_POST);
    enable_event(METH_SWC_Present, WRAP_CB_PRE);
    enable_event(METH_SWC_ResizeBuffers, WRAP_CB_POST);

    // Watch for the swap chain creation event. This is our main initialization point.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")),
        g_al_api->hash_name(L"kxvision_init"),
        [](void* data) {
            d3d9_wrapper_event_data* evd = (d3d9_wrapper_event_data*)data;

            // Cast the event's stack pointer to our known struct to get the function arguments.
            dxgi_CreateSwapChain_cp* params = (dxgi_CreateSwapChain_cp*)evd->stackPtr;

            // Now that we have the device and swap chain, initialize our renderer and ImGui.
            kx::Hooking::D3DRenderHook::InitializeFromGW2AL(
                (ID3D11Device*)params->inDevice,
                *params->ppSwapchain
            );

            // Unwatch this event now that we are initialized. It only needs to run once.
            g_al_api->unwatch_event(g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")), g_al_api->hash_name(L"kxvision_init"));
        },
        -1 // Use a high priority to initialize as early as possible.
    );

    // Watch for the Present call every frame. This is our render loop.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_Present")),
        g_al_api->hash_name(L"kxvision_present"),
        (gw2al_api_event_handler)&OnPresent,
        0 // Default priority is fine for rendering.
    );

    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_SWC_ResizeBuffers")),
        g_al_api->hash_name(L"kxvision_resize"),
        (gw2al_api_event_handler)&OnResize,
        0
    );

    return GW2AL_OK;
}

// This is called by GW2AL when the addon is unloaded (e.g., game exit).
extern "C" __declspec(dllexport) gw2al_api_ret gw2addon_unload(int game_exiting) {
    // Unsubscribe from events to prevent callbacks to an unloaded module.
    g_al_api->unwatch_event(g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_Present")), g_al_api->hash_name(L"kxvision_present"));
    g_al_api->unwatch_event(g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_SWC_ResizeBuffers")), g_al_api->hash_name(L"kxvision_resize")); // <<<--- ADD THIS LINE

    // Perform cleanup in the reverse order of initialization.
    kx::CleanupHooks();
    kx::Hooking::D3DRenderHook::Shutdown();
    ImGuiManager::Shutdown(true);

    // CRITICAL FIX: Clean up your logger to ensure the log file is properly closed.
    LOG_INFO("KXVision shutting down in GW2AL mode...");
    LOG_CLEANUP();

    return GW2AL_OK;
}

#endif // GW2AL_BUILD