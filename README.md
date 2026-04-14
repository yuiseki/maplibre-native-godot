# maplibre-native-godot-poc

`maplibre-native-godot-poc` is a Linux-first reference project for embedding
`maplibre-native` into a Godot 4 project via GDExtension.

This repository is intentionally a PoC, not a polished SDK.

## Goal

Build a natural local pipeline where:

1. `maplibre-native` is built from the local fork with WebGPU enabled.
2. A Godot GDExtension links against that build.
3. A Godot project loads the extension and can evolve toward displaying a map.

The first iteration keeps the GDExtension implementation minimal. It focuses on
the build pipeline and on establishing a stable seam between Godot and
`maplibre-native`.

## Non-goals

- Zero-copy texture sharing in the first step.
- Mobile platforms.
- Packaging for end users.
- Depending on abandoned Godot map plugins.

## Repository layout

- `project.godot`
  - Godot project root.
- `maplibre_native_godot.gdextension`
  - GDExtension manifest loaded from project root.
- `src/`
  - C++ GDExtension sources.
- `scripts/`
  - Linux build helpers.
- `docs/ADR/`
  - Architecture notes.

## Current build strategy

The current strategy is intentionally split into two stages.

### Stage 1: build `maplibre-native`

We build the local fork directly from:

- `/home/yuiseki/Workspaces/repos/_yuiseki/_fork/maplibre-native`

The output stays in this PoC repository under:

- `build/maplibre-native-linux-clean/`

For now the expected core artifact is:

- `build/maplibre-native-linux-clean/libmbgl-core-amalgam.a`

### Stage 2: build the Godot extension

The extension is built as a separate CMake target and linked against the local
`maplibre-native` build. The extension output is placed in:

- `bin/`

The `.gdextension` file points to that directory.

## Requirements

This PoC expects the following to exist locally:

- `cmake`
- `ninja`
- `python3`
- `godot-cpp` sources at:
  - `third_party/godot-cpp`

This repository currently vendors or downloads:

- a local `godot-cpp` checkout at `third_party/godot-cpp`
- a portable Godot 4.3 Linux editor at:
  - `tools/godot-4.3/Godot_v4.3-stable_linux.x86_64`

So a system-wide Godot installation is not required on this machine.

## Build commands

```bash
cd /home/yuiseki/Workspaces/repos/_yuiseki/_draft/maplibre-native-godot-poc

./scripts/build_maplibre_native_linux.sh
./scripts/build_extension_linux.sh
./scripts/run_godot.sh
```

Or run all build steps at once:

```bash
./scripts/build_all_linux.sh
```

## Current extension status

The current extension registers a placeholder `MapLibreMap` node and links
against the local `maplibre-native` core library. It does not render a live map
yet. That is intentional: the first milestone is to make the build and loading
pipeline coherent before introducing headless rendering and texture upload.

The demo scene now also mirrors the interaction surface of
`map_window.slint` at a PoC level:

- style selector
- `Paris` / `New York` / `Tokyo` fly-to buttons
- pitch slider
- bearing slider
- zoom label

The current map output is still a camera-reactive placeholder texture, not a
real `maplibre-native` render target.

## Validation status

Validated locally on Linux:

- `./scripts/build_maplibre_native_linux.sh`
  - builds `libmbgl-core.a`
  - builds `libmbgl-core-amalgam.a`
- `./scripts/build_extension_linux.sh`
  - builds `bin/libmaplibre_native_godot.linux.template_debug.x86_64.so`
- `DISPLAY=:0 ./scripts/run_godot.sh --editor --verbose`
  - initializes `maplibre_native_godot`
  - imports the project with the extension present
- `DISPLAY=:0 ./scripts/run_godot.sh --verbose`
  - initializes `maplibre_native_godot`
  - instantiates `MapLibreMap`
  - reaches `MapLibreMap: ready`

Known current caveat:

- the visible map is still a placeholder texture driven by camera state
- real `maplibre-native` image generation is the next step

## Why this shape

The old Godot map repositories in the local forks are too old to use as an
implementation base.

- `GodotMapLoader` effectively stopped at a Godot 3 era design.
- `MapTileProvider` is newer, but it is not a useful basis for a
  `maplibre-native`-based renderer.

So this PoC starts from the current Godot 4 + GDExtension world and treats
`maplibre-native` as the rendering engine from day one.
