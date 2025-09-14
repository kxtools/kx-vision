#pragma once
#include <d3d11.h>

namespace kx {
    // This struct holds a snapshot of the D3D11 state.
    struct StateBackupD3D11 {
        UINT ScissorRectsCount, ViewportsCount;
        D3D11_RECT ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11RasterizerState* RS;
        ID3D11BlendState* BlendState;
        FLOAT BlendFactor[4];
        UINT SampleMask;
        UINT StencilRef;
        ID3D11DepthStencilState* DepthStencilState;
    };

    void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old);
    void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old);
}