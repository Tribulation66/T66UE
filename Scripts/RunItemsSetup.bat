@echo off
REM Run Items DataTable setup via Unreal Editor Python.
REM Creates DT_Items if missing, fills from Content/Data/Items.csv, runs T66Setup.
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

echo Running Items setup (DT_Items + T66Setup)...
"%EDITOR_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT_DIR%SetupItemsDataTable.py"
echo Done. Check Editor output for "Items Setup Done".
