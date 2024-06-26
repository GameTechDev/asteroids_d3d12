///////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
///////////////////////////////////////////////////////////////////////////////

#include <d3dcompiler.h>
#include <directxmath.h>
#include <math.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <future>
#include <limits>
#include <random>
#include <locale>
#include <codecvt>

#include "asteroids_d3d11.h"
#include "util.h"
#include "mesh.h"
#include "noise.h"
#include "texture.h"
#include "DDSTextureLoader.h"
#include "profile.h"

#include "asteroid_vs.h"
#include "asteroid_ps_d3d11.h"
#include "skybox_vs.h"
#include "skybox_ps.h"
#include "sprite_vs.h"
#include "sprite_ps.h"
#include "font_ps.h"

using namespace DirectX;

namespace AsteroidsD3D11 {

Asteroids::Asteroids(AsteroidsSimulation* asteroids, GUI* gui, bool warp)
    : mAsteroids(asteroids)
    , mGUI(gui)
    , mSwapChain(nullptr)
    , mDevice(nullptr)
    , mDeviceCtxt(nullptr)
    , mRenderTarget(nullptr)
    , mRenderTargetView(nullptr)
    , mDepthStencilView(nullptr)
    , mDepthStencilState(nullptr)
    , mInputLayout(nullptr)
    , mIndexBuffer(nullptr)
    , mVertexBuffer(nullptr)
    , mVertexShader(nullptr)
    , mPixelShader(nullptr)
    , mDrawConstantBuffer(nullptr)
    , mSkyboxVertexShader(nullptr)
    , mSkyboxPixelShader(nullptr)
    , mSkyboxConstantBuffer(nullptr)
    , mSamplerState(nullptr)
{
    memset(&mViewPort, 0, sizeof(mViewPort));
    memset(&mScissorRect, 0, sizeof(mScissorRect));
    memset(mTextures, 0, sizeof(mTextures));
    memset(mTextureSRVs, 0, sizeof(mTextureSRVs));

    // Create device and swap chain
    {
        IDXGIAdapter*       adapter             = nullptr;
        D3D_DRIVER_TYPE     driverType          =  ( warp ) ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
        HMODULE             swModule            = NULL;
        UINT                flags               = 0;
        D3D_FEATURE_LEVEL   featureLevels[]     = { D3D_FEATURE_LEVEL_11_0 };
        UINT                numFeatureLevels    = ARRAYSIZE(featureLevels);
        UINT                sdkVersion          = D3D11_SDK_VERSION;

#ifdef _DEBUG
        flags = flags | D3D11_CREATE_DEVICE_DEBUG;
#endif

        auto hr = D3D11CreateDevice(adapter, driverType, swModule,
            flags, featureLevels, numFeatureLevels, sdkVersion, &mDevice, nullptr, &mDeviceCtxt);
        if (FAILED(hr)) {
            // Try again without the debug flag...
            flags = flags & ~D3D11_CREATE_DEVICE_DEBUG;
            ThrowIfFailed(D3D11CreateDevice(adapter, driverType, swModule,
                flags, featureLevels, numFeatureLevels, sdkVersion, &mDevice, nullptr, &mDeviceCtxt));
        }
    }
    
    // create pipeline state
    {
        D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        ThrowIfFailed(mDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc),
            g_asteroid_vs, sizeof(g_asteroid_vs), &mInputLayout));

