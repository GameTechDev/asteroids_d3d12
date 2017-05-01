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

struct SpriteVertex {
    float x;
    float y;
    float u;
    float v;
};

inline SpriteVertex* DrawSprite(float x, float y, float width, float height, float viewportWidth, float viewportHeight, SpriteVertex* outVertex)
{
    outVertex[0] = {x        , y         , 0.0f, 0.0f};
    outVertex[1] = {x + width, y         , 1.0f, 0.0f};
    outVertex[2] = {x + width, y + height, 1.0f, 1.0f};
    outVertex[3] = outVertex[0];
    outVertex[4] = outVertex[2];
    outVertex[5] = {x        , y + height, 0.0f, 1.0f};

    for (int i = 0; i < 6; ++i) {
        outVertex[i].x =  (outVertex[i].x / viewportWidth  * 2.0f - 1.0f);
        outVertex[i].y = -(outVertex[i].y / viewportHeight * 2.0f - 1.0f);
    }

    outVertex += 6;
    return outVertex;
}
