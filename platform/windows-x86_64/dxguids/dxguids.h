// Shim for <dxguids/dxguids.h> from microsoft/DirectX-Headers
// (see subprojects/DirectX-Headers.wrap).
//
// Upstream DirectX-Headers provides <dxguids/dxguids.h> for non-MSVC toolchains
// (such as GCC on Linux/WSL or MinGW) that lack MSVC's built-in __uuidof()
// operator. On Windows with clang-cl/MSVC, src/vulkan/wsi/wsi_common_win32.cpp
// uses __uuidof() directly and does not require any definitions from this
// header. If we later support the d3d12 Gallium driver, Dozen (vulkan_dzn), or
// another driver with a real dependency on DirectX-Headers, we should bring in
// the external DirectX-Headers package properly instead of using this stub.
#pragma once
