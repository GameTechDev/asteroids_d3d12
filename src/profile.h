// Copyright 2014 Intel Corporation All Rights Reserved
//
// Intel makes no representations about the suitability of this software for any purpose.  
// THIS SOFTWARE IS PROVIDED ""AS IS."" INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES,
// EXPRESS OR IMPLIED, AND ALL LIABILITY, INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES,
// FOR THE USE OF THIS SOFTWARE, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY
// RIGHTS, AND INCLUDING THE WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// Intel does not assume any responsibility for any errors which may appear in this software
// nor any responsibility to update it.

#pragma once

void ProfileBeginFrame(size_t frameIndex);
void ProfileEndFrame();

void ProfileBeginRender();
void ProfileEndRender();

void ProfileBeginRenderSubmit();
void ProfileEndRenderSubmit();

void ProfileBeginFenceWait();
void ProfileEndFenceWait();

void ProfileBeginPresent(size_t backBufferIndex = 0);
void ProfileEndPresent();

void ProfileBeginRenderSubset();
void ProfileEndRenderSubset();

void ProfileBeginSimUpdate();
void ProfileEndSimUpdate();

void ProfileBeginFrameLockWait();
void ProfileEndFrameLockWait();