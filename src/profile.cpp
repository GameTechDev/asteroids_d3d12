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

#include "profile.h"
#include "settings.h"
#include <sstream>
#include <assert.h>

#if ENABLE_VTUNE_TASK_PROFILING
#pragma comment(lib, "libittnotify.lib")
#else
#define INTEL_NO_ITTNOTIFY_API
#endif

#include "ittnotify.h"

static __itt_domain* gDomain = __itt_domain_create("AsteroidsDomain");

static __itt_string_handle* gFrameHandles[NUM_FRAMES_TO_BUFFER] = {};
static __itt_string_handle* gRender = __itt_string_handle_create("Render");
static __itt_string_handle* gFenceWait = __itt_string_handle_create("FenceWait");
static __itt_string_handle* gPresentHandles[NUM_SWAP_CHAIN_BUFFERS] = {};
static __itt_string_handle* gRenderSubset = __itt_string_handle_create("RenderSubset");
static __itt_string_handle* gSimUpdate = __itt_string_handle_create("RenderSimUpdate");
static __itt_string_handle* gRenderSubmit = __itt_string_handle_create("RenderSubmit");
static __itt_string_handle* gFrameLockWait = __itt_string_handle_create("FrameLockWait");

void ProfileBeginFrame(size_t frameIndex)
{
    // Lazy init is fine here (low freq)
    if (!gFrameHandles[frameIndex]) {
        std::ostringstream oss;
        oss << "Frame_" << frameIndex;
        gFrameHandles[frameIndex] = __itt_string_handle_create(oss.str().c_str());
    }
    __itt_task_begin(gDomain, __itt_null, __itt_null, gFrameHandles[frameIndex]);
}
void ProfileEndFrame()           { __itt_task_end(gDomain); }

void ProfileBeginRender()        { __itt_task_begin(gDomain, __itt_null, __itt_null, gRender); }
void ProfileEndRender()          { __itt_task_end(gDomain); }

void ProfileBeginFenceWait()     { __itt_task_begin(gDomain, __itt_null, __itt_null, gFenceWait); }
void ProfileEndFenceWait()       { __itt_task_end(gDomain); }

void ProfileBeginPresent(size_t backBufferIndex)
{
	// Lazy init is fine here (low freq)
	if (!gPresentHandles[backBufferIndex]) {
		std::ostringstream oss;
		oss << "Present_" << backBufferIndex;
		gPresentHandles[backBufferIndex] = __itt_string_handle_create(oss.str().c_str());
	}
	__itt_task_begin(gDomain, __itt_null, __itt_null, gPresentHandles[backBufferIndex]);
}
void ProfileEndPresent()         { __itt_task_end(gDomain); }

void ProfileBeginRenderSubset()  { __itt_task_begin(gDomain, __itt_null, __itt_null, gRenderSubset); }
void ProfileEndRenderSubset()    { __itt_task_end(gDomain); }

void ProfileBeginSimUpdate()     { __itt_task_begin(gDomain, __itt_null, __itt_null, gSimUpdate); }
void ProfileEndSimUpdate()       { __itt_task_end(gDomain); }

void ProfileBeginRenderSubmit()  { __itt_task_begin(gDomain, __itt_null, __itt_null, gRenderSubmit); }
void ProfileEndRenderSubmit()    { __itt_task_end(gDomain); }

void ProfileBeginFrameLockWait() { __itt_task_begin(gDomain, __itt_null, __itt_null, gFrameLockWait); }
void ProfileEndFrameLockWait()   { __itt_task_end(gDomain); }
