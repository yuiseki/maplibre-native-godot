@echo off
:: Launch the Godot project on Windows.
:: Looks for Godot in tools\godot-4.3\, then in PATH.

setlocal

set ROOT_DIR=%~dp0..\..
set LOCAL_GODOT=%ROOT_DIR%\tools\godot-4.3\Godot_v4.3-stable_win64.exe

if defined GODOT_BIN goto :run

if exist "%LOCAL_GODOT%" (
    set GODOT_BIN=%LOCAL_GODOT%
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
echo Place Godot_v4.3-stable_win64.exe in tools\godot-4.3\, set GODOT_BIN, or add Godot to PATH.
exit /b 1

:run
echo Launching: %GODOT_BIN%
echo Project:   %ROOT_DIR%
"%GODOT_BIN%" --path "%ROOT_DIR%" %*
