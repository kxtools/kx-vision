#pragma once
#include <d3d11.h>

namespace kx {
    // This struct holds a snapshot of ALL relevant D3D11 states.
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
        ID3D11ShaderResourceView* PSShaderResource;
        ID3D11SamplerState* PSSampler;
        ID3D11PixelShader* PS;
        ID3D11VertexShader* VS;
        ID3D11GeometryShader* GS;
        UINT PSInstancesCount, VSInstancesCount, GSInstancesCount;
        ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];
        D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
        ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
        UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT IndexBufferFormat;
        ID3D11InputLayout* InputLayout;
        ID3D11RenderTargetView* RenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
        ID3D11DepthStencilView* DepthStencil;
    };

    void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old);
    void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old);
}