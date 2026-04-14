# maplibre-native-godot-poc

A reference PoC for embedding [maplibre-native](https://github.com/maplibre/maplibre-native)
into a [Godot 4](https://godotengine.org/) project via GDExtension.

> **Status:** Working on Linux. Windows support is planned.

![demo](docs/demo.gif)

## What it does

- Exposes a `MapLibreMap` node (extends `TextureRect`) that renders a live map
  using maplibre-native's WebGPU headless backend.
- Uses `MapMode::Continuous` + per-frame `renderOnce()` + `readStillImage()`
  driven from Godot's `_process()`.
- Implements smooth fly_to animations (2.5 s ease-in-out arc with zoom-out,
  ported from [maplibre-native-slint](https://github.com/maplibre/maplibre-native-slint)).
- GDScript API: `set_style_url`, `fly_to`, `jump_to`, `set_pitch`, `set_bearing`,
  `get_current_zoom`, `get_last_render_ms`.
- Demo scene: style picker, Paris / New York / Tokyo buttons, pitch slider,
  bearing slider, FPS benchmark.

## Repository layout

```
src/                  C++ GDExtension sources
  map_runtime.hpp/cpp   MapRuntime — thin wrapper around mbgl::Map (Continuous mode)
  map_view_node.hpp/cpp MapLibreMap Godot node
  register_types.cpp    GDExtension entry point
godot/
  scenes/demo.tscn      Demo scene
  scripts/map_window.gd Demo UI script
docs/ADR/             Architecture decision records
scripts/              Linux build helpers
CMakeLists.txt        Extension build
maplibre_native_godot.gdextension
project.godot
```

## Requirements

### System packages (Ubuntu / Debian)

```bash
sudo apt install cmake ninja-build python3 \
  libssl-dev libcurl4-openssl-dev libuv1-dev \
  libpng-dev libjpeg-dev libwebp-dev libbz2-dev zlib1g-dev \
  libicu-dev libgl1-mesa-dev libvulkan-dev
```

### External source dependency

Clone [maplibre-native](https://github.com/maplibre/maplibre-native) (or your fork)
and export `MLN_SOURCE_DIR`:

```bash
git clone https://github.com/maplibre/maplibre-native /path/to/maplibre-native
export MLN_SOURCE_DIR=/path/to/maplibre-native
```

### godot-cpp

Check out godot-cpp matching Godot 4.3 into `third_party/godot-cpp`:

```bash
git clone --branch godot-4.3 https://github.com/godotengine/godot-cpp third_party/godot-cpp
```

### Godot 4.3 binary

Either install Godot 4.3 system-wide (`godot4` / `godot` on `PATH`), or place
the Linux binary at:

```
tools/godot-4.3/Godot_v4.3-stable_linux.x86_64
```

The run script checks all three locations automatically.

## Build

```bash
export MLN_SOURCE_DIR=/path/to/maplibre-native

# Stage 1: build maplibre-native (RelWithDebInfo, -fPIC)
./scripts/build_maplibre_native_linux.sh

# Stage 2: build the GDExtension
./scripts/build_extension_linux.sh

# Or run both at once
./scripts/build_all_linux.sh
```

## Run

```bash
DISPLAY=:0 ./scripts/run_godot.sh
```

Pass additional Godot flags as needed (e.g. `--editor`, `--headless --quit`).

## Architecture notes

| Topic | Decision |
|-------|----------|
| Render mode | `MapMode::Continuous` + `SwapBehaviour::Flush` |
| Per-frame drive | `NOTIFICATION_PROCESS` → `MapRuntime::tick()` |
| Pixel upload | `readStillImage()` + `unpremultiply()` → `ImageTexture::update()` |
| C++ standard | C++20 (maplibre-native headers require `std::numbers`, `std::span`) |
| Static linking | Individual PIC-compiled vendor libs + `--whole-archive`; system libs dynamic |
| GPU isolation | wgpu-native and Godot's Vulkan renderer use independent contexts |

See `docs/ADR/` for detailed rationale.

## License

This project is released under the [MIT License](LICENSE).

It links against [Godot Engine](https://github.com/godotengine/godot) (MIT)
and [MapLibre Native](https://github.com/maplibre/maplibre-native) (BSD 2-Clause).
See [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for full license texts.
