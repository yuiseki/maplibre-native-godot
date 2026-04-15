#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ADDON_DIR="$ROOT_DIR/addons/maplibre_native_godot"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
STAGE_DIR="$DIST_DIR/release-root"
VERSION="${VERSION:-$(git -C "$ROOT_DIR" rev-parse --short HEAD)}"
ARCHIVE_BASENAME="maplibre-native-godot-${VERSION}"
ARCHIVE_PATH="$DIST_DIR/${ARCHIVE_BASENAME}.zip"

if [[ ! -d "$ADDON_DIR" ]]; then
  printf 'Addon directory not found: %s\n' "$ADDON_DIR" >&2
  exit 1
fi

if ! find "$ADDON_DIR/bin" -maxdepth 1 -type f >/dev/null 2>&1; then
  printf 'No built binaries found under %s/bin\n' "$ADDON_DIR" >&2
  printf 'Build the extension first before packaging a release archive.\n' >&2
  exit 1
fi

if ! command -v zip >/dev/null 2>&1; then
  printf 'zip command is required to package a release archive.\n' >&2
  exit 1
fi

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/addons"
cp -R "$ADDON_DIR" "$STAGE_DIR/addons/"
cp "$ROOT_DIR/LICENSE" "$STAGE_DIR/addons/maplibre_native_godot/LICENSE"
cp "$ROOT_DIR/THIRD_PARTY_LICENSES.md" \
  "$STAGE_DIR/addons/maplibre_native_godot/THIRD_PARTY_LICENSES.md"

mkdir -p "$DIST_DIR"
rm -f "$ARCHIVE_PATH"
(
  cd "$STAGE_DIR"
  zip -r "$ARCHIVE_PATH" addons >/dev/null
)

printf 'Packaged release archive: %s\n' "$ARCHIVE_PATH"
