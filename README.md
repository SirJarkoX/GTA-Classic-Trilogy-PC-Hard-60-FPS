# GTA III / Vice City --- Hard 60 FPS

**Projects:** `GTA3_Hard60FPS_SirJarko.asi` /
`GTA_VC_Hard60FPS_SirJarko.asi`\
**Status:** Beta\
**Targets:** Grand Theft Auto III & Grand Theft Auto: Vice City (PC,
32-bit/x86)\
**Author:** SirJarko

## What is this?

A small ASI plugin project for the PC versions of **Grand Theft Auto III
and Grand Theft Auto: Vice City** that implements a **hard/locked 60 FPS
presentation cap**.

III and VC are being treated as separate implementations from San
Andreas. Although the games share a large amount of technical heritage,
their code and behaviour are not assumed to be identical.

These releases are therefore currently **Beta** while the
implementations are tested across different hardware, software and game
configurations.

### What it does

-   Enforces a hard 60 FPS presentation cap.
-   Runs as an ASI plugin.
-   Targets the original 32-bit/x86 games.
-   Is intended to work regardless of the renderer, including standard
    Direct3D 9 and DXVK where supported.
-   Is intended to coexist with other fixes and modifications where
    possible.

### What it does not do

-   It is not a general-purpose frame-rate or game-engine overhaul.
-   It does not include GTA III, Vice City or any copyrighted Rockstar
    assets.
-   It does not guarantee compatibility with every mod, driver, GPU,
    Windows version or game installation.
-   It does not assume that behaviour confirmed in San Andreas will
    automatically apply to III or VC.

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

------------------------------------------------------------------------

## Installation

### GTA III

You need:

-   A legitimate PC installation of GTA III.
-   A working **ASI loader**.
-   `GTA3_Hard60FPS_SirJarko.asi`.

Copy the ASI into the game's main directory --- the folder containing
`gta3.exe` --- alongside your ASI loader, or into your scripts folder if
your mod setup uses one.

Example:

``` text
Grand Theft Auto III/
├── gta3.exe
├── [ASI loader]
├── GTA3_Hard60FPS_SirJarko.asi
└── ...
```

### GTA Vice City

You need:

-   A legitimate PC installation of Vice City.
-   A working **ASI loader**.
-   `GTA_VC_Hard60FPS_SirJarko.asi`.

Copy the ASI into the game's main directory --- the folder containing
`gta-vc.exe` --- alongside your ASI loader, or into your scripts folder
if your mod setup uses one.

Example:

``` text
Grand Theft Auto Vice City/
├── gta-vc.exe
├── [ASI loader]
├── GTA_VC_Hard60FPS_SirJarko.asi
└── ...
```

No original game files are distributed with these projects.

------------------------------------------------------------------------

## Building from Source

### Required software

The current development setup uses:

-   **Visual Studio / Visual Studio C++ Build Tools**
-   **CMake**
-   **VS Code** (used as the development environment)
-   **x86 Native Tools Command Prompt for Visual Studio**
-   A Windows SDK/toolchain capable of building 32-bit C++

Both projects are built as **Win32/x86**, matching the 32-bit game
executables.

### Configure

From the relevant project directory, open an **x86 Native Tools Command
Prompt for Visual Studio** and run:

``` bat
cmake -S . -B build -A Win32
```

### Build

``` bat
cmake --build build --config Release
```

The resulting build should be renamed to the appropriate ASI filename:

``` text
GTA3_Hard60FPS_SirJarko.asi
```

or:

``` text
GTA_VC_Hard60FPS_SirJarko.asi
```

Then copy the ASI into the relevant game installation with a compatible
ASI loader.

> **Beta note:** Build/output names may change during development. Check
> the relevant repository/project files for the current CMake target and
> output filename.

------------------------------------------------------------------------

## Mod Compatibility / Testing

III and VC are being tested independently rather than assuming that the
mod environment used for San Andreas will behave identically.

Related GTA PC projects/tools of interest include:

