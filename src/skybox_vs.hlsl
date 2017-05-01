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

cbuffer SkyboxConstantBuffer : register(b0)
{
    float4x4 mViewProjection;
};

struct Skybox_VSIn
{
	float3 position : POSITION;
	float3 uvFace   : UVFACE;
};

struct Skybox_VSOut
{
	float4 position : SV_Position;
	float3 coords   : UVFACE;
};

Skybox_VSOut skybox_vs(Skybox_VSIn input)
{
    Skybox_VSOut output;

    // NOTE: Don't translate skybox and make sure depth == 1 (no clipping)
    output.position = mul(mViewProjection, float4(input.position, 0.0f)).xyww;
    output.position.z = 0.0f; // 1-z
    output.coords = input.position.xyz;

    return output;
}
