/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "util.h"
#include <d3d12.h>

// Untyped version
class UploadHeap
{
public:
    UploadHeap(ID3D12Device* device, UINT64 size)
    {
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( size ),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mHeap)
        ));

        ThrowIfFailed(mHeap->Map(0, nullptr, reinterpret_cast<void**>(&mHeapWO)));
    }

    ~UploadHeap()
    {
        SafeRelease(&mHeap);
    }

    ID3D12Resource* Heap() { return mHeap; }
    
    // Write-only!
    void* DataWO() { return mHeapWO; }

private:
    ID3D12Resource* mHeap = nullptr;
    void* mHeapWO = nullptr;
};


// Structure/typed version
template <typename T>
class UploadHeapT : public UploadHeap
{
public:
    UploadHeapT(ID3D12Device* device)
        : UploadHeap(device, sizeof(T))
    {}
    
    T* DataWO() { return (T*)UploadHeap::DataWO(); }
};
