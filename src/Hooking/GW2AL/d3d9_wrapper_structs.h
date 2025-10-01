#pragma once

#include <d3d11.h>
#include <dxgi.h>

/**
 * @file d3d9_wrapper_structs.h
 * @brief Structure definitions for GW2AL d3d9_wrapper event callbacks
 * 
 * These structures define the stack parameters passed to event callbacks
 * by the GW2AL d3d9_wrapper addon.
 */

/**
 * @brief Parameters for DXGI CreateSwapChain call
 */
struct dxgi_CreateSwapChain_cp {
    IDXGIFactory* factory;
    IUnknown* inDevice;
    DXGI_SWAP_CHAIN_DESC* pDesc;
    IDXGISwapChain** ppSwapchain;
};

/**
 * @brief Parameters for IDXGISwapChain::Present call
 */
struct swc_Present_cp {
    IDXGISwapChain* swc;
    UINT SyncInterval;
    UINT Flags;
};

/**
 * @brief Parameters for IDXGISwapChain::ResizeBuffers call
 */
struct swc_ResizeBuffers_cp {
    IDXGISwapChain* swc;
    UINT BufferCount;
    UINT Width;
    UINT Height;
    DXGI_FORMAT NewFormat;
    UINT SwapChainFlags;
};
