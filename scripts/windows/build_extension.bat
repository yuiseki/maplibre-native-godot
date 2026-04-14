@echo off
:: Build the maplibre-native-godot GDExtension DLL for Windows.
::
:: Prerequisites:
::   - Visual Studio 2022 (Community or higher)
::   - CMake + Ninja in PATH
::   - godot-cpp checked out at third_party\godot-cpp
::   - maplibre-native built for Windows (run scripts\windows\build_maplibre_native.bat first,
::     OR point MLN_BUILD_DIR to an existing build such as the maplibre-native-slint
::     project's build-ninja\vendor\maplibre-native directory)
::   - vcpkg at C:\src\vcpkg
::
:: Usage:
::   set MLN_SOURCE_DIR=C:\path\to\maplibre-native
::   set MLN_BUILD_DIR=C:\path\to\maplibre-native-windows-build   (optional override)
::   scripts\windows\build_extension.bat

setlocal EnableDelayedExpansion

set ROOT_DIR=%~dp0..\..
set BUILD_DIR=%ROOT_DIR%\build\godot-extension-windows
set GODOT_CPP_DIR=%ROOT_DIR%\third_party\godot-cpp

:: Default MLN_BUILD_DIR to the output of build_maplibre_native_windows.bat
if not defined MLN_BUILD_DIR (
    set MLN_BUILD_DIR=%ROOT_DIR%\build\maplibre-native-windows
)

if not defined MLN_SOURCE_DIR (
    echo ERROR: MLN_SOURCE_DIR is not set.
    echo Set it before running this script:
    echo   set MLN_SOURCE_DIR=C:\path\to\maplibre-native
    exit /b 1
)

if not exist "%MLN_BUILD_DIR%\MapboxCoreTargets.cmake" (
    echo ERROR: MapboxCoreTargets.cmake not found at %MLN_BUILD_DIR%
    echo Run scripts\windows\build_maplibre_native.bat first.
    echo Or set MLN_BUILD_DIR to an existing maplibre-native Windows build, e.g.:
    echo   set MLN_BUILD_DIR=C:\Users\yuiseki\src\maplibre-native-slint\build-ninja\vendor\maplibre-native
    exit /b 1
)

if not exist "%GODOT_CPP_DIR%\CMakeLists.txt" (
    echo ERROR: godot-cpp not found at %GODOT_CPP_DIR%
    echo Clone it first:
    echo   git clone --branch 4.3 https://github.com/godotengine/godot-cpp third_party\godot-cpp
    exit /b 1
)

:: Initialize MSVC environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo ERROR: vcvars64.bat failed. Is Visual Studio 2022 installed?
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Point CMake to the vcpkg packages already installed by the maplibre-native build.
:: This lets find_package(dlfcn-win32), find_package(libuv), etc. succeed without
:: reinstalling packages.
:: VCPKG_INSTALLED_X64 can be overridden externally (needed when reusing slint's build).
if not defined VCPKG_INSTALLED_X64 (
    set VCPKG_INSTALLED_X64=%MLN_BUILD_DIR%\vcpkg_installed\x64-windows
)

echo.
echo Configuring GDExtension for Windows...
echo   ROOT_DIR        = %ROOT_DIR%
echo   MLN_SOURCE_DIR  = %MLN_SOURCE_DIR%
echo   MLN_BUILD_DIR   = %MLN_BUILD_DIR%
echo   GODOT_CPP_DIR   = %GODOT_CPP_DIR%
echo   BUILD_DIR       = %BUILD_DIR%
echo.

cmake -S "%ROOT_DIR%" ^
  -B "%BUILD_DIR%" ^
  -G "Ninja" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows ^
  -DVCPKG_MANIFEST_MODE=OFF ^
  -DCMAKE_PREFIX_PATH="%VCPKG_INSTALLED_X64%" ^
  -DGODOT_CPP_DIR="%GODOT_CPP_DIR%" ^
  -DMLN_SOURCE_DIR="%MLN_SOURCE_DIR%" ^
  -DMLN_BUILD_DIR="%MLN_BUILD_DIR%"

if errorlevel 1 (
    echo ERROR: cmake configure failed.
    exit /b 1
)

echo.
echo Building GDExtension...
cmake --build "%BUILD_DIR%" -j

if errorlevel 1 (
    echo ERROR: cmake build failed.
    exit /b 1
)

echo.
echo BUILD SUCCESS
echo Extension DLL: %ROOT_DIR%\bin\maplibre_native_godot.windows.template_release.x86_64.dll
echo Runtime DLL:   %ROOT_DIR%\bin\wgpu_native.dll
echo.
echo Make sure bin\wgpu_native.dll is present before launching Godot.
