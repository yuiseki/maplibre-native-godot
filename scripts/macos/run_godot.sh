#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Find Godot binary
if command -v godot &>/dev/null; then
  GODOT_BIN="godot"
elif [[ -x "/Applications/Godot.app/Contents/MacOS/Godot" ]]; then
  GODOT_BIN="/Applications/Godot.app/Contents/MacOS/Godot"
else
  printf 'Error: Godot not found. Install via: brew install --cask godot\n' >&2
  exit 1
fi

cd "$ROOT_DIR"
exec "$GODOT_BIN" --path "$ROOT_DIR"
