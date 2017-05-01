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

#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <directxmath.h>
#include <random>
#include <map>

#include "camera.h"
#include "settings.h"
#include "simulation.h"
#include "util.h"
#include "gui.h"

namespace AsteroidsD3D11 {

struct DrawConstantBuffer {
    DirectX::XMFLOAT4X4 mWorld;
    DirectX::XMFLOAT4X4 mViewProjection;
    DirectX::XMFLOAT3 mSurfaceColor;
    float unused0;
    DirectX::XMFLOAT3 mDeepColor;
    float unused1;
};

struct SkyboxConstantBuffer {
    DirectX::XMFLOAT4X4 mViewProjection;
};

class Asteroids {
public:
    Asteroids(AsteroidsSimulation* asteroids, GUI* gui, bool warp);
    ~Asteroids();

    void Render(float frameTime, const OrbitCamera& camera, const Settings& settings);

    void ReleaseSwapChain();
    void ResizeSwapChain(IDXGIFactory2* dxgiFactory, HWND outputWindow, unsigned int width, unsigned int height);

private:
    void CreateMeshes();
    void InitializeTextureData();
    void CreateGUIResources();

    AsteroidsSimulation*        mAsteroids = nullptr;
    GUI*                        mGUI = nullptr;

    IDXGISwapChain1*            mSwapChain = nullptr;
    ID3D11Device*               mDevice = nullptr;
    ID3D11DeviceContext*        mDeviceCtxt = nullptr;

    // These depend on the swap chain buffer size
    ID3D11Texture2D*            mRenderTarget = nullptr;
    ID3D11RenderTargetView*     mRenderTargetView = nullptr;
    ID3D11DepthStencilView*     mDepthStencilView = nullptr;
    D3D11_VIEWPORT              mViewPort;
    D3D11_RECT                  mScissorRect;

    ID3D11DepthStencilState*    mDepthStencilState = nullptr;
    ID3D11BlendState*           mBlendState = nullptr;
    ID3D11BlendState*           mSpriteBlendState = nullptr;

    ID3D11InputLayout*          mInputLayout = nullptr;
    ID3D11Buffer*               mIndexBuffer = nullptr;
    ID3D11Buffer*               mVertexBuffer = nullptr;
    ID3D11VertexShader*         mVertexShader = nullptr;
    ID3D11PixelShader*          mPixelShader = nullptr;
    ID3D11Buffer*               mDrawConstantBuffer = nullptr;

    ID3D11VertexShader*         mSpriteVertexShader = nullptr;
    ID3D11PixelShader*          mSpritePixelShader = nullptr;
    ID3D11InputLayout*          mSpriteInputLayout = nullptr;
    ID3D11Buffer*               mSpriteVertexBuffer = nullptr;
    std::map<std::string, ID3D11ShaderResourceView*> mSpriteTextures;

    ID3D11PixelShader*          mFontPixelShader = nullptr;
    ID3D11ShaderResourceView*   mFontTextureSRV = nullptr;

    ID3D11VertexShader*         mSkyboxVertexShader = nullptr;
    ID3D11PixelShader*          mSkyboxPixelShader = nullptr;
    ID3D11Buffer*               mSkyboxConstantBuffer = nullptr;
    ID3D11Buffer*               mSkyboxVertexBuffer = nullptr;
    ID3D11InputLayout*          mSkyboxInputLayout = nullptr;
    ID3D11ShaderResourceView*   mSkyboxSRV = nullptr;

    ID3D11Texture2D*            mTextures[NUM_UNIQUE_TEXTURES];
    ID3D11ShaderResourceView*   mTextureSRVs[NUM_UNIQUE_TEXTURES];
    ID3D11SamplerState*         mSamplerState = nullptr;
};

} // namespace AsteroidsD3D11
