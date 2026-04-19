import tempfile
import textwrap
import unittest
import zipfile
from pathlib import Path
import sys

PROJECT_ROOT = Path(__file__).resolve().parents[1]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from scripts.verify_release_package import (
    validate_export_directory,
    validate_gdextension_dependencies,
    validate_release_zip,
)


class ReleaseValidationTests(unittest.TestCase):
    def test_validate_gdextension_dependencies_accepts_windows_wgpu_entries(self) -> None:
        gdextension = textwrap.dedent(
            """\
            [configuration]

            entry_symbol = "maplibre_native_godot_library_init"

            [libraries]

            windows.release.x86_64 = "res://addons/maplibre_native_godot/bin/maplibre_native_godot.windows.template_release.x86_64.dll"

            [dependencies]

            windows.debug.x86_64 = {
                "res://addons/maplibre_native_godot/bin/wgpu_native.dll" : ""
            }
            windows.release.x86_64 = {
                "res://addons/maplibre_native_godot/bin/wgpu_native.dll" : ""
            }
            """
        )
        validate_gdextension_dependencies(gdextension)

    def test_validate_gdextension_dependencies_rejects_missing_windows_release_dependency(self) -> None:
        gdextension = textwrap.dedent(
            """\
            [configuration]

            [libraries]

            windows.release.x86_64 = "res://addons/maplibre_native_godot/bin/maplibre_native_godot.windows.template_release.x86_64.dll"

            [dependencies]

            windows.debug.x86_64 = {
                "res://addons/maplibre_native_godot/bin/wgpu_native.dll" : ""
            }
            """
        )
        with self.assertRaisesRegex(ValueError, "windows.release.x86_64"):
            validate_gdextension_dependencies(gdextension)

    def test_validate_release_zip_checks_required_files_and_gdextension_contents(self) -> None:
        gdextension = textwrap.dedent(
            """\
            [configuration]

            entry_symbol = "maplibre_native_godot_library_init"

            [libraries]

            windows.release.x86_64 = "res://addons/maplibre_native_godot/bin/maplibre_native_godot.windows.template_release.x86_64.dll"

            [dependencies]

            windows.debug.x86_64 = {
                "res://addons/maplibre_native_godot/bin/wgpu_native.dll" : ""
            }
            windows.release.x86_64 = {
                "res://addons/maplibre_native_godot/bin/wgpu_native.dll" : ""
            }
            """
        )

        with tempfile.TemporaryDirectory() as tmpdir:
            zip_path = Path(tmpdir) / "release.zip"
            with zipfile.ZipFile(zip_path, "w") as archive:
                archive.writestr(
                    "addons/maplibre_native_godot/maplibre_native_godot.gdextension",
                    gdextension,
                )
                archive.writestr(
                    "addons/maplibre_native_godot/bin/maplibre_native_godot.windows.template_release.x86_64.dll",
                    b"dll",
                )
                archive.writestr(
                    "addons/maplibre_native_godot/bin/wgpu_native.dll",
                    b"wgpu",
                )
            validate_release_zip(zip_path)

    def test_validate_export_directory_checks_runtime_dependency(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            export_dir = Path(tmpdir)
            (export_dir / "maplibre-native-godot-demo.exe").write_bytes(b"exe")
            (export_dir / "maplibre-native-godot-demo.pck").write_bytes(b"pck")
            (
                export_dir / "maplibre_native_godot.windows.template_release.x86_64.dll"
            ).write_bytes(b"dll")
            (export_dir / "wgpu_native.dll").write_bytes(b"wgpu")
            validate_export_directory(export_dir)

    def test_validate_export_directory_rejects_missing_wgpu_runtime(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            export_dir = Path(tmpdir)
            (export_dir / "maplibre-native-godot-demo.exe").write_bytes(b"exe")
            (export_dir / "maplibre-native-godot-demo.pck").write_bytes(b"pck")
            (
                export_dir / "maplibre_native_godot.windows.template_release.x86_64.dll"
            ).write_bytes(b"dll")
            with self.assertRaisesRegex(ValueError, "wgpu_native.dll"):
                validate_export_directory(export_dir)


if __name__ == "__main__":
    unittest.main()
