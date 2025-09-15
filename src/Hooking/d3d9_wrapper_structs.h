#pragma once

#include <d3d11.h> // For IUnknown, DXGI_SWAP_CHAIN_DESC, IDXGISwapChain

// This struct is passed by d3d9_wrapper to event handlers.
// It contains the return value of the hooked function and a pointer to the function's arguments on the stack.
typedef struct d3d9_wrapper_event_data {
    void* ret;
    void** stackPtr;
} d3d9_wrapper_event_data;

// This struct represents the stack layout for the CreateSwapChain function call
// inside d3d9_wrapper. It must be kept in sync with the wrapper's implementation.
struct dxgi_CreateSwapChain_cp {
    void* dxgi;             // IDXGIFactory
    IUnknown* inDevice;     // ID3D11Device
    DXGI_SWAP_CHAIN_DESC* desc;
    IDXGISwapChain** ppSwapchain;
};

struct swc_ResizeBuffers_cp {
    IDXGISwapChain* swc;
    UINT BufferCount;
    UINT Width;
    UINT Height;
    DXGI_FORMAT NewFormat;
    UINT SwapChainFlags;
};