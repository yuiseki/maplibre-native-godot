#!/usr/bin/env python3

from __future__ import annotations

import argparse
import zipfile
from pathlib import Path


WINDOWS_DEPENDENCY_KEYS = [
    "windows.debug.x86_64",
    "windows.release.x86_64",
]
WINDOWS_WGPU_PATH = 'res://addons/maplibre_native_godot/bin/wgpu_native.dll'
REQUIRED_RELEASE_FILES = [
    "addons/maplibre_native_godot/maplibre_native_godot.gdextension",
    "addons/maplibre_native_godot/bin/maplibre_native_godot.windows.template_release.x86_64.dll",
    "addons/maplibre_native_godot/bin/wgpu_native.dll",
    "addons/maplibre_native_godot/bin/libpng16.dll",
    "addons/maplibre_native_godot/bin/uv.dll",
    "addons/maplibre_native_godot/bin/zlib1.dll",
]
REQUIRED_EXPORT_FILES = [
    "maplibre-native-godot-demo.exe",
    "maplibre-native-godot-demo.pck",
    "maplibre_native_godot.windows.template_release.x86_64.dll",
    "wgpu_native.dll",
    "libpng16.dll",
    "uv.dll",
    "zlib1.dll",
]


def validate_gdextension_dependencies(content: str) -> None:
    if "[dependencies]" not in content:
        raise ValueError("Missing [dependencies] section in .gdextension file")

    for key in WINDOWS_DEPENDENCY_KEYS:
        marker = f"{key} = {{"
        if marker not in content:
            raise ValueError(f"Missing {key} dependency block in .gdextension file")
        start = content.index(marker)
        end = content.find("}", start)
        if end == -1:
            raise ValueError(f"Unterminated dependency block for {key}")
        block = content[start:end]
        if WINDOWS_WGPU_PATH not in block:
            raise ValueError(f"{key} does not reference {WINDOWS_WGPU_PATH}")


def validate_release_zip(zip_path: Path) -> None:
    with zipfile.ZipFile(zip_path) as archive:
        names = set(archive.namelist())
        for required in REQUIRED_RELEASE_FILES:
            if required not in names:
                raise ValueError(f"Release zip missing required file: {required}")
        gdextension_content = archive.read(
            "addons/maplibre_native_godot/maplibre_native_godot.gdextension"
        ).decode("utf-8")
        validate_gdextension_dependencies(gdextension_content)


def validate_export_directory(export_dir: Path) -> None:
    for required in REQUIRED_EXPORT_FILES:
        if not (export_dir / required).exists():
            raise ValueError(f"Export output missing required file: {required}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Validate maplibre-native-godot release packaging and Windows export outputs."
    )
    parser.add_argument("--gdextension", type=Path, help="Validate a source .gdextension file.")
    parser.add_argument("--zip", dest="zip_path", type=Path, help="Validate a packaged release zip.")
    parser.add_argument(
        "--export-dir",
        type=Path,
        help="Validate a Godot-exported Windows demo directory.",
    )
    args = parser.parse_args()

    ran = False
    if args.gdextension:
        validate_gdextension_dependencies(args.gdextension.read_text(encoding="utf-8"))
        print(f"{args.gdextension}: ok")
        ran = True
    if args.zip_path:
        validate_release_zip(args.zip_path)
        print(f"{args.zip_path}: ok")
        ran = True
    if args.export_dir:
        validate_export_directory(args.export_dir)
        print(f"{args.export_dir}: ok")
        ran = True

    if not ran:
        raise SystemExit("Specify at least one of --gdextension, --zip, or --export-dir")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
