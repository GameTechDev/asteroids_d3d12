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

#include <d3d11.h> // For D3D11_SUBRESOURCE_DATA
#include <DirectXMath.h>
#include <vector>
#include <algorithm>
#include <random>

#include "mesh.h"
#include "settings.h"

// We may want to ISPC-ify this down the road and just let it own the data structure in AoSoA format or similar
// For now we'll just do the dumb thing and see if it's fast enough
struct AsteroidDynamic
{
    DirectX::XMMATRIX world;
    // These depend on chosen subdiv level, hence are not constant
    unsigned int indexStart;
    unsigned int indexCount;
};

struct AsteroidStatic
{
    DirectX::XMFLOAT3 surfaceColor;
    DirectX::XMFLOAT3 deepColor;
    DirectX::XMVECTOR spinAxis;
    float scale;
    float spinVelocity;
    float orbitVelocity;
    unsigned int vertexStart;
    unsigned int textureIndex;
};

class AsteroidsSimulation
{
private:
    // NOTE: Memory could be optimized further for efficient cache traversal, etc.
    std::vector<AsteroidStatic> mAsteroidStatic;
    std::vector<AsteroidDynamic> mAsteroidDynamic;

    Mesh mMeshes;
    std::vector<unsigned int> mIndexOffsets;
    unsigned int mSubdivCount;
    unsigned int mVertexCountPerMesh;

    unsigned int mTextureDim;
    unsigned int mTextureCount;
    unsigned int mTextureArraySize;
    unsigned int mTextureMipLevels;
    std::vector<BYTE> mTextureDataBuffer;
    std::vector<D3D11_SUBRESOURCE_DATA> mTextureSubresources;

    unsigned int SubresourceIndex(unsigned int texture, unsigned int arrayElement = 0, unsigned int mip = 0)
    {
        return mip + mTextureMipLevels * (arrayElement + mTextureArraySize * texture);
    }

    void CreateTextures(unsigned int textureCount, unsigned int rngSeed);
    
public:
    AsteroidsSimulation(unsigned int rngSeed, unsigned int asteroidCount,
                        unsigned int meshInstanceCount, unsigned int subdivCount,
                        unsigned int textureCount);

    const Mesh* Meshes() { return &mMeshes; }
    const D3D11_SUBRESOURCE_DATA* TextureData(unsigned int textureIndex)
    {
        return mTextureSubresources.data() + SubresourceIndex(textureIndex);
    }

    const AsteroidStatic* StaticData() const { return mAsteroidStatic.data(); }
    const AsteroidDynamic* DynamicData() const { return mAsteroidDynamic.data(); }

    // Can optionall provide a range of asteroids to update; count = 0 => to the end
    // This is useful for multithreading
    void Update(float frameTime, DirectX::XMVECTOR cameraEye, const Settings& settings,
                size_t startIndex = 0, size_t count = 0);
};
