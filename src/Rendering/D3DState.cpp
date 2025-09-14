#include "D3DState.h"

namespace kx {
    void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old) {
        old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
        ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
        ctx->RSGetState(&old.RS);
        ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
        ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
    }

    void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old) {
        ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
        ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
        ctx->RSSetState(old.RS);
        if (old.RS) old.RS->Release();
        ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask);
        if (old.BlendState) old.BlendState->Release();
        ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef);
        if (old.DepthStencilState) old.DepthStencilState->Release();
    }
}