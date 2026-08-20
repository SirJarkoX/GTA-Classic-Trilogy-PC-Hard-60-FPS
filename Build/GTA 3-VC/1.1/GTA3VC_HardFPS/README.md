# GTA III / Vice City HardFPS v1.1

Configurable native D3D8 FPS limiter for GTA III and Vice City.

The same ASI targets both games and hooks the common Direct3D 8 presentation
interface.

## Configuration

Create `HardFPS.ini` in the same directory as the ASI and add:

    fps=60

The limiter accepts **any integer from 2 to 1000 FPS**.

Examples:

    fps=30
    fps=60
    fps=144
    fps=582
    fps=1000

If `HardFPS.ini` is missing or the value is invalid, the limiter defaults to
30 FPS.

Values below 2 are clamped to 2 FPS. Values above 1000 are clamped to
1000 FPS.

The configured value is a presentation target. Game-engine behaviour at
higher or lower frame rates is outside the limiter's control.

## Build

Use the x86 Native Tools Command Prompt:

    cmake -S . -B build -A Win32
    cmake --build build --config Release

The resulting DLL can be renamed to:

    GTA3VC_HardFPS_SirJarko.asi

## Intended compatibility

- GTA III
- GTA Vice City
- Native D3D8
- D3D8 wrappers that preserve the standard D3D8 device interface

Wrapper compatibility should be verified against each specific wrapper.

## Implementation

- Direct3DCreate8 hook
- IDirect3D8::CreateDevice hook (slot 15)
- IDirect3DDevice8::Present hook (slot 15)
- QPC-based configurable FPS pacing
- No dependency on the legacy Direct3D 8 SDK headers or libraries
