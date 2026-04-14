#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# MLN_SOURCE_DIR must point to a maplibre-native checkout.
if [[ -z "${MLN_SOURCE_DIR:-}" ]]; then
  printf 'Error: MLN_SOURCE_DIR is not set.\n' >&2
  printf 'Export it before running this script:\n' >&2
  printf '  export MLN_SOURCE_DIR=/path/to/maplibre-native\n' >&2
  exit 1
fi

MLN_BUILD_DIR="${MLN_BUILD_DIR:-$ROOT_DIR/build/maplibre-native-macos}"

cmake -S "$MLN_SOURCE_DIR" \
  -B "$MLN_BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DMLN_WITH_WEBGPU=ON \
  -DMLN_WEBGPU_IMPL_WGPU=ON \
  -DMLN_CREATE_AMALGAMATION=ON

cmake --build "$MLN_BUILD_DIR" --target mbgl-core --parallel

printf 'Built maplibre-native in %s\n' "$MLN_BUILD_DIR"
