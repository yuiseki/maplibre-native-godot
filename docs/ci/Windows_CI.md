# Windows CI Notes

Lessons learned from getting the Windows GitHub Actions build working.
This document exists because Windows CI is significantly harder than Linux/macOS
due to vcpkg, MSVC, and maplibre-native's own build system interactions.

## Status: WIP

The Windows CI build is not yet passing. This document captures what we know
so far to avoid repeating failed approaches.

---

## maplibre-native has its own vcpkg

maplibre-native ships a vendored vcpkg at `platform/windows/vendor/vcpkg/`.
Its CMakeLists.txt references this internally. **Do NOT pass
`-DCMAKE_TOOLCHAIN_FILE=$VCPKG_INSTALLATION_ROOT/...`** to the maplibre-native
build step -- it conflicts with the internal vcpkg and causes `find_package`
failures (e.g. CURL not found).

Let maplibre-native manage its own vcpkg. Only pass vcpkg flags to the
GDExtension build step.

## vcpkg_installed location is unpredictable

When maplibre-native builds with its own vcpkg on CI, the `vcpkg_installed`
directory may end up in different locations depending on the vcpkg binary
cache backend (`Z:/` on GitHub Actions runners).

**Current approach:** After the maplibre-native build completes, dynamically
find the `vcpkg_installed/x64-windows` directory and pass it as
`CMAKE_PREFIX_PATH` to the GDExtension build.

```yaml
- name: Find vcpkg_installed directory
  id: vcpkg-dir
  shell: bash
  run: |
    VCPKG_DIR=$(find build/maplibre-native-windows -path "*/vcpkg_installed/x64-windows" -type d | head -1)
    echo "path=$(cd "$VCPKG_DIR" && pwd)" >> "$GITHUB_OUTPUT"
```

## GDExtension needs vcpkg packages from maplibre-native

The GDExtension's `CMakeLists.txt` (Windows block) calls:

```cmake
find_package(dlfcn-win32 CONFIG REQUIRED)
find_package(libuv CONFIG REQUIRED)
find_package(ICU COMPONENTS i18n uc data QUIET)
find_package(PNG REQUIRED)
```

These are installed by maplibre-native's vcpkg. The GDExtension build must be
able to find them via `CMAKE_PREFIX_PATH` pointing to the vcpkg_installed
directory from the maplibre-native build.

## CMake 4.x + godot-cpp 4.3 incompatibility

See `docs/build-guide/Windows_11.md` gotcha #3. The same fix applies in CI:

```
"-DCMAKE_CXX_FLAGS_DEBUG=/MDd /Zi /Ob0 /Od /RTC1"
```

## Cache strategy

| What | Key | Path |
|------|-----|------|
| maplibre-native build | `mln-windows-<commit-sha>` | `build/maplibre-native-windows` + `wgpu_native.dll` |
| godot-cpp build | `godot-cpp-windows-<branch>-Release` | `build/godot-extension-windows/godot-cpp` |

**Open question:** When maplibre-native cache hits, the vcpkg_installed
directory inside `build/maplibre-native-windows` must also be cached.
If vcpkg installs to an external location (Z:/) during the initial build,
vcpkg_installed may be empty in the cache. This needs verification.

## Failed approaches

1. **Passing `CMAKE_TOOLCHAIN_FILE` to maplibre-native build** -- Conflicts
   with maplibre-native's internal vcpkg. Results in CURL not found.

2. **Passing `VCPKG_INSTALLED_DIR` to maplibre-native build** -- Overrides
   the internal vcpkg install location. Same CURL failure.

3. **Using relative path for `CMAKE_PREFIX_PATH`** -- CMake on Windows does
   not reliably resolve relative paths. Use absolute paths via
   `${{ github.workspace }}`.

4. **Dynamically finding `vcpkg_installed` after build** -- maplibre-native's
   own vcpkg installs packages to `Z:/` (the runner's binary cache drive),
   NOT into `build/maplibre-native-windows/vcpkg_installed/`. The directory
   simply does not exist in the build output tree. `find` returns nothing.

## Next steps to try

The core problem: maplibre-native's vcpkg puts installed packages on `Z:/`,
which is ephemeral and not part of the build output we cache. Possible solutions:

- **Option A:** After maplibre-native builds, explicitly copy `Z:/installed/x64-windows`
  (or wherever vcpkg actually installed) into `build/maplibre-native-windows/vcpkg_installed/`.
  Requires finding the exact Z:/ path from CI logs.

- **Option B:** Use the runner's vcpkg (`$VCPKG_INSTALLATION_ROOT`) for BOTH
  maplibre-native and the GDExtension, but configure maplibre-native to NOT
  use its internal vcpkg. This requires setting `VCPKG_OVERLAY_TRIPLETS` to
  maplibre-native's custom triplets while using the runner's vcpkg toolchain.
  See `scripts/windows/build_maplibre_native.bat` for how local builds do this:
  ```bat
  set VCPKG_OVERLAY_TRIPLETS=%MLN_SOURCE_DIR%\platform\windows\vendor\vcpkg-custom-triplets
  ```

- **Option C:** Make the GDExtension Windows CMake not depend on `find_package`
  for vcpkg packages at all. Instead, import only `MapboxCoreTargets.cmake`
  (like macOS does) and let transitive dependencies handle everything.
  This would require refactoring the WIN32 block in CMakeLists.txt.

## What works locally

On a local Windows machine with maplibre-native-slint already built,
`build_extension_reuse_slint.bat` works perfectly because:

- MLN_SOURCE_DIR and MLN_BUILD_DIR point to the same commit (no ABI mismatch)
- vcpkg_installed is at a known relative location in the slint build tree
- VCPKG_INSTALLED_X64 is set explicitly by the script
