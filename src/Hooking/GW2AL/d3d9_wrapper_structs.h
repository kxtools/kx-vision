#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h> // For DXGI_SWAP_CHAIN_DESC1, etc.

/**
 * @file d3d9_wrapper_structs.h
 * @brief Structure definitions for GW2AL d3d9_wrapper event callbacks
 */

 // These are the structs that the d3d9_wrapper expects in stackPtr

typedef struct com_vtable {
    void* methods[1024];
} com_vtable;

typedef struct com_orig_obj {
    com_vtable* vtable;
    void* original_obj; // Points to the original COM object
} com_orig_obj;

typedef struct wrapped_com_obj {
    com_orig_obj* orig_obj; // Points to the original COM object wrapper
    union {
        ID3D11Device* orig_dev11;
        IDXGISwapChain* orig_swc;
        IDXGIFactory* orig_dxgi;
    };
} wrapped_com_obj;

// CreateSwapChain params structure
typedef struct dxgi_CreateSwapChain_cp {
    IDXGIFactory* dxgi;
    IUnknown* inDevice;
    DXGI_SWAP_CHAIN_DESC* desc;
    IDXGISwapChain** ppSwapchain;
} dxgi_CreateSwapChain_cp;

// Present params structure
typedef struct swc_Present_cp {
    IDXGISwapChain* swc;
    UINT SyncInterval;
    UINT Flags;
} swc_Present_cp;

// ResizeBuffers params structure
typedef struct swc_ResizeBuffers_cp {
    IDXGISwapChain* swc;
    UINT BufferCount;
    UINT Width;
    UINT Height;
    DXGI_FORMAT NewFormat;
    UINT SwapChainFlags;
} swc_ResizeBuffers_cp;