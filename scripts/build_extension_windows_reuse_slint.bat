@echo off
:: Build the GDExtension by reusing the maplibre-native build from the
:: maplibre-native-slint project. This skips the lengthy maplibre-native build step.
::
:: Assumes maplibre-native-slint has already been built at:
::   C:\Users\yuiseki\src\maplibre-native-slint\build-ninja
::
:: Usage (no env vars needed if defaults match):
::   scripts\build_extension_windows_reuse_slint.bat

setlocal EnableDelayedExpansion

set ROOT_DIR=%~dp0..
set SLINT_DIR=C:\Users\yuiseki\src\maplibre-native-slint

set MLN_SOURCE_DIR=%SLINT_DIR%\vendor\maplibre-native
set MLN_BUILD_DIR=%SLINT_DIR%\build-ninja\vendor\maplibre-native

if not exist "%MLN_BUILD_DIR%\MapboxCoreTargets.cmake" (
    echo ERROR: MapboxCoreTargets.cmake not found at %MLN_BUILD_DIR%
    echo Build the maplibre-native-slint project first:
    echo   cd %SLINT_DIR%
    echo   build_windows.bat
    exit /b 1
)

echo Reusing maplibre-native build from: %MLN_BUILD_DIR%
echo.

:: The slint build keeps vcpkg_installed one level above the maplibre-native subdir
set VCPKG_INSTALLED_X64=%SLINT_DIR%\build-ninja\vcpkg_installed\x64-windows

set MLN_SOURCE_DIR=%MLN_SOURCE_DIR%
set MLN_BUILD_DIR=%MLN_BUILD_DIR%
set VCPKG_INSTALLED_X64=%VCPKG_INSTALLED_X64%
call "%ROOT_DIR%\scripts\build_extension_windows.bat"
