param(
    [string]$SourceRoot = (Join-Path $PSScriptRoot "..\CodexSkills\UI"),
    [string]$TargetRoot = "",
    [switch]$Validate
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($TargetRoot)) {
    if (-not [string]::IsNullOrWhiteSpace($env:CODEX_HOME)) {
        $TargetRoot = Join-Path $env:CODEX_HOME "skills"
    }
    else {
        $TargetRoot = Join-Path $HOME ".codex\skills"
    }
}

$SourceRoot = [System.IO.Path]::GetFullPath($SourceRoot)
$TargetRoot = [System.IO.Path]::GetFullPath($TargetRoot)

if (-not (Test-Path -LiteralPath $SourceRoot)) {
    throw "Missing source skill mirror root: $SourceRoot"
}

if (-not (Test-Path -LiteralPath $TargetRoot)) {
    New-Item -ItemType Directory -Path $TargetRoot | Out-Null
}

$skills = @(
    "ui-reference-prep",
    "ui-style-reference",
    "ui-layout-manifest",
    "ui-sprite-families",
    "ui-runtime-reconstruction",
    "ui-packaged-review",
    "ui-reconstruction-orchestrator",
    "reconstruction-ui"
)

foreach ($skill in $skills) {
    $sourceSkill = Join-Path $SourceRoot $skill
    $targetSkill = Join-Path $TargetRoot $skill

    if (-not (Test-Path -LiteralPath $sourceSkill)) {
        throw "Missing mirrored skill: $sourceSkill"
    }

    if (Test-Path -LiteralPath $targetSkill) {
        Remove-Item -Recurse -Force -LiteralPath $targetSkill
    }

    Copy-Item -Recurse -Force -LiteralPath $sourceSkill -Destination $targetSkill
    Write-Host "Installed $skill -> $targetSkill"
}

if ($Validate) {
    $validator = "C:\Users\DoPra\.codex\skills\.system\skill-creator\scripts\quick_validate.py"
    if (Test-Path -LiteralPath $validator) {
        foreach ($skill in $skills) {
            & python $validator (Join-Path $TargetRoot $skill)
            if ($LASTEXITCODE -ne 0) {
                throw "Validation failed for $skill"
            }
        }
    }
    else {
        Write-Warning "Validator not found at $validator; skipped validation."
    }
}