        {
            D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
            desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
            ThrowIfFailed(mDevice->CreateDepthStencilState(&desc, &mDepthStencilState));
        }
        {
            CD3D11_BLEND_DESC desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
            ThrowIfFailed(mDevice->CreateBlendState(&desc, &mBlendState));

            // Premultiplied over blend
            desc.RenderTarget[0].BlendEnable = TRUE;
            desc.RenderTarget[0].SrcBlend    = D3D11_BLEND_ONE;
            desc.RenderTarget[0].BlendOp     = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].DestBlend   = D3D11_BLEND_INV_SRC_ALPHA;
            ThrowIfFailed(mDevice->CreateBlendState(&desc, &mSpriteBlendState));
        }
        
        ThrowIfFailed(mDevice->CreateVertexShader(g_asteroid_vs, sizeof(g_asteroid_vs), NULL, &mVertexShader));
        ThrowIfFailed(mDevice->CreatePixelShader(g_asteroid_ps_d3d11, sizeof(g_asteroid_ps_d3d11), NULL, &mPixelShader));
    }
    // create skybox pipeline state
    {
        D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "UVFACE",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        ThrowIfFailed(mDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc),
            g_skybox_vs, sizeof(g_skybox_vs), &mSkyboxInputLayout));

        ThrowIfFailed(mDevice->CreateVertexShader(g_skybox_vs, sizeof(g_skybox_vs), NULL, &mSkyboxVertexShader));
        ThrowIfFailed(mDevice->CreatePixelShader(g_skybox_ps, sizeof(g_skybox_ps), NULL, &mSkyboxPixelShader));
    }

    // Sprites and fonts
    {
        D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "UV",       0, DXGI_FORMAT_R32G32_FLOAT, 0,  8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        ThrowIfFailed(mDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc),
            g_sprite_vs, sizeof(g_sprite_vs), &mSpriteInputLayout));

        ThrowIfFailed(mDevice->CreateVertexShader(g_sprite_vs, sizeof(g_sprite_vs), NULL, &mSpriteVertexShader));
        ThrowIfFailed(mDevice->CreatePixelShader(g_sprite_ps, sizeof(g_sprite_ps), NULL, &mSpritePixelShader));
        ThrowIfFailed(mDevice->CreatePixelShader(g_font_ps, sizeof(g_font_ps), NULL, &mFontPixelShader));
    }
    
    // Create draw constant buffer
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(DrawConstantBuffer);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ThrowIfFailed(mDevice->CreateBuffer(&desc, nullptr, &mDrawConstantBuffer));
    }
    // Create skybox constant buffer
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(SkyboxConstantBuffer);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ThrowIfFailed(mDevice->CreateBuffer(&desc, nullptr, &mSkyboxConstantBuffer));
    }

    // Create sampler
    {
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter         = D3D11_FILTER_ANISOTROPIC;
        desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MinLOD         = -D3D11_FLOAT32_MAX;
        desc.MaxLOD         = D3D11_FLOAT32_MAX;
        desc.MipLODBias     = 0.0f;
        desc.MaxAnisotropy  = TEXTURE_ANISO;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        ThrowIfFailed(mDevice->CreateSamplerState(&desc, &mSamplerState));
    }

    CreateMeshes();
    InitializeTextureData();
    CreateGUIResources();

    // Load textures
    ThrowIfFailed(CreateDDSTextureFromFile(mDevice, L"starbox_1024.dds", &mSkyboxSRV, true));
}

Asteroids::~Asteroids()
{
    ReleaseSwapChain();

    SafeRelease(&mDepthStencilState);
    SafeRelease(&mInputLayout);
    SafeRelease(&mIndexBuffer);
    SafeRelease(&mVertexBuffer);
    SafeRelease(&mVertexShader);
    SafeRelease(&mPixelShader);
    SafeRelease(&mDrawConstantBuffer);
    SafeRelease(&mSamplerState);

    SafeRelease(&mBlendState);
    SafeRelease(&mSpriteBlendState);

    for (auto i : mSpriteTextures) {
        i.second->Release();
    }
    SafeRelease(&mSpriteInputLayout);
    SafeRelease(&mSpriteVertexShader);
    SafeRelease(&mSpritePixelShader);
    SafeRelease(&mSpriteVertexBuffer);

    SafeRelease(&mFontPixelShader);
    SafeRelease(&mFontTextureSRV);

    SafeRelease(&mSkyboxVertexShader);
    SafeRelease(&mSkyboxPixelShader);
    SafeRelease(&mSkyboxConstantBuffer);
    SafeRelease(&mSkyboxVertexBuffer);
    SafeRelease(&mSkyboxInputLayout);
    SafeRelease(&mSkyboxSRV);

    for (auto& texture : mTextures) SafeRelease(&texture);
    for (auto& srv : mTextureSRVs) SafeRelease(&srv);

    if (mSwapChain != nullptr) {
        mSwapChain->Release();
        mSwapChain = nullptr;
    }
    SafeRelease(&mDeviceCtxt);
    SafeRelease(&mDevice);
}


void Asteroids::ReleaseSwapChain()
{
    // Cleanup any references...
    SafeRelease(&mRenderTarget);
    SafeRelease(&mRenderTargetView);
    SafeRelease(&mDepthStencilView);
    SafeRelease(&mSwapChain);
    mDeviceCtxt->ClearState();
    mDeviceCtxt->Flush();
}

