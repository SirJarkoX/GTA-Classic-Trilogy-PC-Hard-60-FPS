# GTA III / Vice City Hard60FPS

Shared native D3D8 60 FPS limiter for GTA III and Vice City.

The same ASI targets both games and hooks the common Direct3D 8 presentation
interface.

Verified development path:
- Direct3DCreate8 hook
- IDirect3D8::CreateDevice hook (slot 15)
- IDirect3DDevice8::Present hook (slot 15)
- QPC-based 60 FPS pacing

The implementation does not depend on the legacy Direct3D 8 SDK headers or
libraries.

## Build

Use the x86 Native Tools Command Prompt:

    cmake -S . -B build -A Win32
    cmake --build build --config Release

The resulting DLL can be renamed to:

    GTA3VC_Hard60FPS_SirJarko.asi

## Intended compatibility

- GTA III
- GTA Vice City
- Native D3D8
- D3D8 wrappers that preserve the standard D3D8 device interface

Wrapper compatibility should be verified against each specific wrapper.
