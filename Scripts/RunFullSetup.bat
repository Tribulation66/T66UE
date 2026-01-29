@echo off
REM Full T66 project setup via Unreal Editor Python.
REM 1) CreateAssets.py - DataTables (Heroes, Companions, Items), levels.
REM 2) FullSetup.py - Button widget, Game Instance, Player Controller, GameModes, CSV import, verify.
REM Set UE_ROOT if engine is not in default location, e.g.:
REM   set UE_ROOT=C:\Program Files\Epic Games\UE_5.7

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "UPROJECT=%PROJECT_DIR%\T66.uproject"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.7"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if not exist "%EDITOR_CMD%" (
    echo UnrealEditor-Cmd not found at: %EDITOR_CMD%
    echo Set UE_ROOT to your engine install, e.g. set UE_ROOT=C:\Program Files\Epic Games\UE_5.7
    exit /b 1
)
if not exist "%UPROJECT%" (
    echo Project not found: %UPROJECT%
    exit /b 1
)

echo === Full project setup (CreateAssets + FullSetup) ===
echo Step 1/2: CreateAssets...
"%EDITOR_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT_DIR%CreateAssets.py"
if errorlevel 1 (
    echo CreateAssets failed.
    exit /b 1
)
echo Step 2/2: FullSetup...
"%EDITOR_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT_DIR%FullSetup.py"
if errorlevel 1 (
    echo FullSetup failed.
    exit /b 1
)
echo === Full project setup complete ===
