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

Font_VSOut sprite_vs(Font_VSIn input)
{
    Font_VSOut output;
    output.position = float4(input.position, 0, 1);
    output.uv = input.uv;
    return output;
}
