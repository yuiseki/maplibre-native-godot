# maplibre-native-godot

A GDExtension for embedding [maplibre-native](https://github.com/maplibre/maplibre-native)
into a [Godot 4](https://godotengine.org/) project.

> **Status:** Working on Linux, Windows, and macOS.

## Screenshots

**macOS**

[![Image from Gyazo](https://i.gyazo.com/efa197cbe634475b2efdda0c33afc691.png)](https://gyazo.com/efa197cbe634475b2efdda0c33afc691)

**Windows**

[![Image from Gyazo](https://i.gyazo.com/dfd10904d4c9258bba6843d3c2a8b305.png)](https://gyazo.com/dfd10904d4c9258bba6843d3c2a8b305)

**Linux**

[![Image from Gyazo](https://i.gyazo.com/48401b553c80dc998c2fa58490d6c87a.png)](https://gyazo.com/48401b553c80dc998c2fa58490d6c87a)

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
scripts/
  macos/                macOS build helpers
  linux/                Linux build helpers
  windows/              Windows build helpers
CMakeLists.txt        Extension build
maplibre_native_godot.gdextension
project.godot
```

## Requirements

### godot-cpp

Check out godot-cpp matching Godot 4.3 into `third_party/godot-cpp`:

```bash
git clone --branch godot-4.3-stable https://github.com/godotengine/godot-cpp third_party/godot-cpp
```

---

## macOS

### System packages (Homebrew)

```bash
brew install cmake ninja pkg-config libuv
brew install --cask godot
```

### External source dependency

```bash
git clone https://github.com/maplibre/maplibre-native /path/to/maplibre-native
export MLN_SOURCE_DIR=/path/to/maplibre-native
```

### Build

```bash
export MLN_SOURCE_DIR=/path/to/maplibre-native

# Stage 1: build maplibre-native (RelWithDebInfo)
./scripts/macos/build_maplibre_native.sh

# Stage 2: build the GDExtension
./scripts/macos/build_extension.sh

# Or run both at once
./scripts/macos/build_all.sh
```

If you already have a maplibre-native macOS build, you can skip Stage 1
and point `MLN_BUILD_DIR` to it:

```bash
export MLN_BUILD_DIR=/path/to/maplibre-native/build-macos-webgpu-wgpu
./scripts/macos/build_extension.sh
```

### Run

```bash
./scripts/macos/run_godot.sh
```

---

## Linux

### System packages (Ubuntu / Debian)

```bash
sudo apt install cmake ninja-build python3 \
  libssl-dev libcurl4-openssl-dev libuv1-dev \
  libpng-dev libjpeg-dev libwebp-dev libbz2-dev zlib1g-dev \
  libicu-dev libgl1-mesa-dev libvulkan-dev
```

### External source dependency

```bash
git clone https://github.com/maplibre/maplibre-native /path/to/maplibre-native
export MLN_SOURCE_DIR=/path/to/maplibre-native
```

### Godot 4.3 binary

Install Godot 4.3 system-wide (`godot4` / `godot` on `PATH`), or place
the binary at `tools/godot-4.3/Godot_v4.3-stable_linux.x86_64`.

### Build

```bash
export MLN_SOURCE_DIR=/path/to/maplibre-native

# Stage 1: build maplibre-native (RelWithDebInfo, -fPIC)
./scripts/linux/build_maplibre_native.sh

# Stage 2: build the GDExtension
./scripts/linux/build_extension.sh

# Or run both at once
./scripts/linux/build_all.sh
```

### Run

```bash
DISPLAY=:0 ./scripts/linux/run_godot.sh
```

---

## Windows

### Prerequisites

- Visual Studio 2022 (Community or higher) with C++ Desktop workload
- CMake + Ninja in `PATH`
- [vcpkg](https://github.com/microsoft/vcpkg) at `C:\src\vcpkg`
- maplibre-native Windows build (see below)

### External source dependency

```bat
set MLN_SOURCE_DIR=C:\path\to\maplibre-native
```

If you have an existing maplibre-native Windows build (e.g. from
[maplibre-native-slint](https://github.com/maplibre/maplibre-native-slint)),
you can reuse it:

```bat
scripts\windows\build_extension_reuse_slint.bat
```

Otherwise build maplibre-native from scratch first:

```bat
scripts\windows\build_all.bat
```

### Godot 4.3 binary

Place `Godot_v4.3-stable_win64.exe` at:

```
tools\godot-4.3\Godot_v4.3-stable_win64.exe
```

### Build

```bat
set MLN_SOURCE_DIR=C:\path\to\maplibre-native
scripts\windows\build_extension.bat
```

### Run

```bat
scripts\windows\run_godot.bat
```

---

## Architecture notes

| Topic | Decision |
|-------|----------|
| Render mode | `MapMode::Continuous` + `SwapBehaviour::Flush` |
| Per-frame drive | `NOTIFICATION_PROCESS` → `MapRuntime::tick()` |
| Pixel upload | `readStillImage()` + `unpremultiply()` → `ImageTexture::update()` |
| C++ standard | C++20 (maplibre-native headers require `std::numbers`, `std::span`) |
| Linux linking | Individual PIC-compiled vendor libs + `--whole-archive`; system libs dynamic |
| macOS linking | Imports `MapboxCoreTargets.cmake`; `libwgpu_native.dylib` via `@rpath`/`@loader_path`; Metal backend |
| Windows linking | MSVC Release, imports `MapboxCoreTargets.cmake`; `wgpu_native.dll` alongside extension DLL |
| GPU isolation | wgpu-native and Godot's Vulkan/Metal renderer use independent contexts |

See `docs/ADR/` for detailed rationale.

### Performance note: GPU isolation and resolution scaling

maplibre-native renders via wgpu-native while Godot uses its own
Vulkan/Metal renderer. The two GPU contexts are completely independent
and pixel data is transferred through CPU every frame:

```
wgpu (map render) -> readStillImage -> CPU memcpy -> ImageTexture::update -> Godot renderer
```

This CPU roundtrip makes the extension **GPU-backend agnostic** (no need
to match backends like
[maplibre-native-slint](https://github.com/maplibre/maplibre-native-slint)
requires on macOS), and at typical widget sizes (512x512 - 1024x1024,
1-4 MB/frame RGBA) performance is smooth on all platforms.

However, **at fullscreen / high resolutions the per-frame CPU copy becomes
a bottleneck.** Confirmed on both macOS (M4 Max) and Windows (RTX 3060):

| Resolution | RGBA/frame | Result |
|------------|-----------|--------|
| 1920x1080 (Full HD) | ~8 MB | Smooth on both platforms |
| 3840x2160 (4K) | ~33 MB | Noticeable frame drops on both platforms |

The bottleneck is the CPU memcpy, not GPU power, so higher-spec machines
help only marginally. Keep the `MapLibreMap` node at Full HD or below for
interactive use, or render at a lower resolution and let Godot upscale.

## License

This project is released under the [MIT License](LICENSE).

It links against [Godot Engine](https://github.com/godotengine/godot) (MIT)
and [MapLibre Native](https://github.com/maplibre/maplibre-native) (BSD 2-Clause).
See [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for full license texts.
