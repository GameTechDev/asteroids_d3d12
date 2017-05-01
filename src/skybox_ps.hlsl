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

#include "skybox_vs.hlsl"

TextureCube Tex : register(t0);
sampler Sampler : register(s0);

float4 skybox_ps(Skybox_VSOut input) : SV_Target
{   
    //return input.coords.xyzx;
    float4 tex = Tex.Sample(Sampler, input.coords);
    return float4(tex.xyz, 1.0f);
}
