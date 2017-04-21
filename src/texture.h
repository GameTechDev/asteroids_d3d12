/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <d3d11.h>

void GenerateMips2D_XXXX8(D3D11_SUBRESOURCE_DATA* subresources, size_t widthLevel0, size_t heightLevel0, size_t mipLevels);

// Will generate mips (into subresources array) is mipLevels > 0
void FillNoise2D_RGBA8(D3D11_SUBRESOURCE_DATA* subresources, size_t width, size_t height, size_t mipLevels,
                       float seed, float persistence, float noiseScale, float noiseStrength,
					   float redScale = 255.0f, float greenScale = 255.0f, float blueScale = 255.0f);


// Helper for uploading initial texture data in D3D12; as with D3D11, one initialData structure per subresource
// Creates temporary resources internally and syncs with GPU... this is a convenience function for init time!
// NOTE: Currently textures with mip chain must be pow2!
// Transitions resource from D3D12_RESOURCE_USAGE_INITIAL to "stateAfter"
void InitializeTexture2D(
    ID3D12Device* device, 
    ID3D12CommandQueue* cmdQueue,
    ID3D12Resource* texture, 
    const D3D12_RESOURCE_DESC* desc, 
    UINT bytesPerPixel, 
    const D3D11_SUBRESOURCE_DATA* initialData,
    D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

// NOTE: This function very much only works for the specific path(s) that we use it for!
// Not very general-purpose yet.
HRESULT CreateTexture2DFromDDS_XXXX8(
    ID3D12Device* device, 
    ID3D12CommandQueue* cmdQueue,
    ID3D12Resource** texture, 
    const char* fileName,
    DXGI_FORMAT format, // Should match file otherwise expect explosion/wackiness...
    D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
