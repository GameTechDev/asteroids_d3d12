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

#define WIN32_LEAN_AND_MEAN
#include <sdkddkver.h>
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h> // must be after windows.h
#include <ShellScalingApi.h>
#include <interactioncontext.h>
#include <mmsystem.h>
#include <map>
#include <vector>
#include <iostream>

#include "asteroids_d3d11.h"
#include "asteroids_d3d12.h"
#include "camera.h"
#include "profile.h"
#include "gui.h"

using namespace DirectX;

namespace {

// Global demo state
Settings gSettings;
OrbitCamera gCamera;

IDXGIFactory2* gDXGIFactory = nullptr;

AsteroidsD3D11::Asteroids* gWorkloadD3D11 = nullptr;
AsteroidsD3D12::Asteroids* gWorkloadD3D12 = nullptr;

GUI gGUI;
GUISprite* gD3D11Control;
GUISprite* gD3D12Control;
GUIText* gFPSControl;

enum
{
    basicStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
    windowedStyle = basicStyle | WS_OVERLAPPEDWINDOW,
    fullscreenStyle = basicStyle
};

bool CheckDll(char const* dllName)
{
    auto hModule = LoadLibrary(dllName);
    if (hModule == NULL) {
        return false;
    }
    FreeLibrary(hModule);
    return true;
}


UINT SetupDPI()
{
    // Just do system DPI awareness for now for simplicity... scale the 3D content
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

    UINT dpiX = 0, dpiY;
    POINT pt = {1, 1};
    auto hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
        return dpiX;
    } else {
        return 96; // default
    }
}


void ResetCameraView()
{
    auto center    = XMVectorSet(0.0f, -0.4f*SIM_DISC_RADIUS, 0.0f, 0.0f);
    auto radius    = SIM_ORBIT_RADIUS + SIM_DISC_RADIUS + 10.f;
    auto minRadius = SIM_ORBIT_RADIUS - 3.0f * SIM_DISC_RADIUS;
    auto maxRadius = SIM_ORBIT_RADIUS + 3.0f * SIM_DISC_RADIUS;
    auto longAngle = 4.50f;
    auto latAngle  = 1.45f;
    gCamera.View(center, radius, minRadius, maxRadius, longAngle, latAngle);
}

void ToggleFullscreen(HWND hWnd)
{
    static WINDOWPLACEMENT prevPlacement = { sizeof(prevPlacement) };
    DWORD dwStyle = (DWORD)GetWindowLongPtr(hWnd, GWL_STYLE);
    if ((dwStyle & windowedStyle) == windowedStyle)
    {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hWnd, &prevPlacement) &&
            GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(hWnd, GWL_STYLE, fullscreenStyle);
            SetWindowPos(hWnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hWnd, GWL_STYLE, windowedStyle);
        SetWindowPlacement(hWnd, &prevPlacement);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}


} // namespace



