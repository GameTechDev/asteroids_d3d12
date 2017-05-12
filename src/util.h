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

#include <assert.h>
#include <d3d12.h>

#include <algorithm>
#include <vector>

#define CBUFFER_ALIGN __declspec(align(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))

#define WIDE_HELPER_2(x) L##x
#define WIDE_HELPER_1(x) WIDE_HELPER_2(x)
#define W__FILE__ WIDE_HELPER_1(__FILE__)

template<typename T>
inline void SafeRelease(T** p)
{
    auto q = *p;
    if (q != nullptr) {
        q->Release();
        *p = nullptr;
    }
}

inline HRESULT ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr)) throw;
    return hr;
}

template <typename T>
inline T Align(T v, T align)
{
    return (v + (align-1)) & ~(align-1);
}

struct ResourceBarrier {
    std::vector<D3D12_RESOURCE_BARRIER> mDescs;

    void AddTransition(
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
    {
        D3D12_RESOURCE_BARRIER desc = {};
        desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        desc.Transition.pResource = resource;
        desc.Transition.StateBefore = stateBefore;
        desc.Transition.StateAfter = stateAfter;
        desc.Transition.Subresource = subresource;
        mDescs.push_back(desc);
    }

    void ReverseTransitions()
    {
        for (auto ii = mDescs.begin(), ie = mDescs.end(); ii != ie; ++ii) {
            if (ii->Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
                std::swap(ii->Transition.StateBefore, ii->Transition.StateAfter);
            }
        }
    }

    void Submit(ID3D12GraphicsCommandList* commandList) const
    {
        assert(mDescs.size() <= UINT_MAX);
        commandList->ResourceBarrier((UINT)mDescs.size(), mDescs.data());
    }
};
