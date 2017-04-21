/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

cbuffer DrawConstantBuffer : register(b0)
{
	float4x4 mWorld;
	float4x4 mViewProjection;
	float4 mSurfaceColor;
	float4 mDeepColor;
	uint mTextureIndex;
};

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

float linstep(float min, float max, float s)
{
    return saturate((s - min) / (max - min));
}


VSOut asteroid_vs(VSIn input)
{
    VSOut output;

    float3 positionWorld = mul(mWorld, float4(input.position, 1.0f)).xyz;
    output.position = mul(mViewProjection, float4(positionWorld, 1.0f));

    output.positionModel = input.position;
    output.normalWorld = mul(mWorld, float4(input.normal, 0.0f)).xyz; // No non-uniform scaling
    
    float depth = linstep(0.5f, 0.7f, length(input.position.xyz));
    output.albedo = lerp(mDeepColor.xyz, mSurfaceColor.xyz, depth);

    return output;
}
