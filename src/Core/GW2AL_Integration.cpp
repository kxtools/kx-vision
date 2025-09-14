#include "AppState.h"
#ifdef GW2AL_BUILD // This file is now for GW2AL mode only

#include "Integration.h"
#include <windows.h>
#include "../../libs/gw2al/gw2al_api.h"
#include "../../libs/gw2al/gw2al_d3d9_wrapper.h"

#include "../Game/AddressManager.h"
#include "../Hooking/Hooks.h"
#include "../Rendering/ImGuiManager.h"
#include "../Hooking/D3DRenderHook.h" // We need this for the new initializer
#include "../Hooking/d3d9_wrapper_structs.h"

// Called by d3d9_wrapper on Present
void OnPresent(d3d9_wrapper_event_data* evd) {
    if (kx::Hooking::D3DRenderHook::IsInitialized()) {
        // This is our new render loop for GW2AL mode
        ImGuiManager::NewFrame();
        ImGuiManager::RenderUI();
        ImGuiManager::Render(
            kx::Hooking::D3DRenderHook::GetContext(), 
            nullptr // We can get the RTV inside Render if needed, or get it once on init
        );
    }
}

// Global API pointers
gw2al_core_vtable* g_al_api = nullptr;

// Exported functions for GW2AL
extern "C" __declspec(dllexport) gw2al_addon_dsc* gw2addon_get_description() {
    static gw2al_addon_dsc desc = {};
    desc.name = L"KXVision";
    desc.description = L"KXVision universal addon";
    desc.majorVer = 1;
    desc.minorVer = 0;
    desc.revision = 1;
    static gw2al_addon_dsc deps[] = { GW2AL_CORE_DEP_ENTRY, D3D_WRAPPER_DEP_ENTRY, {0,0,0,0,0,0} };
    desc.dependList = deps;
    return &desc;
}

extern "C" __declspec(dllexport) gw2al_api_ret gw2addon_load(gw2al_core_vtable* core_api) {
    g_al_api = core_api;
    g_addon_loader_present = true;

    // We still need to scan for game functions
    kx::AddressManager::Initialize(); 
    // This will now only hook the game thread, not Present
    kx::InitializeHooks(); 
    
    // Get the d3d9_wrapper API to subscribe to events
    pD3D9_wrapper_enable_event enable_event = (pD3D9_wrapper_enable_event)g_al_api->query_function(
        g_al_api->hash_name((wchar_t*)D3D9_WRAPPER_ENABLE_EVENT_FNAME)
    );
    
    // Enable the Pre-Present event for the DX11 SwapChain
    enable_event(METH_SWC_Present, WRAP_CB_PRE);

    // Watch the event and set our render function as the callback
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_PRE_SWC_Present")),
        g_al_api->hash_name(L"kxvision"),
        (gw2al_api_event_handler)&OnPresent,
        0
    );

    // We need to wait for the device to be created. We can do this by listening
    // to the CreateSwapChain event from d3d9_wrapper.
    g_al_api->watch_event(
        g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")),
        g_al_api->hash_name(L"kxvision_init"),
        [](void* data) {
            d3d9_wrapper_event_data* evd = (d3d9_wrapper_event_data*)data;

            dxgi_CreateSwapChain_cp* params = (dxgi_CreateSwapChain_cp*)evd->stackPtr;
            
            // Now we have the device and swapchain, initialize our renderer!
            kx::Hooking::D3DRenderHook::InitializeFromGW2AL(
                (ID3D11Device*)params->inDevice,
                *params->ppSwapchain
            );

            // Unwatch this event now that we are initialized
            g_al_api->unwatch_event(g_al_api->query_event(g_al_api->hash_name(L"D3D9_POST_DXGI_CreateSwapChain")), g_al_api->hash_name(L"kxvision_init"));
        },
        -1 // High priority to initialize early
    );

    return GW2AL_OK;
}

extern "C" __declspec(dllexport) void gw2addon_unload() {
    kx::CleanupHooks();
    kx::Hooking::D3DRenderHook::Shutdown();
    ImGuiManager::Shutdown(true); // Pass true for gw2al mode
}

#endif // GW2AL_BUILDturn GW2AL_OK;
