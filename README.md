# GTA Classic Trilogy PC - Hard FPS Limiter

**Projects:** `GTA3VC_HardFPS_v1_1.asi` / `SA_HardFPS_v1_1.asi`  
**Status:** Release 1.1  
**Targets:** Grand Theft Auto III, Grand Theft Auto: Vice City & Grand Theft Auto: San Andreas (classic PC releases, 32-bit/x86)  
**Author:** SirJarko

## What is this?

**GTA Classic Trilogy PC - Hard FPS Limiter** is a small ASI plugin project for the classic PC versions of **Grand Theft Auto III, Grand Theft Auto: Vice City and Grand Theft Auto: San Andreas**.

The goal is simple:

> **Give the user a hard, configurable FPS limit at the game's presentation layer.**

Version 1.0 provided a fixed 60 FPS limit. Version 1.1 expands this so the user can choose **any integer FPS value from 2 to 1000**.

The trilogy uses **two implementations** rather than one universal codebase:

- **GTA III / Vice City:** one shared ASI using the games' common Direct3D 8 presentation interface.
- **GTA San Andreas:** a separate ASI using San Andreas' Direct3D 9 presentation path.

The implementations remain separate because the games do not use the same renderer/API path, even though they share substantial technical heritage.

The limiter works at the game's presentation layer rather than relying on GPU driver frame limiting. This was created because driver-level caps can be inconsistent in older games, particularly across gameplay, menus, cutscenes and other parts of the game.

The project is also intended as a small preservation-oriented project: the source is kept relatively simple and dependency-light so it can remain useful as hardware, drivers and graphics wrappers change.

## Installation

You do **not** need to understand how the limiter works to use it.

### You need

- A legitimate PC installation of GTA III, Vice City or San Andreas.
- A working **ASI loader**.
- The ASI for the game you want to use.
- `HardFPS.ini` if you want to choose a specific FPS target.

### GTA III

Use:

```text
GTA3VC_HardFPS_v1_1.asi
HardFPS.ini
```

Put it into the game's main directory (the folder containing `gta3.exe`) alongside your ASI loader, or into your `scripts` folder if your setup uses one.

### Grand Theft Auto: Vice City

Use:

```text
GTA3VC_HardFPS_v1_1.asi
HardFPS.ini
```

Put the same ASI into the game's main directory (the folder containing `gta-vc.exe`) alongside your ASI loader, or into your `scripts` folder if your setup uses one.

### Grand Theft Auto: San Andreas

Use:

```text
SA_HardFPS_v1_1.asi
HardFPS.ini
```

Put it into the game's main directory (the folder containing `gta_sa.exe`) alongside your ASI loader, or into your `scripts` folder if your setup uses one.

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

Examples:

```ini
fps=30
```

```ini
fps=60
```

```ini
fps=144
```

```ini
fps=582
```

```ini
fps=1000
```

The limiter calculates the target frame interval from the value provided.

### Invalid or missing configuration

If `HardFPS.ini` is missing or the value cannot be read as a valid number, the limiter defaults to **30 FPS**.

Values outside the supported range are clamped:

- Below `2` → `2 FPS`
- Above `1000` → `1000 FPS`

The limiter does not attempt to determine whether a particular FPS target is suitable for a specific game, engine configuration or mod setup. Higher or lower targets may expose engine, physics, streaming or other game-specific limitations. The configured value is the user's choice.

## Why is this?

I moved to an AMD GPU well over a year ago. In my experience, driver-level framerate capping is not quite as consistent with these older games as I wanted.

For example, a driver-level 60 FPS cap may behave correctly during gameplay while menus, cutscenes or other parts of the game behave differently.

For classic games such as **GTA III, Vice City and San Andreas**, I wanted a more consistent solution that controls presentation from the game/plugin side rather than relying entirely on the GPU driver.

After more than a year of not finding a solution that properly addressed what I wanted, I decided to develop one myself.

The project has since grown into a single open-source trilogy project. The three games are not forced into one implementation: III and VC share their D3D8 implementation, while San Andreas has its own D3D9 implementation.

The long-term goal is also preservation. The code is intentionally kept relatively small and dependency-light so that it can remain understandable, buildable and useful as hardware, drivers and graphics wrappers change.

### Renderer / wrapper notes

You do not need a wrapper to use the limiters.

- **GTA III / Vice City:** native Direct3D 8 is supported. The shared implementation has also been tested with D3D8/D3D9 wrapper configurations.
- **San Andreas:** native Direct3D 9 is supported. The SA implementation has also been tested with DXVK.

The wrapper/translation layer is separate from this project and is not bundled with it. Compatibility with an arbitrary future wrapper is not guaranteed and should be tested.