inline void DisableDXGIWindowChanges(IUnknown* device, HWND window)
{
    IDXGIDevice * pDXGIDevice;
    ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)));
    IDXGIAdapter * pDXGIAdapter;
    ThrowIfFailed(pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)));
    IDXGIFactory * pIDXGIFactory;
    ThrowIfFailed(pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory)));

    ThrowIfFailed(pIDXGIFactory->MakeWindowAssociation(window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER));

    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
}

void Asteroids::ResizeSwapChain(IDXGIFactory2* dxgiFactory, HWND outputWindow, unsigned int width, unsigned int height, bool allowTearing)
{
    ReleaseSwapChain();

    // Create swap chain
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Can create an SRGB render target view on the swap chain buffer
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = NUM_SWAP_CHAIN_BUFFERS;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Not used
        swapChainDesc.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            mDevice, outputWindow, &swapChainDesc, nullptr, nullptr, &mSwapChain));
        // MakeWindowAssociation must be called after CreateSwapChain
        DisableDXGIWindowChanges(mDevice, outputWindow);
    }

    // create render target view
    {
        ThrowIfFailed(mSwapChain->GetBuffer(0, IID_PPV_ARGS(&mRenderTarget)));

        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        desc.Texture2D.MipSlice = 0;
        ThrowIfFailed(mDevice->CreateRenderTargetView(mRenderTarget, &desc, &mRenderTargetView));
    }

    // create depth stencil view
    {
        CD3D11_TEXTURE2D_DESC desc = CD3D11_TEXTURE2D_DESC(
            DXGI_FORMAT_D32_FLOAT, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL);

        ID3D11Texture2D* depthStencil = nullptr;
        ThrowIfFailed(mDevice->CreateTexture2D(&desc, nullptr, &depthStencil));
        ThrowIfFailed(mDevice->CreateDepthStencilView(depthStencil, nullptr, &mDepthStencilView));
        depthStencil->Release();
    }

    // update the viewport and scissor
    ZeroMemory(&mViewPort, sizeof(D3D11_VIEWPORT));
    mViewPort.TopLeftX = 0;
    mViewPort.TopLeftY = 0;
    mViewPort.Width = (float)width;
    mViewPort.Height = (float)height;
    mViewPort.MinDepth = 0.0f;
    mViewPort.MaxDepth = 1.0f;

    mScissorRect.left = 0;
    mScissorRect.top = 0;
    mScissorRect.right = width;
    mScissorRect.bottom = height;
}


void Asteroids::CreateMeshes()
{
    auto asteroidMeshes = mAsteroids->Meshes();

    // create vertex buffer
    {
        CD3D11_BUFFER_DESC desc(
            (UINT)asteroidMeshes->vertices.size() * sizeof(asteroidMeshes->vertices[0]),
            D3D11_BIND_VERTEX_BUFFER,
            D3D11_USAGE_DEFAULT);

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = asteroidMeshes->vertices.data();

        ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mVertexBuffer));
    }

    // create index buffer
    {
        CD3D11_BUFFER_DESC desc(
            (UINT)asteroidMeshes->indices.size() * sizeof(asteroidMeshes->indices[0]),
            D3D11_BIND_INDEX_BUFFER,
            D3D11_USAGE_DEFAULT);

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = asteroidMeshes->indices.data();

        ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mIndexBuffer));
    }

    std::vector<SkyboxVertex> skyboxVertices;
    CreateSkyboxMesh(&skyboxVertices);

    // create skybox vertex buffer
    {
        CD3D11_BUFFER_DESC desc(
            (UINT)skyboxVertices.size() * sizeof(skyboxVertices[0]),
            D3D11_BIND_VERTEX_BUFFER,
            D3D11_USAGE_DEFAULT);

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = skyboxVertices.data();

        ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mSkyboxVertexBuffer));
    }

    // create sprite vertex buffer (dynamic)
    {
        CD3D11_BUFFER_DESC desc(
            MAX_SPRITE_VERTICES_PER_FRAME * sizeof(SpriteVertex),
            D3D11_BIND_VERTEX_BUFFER,
            D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        
        ThrowIfFailed(mDevice->CreateBuffer(&desc, nullptr, &mSpriteVertexBuffer));
    }
}

