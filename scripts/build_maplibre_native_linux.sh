#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# MLN_SOURCE_DIR must point to a maplibre-native checkout.
# Override with: export MLN_SOURCE_DIR=/path/to/maplibre-native
if [[ -z "${MLN_SOURCE_DIR:-}" ]]; then
  printf 'Error: MLN_SOURCE_DIR is not set.\n' >&2
  printf 'Export it before running this script:\n' >&2
  printf '  export MLN_SOURCE_DIR=/path/to/maplibre-native\n' >&2
  exit 1
fi

MLN_BUILD_DIR="${MLN_BUILD_DIR:-$ROOT_DIR/build/maplibre-native-linux}"

# Isolate PATH from Conda/Homebrew/pyenv to avoid cmake/ninja version conflicts.
SYSTEM_PATH="$HOME/.local/bin:/usr/bin:/bin:/usr/sbin:/sbin:$HOME/.cargo/bin"

CMAKE_ENV=(env PATH="$SYSTEM_PATH")
if [[ -n "${CONDA_PREFIX:-}" ]]; then
  CMAKE_ENV+=(CMAKE_IGNORE_PREFIX_PATH="$CONDA_PREFIX")
fi

"${CMAKE_ENV[@]}" \
  /usr/bin/cmake -S "$MLN_SOURCE_DIR" \
  -B "$MLN_BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config \
  -DMLN_WITH_WEBGPU=ON \
  -DMLN_WEBGPU_IMPL_WGPU=ON \
  -DMLN_CREATE_AMALGAMATION=ON

env PATH="$SYSTEM_PATH" /usr/bin/cmake --build "$MLN_BUILD_DIR" --target mbgl-core --parallel

printf 'Built maplibre-native in %s\n' "$MLN_BUILD_DIR"
