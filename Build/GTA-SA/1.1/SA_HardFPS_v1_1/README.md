# GTA San Andreas Hard FPS — v1.1

Configurable FPS limiter for the classic PC version of GTA San Andreas.

Supports:
- Native Direct3D 9
- DXVK's Direct3D 9 implementation

The limiter operates at `IDirect3DDevice9::Present` and does not hook Vulkan directly.

## Configuration

Create `HardFPS.ini` in the same folder as the ASI and add a single line:

    fps=60

Any integer from **2 to 1000** is accepted.

- Missing or invalid `HardFPS.ini` → 30 FPS
- Values below 2 → 2 FPS
- Values above 1000 → 1000 FPS

Examples:

    fps=30
    fps=60
    fps=582
    fps=1000

The limiter only controls presentation timing. Game-engine, CPU, GPU, streaming, and other performance limitations remain outside its control.

## Build

Use the **x86 Native Tools Command Prompt for Visual Studio**:

    cmake -S . -B build -A Win32
    cmake --build build --config Release

The resulting DLL is:

    build\Release\SA_HardFPS_v1_1.dll

Rename it to:

    SA_HardFPS_v1_1.asi

## Testing

Test first with native D3D9 (no local DXVK `d3d9.dll`), then with DXVK.

The 1.1 implementation retains the existing D3D9 `CreateDevice` and `Present` hook architecture while making the target FPS configurable.
