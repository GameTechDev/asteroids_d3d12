# DISCONTINUATION OF PROJECT #  
This project will no longer be maintained by Intel.  
Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.  
Intel no longer accepts patches to this project.  
 If you have an ongoing need to use this project, are interested in independently developing it, or would like to maintain patches for the open source software community, please create your own fork of this project.  
  
Asteroids D3D12
===============

The Intel asteroids sample was developed as an example of how to use the
DirectX 12 graphics API to get performance and power benefits over previous
APIs. For more information on the sample, please see:
https://software.intel.com/en-us/blogs/2014/08/11/siggraph-2014-directx-12-on-intel

Usage
========

```
usage: asteroids_d3d12 [options]
options:
  -close_after [seconds]
  -nod3d11
  -nod3d12
  -fullscreen
  -window [width] [height]
  -render_scale [scale]
  -locked_fps [fps]
  -perf_output [path]
  -warp
```

Controls
========

| Key | Operation |
| --- | --------- |
| 1 | Use D3D11 |
| 2 | Use D3D12 |
| Space | Toggle animation |
| V | Toggle vsync |
| M | Toggle multi-threaded rendering (D3D12 only) |
| I | Toggle execute indirect rendering (D3D12 only) |
| S | Toggle commandlist submission (D3D12 only) |
| Esc | Exit application |

Requirements
============
- Windows 10. The application will run on older operating systems but the DirectX 12 path will not be available.
- Visual Studio 2019
- DirectX 12 capable hardware and drivers. For instance, Intel HD Graphics 4400 or newer.

For more information on Intel graphics and game code, please visit https://software.intel.com/gamedev

Last touched 6/9/23

