#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GODOT_BIN="${GODOT_BIN:-}"
LOCAL_GODOT_BIN="$ROOT_DIR/tools/godot-4.3/Godot_v4.3-stable_linux.x86_64"

if [[ -z "$GODOT_BIN" ]]; then
  if [[ -x "$LOCAL_GODOT_BIN" ]]; then
    GODOT_BIN="$LOCAL_GODOT_BIN"
  elif command -v godot4 >/dev/null 2>&1; then
    GODOT_BIN="$(command -v godot4)"
  elif command -v godot >/dev/null 2>&1; then
    GODOT_BIN="$(command -v godot)"
  else
    printf 'Godot binary was not found. Set GODOT_BIN or install a Godot 4 binary.\n' >&2
    exit 1
  fi
fi

exec "$GODOT_BIN" --path "$ROOT_DIR" "$@"
