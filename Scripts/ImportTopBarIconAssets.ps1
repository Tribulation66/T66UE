$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$EditorCmd = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$ProjectFile = Join-Path $ProjectRoot "T66.uproject"
$PythonScript = Join-Path $PSScriptRoot "ImportTopBarIconAssets.py"

if (-not (Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found at $EditorCmd"
}

if (-not (Test-Path $ProjectFile)) {
    throw "Project file not found at $ProjectFile"
}

if (-not (Test-Path $PythonScript)) {
    throw "Python import script not found at $PythonScript"
}

& $EditorCmd $ProjectFile -ExecutePythonScript="$PythonScript" -nosourcecontrol -nop4 -unattended -nosplash -NullRHI
