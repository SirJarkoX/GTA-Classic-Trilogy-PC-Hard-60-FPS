# GTA Classic Trilogy PC - Hard 60 FPS

**Projects:** `GTA3VC_Hard60FPS_SirJarko.asi` / `GTASA_Hard60FPS_SirJarko.asi`  \
**Status:** Release (Open for testing)  \
**Targets:** Grand Theft Auto III, Grand Theft Auto: Vice City & Grand Theft Auto: San Andreas (classic PC releases, 32-bit/x86)  \
**Author:** SirJarko

## What is this?

**GTA Classic Trilogy PC - Hard 60 FPS** is a small ASI plugin project for the
classic PC versions of **Grand Theft Auto III, Grand Theft Auto: Vice City
and Grand Theft Auto: San Andreas**.

The goal is simple:

> **Hard 60 FPS for the classic GTA trilogy on PC.**

The trilogy uses **two implementations** rather than one universal codebase:

- **GTA III / Vice City:** one shared ASI using the games' common Direct3D 8
  presentation interface.
- **GTA San Andreas:** a separate ASI using San Andreas' Direct3D 9
  presentation path.

The implementations are intentionally separate because the games do not
use the same renderer/API path, even though they share substantial technical
heritage.

The limiter works at the game's presentation layer rather than relying on
GPU driver frame limiting. This was created because driver-level caps can
be inconsistent in older games, particularly across gameplay, menus,
cutscenes and other parts of the game.

The project is also intended as a small preservation-oriented project:
the source is kept relatively simple and dependency-light so it can remain
useful as hardware, drivers and graphics wrappers change.

## Why is this?

I moved to an AMD GPU well over a year ago. In my experience, driver-level
framerate capping is not quite as consistent with these older games as I
wanted.

For example, a driver-level 60 FPS cap may behave correctly during
gameplay while menus, cutscenes or other parts of the game behave
differently.

For classic games such as **GTA III, Vice City and San Andreas**, I wanted
a more consistent solution that controls presentation from the game/plugin
side rather than relying entirely on the GPU driver.

After more than a year of not finding a solution that properly addressed
what I wanted, I decided to develop one myself.

The project has since grown into a single open-source trilogy project.
The three games are not forced into one implementation: III and VC share
their D3D8 implementation, while San Andreas has its own D3D9
implementation.

The long-term goal is also preservation. The code is intentionally kept
relatively small and dependency-light so that it can remain understandable,
buildable and useful as hardware, drivers and graphics wrappers change.

## Installation

You do **not** need to understand how the limiter works to use it.

### You need

- A legitimate PC installation of GTA III, Vice City or San Andreas.
- A working **ASI loader**.
- The ASI for the game you want to use.

### GTA III

Use:

```text
GTA3VC_Hard60FPS_SirJarko.asi
```

Put it into the game's main directory (the folder containing `gta3.exe`)
alongside your ASI loader, or into your `scripts` folder if your setup
uses one.

### Grand Theft Auto: Vice City

Use:

```text
GTA3VC_Hard60FPS_SirJarko.asi
```

Put the same ASI into the game's main directory (the folder containing
`gta-vc.exe`) alongside your ASI loader, or into your `scripts` folder if
your setup uses one.

### Grand Theft Auto: San Andreas

Use:

```text
GTASA_Hard60FPS_SirJarko.asi
```

Put it into the game's main directory (the folder containing `gta_sa.exe`)
alongside your ASI loader, or into your `scripts` folder if your setup
uses one.

### Renderer / wrapper notes

You do not need a wrapper to use the limiters.

- **GTA III / Vice City:** native Direct3D 8 is supported. The shared
  implementation has also been tested with D3D8 wrapper configurations.
- **San Andreas:** native Direct3D 9 is supported. The SA implementation
  has also been tested with DXVK.

The wrapper/translation layer is separate from this project and is not
bundled with it. Compatibility with an arbitrary future wrapper is not
guaranteed and should be tested.

No original game files are distributed with this project.

## What it does

- Locks GTA III to 60 FPS.
- Locks Vice City to 60 FPS.
- Locks San Andreas to 60 FPS.
- Uses one shared ASI for GTA III and Vice City.
- Uses a separate SA-specific ASI for San Andreas.
- Targets the original 32-bit/x86 PC releases.
- Uses high-resolution Windows timing for frame pacing.
- Is designed around the game's rendering/presentation interfaces rather
  than a specific GPU vendor or driver.
- Is intended to coexist with other fixes and modifications where possible.

It does **not**:

- Overhaul the game engines.
- Include copyrighted Rockstar game assets.
- Fix unrelated rendering bugs.
- Guarantee compatibility with every game build, mod, driver, GPU, Windows
  version or third-party wrapper.

## Confirmed development tests

| Game | Native renderer | Tested wrapper / translation | 60 FPS |
| --- | --- | --- | --- |
| GTA III | Direct3D 8 | D3D8 wrapper | Confirmed |
| Vice City | Direct3D 8 | D3D8 wrapper | Confirmed |
| San Andreas | Direct3D 9 | DXVK | Confirmed |