-   **SilentPatch**
-   **Widescreen Fix**
-   **Open Limit Adjuster (OLA)**
-   **DXVK**

These projects are separate from this project and are not bundled with
it.

Compatibility with individual mods should be considered **game and
version specific** and should be confirmed through testing.

### DXVK note

The games use their native Direct3D rendering path. When DXVK is
installed, its `d3d9.dll` can translate a D3D9 rendering path to Vulkan.

DXVK is a separate project and is **not bundled with or specific to this
project**. Compatibility with a particular DXVK release should therefore
be treated as a testing result rather than a fixed project dependency.

------------------------------------------------------------------------

## Compatibility

Both implementations are currently **Beta**.

Particularly useful test environments include:

-   Windows Vista
-   Windows 7
-   Windows 10
-   Windows 11
-   AMD GPUs
-   NVIDIA GPUs
-   Different driver generations
-   Clean and modded installations
-   Different display resolutions/configurations
-   Different GTA III and Vice City releases/builds

**Windows 7 and Vista are of particular interest because of the age of
the original games and their graphics stacks.**

A configuration should be considered confirmed only after it has
actually been tested.

A successful result on GTA III does not automatically confirm the same
result on Vice City, and vice versa.

------------------------------------------------------------------------

## AI-Assisted Development

These projects were developed with **significant AI assistance**.

AI was used for things such as:

-   C/C++ development assistance
-   Understanding unfamiliar code
-   Debugging
-   Exploring implementation approaches
-   Technical explanations
-   Code review
-   Documentation

That does **not** mean the projects were generated and automatically
validated by AI.

The implementations required human direction, experimentation, testing
and decisions about what to implement and how to validate it.

I am also **not presenting myself as an expert reverse engineer or
claiming that every line was manually written from scratch**.

AI-generated code can be wrong, make incorrect assumptions or introduce
bugs. If you find a problem, please report it.

------------------------------------------------------------------------

## Limitations

These are independently developed projects, and there are limits to the
amount of hardware, software and game configurations that can be tested.

I cannot guarantee:

-   Every Windows version.
-   Every GPU or driver.
-   Every GTA III or Vice City release/build.
-   Compatibility with every ASI/plugin/mod.
-   Compatibility with heavily modified installations.
-   That future third-party mod updates will remain compatible.
-   Identical behaviour between GTA III, Vice City and San Andreas.

Reports from other users are therefore extremely useful.

When reporting an issue, include your:

-   Windows version/build
-   CPU
-   GPU and driver
-   Game and version/build
-   ASI loader
-   Other installed mods
-   Resolution/display setup
-   Steps to reproduce the problem
-   Crash information/logs where available

------------------------------------------------------------------------

## Why release the source?

The projects are being released openly so that others can:

-   Review the implementation.
-   Compile it themselves.
-   Test it on hardware I cannot access.
-   Find bugs.
-   Improve compatibility.
-   Submit fixes.
-   Learn from or build upon the work.

This is a practical project rather than a claim of complete mastery of
the GTA III or Vice City engines.

If an assumption or implementation is wrong, I'd rather it be identified
and corrected than left undocumented.

The **Beta** status is intentional: real-world testing is part of the
development process.

------------------------------------------------------------------------

## Disclaimer

Grand Theft Auto III, Grand Theft Auto: Vice City and their associated
intellectual property belong to **Rockstar Games / Take-Two
Interactive**.

These are unofficial community projects and are not affiliated with,
endorsed by or sponsored by Rockstar Games or Take-Two Interactive.

No copyrighted Rockstar game assets are distributed with these projects.

Users are responsible for obtaining and legally owning their own copies
of the games.

------------------------------------------------------------------------

## Credits

**Project / development:** SirJarko

**AI assistance:** AI tools were used extensively for programming
assistance, debugging, technical research, code review and
documentation.

**Third-party projects/tools relevant to development/testing:**
SilentPatch, Widescreen Fix, Open Limit Adjuster and DXVK.

Please refer to each project's own documentation and licence for its
respective terms.

------------------------------------------------------------------------

## License

See `LICENSE` in this repository.