No original game files are distributed with this project.

## What it does

- Provides a configurable FPS limit from **2–1000 FPS**.
- Defaults to **30 FPS** when `HardFPS.ini` is missing or invalid.
- Accepts arbitrary integer FPS targets within the supported range.
- Uses one shared ASI for GTA III and Vice City.
- Uses a separate SA-specific ASI for San Andreas.
- Supports the original 32-bit/x86 PC releases.
- Uses high-resolution Windows timing for frame pacing.
- Is designed around the game's rendering/presentation interfaces rather than a specific GPU vendor or driver.
- Is intended to coexist with other fixes and modifications where possible.

It does **not**:

- Overhaul the game engines.
- Guarantee that the original games will behave correctly at every possible FPS target.
- Include copyrighted Rockstar game assets.
- Fix unrelated rendering or engine bugs.
- Guarantee compatibility with every game build, mod, driver, GPU, Windows version or third-party wrapper.

## Confirmed development tests

| Game | Native renderer | Tested wrapper / translation | Configurable limiter |
| --- | --- | --- | --- |
| GTA III | Direct3D 8 | D3D9 wrapper | Confirmed |
| Vice City | Direct3D 8 | D3D9 wrapper | Confirmed |
| San Andreas | Direct3D 9 | DXVK | Confirmed |

The configurable limiter has been tested across a range of FPS values, including very low, typical, high and unusual targets, as well as the supported boundaries and invalid configuration handling.

GTA III and Vice City use the same final ASI implementation. San Andreas uses its own implementation because its rendering path and hooking requirements differ.

A GTA III taxi-related visual issue was observed with native D3D8. It also occurred without this ASI and disappeared when the tested wrapper was enabled, so it is not currently considered an issue caused by the limiter.

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

The important architectural distinction is that the repository is one project, but the game implementations remain separate where the rendering interfaces require it.

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

Then install the ASI using a compatible ASI loader.

The repository intentionally keeps the game implementations organized separately rather than combining unrelated renderer-specific code into a single source tree.

## Compatibility

The current release is **1.1**. The configurations listed in the testing table above are confirmed development results, not a guarantee for every possible environment.

Confirmed testing includes:

- Windows 11
- AMD and NVIDIA GPUs
- Native Direct3D 8 for GTA III / Vice City
- D3D9 wrapper configurations for GTA III / Vice City
- Native Direct3D 9 for San Andreas
- DXVK for San Andreas
- Clean and modded installations
- Different resolutions and display configurations
- A range of configured FPS targets, including low, high, arbitrary and boundary values

Additional testing of older Windows versions, other driver generations, game builds, wrappers and translation layers is welcome.

Compatibility with individual mods should be considered game and version specific.

Related projects/tools include **SilentPatch**, **Widescreen Fix**, **Open Limit Adjuster (OLA)** and **DXVK**. These are separate projects and are not bundled with this project.

## Why open source?

The project is being released openly so others can:

- Review the implementation.
- Compile it themselves.
- Test hardware and game configurations I cannot access across all three games.
- Find bugs and improve compatibility.
- Test additional game builds and graphics wrappers.
- Submit fixes or improvements.
- Learn from or build upon the work.

This is a practical project, not a claim of complete mastery of the GTA III, Vice City or San Andreas engines. If an assumption or implementation is wrong, I'd rather it be visible and correctable.

## AI-assisted development

These projects were developed with **significant AI assistance**.

AI was used for C/C++ development assistance, debugging, understanding unfamiliar code, exploring implementation approaches, technical explanations, code review and documentation.

The projects were not automatically generated and validated by AI. They required human direction, experimentation, testing and decisions about implementation and validation.

AI-generated code can be wrong or introduce bugs. If you find a problem, please report it.

## Disclaimer

Grand Theft Auto III, Grand Theft Auto: Vice City, Grand Theft Auto: San Andreas and their associated intellectual property belong to **Rockstar Games / Take-Two Interactive**.

This is an unofficial community project and is not affiliated with, endorsed by or sponsored by Rockstar Games or Take-Two Interactive.

No copyrighted Rockstar game assets are distributed with this project. Users are responsible for obtaining and legally owning their own copies of the games.

## Credits

**Project / development:** SirJarko

**Implementations:** GTA III / Vice City share a D3D8 implementation; San Andreas uses a separate D3D9 implementation.

**AI assistance:** AI tools were used extensively for programming assistance, debugging, technical research, code review and documentation.

**Related third-party projects/tools:** SilentPatch, Widescreen Fix, Open Limit Adjuster and DXVK.

Please refer to each project's own documentation and licence for its respective terms.

## License

See `LICENSE` in this repository.