GTA III and Vice City use the same final ASI implementation. San Andreas
uses its own implementation because its rendering path and hooking
requirements differ.

A GTA III taxi-related visual issue was observed with native D3D8. It also
occurred without this ASI and disappeared when the tested wrapper was
enabled, so it is not currently considered an issue caused by the limiter.

## How it works

The project contains **two implementations** for the three games.

### GTA III / Vice City

III and VC share a common Direct3D 8 implementation:

```text
GTA III / Vice City
        |
        v
Direct3DCreate8
        |
        v
IDirect3D8::CreateDevice
        |
        v
IDirect3DDevice8::Present
        |
        v
60 FPS frame pacing
```

The implementation hooks the D3D8 `CreateDevice` and `Present` path and
applies frame pacing at `IDirect3DDevice8::Present`.

### San Andreas

San Andreas uses a separate Direct3D 9 implementation because its
rendering path is different from III and VC.

The SA implementation performs its frame-pacing work against the game's
D3D9 presentation path rather than using the D3D8 hooks used by III/VC.

### Timing

The implementations use Windows high-resolution performance timing
(`QueryPerformanceCounter` / `QueryPerformanceFrequency`) for frame
pacing, with `Sleep` for coarse waiting and `SwitchToThread` for the final
portion.

The project does not require the legacy Direct3D 8 SDK headers or
libraries for the III/VC implementation. The required interface types are
declared locally so it can be built with a modern Visual Studio C++
toolchain.

The important architectural distinction is that the repository is one
project, but the game implementations remain separate where the rendering
interfaces require it.

## Building from source

### Requirements

- Visual Studio / Visual Studio C++ Build Tools
- CMake
- VS Code (development environment used for the project)
- x86 Native Tools Command Prompt for Visual Studio
- A Windows SDK/toolchain capable of building 32-bit C++

All three games are 32-bit/x86, so the project is built as **Win32**.

### Build

Each implementation has its own project/build files. From the relevant
implementation directory, use an x86 Native Tools Command Prompt:

```bat
cmake -S . -B build -A Win32
cmake --build build --config Release
```

The resulting DLL is renamed to the appropriate ASI filename:

```text
GTA3VC_Hard60FPS_SirJarko.asi
GTASA_Hard60FPS_SirJarko.asi
```

Then install the ASI using a compatible ASI loader.

The repository intentionally keeps the game implementations organized
separately rather than combining unrelated renderer-specific code into a
single source tree.

## Compatibility

The current release is **1.0**. The configurations listed in the testing
table above are confirmed development results, not a guarantee for every
possible environment.

Useful additional test environments include:

- Windows Vista
- Windows 7
- Windows 10
- Windows 11
- AMD and NVIDIA GPUs
- Different driver generations
- Clean and modded installations
- Different resolutions and display configurations
- Different GTA III, Vice City and San Andreas releases/builds
- Additional D3D8 wrappers
- Additional D3D9 translation layers

Compatibility with individual mods should be considered game and version
specific.

Related projects/tools include **SilentPatch**, **Widescreen Fix**,
**Open Limit Adjuster (OLA)** and **DXVK**. These are separate projects
and are not bundled with this project.

## Why open source?

The project is being released openly so others can:

- Review the implementation.
- Compile it themselves.
- Test hardware and game configurations I cannot access.
- Find bugs and improve compatibility.
- Test additional game builds and graphics wrappers.
- Test configurations I cannot access across all three games.
- Submit fixes or improvements.
- Learn from or build upon the work.

This is a practical project, not a claim of complete mastery of the GTA
III, Vice City or San Andreas engines. If an assumption or implementation is wrong,
I'd rather it be visible and correctable.

## AI-assisted development

These projects were developed with **significant AI assistance**.

AI was used for C/C++ development assistance, debugging, understanding
unfamiliar code, exploring implementation approaches, technical
explanations, code review and documentation.

The projects were not automatically generated and validated by AI.
They required human direction, experimentation, testing and decisions
about implementation and validation.

AI-generated code can be wrong or introduce bugs. If you find a problem,
please report it.

## Disclaimer

Grand Theft Auto III, Grand Theft Auto: Vice City, Grand Theft Auto: San Andreas and their associated
intellectual property belong to **Rockstar Games / Take-Two Interactive**.

This is an unofficial community project and is not affiliated with,
endorsed by or sponsored by Rockstar Games or Take-Two Interactive.

No copyrighted Rockstar game assets are distributed with this project.
Users are responsible for obtaining and legally owning their own copies
of the games.

## Credits

**Project / development:** SirJarko

**Implementations:** GTA III / Vice City share a D3D8 implementation; San Andreas uses a separate D3D9 implementation.

**AI assistance:** AI tools were used extensively for programming
assistance, debugging, technical research, code review and documentation.

**Related third-party projects/tools:** SilentPatch, Widescreen Fix,
Open Limit Adjuster and DXVK.

Please refer to each project's own documentation and licence for its
respective terms.

## License

See `LICENSE` in this repository.
