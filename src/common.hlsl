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
#ifndef COMMON_HLSL
#define COMMON_HLSL

struct VSIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VSOut
{
    float4 position      : SV_Position;
    float3 positionModel : POSITIONMODEL;
    float3 normalWorld   : NORMAL;
    float3 albedo        : ALBEDO; // Alternatively, can pass just "ao" to PS and read cbuffer in PS
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


struct Font_VSIn
{
    float2 position : POSITION;
    float2 uv       : UV;
};

struct Font_VSOut
{
    float4 position : SV_Position;
    float2 uv       : UV;
};


#endif
