@echo off
:: Launch the Godot project on Windows.
:: Looks for Godot 4.6 first, then 4.3, then in PATH.

setlocal

set ROOT_DIR=%~dp0..\..

if defined GODOT_BIN goto :run

if exist "%ROOT_DIR%\tools\godot-4.6\Godot_v4.6.2-stable_win64.exe" (
    set GODOT_BIN=%ROOT_DIR%\tools\godot-4.6\Godot_v4.6.2-stable_win64.exe
    goto :run
)

if exist "%ROOT_DIR%\tools\godot-4.3\Godot_v4.3-stable_win64.exe" (
    set GODOT_BIN=%ROOT_DIR%\tools\godot-4.3\Godot_v4.3-stable_win64.exe
    goto :run
)

where godot4 >nul 2>&1 && (
    for /f "delims=" %%i in ('where godot4') do set GODOT_BIN=%%i
    goto :run
)

where godot >nul 2>&1 && (
    for /f "delims=" %%i in ('where godot') do set GODOT_BIN=%%i
    goto :run
)

echo ERROR: Godot binary not found.
echo Place Godot_v4.6.2-stable_win64.exe in tools\godot-4.6\, or Godot_v4.3-stable_win64.exe in tools\godot-4.3\
exit /b 1

:run
echo Launching: %GODOT_BIN%
echo Project:   %ROOT_DIR%
"%GODOT_BIN%" --path "%ROOT_DIR%" %*