LRESULT CALLBACK WindowProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            UINT ww = LOWORD(lParam);
            UINT wh = HIWORD(lParam);

            // Ignore resizing to minimized
            if (ww == 0 || wh == 0) return 0;

            gSettings.windowWidth = (int)ww;
            gSettings.windowHeight = (int)wh;
            gSettings.renderWidth = (UINT)(double(gSettings.windowWidth)  * gSettings.renderScale);
            gSettings.renderHeight = (UINT)(double(gSettings.windowHeight) * gSettings.renderScale);

            // Update camera projection
            float aspect = (float)gSettings.renderWidth / (float)gSettings.renderHeight;
            gCamera.Projection(XM_PIDIV2 * 0.8f * 3 / 2, aspect);

            // Resize currently active swap chain
            if (gSettings.d3d12)
                gWorkloadD3D12->ResizeSwapChain(gDXGIFactory, hWnd, gSettings.renderWidth, gSettings.renderHeight, gSettings.allowTearing);
            else
                gWorkloadD3D11->ResizeSwapChain(gDXGIFactory, hWnd, gSettings.renderWidth, gSettings.renderHeight, gSettings.allowTearing);

            return 0;
        }

        case WM_KEYDOWN:
            if (lParam & (1 << 30)) {
                // Ignore repeats
                return 0;
            }
            switch (wParam) {
            case VK_SPACE:
                gSettings.animate = !gSettings.animate;
                std::cout << "Animate: " << gSettings.animate << std::endl;
                return 0;

                /* Disabled for demo setup */
            case 'V':
                gSettings.vsync = !gSettings.vsync;
                std::cout << "Vsync: " << gSettings.vsync << std::endl;
                return 0;
            case 'M':
                gSettings.multithreadedRendering = !gSettings.multithreadedRendering;
                std::cout << "Multithreaded Rendering: " << gSettings.multithreadedRendering << std::endl;
                return 0;
            case 'I':
                gSettings.executeIndirect = !gSettings.executeIndirect;
                std::cout << "ExecuteIndirect Rendering: " << gSettings.executeIndirect << std::endl;
                return 0;
            case 'S':
                gSettings.submitRendering = !gSettings.submitRendering;
                std::cout << "Submit Rendering: " << gSettings.submitRendering << std::endl;
                return 0;

            case '1': gSettings.d3d12 = (gWorkloadD3D11 == nullptr); return 0;
            case '2': gSettings.d3d12 = (gWorkloadD3D12 != nullptr); return 0;

            case VK_ESCAPE:
                SendMessage(hWnd, WM_CLOSE, 0, 0);
                return 0;
            } // Switch on key code
            return 0;

        case WM_SYSKEYDOWN:
            if (lParam & (1 << 30)) {
                // Ignore repeats
                return 0;
            }
            switch (wParam) {
            case VK_RETURN:
                ToggleFullscreen(hWnd);
                break;
            }
            return 0;

        case WM_MOUSEWHEEL: {
            auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
            gCamera.ZoomRadius(-0.07f * delta);
            return 0;
        }

        case WM_POINTERDOWN:
        case WM_POINTERUPDATE:
        case WM_POINTERUP: {
            auto pointerId = GET_POINTERID_WPARAM(wParam);
            POINTER_INFO pointerInfo;
            if (GetPointerInfo(pointerId, &pointerInfo)) {
                if (message == WM_POINTERDOWN) {
                    // Compute pointer position in render units
                    POINT p = pointerInfo.ptPixelLocation;
                    ScreenToClient(hWnd, &p);
                    RECT clientRect;
                    GetClientRect(hWnd, &clientRect);
                    p.x = p.x * gSettings.renderWidth / (clientRect.right - clientRect.left);
                    p.y = p.y * gSettings.renderHeight / (clientRect.bottom - clientRect.top);

                    auto guiControl = gGUI.HitTest(p.x, p.y);
                    if (guiControl == gFPSControl) {
                        gSettings.lockFrameRate = !gSettings.lockFrameRate;
                    } else if (guiControl == gD3D11Control) { // Switch to D3D12
                        gSettings.d3d12 = (gWorkloadD3D12 != nullptr);
                    } else if (guiControl == gD3D12Control) { // Switch to D3D11
                        gSettings.d3d12 = (gWorkloadD3D11 == nullptr);
                    } else { // Camera manipulation
                        gCamera.AddPointer(pointerId);
                    }
                }

                // Otherwise send it to the camera controls
                gCamera.ProcessPointerFrames(pointerId, &pointerInfo);
                if (message == WM_POINTERUP) gCamera.RemovePointer(pointerId);
            }
            return 0;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}


