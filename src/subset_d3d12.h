/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "util.h"
#include "descriptor.h"

#include <d3d12.h>

__declspec(align(64)) // Avoid false sharing
class SubsetD3D12
{
public:
    SubsetD3D12(ID3D12Device* mDevice, UINT srvCount, ID3D12PipelineState* pso)
    {
        ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAlloc)));
        ThrowIfFailed(mDevice->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAlloc, pso, IID_PPV_ARGS(&mCmdLst)));
        ThrowIfFailed(mCmdLst->Close());
    }

    ~SubsetD3D12()
    {
        SafeRelease(&mCmdLst);
        SafeRelease(&mCmdAlloc);
    }
    
    ID3D12GraphicsCommandList* Begin(ID3D12PipelineState* pso)
    {
        ThrowIfFailed(mCmdAlloc->Reset());
        ThrowIfFailed(mCmdLst->Reset(mCmdAlloc, pso));
        return mCmdLst;
    }

    ID3D12GraphicsCommandList* End()
    {
        ThrowIfFailed(mCmdLst->Close());
        return mCmdLst;
    }

    ID3D12GraphicsCommandList* mCmdLst = nullptr;
    ID3D12CommandAllocator*    mCmdAlloc = nullptr;
};
