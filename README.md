# GTA Classic Trilogy PC - Hard FPS Limiter

**Projects:** `GTA3VC_HardFPS_v1_1.asi` / `SA_HardFPS_v1_1.asi`  
**Status:** Release 1.1  
**Targets:** Grand Theft Auto III, Grand Theft Auto: Vice City & Grand Theft Auto: San Andreas (classic PC releases, 32-bit/x86)  
**Author:** SirJarko

## What it does

**GTA Classic Trilogy PC - Hard FPS Limiter** is a small ASI plugin project for the classic PC versions of **Grand Theft Auto III, Grand Theft Auto: Vice City and Grand Theft Auto: San Andreas**.

Version 1.1 provides a configurable FPS limit from **2 to 1000 FPS** using a simple `HardFPS.ini` at the game's presentation layer.

The trilogy uses two renderer-specific implementations:

- **GTA III / Vice City:** one shared ASI using their common Direct3D 8 presentation interface.
- **San Andreas:** a separate ASI using San Andreas' Direct3D 9 presentation path.

The limiter operates at the game's presentation layer rather than relying on GPU driver frame limiting. Native D3D8/D3D9 is supported, with tested wrapper/translation configurations for GTA III, Vice City and San Andreas.

The source is intentionally kept relatively simple and dependency-light so it can remain useful as hardware, drivers and graphics wrappers change.

## Installation

You do **not** need to understand how the limiter works to use it.

### You need

- A legitimate PC installation of GTA III, Vice City or San Andreas.
- A working **ASI loader**.
- The ASI and bundled `HardFPS.ini` for the game you want to use.

### GTA III, Grand Theft Auto: Vice City

Use:

```text
GTA3VC_HardFPS_v1_1.asi
HardFPS.ini
```

Put both files into the corrosponding game's main directory (the folder containing `gta3.exe`) for GTA III or (the folder containing `gta-vc.exe`) for GTA: Vice City 
alongside your ASI loader, or into your `scripts` folder if your setup uses one.

### Grand Theft Auto: San Andreas

Use:

```text
SA_HardFPS_v1_1.asi
HardFPS.ini
```

Put both files into the game's main directory (the folder containing `gta_sa.exe`) alongside your ASI loader, or into your `scripts` folder if your setup uses one.

## FPS configuration

Version 1.1 uses a deliberately simple configuration file:

```text
HardFPS.ini
```

The file contains only:

```ini
fps=60
```

The value can be **any integer from 2 to 1000**. There is no predefined list of supported FPS targets.

For example:

```ini
fps=144
```

The limiter calculates the target frame interval from the value provided.

### Invalid or missing configuration

If `HardFPS.ini` is missing or the value cannot be read as a valid number, the limiter falls back to **30 FPS**.

Values outside the supported range are clamped:

- Below `2` → `2 FPS`
- Above `1000` → `1000 FPS`

The limiter does not attempt to determine whether a particular FPS target is suitable for a specific game, engine configuration or mod setup. Higher or lower targets may expose engine, physics, streaming or other game-specific limitations. The configured value is the user's choice.

## Why I made this

I moved to an AMD GPU well over a year ago. In my experience, driver-level framerate capping is not quite as consistent with these older games as I wanted.

For example, a driver-level 60 FPS cap may behave correctly during gameplay while menus, cutscenes or other parts of the game behave differently.

For classic games such as **GTA III, Vice City and San Andreas**, I wanted a more consistent solution that controls presentation from the game/plugin side rather than relying entirely on the GPU driver.

After more than a year of not finding a solution that properly addressed what I wanted, I decided to develop one myself.

The project is also intended as a small preservation-oriented project: the code is kept relatively simple and dependency-light so that it can remain understandable, buildable and useful as hardware, drivers and graphics wrappers change.

## Compatibility

| Game | Native renderer | Tested wrapper / translation | Configurable limiter |
| --- | --- | --- | --- |
| GTA III | Direct3D 8 | D3D9 wrapper | Confirmed |
| Vice City | Direct3D 8 | D3D9 wrapper | Confirmed |
| San Andreas | Direct3D 9 | DXVK | Confirmed |

Testing has also been performed on:

- Windows 11
- AMD and NVIDIA GPUs
- Clean and modded installations
- Different resolutions and display configurations
- A range of configured FPS targets, including low, high, arbitrary and boundary values
- Invalid and missing configuration handling

The wrapper/translation layer is separate from this project and is not bundled with it. Compatibility with an arbitrary future wrapper is not guaranteed and should be tested.