int main(int argc, char** argv)
{
    auto d3d11Available = CheckDll("d3d11.dll");
    auto d3d12Available = CheckDll("d3d12.dll");

    // Must be done before any windowing-system-like things or else virtualization will kick in
    auto dpi = SetupDPI();
    // By default render at the lower resolution and scale up based on system settings
    gSettings.renderScale = 96.0 / double(dpi);

    // Scale default window size based on dpi
    gSettings.windowWidth *= dpi / 96;
    gSettings.windowHeight *= dpi / 96;

    char* perfOutputPath = nullptr;
    for (int a = 1; a < argc; ++a) {
        if (_stricmp(argv[a], "-close_after") == 0 && a + 1 < argc) {
            gSettings.closeAfterSeconds = atof(argv[++a]);
            printf("Close after %f seconds\n", gSettings.closeAfterSeconds);
        } else if (_stricmp(argv[a], "-nod3d11") == 0) {
            d3d11Available = false;
            printf("Disable D3D11 path\n");
        } else if (_stricmp(argv[a], "-warp") == 0) {
            gSettings.warp = true;
            printf("Use WARP adapter\n");
        } else if (_stricmp(argv[a], "-nod3d12") == 0) {
            d3d12Available = false;
            printf("Disable D3D12 path\n");
        } else if (_stricmp(argv[a], "-indirect") == 0) {
            gSettings.executeIndirect = true;
            printf("Enable ExecuteIndirect implementation\n");
        } else if (_stricmp(argv[a], "-fullscreen") == 0) {
            gSettings.windowed = false;
            printf("Fullscreen\n");
        } else if (_stricmp(argv[a], "-allow_tearing") == 0) {
            gSettings.allowTearing = true;
            printf("Tearing allowed\n");
        } else if (_stricmp(argv[a], "-window") == 0 && a + 2 < argc) {
            gSettings.windowWidth = atoi(argv[++a]);
            gSettings.windowHeight = atoi(argv[++a]);
            printf("%ux%u window\n", gSettings.windowWidth, gSettings.windowHeight);
        } else if (_stricmp(argv[a], "-render_scale") == 0 && a + 1 < argc) {
            gSettings.renderScale = atof(argv[++a]);
            printf("%f render scale\n", gSettings.renderScale);
        } else if (_stricmp(argv[a], "-locked_fps") == 0 && a + 1 < argc) {
            gSettings.lockFrameRate = true;
            gSettings.lockedFrameRate = atoi(argv[++a]);
            printf("FPS locked to %u\n", gSettings.lockedFrameRate);
        } else if (_stricmp(argv[a], "-perf_output") == 0 && a + 1 < argc) {
            perfOutputPath = argv[++a];
            printf("Output frame performance to '%s'\n", perfOutputPath);
        } else {
            fprintf(stderr, "error: unrecognized argument '%s'\n", argv[a]);
            fprintf(stderr, "usage: asteroids_d3d12 [options]\n");
            fprintf(stderr, "options:\n");
            fprintf(stderr, "  -close_after [seconds]\n");
            fprintf(stderr, "  -nod3d11\n");
            fprintf(stderr, "  -nod3d12\n");
            fprintf(stderr, "  -fullscreen\n");
            fprintf(stderr, "  -allow_tearing\n");
            fprintf(stderr, "  -window [width] [height]\n");
            fprintf(stderr, "  -render_scale [scale]\n");
            fprintf(stderr, "  -locked_fps [fps]\n");
            fprintf(stderr, "  -perf_output [path]\n");
            fprintf(stderr, "  -warp\n");
            return -1;
        }
    }

    if (!d3d11Available && !d3d12Available) {
        fprintf(stderr, "error: neither D3D11 nor D3D12 available.\n");
        return -1;
    }

    // DXGI Factory
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&gDXGIFactory)));

    if (gSettings.allowTearing)
    {
        if (gSettings.vsync)
        {
            std::cout << "Tearing cannot be used when vsync is on. Disabling tearing!" << std::endl;
            gSettings.allowTearing = false;
        }
    }

    // Setup GUI
    gD3D12Control = gGUI.AddSprite(5, 10, 140, 50, "directx12.dds");
    gD3D11Control = gGUI.AddSprite(5, 10, 140, 50, "directx11.dds");
    gFPSControl = gGUI.AddText(150, 10);

    ResetCameraView();
    // Camera projection set up in WM_SIZE

    AsteroidsSimulation asteroids(1337, NUM_ASTEROIDS, NUM_UNIQUE_MESHES, MESH_MAX_SUBDIV_LEVELS, NUM_UNIQUE_TEXTURES);

    // Create workloads
    if (d3d11Available) {
        gWorkloadD3D11 = new AsteroidsD3D11::Asteroids(&asteroids, &gGUI, gSettings.warp);
    }

    if (d3d12Available) {
        // If requested, enumerate the warp adapter
        // TODO: Allow picking from multiple hardware adapters
        IDXGIAdapter1* adapter = nullptr;

        if (gSettings.warp) {
            IDXGIFactory4* DXGIFactory4 = nullptr;
            if (FAILED(gDXGIFactory->QueryInterface(&DXGIFactory4))) {
                fprintf(stderr, "error: WARP requires IDXGIFactory4 interface which is not present on this system!\n");
                return -1;
            }

            auto hr = DXGIFactory4->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
            DXGIFactory4->Release();

            if (FAILED(hr)) {
                fprintf(stderr, "error: WARP adapter not present on this system!\n");
                return -1;
            }
        }

        gWorkloadD3D12 = new AsteroidsD3D12::Asteroids(&asteroids, &gGUI, NUM_SUBSETS, adapter);
    }
    gSettings.d3d12 = (gWorkloadD3D12 != nullptr);


    // init window class
    WNDCLASSEX windowClass;
    ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "AsteroidsD3D12WindowClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, gSettings.windowWidth, gSettings.windowHeight };
    AdjustWindowRect(&windowRect, windowedStyle, FALSE);

    // create the window and store a handle to it
    auto hWnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        "AsteroidsD3D12WindowClass",
        "AsteroidsD3D12",
        windowedStyle,
        0, // CW_USE_DEFAULT
        0, // CW_USE_DEFAULT
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        windowClass.hInstance,
        NULL);

    if (!gSettings.windowed) {
        ToggleFullscreen(hWnd);
    }

    SetForegroundWindow(hWnd);

    // Initialize performance counters
    UINT64 perfCounterFreq = 0;
    UINT64 lastPerfCount = 0;
    QueryPerformanceFrequency((LARGE_INTEGER*)&perfCounterFreq);
    QueryPerformanceCounter((LARGE_INTEGER*)&lastPerfCount);

    FILE* perfOutputFp = nullptr;
    if (perfOutputPath != nullptr) {
        perfOutputFp = fopen(perfOutputPath, "wb");
        if (perfOutputFp == nullptr) {
            fprintf(stderr, "warning: failed to open performance output file '%s'\n", perfOutputPath);
        } else {
            fprintf(perfOutputFp, "Frame time (ms),\n");
        }
    }

    // main loop
    double elapsedTime = 0.0;
    double frameTime = 0.0;
    int lastMouseX = 0;
    int lastMouseY = 0;

    timeBeginPeriod(1);
    EnableMouseInPointer(TRUE);

    for (;;)
    {
        bool d3d12LastFrame = gSettings.d3d12;

        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                // Cleanup
                if (perfOutputFp != nullptr) {
                    fclose(perfOutputFp);
                    perfOutputFp = nullptr;
                }

                delete gWorkloadD3D11;
                delete gWorkloadD3D12;
                SafeRelease(&gDXGIFactory);
                timeEndPeriod(1);
                EnableMouseInPointer(FALSE);
                return (int)msg.wParam;
            };

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Get time delta
        UINT64 count;
        QueryPerformanceCounter((LARGE_INTEGER*)&count);
        auto rawFrameTime = (double)(count - lastPerfCount) / perfCounterFreq;
        elapsedTime += rawFrameTime;
        lastPerfCount = count;

        // Maintaining absolute time sync is not important in this demo so we can err on the "smoother" side
        double alpha = 0.2f;
        frameTime = alpha * rawFrameTime + (1.0f - alpha) * frameTime;
        // If we swap to a new API we need to recreate swap chains
        if (d3d12LastFrame != gSettings.d3d12) {
            if (gSettings.d3d12) {
                gWorkloadD3D11->ReleaseSwapChain();
                gWorkloadD3D12->ResizeSwapChain(gDXGIFactory, hWnd, gSettings.renderWidth, gSettings.renderHeight, gSettings.allowTearing);
            } else {
                gWorkloadD3D12->ReleaseSwapChain();
                gWorkloadD3D11->ResizeSwapChain(gDXGIFactory, hWnd, gSettings.renderWidth, gSettings.renderHeight, gSettings.allowTearing);
            }
        }

        // Still need to process inertia even when no interaction is happening
        gCamera.ProcessInertia();

        // In D3D12 we'll wait on the GPU before taking the timestamp (more consistent)
        if (gSettings.d3d12) {
            gWorkloadD3D12->WaitForReadyToRender();
        }

        // Update GUI
        {
            char buffer[256];
            sprintf(buffer, "Asteroids D3D1%c - %4.1f ms", gSettings.d3d12 ? '2' : '1', 1000.f * frameTime);
            SetWindowText(hWnd, buffer);

            if (gSettings.lockFrameRate) {
                sprintf(buffer, "%u fps (Locked)", gSettings.lockedFrameRate);
            } else {
                sprintf(buffer, "%.0f fps", 1.0f / frameTime);
            }
            gFPSControl->Text(buffer);

            gD3D12Control->Visible(gSettings.d3d12);
            gD3D11Control->Visible(!gSettings.d3d12);
        }

        if (gSettings.d3d12) {
            gWorkloadD3D12->Render((float)frameTime, gCamera, gSettings);
        } else {
            gWorkloadD3D11->Render((float)frameTime, gCamera, gSettings);
        }

        if (perfOutputFp != nullptr) {
            fprintf(perfOutputFp, "%lf,\n", 1000.0 * frameTime);
        }

        if (gSettings.lockFrameRate) {
            ProfileBeginFrameLockWait();

            UINT64 afterRenderCount;
            QueryPerformanceCounter((LARGE_INTEGER*)&afterRenderCount);
            double renderTime = (double)(afterRenderCount - count) / perfCounterFreq;

            double targetRenderTime = 1.0 / double(gSettings.lockedFrameRate);
            double deltaMs = (targetRenderTime - renderTime) * 1000.0;
            if (deltaMs > 1.0) {
                Sleep((DWORD)deltaMs);
            }

            ProfileEndFrameLockWait();
        }

        // All done?
        if (gSettings.closeAfterSeconds > 0.0 && elapsedTime > gSettings.closeAfterSeconds) {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
    }

    // Shouldn't get here
    return 1;
}
