# GTA III / Vice City — Hard 60 FPS

**Project:** `GTA3VC_Hard60FPS_SirJarko.asi`  \
**Status:** Beta  \
**Targets:** Grand Theft Auto III & Grand Theft Auto: Vice City (PC, 32-bit/x86)  \
**Author:** SirJarko

## What is this?

**GTA III / Vice City — Hard 60 FPS** is a small ASI plugin that locks the
classic PC versions of **Grand Theft Auto III** and **Grand Theft Auto Vice City** to **60 FPS**.

The important part is simple:

> **One ASI, two games, native D3D8 or a compatible D3D8 wrapper.**

The limiter works at the game's presentation layer rather than relying on
GPU driver frame limiting. This was created because driver-level caps can
be inconsistent in older games, particularly across gameplay, menus,
cutscenes and other parts of the game.

The project is also intended as a small preservation-oriented project:
the source is kept relatively simple and dependency-light so it can remain
useful as hardware, drivers and graphics wrappers change.

## Why is this?

I moved to an AMD GPU well over a year ago. In my experience, AMD's
driver-level framerate capping is not quite on par with NVIDIA's for
these older games.

For example, when these games are capped to 60 FPS through driver
software, the cap may work in gameplay while menus, cutscenes and other
parts of the game may or may not remain locked.

For classic games such as **GTA III, Vice City and San Andreas**, this
does not provide the consistent result I wanted.

After more than a year of not finding a concrete solution that properly
addressed the problem, I decided to take it upon myself to develop one.

The San Andreas implementation was the first version developed. III and
VC are being approached separately and released as beta implementations
while their individual behaviour is investigated and tested.

## Installation

You do **not** need to understand how the limiter works to use it.

### You need

- A legitimate PC installation of GTA III or Vice City.
- A working **ASI loader**.
- `GTA3VC_Hard60FPS_SirJarko.asi`.

### GTA III

Put the ASI into the game's main directory (the folder containing
`gta3.exe`) alongside your ASI loader, or into your `scripts` folder if
your setup uses one.

```text
Grand Theft Auto III/
├── gta3.exe
├── [ASI loader]
├── GTA3VC_Hard60FPS_SirJarko.asi
└── ...
```

### Vice City

Use the **same ASI**. Put it into the game's main directory (the folder
containing `gta-vc.exe`) alongside your ASI loader, or into your `scripts`
folder if your setup uses one.

```text
Grand Theft Auto Vice City/
├── gta-vc.exe
├── [ASI loader]
├── GTA3VC_Hard60FPS_SirJarko.asi
└── ...
```

No original game files are distributed with this project.

### Wrappers

You do not need a wrapper to use the limiter. Native Direct3D 8 is
supported.

The same ASI was also tested successfully with **two different D3D8 wrapper
configurations**, one for each game. Compatibility with an arbitrary future
wrapper is not guaranteed, but a wrapper that preserves the normal D3D8
device/presentation interface is a candidate for testing.

## What it does

- Locks GTA III and Vice City to 60 FPS.
- Uses one shared ASI implementation for both games.
- Targets the original 32-bit/x86 PC versions.
- Works with native Direct3D 8.
- Has been tested with D3D8 wrapper configurations.
- Uses high-resolution Windows timing for frame pacing.
- Is intended to coexist with other fixes and modifications where possible.

It does **not**:

- Overhaul the game engines.
- Include copyrighted Rockstar game assets.
- Fix unrelated rendering bugs.
- Guarantee compatibility with every game build, mod, driver, GPU,
  Windows version or wrapper.

## Confirmed development tests

| | GTA III | Vice City |
| --- | --- | --- |
| Native Direct3D 8 | Tested | Tested |
| D3D8 wrapper configuration | Tested | Tested |
| Hard 60 FPS | Tested | Tested |
| Shared ASI | Yes | Yes |

The same final ASI binary was used for the tested GTA III and Vice City
configurations.

A GTA III taxi-related visual issue was observed with native D3D8. It also
occurred without this ASI and disappeared when the tested wrapper was
enabled, so it is not currently considered an issue caused by the limiter.

## How it works

The plugin operates at the common Direct3D 8 presentation interface:

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

It uses Windows `QueryPerformanceCounter` / `QueryPerformanceFrequency`
for high-resolution timing, with `Sleep` for coarse waiting and
`SwitchToThread` for the final portion.

The project does not require the legacy Direct3D 8 SDK headers or
libraries. The required interface types are declared locally so the
project can be built with a modern Visual Studio C++ toolchain.

The design is intentionally tied to the D3D8 interface rather than to one
specific underlying renderer or wrapper.

## Building from source

### Requirements

- Visual Studio / Visual Studio C++ Build Tools
- CMake
- VS Code (development environment used for the project)
- x86 Native Tools Command Prompt for Visual Studio
- A Windows SDK/toolchain capable of building 32-bit C++

Both games are 32-bit/x86, so the project is built as **Win32**.

### Build

From the project directory:

```bat
cmake -S . -B build -A Win32
cmake --build build --config Release
```

Rename the resulting DLL to:

```text
GTA3VC_Hard60FPS_SirJarko.asi
```

Then install it using a compatible ASI loader.

## Compatibility

The current status is **Beta**. The tested results above confirm the
configurations that were actually tested; they are not a guarantee for
every possible environment.

Useful additional test environments include:

- Windows Vista
- Windows 7
- Windows 10
- Windows 11
- AMD and NVIDIA GPUs
- Different driver generations
- Clean and modded installations
- Different resolutions and display configurations
- Different GTA III / Vice City releases or builds
- Additional D3D8 wrappers

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
- Test additional game builds and D3D8 wrappers.
- Submit fixes or improvements.
- Learn from or build upon the work.

This is a practical project, not a claim of complete mastery of the GTA
III or Vice City engines. If an assumption or implementation is wrong,
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

Grand Theft Auto III, Grand Theft Auto: Vice City and their associated
intellectual property belong to **Rockstar Games / Take-Two Interactive**.

This is an unofficial community project and is not affiliated with,
endorsed by or sponsored by Rockstar Games or Take-Two Interactive.

No copyrighted Rockstar game assets are distributed with this project.
Users are responsible for obtaining and legally owning their own copies
of the games.

## Credits

**Project / development:** SirJarko

**AI assistance:** AI tools were used extensively for programming
assistance, debugging, technical research, code review and documentation.

**Related third-party projects/tools:** SilentPatch, Widescreen Fix,
Open Limit Adjuster and DXVK.

Please refer to each project's own documentation and licence for its
respective terms.

## License

See `LICENSE` in this repository.
