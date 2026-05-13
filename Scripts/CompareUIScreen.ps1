param(
    [Parameter(Mandatory = $true)]
    [string]$Reference,

    [Parameter(Mandatory = $true)]
    [string]$Actual,

    [string]$OutputDir,

    [string]$Name,

    [int]$Threshold = 24
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PythonScript = Join-Path $ScriptDir "CompareUIScreen.py"
if (-not (Test-Path -LiteralPath $PythonScript)) {
    throw "Missing comparison engine: $PythonScript"
}

$Python = Get-Command python -ErrorAction SilentlyContinue
$UsePyLauncher = $false
if (-not $Python) {
    $Python = Get-Command py -ErrorAction SilentlyContinue
    $UsePyLauncher = $true
}
if (-not $Python) {
    throw "Python was not found on PATH. Install Python or add it to PATH before running UI screenshot comparisons."
}

$ArgsList = @(
    $PythonScript,
    "--reference", $Reference,
    "--actual", $Actual,
    "--threshold", "$Threshold"
)

if ($OutputDir) {
    $ArgsList += @("--output-dir", $OutputDir)
}
if ($Name) {
    $ArgsList += @("--name", $Name)
}

if ($UsePyLauncher) {
    & $Python.Source -3 @ArgsList
} else {
    & $Python.Source @ArgsList
}

if ($LASTEXITCODE -ne 0) {
    throw "UI screenshot comparison failed with exit code $LASTEXITCODE."
}