void Asteroids::InitializeTextureData()
{
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width            = TEXTURE_DIM;
    textureDesc.Height           = TEXTURE_DIM;
    textureDesc.ArraySize        = 3;
    textureDesc.MipLevels        = 0; // Full chain
    textureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage            = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    for (UINT t = 0; t < NUM_UNIQUE_TEXTURES; ++t) {
        ThrowIfFailed(mDevice->CreateTexture2D(&textureDesc, mAsteroids->TextureData(t), &mTextures[t]));
        ThrowIfFailed(mDevice->CreateShaderResourceView(mTextures[t], nullptr, &mTextureSRVs[t]));
    }
}

void Asteroids::CreateGUIResources()
{
    auto font = mGUI->Font();
    D3D11_TEXTURE2D_DESC textureDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_A8_UNORM, font->BitmapWidth(), font->BitmapHeight(), 1, 1);

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = font->Pixels();
    initialData.SysMemPitch = font->BitmapWidth();

    ID3D11Texture2D* texture = nullptr;
    ThrowIfFailed(mDevice->CreateTexture2D(&textureDesc, &initialData, &texture));
    ThrowIfFailed(mDevice->CreateShaderResourceView(texture, nullptr, &mFontTextureSRV));
    SafeRelease(&texture);

    // Load any GUI sprite textures
    for (size_t i = 0; i < mGUI->size(); ++i) {
        auto control = (*mGUI)[i];
        if (control->TextureFile().length() > 0 && mSpriteTextures.find(control->TextureFile()) == mSpriteTextures.end()) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;            
            ID3D11ShaderResourceView* textureSRV = nullptr;
            ThrowIfFailed(CreateDDSTextureFromFile(mDevice, converter.from_bytes(control->TextureFile()).c_str(), &textureSRV, true));
            mSpriteTextures[control->TextureFile()] = textureSRV;
        }
    }
}

static_assert(sizeof(IndexType) == 2, "Expecting 16-bit index buffer");

