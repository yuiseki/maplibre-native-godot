#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"$SCRIPT_DIR/build_maplibre_native_macos.sh"
"$SCRIPT_DIR/build_extension_macos.sh"
