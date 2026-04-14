# ADR 0001: Linux PoC build pipeline

## Status

Accepted — validated

## Context

We want a realistic path toward a `maplibre-native` renderer inside Godot 4,
but the starting constraint set is hostile:

- `maplibre-native` is increasingly centered on WebGPU-oriented paths.
- Godot desktop integration through GDExtension is viable, but direct GPU
  resource sharing is not the right first step.
- Existing Godot map plugins are too old (Godot 3 era) to guide a modern
  implementation.

The immediate need is not a polished renderer. The immediate need is a build
pipeline that makes the relationship between the Godot project, the GDExtension,
and the local `maplibre-native` checkout clear and reproducible.

## Decision

For the Linux PoC, use a two-stage build:

1. Build `maplibre-native` separately into a local build directory.
2. Build a thin C++ GDExtension against that result.

Key build decisions:

- **C++20** — maplibre-native public headers use `std::numbers`, `std::span`,
  and `requires` constraints that require C++20.
- **Individual PIC-compiled static libs, not the amalgam** — `libmbgl-core-amalgam.a`
  merges system static libs (libssl, libuv, libz, …) that system package managers
  do not compile with `-fPIC`. Linking them into a shared library produces
  `R_X86_64_PC32` relocation errors. The fix is to use individual vendor libs
  from the maplibre-native build (all compiled with `-fPIC=ON`) and link
  system libs dynamically.
- **`--whole-archive`** — self-registering factory objects inside maplibre-native
  are dropped by the linker unless wrapped in `--whole-archive`.
- **`-DCMAKE_POSITION_INDEPENDENT_CODE=ON`** — ensures all maplibre-native
  object files are compiled with `-fPIC`.

## Consequences

### Positive

- The seam between Godot and `maplibre-native` is explicit and reproducible.
- Failures are easy to localize (build stage 1 vs. stage 2 vs. runtime).
- The PoC validates that wgpu-native and Godot's own Vulkan renderer can
  coexist in the same process (each owns a separate Vulkan context).

### Negative

- The pipeline is Linux-only in this iteration.
- `MLN_SOURCE_DIR` must be set by the user — no bundled maplibre-native.
- `godot-cpp` is an explicit git dependency checked out separately.

## Validation

Validated on Ubuntu 24.04 / Linux 6.x, RTX 3060:

```bash
export MLN_SOURCE_DIR=/path/to/maplibre-native
./scripts/linux/build_maplibre_native.sh   # builds libmbgl-core.a
./scripts/linux/build_extension.sh         # builds libmaplibre_native_godot.so
DISPLAY=:0 ./scripts/linux/run_godot.sh    # map renders, fly_to animates
```
