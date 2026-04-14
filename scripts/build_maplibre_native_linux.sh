#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MLN_SOURCE_DIR="${MLN_SOURCE_DIR:-/home/yuiseki/Workspaces/repos/_yuiseki/_fork/maplibre-native}"
MLN_BUILD_DIR="${MLN_BUILD_DIR:-$ROOT_DIR/build/maplibre-native-linux-clean}"
SYSTEM_PATH="$HOME/.local/bin:/usr/bin:/bin:/usr/sbin:/sbin:$HOME/.cargo/bin"
CONDA_PREFIX_PATH="${CONDA_PREFIX:-/home/yuiseki/anaconda3}"

env \
  PATH="$SYSTEM_PATH" \
  CMAKE_IGNORE_PREFIX_PATH="$CONDA_PREFIX_PATH" \
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
