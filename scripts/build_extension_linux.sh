#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/godot-extension-linux}"
MLN_BUILD_DIR="${MLN_BUILD_DIR:-$ROOT_DIR/build/maplibre-native-linux}"
GODOT_CPP_DIR="${GODOT_CPP_DIR:-$ROOT_DIR/third_party/godot-cpp}"
SYSTEM_PATH="$HOME/.local/bin:/usr/bin:/bin:/usr/sbin:/sbin:$HOME/.cargo/bin"

if [[ -z "${MLN_SOURCE_DIR:-}" ]]; then
  printf 'Error: MLN_SOURCE_DIR is not set.\n' >&2
  printf 'Export it before running this script:\n' >&2
  printf '  export MLN_SOURCE_DIR=/path/to/maplibre-native\n' >&2
  exit 1
fi

if [[ ! -f "$MLN_BUILD_DIR/libmbgl-core.a" ]]; then
  printf 'Expected %s/libmbgl-core.a. Run scripts/build_maplibre_native_linux.sh first.\n' "$MLN_BUILD_DIR" >&2
  exit 1
fi

if [[ ! -f "$GODOT_CPP_DIR/CMakeLists.txt" ]]; then
  printf 'Expected godot-cpp at %s. Check it out before building the extension.\n' "$GODOT_CPP_DIR" >&2
  exit 1
fi

env PATH="$SYSTEM_PATH" /usr/bin/cmake -S "$ROOT_DIR" \
  -B "$BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DGODOT_CPP_DIR="$GODOT_CPP_DIR" \
  -DMLN_SOURCE_DIR="$MLN_SOURCE_DIR" \
  -DMLN_BUILD_DIR="$MLN_BUILD_DIR" \
  -DMLN_CORE_LIB="$MLN_BUILD_DIR/libmbgl-core.a"

env PATH="$SYSTEM_PATH" /usr/bin/cmake --build "$BUILD_DIR" --parallel

printf 'Built GDExtension in %s and emitted binaries into %s/bin\n' "$BUILD_DIR" "$ROOT_DIR"
