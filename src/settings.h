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

#include "camera.h"
#include "common_defines.h"

// Profiling
#define ENABLE_VTUNE_TASK_PROFILING 1

// Content settings
enum { NUM_ASTEROIDS = 50000 };
enum { TEXTURE_DIM = 256 }; // Req'd to be pow2 at the moment
enum { TEXTURE_ANISO = 2 };
enum { NUM_UNIQUE_MESHES = 1000 };
enum { MESH_MAX_SUBDIV_LEVELS = 3 }; // 4x polys for each step.
// See common_defines.h for NUM_UNIQUE_TEXTURES (also needed by shader now)

#define SIM_ORBIT_RADIUS 450.f
#define SIM_DISC_RADIUS  120.f
#define SIM_MIN_SCALE    0.2f

// In FLIP swap chains the compositor owns one of your buffers at any given point
// Thus to run unconstrained (>vsync) frame rates, you need 3 buffers
enum { NUM_SWAP_CHAIN_BUFFERS = 5 };

// In D3D12, this is the pipeline depth in frames - larger = more latency, shorter = potential bubbles.
// Usually 2-4 are good values.
enum { NUM_FRAMES_TO_BUFFER = 3 };

// In D3D12 this is the number of command buffers we generate for the main scene rendering
// Also effectively max thread parallelism
enum { NUM_SUBSETS = 4 };

// Buffer size for dynamic sprite data
enum { MAX_SPRITE_VERTICES_PER_FRAME = 6 * 1024 };


// This structure is often copied/passed by value so don't put anything really expensive in it.
struct Settings
{
    double closeAfterSeconds = 0.0;

    // Will be scaled based on DPI at app start for convenience
    double renderScale = 1.0; // 1.0 = window dimensions, <1 = upscaling, >1 = supersampling
    int windowWidth = 1080;
    int windowHeight = 720;
    int renderWidth;
    int renderHeight;

    unsigned int lockedFrameRate = 15;
    bool lockFrameRate = false;

    bool logFrameTimes = false;

    bool warp = false;                      // Use WARP device
    bool d3d12 = true;                      // Use D3D12 API (else, D3D11)
    bool windowed = true;                   // Use non-fullscreen window
    bool vsync = false;                     // Use v-synced presentation
    bool animate = true;                    // Animate asteroids

    bool allowTearing = false;

    // D3D12-only:
    bool multithreadedRendering = true;     // Generate command lists on multiple threads
    bool submitRendering = true;            // Submit command lists onto the queue after generating them
    bool executeIndirect = false;           // Draw asteroids using ExecuteIndirect
};
