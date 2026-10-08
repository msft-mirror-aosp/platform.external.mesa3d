// Shim for <directx/d3d12.h> from microsoft/DirectX-Headers
// (see subprojects/DirectX-Headers.wrap).
//
// For Lavapipe (vulkan_lvp), src/vulkan/wsi/wsi_common_win32.cpp only needs
// baseline D3D12 COM interfaces that are already provided by the Windows SDK's
// <d3d12.h>. If we later support the d3d12 Gallium driver, Dozen (vulkan_dzn),
// or another driver with a real dependency on DirectX-Headers (such as newer
// D3D12 Agility SDK types or d3dx12 helpers), we should bring in the external
// DirectX-Headers package properly instead of using this forwarding header.
#pragma once
#include <d3d12.h>