Compatibility with individual mods should be considered game and version specific.

A GTA III taxi-related visual issue was observed with native D3D8. It also occurred without this ASI and disappeared when the tested wrapper was enabled, so it is not currently considered an issue caused by the limiter.

Related projects/tools include **SilentPatch**, **Widescreen Fix**, **Open Limit Adjuster (OLA)** and **DXVK**. These are separate projects and are not bundled with this project.

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
Configurable FPS frame pacing
```

The implementation hooks the D3D8 `CreateDevice` and `Present` path and applies frame pacing at `IDirect3DDevice8::Present`.

### San Andreas

San Andreas uses a separate Direct3D 9 implementation because its rendering path is different from III and VC.

The SA implementation performs its frame-pacing work against the game's D3D9 presentation path rather than using the D3D8 hooks used by III/VC.

### Timing

The implementations use Windows high-resolution performance timing (`QueryPerformanceCounter` / `QueryPerformanceFrequency`) for frame pacing, with `Sleep` for coarse waiting and `SwitchToThread` for the final portion.

The project does not require the legacy Direct3D 8 SDK headers or libraries for the III/VC implementation. The required interface types are declared locally so it can be built with a modern Visual Studio C++ toolchain.

## Building from source

### Requirements

- Visual Studio / Visual Studio C++ Build Tools
- CMake
- VS Code (development environment used for the project)
- x86 Native Tools Command Prompt for Visual Studio
- A Windows SDK/toolchain capable of building 32-bit C++

All three games are 32-bit/x86, so the project is built as **Win32**.

### Source organization

The source/build tree is separated by implementation:

```text
Build/
├── GTA 3-VC/
│   ├── 1.0/
│   └── 1.1/
└── GTA-SA/
    ├── 1.0/
    └── 1.1/
```

Each version keeps its own source/build material. The 1.0 directories preserve the original fixed-60 implementation and its build/release material, while the 1.1 directories contain the configurable FPS implementation and its corresponding build/release material.

The built ASIs and release ZIPs are kept alongside their respective version's build material so the source tree retains the relationship between source, compiled output and release files.

### Build

From the relevant version/implementation directory, use an x86 Native Tools Command Prompt:

```bat
cmake -S . -B build -A Win32
cmake --build build --config Release
```

The resulting DLL is renamed to the appropriate ASI filename:

```text
GTA3VC_HardFPS_v1_1.asi
SA_HardFPS_v1_1.asi
```

For example, from the relevant project directory:

```bat
cmake -S . -B build -A Win32
cmake --build build --config Release
```

The compiled output will be located in the CMake build directory under the `Release` configuration. Rename the resulting DLL to the appropriate `.asi` filename above, then install it using a compatible ASI loader.

The repository's 1.0 and 1.1 directories contain their own build material, so build from the version and game implementation you want to compile.

## Why open source?

The project is open source so others can review, compile, test and improve it, including across hardware, game builds and graphics wrappers I cannot personally access.

This is a practical project, not a claim of complete mastery of the GTA III, Vice City or San Andreas engines. If an assumption or implementation is wrong, I'd rather it be visible and correctable.

## Disclaimer

Grand Theft Auto III, Grand Theft Auto: Vice City, Grand Theft Auto: San Andreas and their associated intellectual property belong to **Rockstar Games / Take-Two Interactive**.

This is an unofficial community project and is not affiliated with, endorsed by or sponsored by Rockstar Games or Take-Two Interactive.

No copyrighted Rockstar game assets are distributed with this project. Users are responsible for obtaining and legally owning their own copies of the games.

## AI-assisted development

These projects were developed with **significant AI assistance**.

AI was used for C/C++ development assistance, debugging, understanding unfamiliar code, exploring implementation approaches, technical explanations, code review and documentation.

The projects were not automatically generated and validated by AI. They required human direction, experimentation, testing and decisions about implementation and validation.

AI-generated code can be wrong or introduce bugs. If you find a problem, please report it.

## Credits

**Project / development:** SirJarko

**Implementations:** GTA III / Vice City share a D3D8 implementation; San Andreas uses a separate D3D9 implementation.

**AI assistance:** AI tools were used extensively for programming assistance, debugging, technical research, code review and documentation.

**Related third-party projects/tools:** SilentPatch, Widescreen Fix, Open Limit Adjuster and DXVK.

Please refer to each project's own documentation and licence for its respective terms.

## License

See `LICENSE` in this repository.
