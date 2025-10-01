#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <d3d11.h>
#include <windows.h>

namespace kx {

/**
 * @brief Structure to hold D3D11 state for backup/restore operations
 * 
 * This is essential in GW2AL mode where we share the rendering pipeline
 * with the game and other addons. We must save and restore all state.
 * 
 * Simplified version: Only backs up state that ImGui actually modifies.
 */
struct StateBackupD3D11 {
    // Viewport and scissor
    UINT ScissorRectsCount = 0;
    UINT ViewportsCount = 0;
    D3D11_RECT ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
    D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};
    
    // Rasterizer and blend state
    ID3D11RasterizerState* RS = nullptr;
    ID3D11BlendState* BlendState = nullptr;
    FLOAT BlendFactor[4] = {};
    UINT SampleMask = 0;
    
    // Shaders (ImGui uses VS and PS only)
    ID3D11PixelShader* PS = nullptr;
    ID3D11VertexShader* VS = nullptr;
    
    // Input assembly
    D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* IndexBuffer = nullptr;
    ID3D11Buffer* VertexBuffer = nullptr;
    ID3D11Buffer* VSConstantBuffer = nullptr;
    UINT IndexBufferOffset = 0;
    UINT VertexBufferStride = 0;
    UINT VertexBufferOffset = 0;
    DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
    ID3D11InputLayout* InputLayout = nullptr;
    
    // Texture and sampler
    ID3D11ShaderResourceView* PSShaderResource = nullptr;
    ID3D11SamplerState* PSSampler = nullptr;
};

/**
 * @brief Backup D3D11 state from the device context
 * 
 * Only backs up state that ImGui modifies. This is faster than backing up
 * everything and is sufficient for addon compatibility.
 */
inline void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& backup) {
    backup.ScissorRectsCount = backup.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    ctx->RSGetScissorRects(&backup.ScissorRectsCount, backup.ScissorRects);
    ctx->RSGetViewports(&backup.ViewportsCount, backup.Viewports);
    ctx->RSGetState(&backup.RS);
    ctx->OMGetBlendState(&backup.BlendState, backup.BlendFactor, &backup.SampleMask);
    ctx->PSGetShaderResources(0, 1, &backup.PSShaderResource);
    ctx->PSGetSamplers(0, 1, &backup.PSSampler);
    ctx->PSGetShader(&backup.PS, nullptr, nullptr);
    ctx->VSGetShader(&backup.VS, nullptr, nullptr);
    ctx->VSGetConstantBuffers(0, 1, &backup.VSConstantBuffer);
    ctx->IAGetPrimitiveTopology(&backup.PrimitiveTopology);
    ctx->IAGetIndexBuffer(&backup.IndexBuffer, &backup.IndexBufferFormat, &backup.IndexBufferOffset);
    ctx->IAGetVertexBuffers(0, 1, &backup.VertexBuffer, &backup.VertexBufferStride, &backup.VertexBufferOffset);
    ctx->IAGetInputLayout(&backup.InputLayout);
}

/**
 * @brief Restore D3D11 state to the device context
 * 
 * Restores the state that was backed up by BackupD3D11State().
 */
inline void RestoreD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& backup) {
    ctx->RSSetScissorRects(backup.ScissorRectsCount, backup.ScissorRects);
    ctx->RSSetViewports(backup.ViewportsCount, backup.Viewports);
    ctx->RSSetState(backup.RS); if (backup.RS) backup.RS->Release();
    ctx->OMSetBlendState(backup.BlendState, backup.BlendFactor, backup.SampleMask); if (backup.BlendState) backup.BlendState->Release();
    ctx->PSSetShaderResources(0, 1, &backup.PSShaderResource); if (backup.PSShaderResource) backup.PSShaderResource->Release();
    ctx->PSSetSamplers(0, 1, &backup.PSSampler); if (backup.PSSampler) backup.PSSampler->Release();
    ctx->PSSetShader(backup.PS, nullptr, 0); if (backup.PS) backup.PS->Release();
    ctx->VSSetShader(backup.VS, nullptr, 0); if (backup.VS) backup.VS->Release();
    ctx->VSSetConstantBuffers(0, 1, &backup.VSConstantBuffer); if (backup.VSConstantBuffer) backup.VSConstantBuffer->Release();
    ctx->IASetPrimitiveTopology(backup.PrimitiveTopology);
    ctx->IASetIndexBuffer(backup.IndexBuffer, backup.IndexBufferFormat, backup.IndexBufferOffset); if (backup.IndexBuffer) backup.IndexBuffer->Release();
    ctx->IASetVertexBuffers(0, 1, &backup.VertexBuffer, &backup.VertexBufferStride, &backup.VertexBufferOffset); if (backup.VertexBuffer) backup.VertexBuffer->Release();
    ctx->IASetInputLayout(backup.InputLayout); if (backup.InputLayout) backup.InputLayout->Release();
}

} // namespace kx
