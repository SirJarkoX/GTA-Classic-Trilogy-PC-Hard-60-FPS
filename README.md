# GTA III Hard 60 FPS Cap (v3)

Experimental ASI plugin for the classic PC version of GTA III.

This build targets the D3D8 interface exposed by the game's existing D3D8->D3D9 wrapper and caps `Present()` cadence at 60 Hz. It does not depend on GTA III's internal frame limiter.

v3 intentionally avoids the legacy DirectX 8 SDK: it uses minimal ABI declarations and does not require `d3d8.h` or `d3d8.lib`.

Build as Win32/x86. After building, rename `GTA3_Hard60FPS.dll` to `GTA3_Hard60FPS.asi` and place it in the GTA III directory with your other ASI plugins.

This is experimental. Keep a backup of your game folder and test it alongside your existing fixes.
This is currently in BETA, a full release that doesnt depend on the DXD9 wrapper will come later
