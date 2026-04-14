@echo off
:: Build maplibre-native for Windows with the WebGPU (wgpu-native) backend.
:: Produces MapboxCoreTargets.cmake and .lib files consumed by build_extension_windows.bat.
::
:: Prerequisites:
::   - Visual Studio 2022 (Community or higher)
::   - CMake (in PATH)
::   - Ninja (in PATH)
::   - Rust toolchain (rustup, cargo in PATH)
::   - LLVM installed (for wgpu-native's bindgen step)
::   - vcpkg cloned at C:\src\vcpkg and bootstrapped
::   - Python 3 at C:\Python313\python.exe  (or set PYTHON_EXECUTABLE)
::
:: Usage:
::   set MLN_SOURCE_DIR=C:\path\to\maplibre-native
::   scripts\windows\build_maplibre_native.bat

setlocal EnableDelayedExpansion

set ROOT_DIR=%~dp0..\..
set MLN_BUILD_DIR=%ROOT_DIR%\build\maplibre-native-windows

if not defined MLN_SOURCE_DIR (
    echo ERROR: MLN_SOURCE_DIR is not set.
    echo Set it before running this script:
    echo   set MLN_SOURCE_DIR=C:\path\to\maplibre-native
    exit /b 1
)

if not exist "%MLN_SOURCE_DIR%\CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found at %MLN_SOURCE_DIR%
    echo Make sure MLN_SOURCE_DIR points to a maplibre-native checkout.
    exit /b 1
)

:: Initialize MSVC environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo ERROR: vcvars64.bat failed. Is Visual Studio 2022 installed?
    exit /b 1
)

:: LLVM needed for wgpu-native bindgen
set LIBCLANG_PATH=C:\Program Files\LLVM\lib

:: vcpkg overlay triplets from maplibre-native's Windows platform dir
set VCPKG_OVERLAY_TRIPLETS=%MLN_SOURCE_DIR%\platform\windows\vendor\vcpkg-custom-triplets

:: Python (used by maplibre-native's configure step)
if not defined PYTHON_EXECUTABLE (
    set PYTHON_EXECUTABLE=C:\Python313\python.exe
)

if not exist "%MLN_BUILD_DIR%" mkdir "%MLN_BUILD_DIR%"

echo.
echo Configuring maplibre-native for Windows...
echo   MLN_SOURCE_DIR = %MLN_SOURCE_DIR%
echo   MLN_BUILD_DIR  = %MLN_BUILD_DIR%
echo.

cmake -S "%MLN_SOURCE_DIR%" ^
  -B "%MLN_BUILD_DIR%" ^
  -G "Ninja" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows ^
  -DVCPKG_HOST_TRIPLET=x64-windows ^
  -DVCPKG_OVERLAY_TRIPLETS="%VCPKG_OVERLAY_TRIPLETS%" ^
  -DMLN_WITH_WEBGPU=ON ^
  -DMLN_WEBGPU_IMPL_WGPU=ON ^
  -DMLN_WITH_TESTS=OFF ^
  -DMLN_WITH_BENCHMARKS=OFF ^
  -DMLN_WITH_RENDER_TESTS=OFF ^
  -DPYTHON_EXECUTABLE="%PYTHON_EXECUTABLE%"

if errorlevel 1 (
    echo.
    echo ERROR: cmake configure failed.
    echo If the error mentions libclang, ensure LLVM is installed and
    echo LIBCLANG_PATH points to the directory containing libclang.dll.
    exit /b 1
)

echo.
echo Building mbgl-core...
cmake --build "%MLN_BUILD_DIR%" --target mbgl-core -j

if errorlevel 1 (
    echo ERROR: cmake build failed.
    exit /b 1
)

echo.
echo BUILD SUCCESS
echo MapboxCoreTargets.cmake: %MLN_BUILD_DIR%\MapboxCoreTargets.cmake
echo.
echo Next step: run scripts\windows\build_extension.bat
