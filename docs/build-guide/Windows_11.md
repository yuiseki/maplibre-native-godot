# Windows 11 Build Guide

This guide documents how to build and run the maplibre-native-godot GDExtension on Windows 11,
including the non-obvious gotchas that are easy to get wrong.

## Prerequisites

| Tool | Version tested | Notes |
|------|---------------|-------|
| Visual Studio 2022 | Community or higher | C++ workload required |
| CMake | 4.x | Must be in PATH |
| Ninja | any | Must be in PATH |
| vcpkg | any | Expected at `C:\src\vcpkg` |
| Godot | 4.6 | Place in `tools\godot-4.6\` or set `GODOT_BIN` |

## Quick start (reusing maplibre-native-slint build)

If you already have [maplibre-native-slint](https://github.com/maplibre/maplibre-native-slint)
built at `C:\Users\<you>\src\maplibre-native-slint`, you can skip the lengthy
maplibre-native build entirely:

```bat
scripts\windows\build_extension_reuse_slint.bat
scripts\windows\run_godot.bat
```

That is the recommended workflow for day-to-day development.

## Full build (standalone)

If you do not have a maplibre-native-slint build available:

```bat
:: 1. Build maplibre-native for Windows (takes ~30 min)
set MLN_SOURCE_DIR=C:\path\to\maplibre-native
scripts\windows\build_maplibre_native.bat

:: 2. Build the GDExtension
scripts\windows\build_extension.bat

:: 3. Run Godot
scripts\windows\run_godot.bat
```

---

## Known gotchas

### 1. MLN_SOURCE_DIR and MLN_BUILD_DIR must be the same commit

`CMakeLists.txt` uses `MLN_SOURCE_DIR` for C++ **headers** and `MLN_BUILD_DIR` for
pre-built **binaries** (`MapboxCoreTargets.cmake`, `.lib` files, etc.).
If these two directories point to different commits of maplibre-native you will get
silent ABI mismatches that produce wrong behaviour at runtime (e.g. camera zoom
returning `nullopt` on every call).

**How to verify:**

```bat
git -C %MLN_SOURCE_DIR% log --oneline -1
git -C %MLN_BUILD_DIR%\..  log --oneline -1
```

Both hashes must match.

**`build_extension_reuse_slint.bat` handles this automatically** by pointing both
variables to `maplibre-native-slint\vendor\maplibre-native` (source) and
`maplibre-native-slint\build-ninja\vendor\maplibre-native` (build output).

---

### 2. Reusing the maplibre-native-slint build

`maplibre-native-slint` builds maplibre-native as a submodule under
`vendor\maplibre-native`. You can reuse its build output without rebuilding:

| CMake variable | Value |
|---------------|-------|
| `MLN_SOURCE_DIR` | `<slint>\vendor\maplibre-native` |
| `MLN_BUILD_DIR` | `<slint>\build-ninja\vendor\maplibre-native` |
| `VCPKG_INSTALLED_X64` | `<slint>\build-ninja\vcpkg_installed\x64-windows` |

`build_extension_reuse_slint.bat` sets all three automatically.
Note that vcpkg packages installed by the slint build live **one level above**
the maplibre-native subdirectory, so `VCPKG_INSTALLED_X64` must point there,
not inside `vendor\maplibre-native`.

---

### 3. CMake 4.x is incompatible with godot-cpp 4.3-stable out of the box

CMake 4.x changed how empty variables are handled in `string(REPLACE ...)`.
`third_party/godot-cpp/CMakeLists.txt` (4.3-stable) has:

```cmake
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
```

When `CMAKE_CXX_FLAGS_DEBUG` is empty this call receives fewer arguments than
CMake 4.x requires, producing:

```
CMake Error at third_party/godot-cpp/CMakeLists.txt:100 (string):
  string sub-command REPLACE requires at least four arguments.
```

**Fix (already applied to `build_extension.bat`):** pass the default MSVC debug
flags explicitly so the variable is never empty:

```bat
cmake ... "-DCMAKE_CXX_FLAGS_DEBUG=/MDd /Zi /Ob0 /Od /RTC1"
```

If you regenerate `build_extension.bat` from scratch, remember to include this flag.

**Alternative workaround:** add the CMake policy flag that restores the old
empty-variable behaviour:

```bat
cmake ... -DCMAKE_POLICY_DEFAULT_CMP0178=OLD
```

---

### 4. Deleting the build directory forces a full godot-cpp recompile

`build\godot-extension-windows` caches the compiled godot-cpp static library
(`godot-cpp.windows.release.64.lib`, ~900 translation units). Deleting this
directory discards that cache and makes the next build take an extra 5-10 minutes.

Only delete the build directory when you need a completely clean configure
(e.g. after changing CMake variables). For a normal rebuild, just run
`build_extension.bat` again — CMake will only recompile changed files.

---

### 5. vcpkg path must match the maplibre-native build

`build_extension.bat` sets `VCPKG_INSTALLED_X64` from `MLN_BUILD_DIR` by default:

```bat
set VCPKG_INSTALLED_X64=%MLN_BUILD_DIR%\vcpkg_installed\x64-windows
```

When reusing the slint build, this default path does not exist because slint
puts vcpkg one directory higher. Override it before calling `build_extension.bat`,
or use `build_extension_reuse_slint.bat` which handles this for you.

---

### 6. getCameraOptions().zoom returns nullopt -- caused by ABI mismatch

When `MLN_SOURCE_DIR` and `MLN_BUILD_DIR` point to different commits (see gotcha #1),
`mbgl::Map::getCameraOptions().zoom` can return `std::nullopt` on every call.
This was initially suspected to be a wgpu-Vulkan backend issue, but the root cause
was confirmed to be ABI mismatch between headers and binaries. Unifying the commits
resolved the nullopt entirely.

The `current_zoom_hint` fallback in the code remains as a defensive guard, but if
you encounter this symptom, **check gotcha #1 first** -- it is almost certainly an
ABI mismatch rather than a backend bug.

**Fix (already in `src/map_view_node.cpp`):** the node tracks `current_zoom`
itself and passes it as `current_zoom_hint` to `MapRuntime::fly_to`, which
uses it as the `value_or` fallback:

```cpp
void MapLibreMap::fly_to(double p_lat, double p_lon, double p_zoom) {
    const double prev_zoom = current_zoom;
    current_zoom = p_zoom;
    runtime_->fly_to(p_lat, p_lon, p_zoom, prev_zoom);
}
```

---

## Launching Godot

```bat
scripts\windows\run_godot.bat
```

The script searches for Godot in this order:

1. `GODOT_BIN` environment variable
2. `tools\godot-4.6\Godot_v4.6*_win64.exe`
3. `godot4` in PATH
4. `godot` in PATH

Download Godot 4.6 from the [official site](https://godotengine.org/download/) and
place the executable in `tools\godot-4.6\`.
