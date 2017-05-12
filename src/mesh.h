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

#include <vector>
#include <directxmath.h>

typedef unsigned short IndexType;

// NOTE: This data could be compressed, but it's not really the bottleneck at the moment
struct Vertex
{
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
};

struct Mesh
{
    void clear()
    {
        vertices.clear();
        indices.clear();
    }

    std::vector<Vertex> vertices;
    std::vector<IndexType> indices;
};

void CreateIcosahedron(Mesh *outMesh);

// 1 face -> 4 faces
void SubdivideInPlace(Mesh *outMesh);

void SpherifyInPlace(Mesh *outMesh, float radius = 1.0f);

void ComputeAvgNormalsInPlace(Mesh *outMesh);

// subdivIndexOffset array should be [subdivLevels+2] in size
void CreateGeospheres(Mesh *outMesh, unsigned int subdivLevelCount, unsigned int* outSubdivIndexOffsets);

// Returns a combined "mesh" that includes:
// - A set of indices for each subdiv level (outSubdivIndexOffsets for offsets/counts)
// - A set of vertices for each mesh instance (base vertices per mesh computed from vertexCountPerMesh)
// - Indices already have the vertex offsets for the correct subdiv level "baked-in", so only need the mesh offset
void CreateAsteroidsFromGeospheres(Mesh *outMesh,
                                   unsigned int subdivLevelCount, unsigned int meshInstanceCount,
                                   unsigned int rngSeed,
                                   unsigned int* outSubdivIndexOffsets, unsigned int* vertexCountPerMesh);


struct SkyboxVertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
    float face;
};

// Skybox mesh... vertices-only currently
void CreateSkyboxMesh(std::vector<SkyboxVertex>* outVertices);
