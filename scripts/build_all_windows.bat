@echo off
:: Full Windows build: maplibre-native, then the GDExtension.
::
:: Usage:
::   set MLN_SOURCE_DIR=C:\path\to\maplibre-native
::   scripts\build_all_windows.bat

setlocal EnableDelayedExpansion

set ROOT_DIR=%~dp0..

if not defined MLN_SOURCE_DIR (
    echo ERROR: MLN_SOURCE_DIR is not set.
    echo   set MLN_SOURCE_DIR=C:\path\to\maplibre-native
    exit /b 1
)

echo ============================================================
echo Step 1: Build maplibre-native for Windows
echo ============================================================
call "%ROOT_DIR%\scripts\build_maplibre_native_windows.bat"
if errorlevel 1 exit /b 1

echo.
echo ============================================================
echo Step 2: Build GDExtension
echo ============================================================
call "%ROOT_DIR%\scripts\build_extension_windows.bat"
if errorlevel 1 exit /b 1

echo.
echo ============================================================
echo ALL DONE
echo ============================================================
echo Extension: %ROOT_DIR%\bin\maplibre_native_godot.windows.template_release.x86_64.dll
echo Runtime:   %ROOT_DIR%\bin\wgpu_native.dll
