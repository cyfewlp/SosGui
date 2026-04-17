# SosGui — ImGui Frontend for SkyrimOutfitSystemSE

Managing outfits through MCM is painful. The constant cycle of opening MCM, navigating to
SkyrimOutfitSystemSE, waiting for fade transitions, making a change, closing, checking the result,
and repeating is tedious — and you can never preview armor without leaving the menu entirely.

ImGui is a natural fit for this kind of workflow. SosGui replaces that experience with a fast,
immediate-mode UI where you can filter, preview, and batch-edit outfits without ever leaving the panel.

## Features

* Full feature parity with `SkyrimOutfitSystemSE`
* In-menu armor preview
* Fast — designed for rapid iteration
  * Batch add / delete items
* Gamepad support (follows standard ImGui gamepad navigation)
  * **Left stick / D-Pad** — navigate between items
  * **A** — Activate / Open / Toggle (hold 0.6 s to enter Text Input mode)
  * **B** — Cancel / Close / back up navigation hierarchy
  * **X** — Toggle menu / hold for Windowing mode (Focus / Move / Resize)
  * **LB** — Tweak slower / Focus previous window (Windowing mode)
  * **RB** — Tweak faster / Focus next window (Windowing mode)

## Requirements

* CMake
* [Skyrim Outfit System SE Revived](https://www.nexusmods.com/skyrimspecialedition/mods/42162)
  > Allows you to change the visual appearance of your armor while keeping the stats of your equipped gear.
  > An updated version of DavidJCobb and aers's original work.
* [Dear ImGui](https://github.com/ocornut/imgui)
  > Bloat-free graphical user interface library for C++ with minimal dependencies.

Themes are loaded from `ImThemes/themes.toml` at startup — drop any compatible ImGui theme file there.

## Environment Variables

| Variable | Description |
|---|---|
| `MO2_MODS_PATH` | Path to your Mod Organizer 2 `mods` folder. When set, the build system automatically copies the compiled `dll`, `pdb`, and supporting files to `MO2_MODS_PATH/{PLUGIN_NAME}` after a successful build. |

## Build

```shell
cmake --preset debug-clangcl-ninja-vcpkg
cmake --build build\debug-clangcl-ninja-vcpkg --target SosGui
cd build\debug-clangcl-ninja-vcpkg
cpack
```

## Documentation

[Obsidian](https://obsidian.md/) is used as the documentation tool, with Wikilinks and Backlinks for cross-referencing notes.