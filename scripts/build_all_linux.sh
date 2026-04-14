#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$ROOT_DIR/scripts/build_maplibre_native_linux.sh"
"$ROOT_DIR/scripts/build_extension_linux.sh"