void Asteroids::Render(float frameTime, const OrbitCamera& camera, const Settings& settings)
{
    ProfileBeginFrame(0);

    ProfileBeginRender();

    // Frame data
    ProfileBeginSimUpdate();
    mAsteroids->Update(frameTime, camera.Eye(), settings);
    auto staticAsteroidData = mAsteroids->StaticData();
    auto dynamicAsteroidData = mAsteroids->DynamicData();
    ProfileEndSimUpdate();
    
    // Clear the render target
    float clearcol[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    mDeviceCtxt->ClearRenderTargetView(mRenderTargetView, clearcol);
    mDeviceCtxt->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 0.0f, 0);

    {
        ID3D11Buffer* ia_buffers[] = { mVertexBuffer };
        UINT ia_strides[] = { sizeof(Vertex) };
        UINT ia_offsets[] = { 0 };
        mDeviceCtxt->IASetInputLayout(mInputLayout);
        mDeviceCtxt->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mDeviceCtxt->IASetVertexBuffers(0, 1, ia_buffers, ia_strides, ia_offsets);
        mDeviceCtxt->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    }

    mDeviceCtxt->VSSetShader(mVertexShader, nullptr, 0);
    mDeviceCtxt->VSSetConstantBuffers(0, 1, &mDrawConstantBuffer);

    mDeviceCtxt->RSSetViewports(1, &mViewPort);
    mDeviceCtxt->RSSetScissorRects(1, &mScissorRect);

    mDeviceCtxt->PSSetShader(mPixelShader, nullptr, 0);
    mDeviceCtxt->PSSetSamplers(0, 1, &mSamplerState);

    mDeviceCtxt->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
    mDeviceCtxt->OMSetDepthStencilState(mDepthStencilState, 0);
    mDeviceCtxt->OMSetBlendState(mBlendState, nullptr, 0xFFFFFFFF);

    ProfileBeginRenderSubset();

    auto viewProjection = camera.ViewProjection();
    for (UINT drawIdx = 0; drawIdx < settings.numAsteroids; ++drawIdx)
    {
        auto staticData = &staticAsteroidData[drawIdx];
        auto dynamicData = &dynamicAsteroidData[drawIdx];

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        ThrowIfFailed(mDeviceCtxt->Map(mDrawConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

        auto drawConstants = (DrawConstantBuffer*) mapped.pData;
        XMStoreFloat4x4(&drawConstants->mWorld,          dynamicData->world);
        XMStoreFloat4x4(&drawConstants->mViewProjection, viewProjection);
        drawConstants->mSurfaceColor = staticData->surfaceColor;
        drawConstants->mDeepColor    = staticData->deepColor;

        mDeviceCtxt->Unmap(mDrawConstantBuffer, 0);

        mDeviceCtxt->PSSetShaderResources(0, 1, &mTextureSRVs[staticData->textureIndex]);

        mDeviceCtxt->DrawIndexedInstanced(dynamicData->indexCount, 1, dynamicData->indexStart, staticData->vertexStart, 0);
    }

    ProfileEndRenderSubset();

    // Draw skybox
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        ThrowIfFailed(mDeviceCtxt->Map(mSkyboxConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        auto skyboxConstants = (SkyboxConstantBuffer*) mapped.pData;
        XMStoreFloat4x4(&skyboxConstants->mViewProjection, camera.ViewProjection());
        mDeviceCtxt->Unmap(mSkyboxConstantBuffer, 0);

        ID3D11Buffer* ia_buffers[] = { mSkyboxVertexBuffer };
        UINT ia_strides[] = { sizeof(SkyboxVertex) };
        UINT ia_offsets[] = { 0 };
        mDeviceCtxt->IASetInputLayout(mSkyboxInputLayout);
        mDeviceCtxt->IASetVertexBuffers(0, 1, ia_buffers, ia_strides, ia_offsets);

        mDeviceCtxt->VSSetShader(mSkyboxVertexShader, nullptr, 0);
        mDeviceCtxt->VSSetConstantBuffers(0, 1, &mSkyboxConstantBuffer);

        mDeviceCtxt->PSSetShader(mSkyboxPixelShader, nullptr, 0);
        mDeviceCtxt->PSSetSamplers(0, 1, &mSamplerState);
        mDeviceCtxt->PSSetShaderResources(0, 1, &mSkyboxSRV);

        mDeviceCtxt->Draw(6*6, 0);
    }

    mDeviceCtxt->OMSetRenderTargets(1, &mRenderTargetView, 0); // No more depth buffer

    // Draw sprites and fonts
    {
        // Fill in vertices (TODO: could move this vector to be a member - not a big deal)
        std::vector<UINT> controlVertices;
        controlVertices.reserve(mGUI->size());

        {
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            ThrowIfFailed(mDeviceCtxt->Map(mSpriteVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
            auto vertexBase = (SpriteVertex*)mapped.pData;
            auto vertexEnd = vertexBase;
            
            for (size_t i = 0; i < mGUI->size(); ++i) {
                auto control = (*mGUI)[i];
                controlVertices.push_back((UINT)(control->Draw(mViewPort.Width, mViewPort.Height, vertexEnd) - vertexEnd));
                vertexEnd += controlVertices.back();
            }

            mDeviceCtxt->Unmap(mSpriteVertexBuffer, 0);
        }

        ID3D11Buffer* ia_buffers[] = { mSpriteVertexBuffer };
        UINT ia_strides[] = { sizeof(SpriteVertex) };
        UINT ia_offsets[] = { 0 };
        mDeviceCtxt->IASetInputLayout(mSpriteInputLayout);
        mDeviceCtxt->IASetVertexBuffers(0, 1, ia_buffers, ia_strides, ia_offsets);
        mDeviceCtxt->VSSetShader(mSpriteVertexShader, 0, 0);
        mDeviceCtxt->OMSetBlendState(mSpriteBlendState, nullptr, 0xFFFFFFFF);

        // Draw
        UINT vertexStart = 0;
        for (size_t i = 0; i < mGUI->size(); ++i) {
            auto control = (*mGUI)[i];
            if (control->Visible()) {
                if (control->TextureFile().length() == 0) { // Font
                    mDeviceCtxt->PSSetShader(mFontPixelShader, 0, 0);
                    mDeviceCtxt->PSSetShaderResources(0, 1, &mFontTextureSRV);
                } else { // Sprite
                    auto textureSRV = mSpriteTextures[control->TextureFile()];
                    mDeviceCtxt->PSSetShader(mSpritePixelShader, 0, 0);
                    mDeviceCtxt->PSSetShaderResources(0, 1, &textureSRV);
                }
                mDeviceCtxt->Draw(controlVertices[i], vertexStart);
            }
            vertexStart += controlVertices[i];
        }
    }

    ProfileEndRender();

    ProfileBeginPresent();
    if (settings.vsync) {
        ThrowIfFailed(mSwapChain->Present(1, 0));
    } else if (settings.allowTearing) {
        ThrowIfFailed(mSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
    } else {
        ThrowIfFailed(mSwapChain->Present(0, 0));
    }
    ProfileEndPresent();

    ProfileEndFrame();
}

} // namespace AsteroidsD3D11
