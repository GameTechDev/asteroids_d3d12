/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <DirectXMath.h>
#include <interactioncontext.h>

class OrbitCamera
{
public:
    OrbitCamera();
    ~OrbitCamera();

    void View(DirectX::XMVECTOR center,
              float radius, float minRadius, float maxRadius,
              float longAngle, float latAngle);

    // Uses the provided fov for the larger dimension
    void Projection(float fov, float aspect);

    DirectX::XMVECTOR const& Eye() const { return mEye; }
    DirectX::XMMATRIX const& ViewProjection() const { return mViewProjection; }

    void AddPointer(UINT pointerId);
    void ProcessPointerFrames(UINT pointerId, const POINTER_INFO* pointerInfo);
    void ProcessInertia();
    void RemovePointer(UINT pointerId);

    void OrbitX(float angle);
    void OrbitY(float angle);
    void ZoomRadius(float delta);
    void ZoomRadiusScale(float delta);
    
private:
    void UpdateData();
    static VOID CALLBACK StaticInteractionOutputCallback(VOID *clientData, const INTERACTION_CONTEXT_OUTPUT *output);
    void InteractionOutputCallback(const INTERACTION_CONTEXT_OUTPUT *output);

    DirectX::XMVECTOR mCenter;
    DirectX::XMVECTOR mUp;
    float mMinRadius;
    float mMaxRadius;
    
    float mLatAngle;
    float mLongAngle;
    float mRadius;

    DirectX::XMVECTOR mEye;
    DirectX::XMMATRIX mView;
    DirectX::XMMATRIX mProjection;
    DirectX::XMMATRIX mViewProjection;

    HINTERACTIONCONTEXT mInteractionContext;
};
