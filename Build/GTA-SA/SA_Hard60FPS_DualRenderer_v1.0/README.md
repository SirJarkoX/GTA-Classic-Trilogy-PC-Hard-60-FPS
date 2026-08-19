# GTA San Andreas Hard 60 FPS — Dual Renderer

This build is intended to support both:

- Native Windows Direct3D 9
- DXVK's Direct3D 9 implementation

The limiter operates at the common IDirect3DDevice9::Present interface.
It does not inspect or depend on the DXVK version and does not hook Vulkan.

The implementation keeps the device's original vtable in place and replaces
only the Present entry.

## Build

Use the x86 Native Tools Command Prompt:

    cmake -S . -B build -A Win32
    cmake --build build --config Release

Rename the resulting DLL to:

    SA_Hard60FPS_SirJarko.asi

## Testing

Test first with native D3D9 (no local DXVK d3d9.dll), then with DXVK.

Target presentation rate: 60 FPS.
