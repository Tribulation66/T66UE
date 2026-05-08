param(
    [Parameter(Mandatory=$true)]
    [string]$InputModel,
    [Parameter(Mandatory=$true)]
    [string]$OutputDir,
    [string]$Label = "QuadRetroAsset",
    [int]$TargetQuads = 12000,
    [double]$AdaptiveSize = 50,
    [bool]$AdaptQuadCount = $false,
    [bool]$UseMaterials = $true,
    [bool]$UseNormals = $false,
    [bool]$AutodetectHardEdges = $true,
    [int]$QRemeshSourceTargetTris = 0,
    [int]$BakeSize = 1024,
    [int]$TextureSize = 512,
    [string]$PaletteMode = "none",
    [int]$PaletteSize = 0,
    [int]$PaletteSteps = 256,
    [string]$DitherType = "none",
    [double]$DitherStrength = 0,
    [object]$RenderQA = $false,
    [object]$Background = $false,
    [int]$TimeoutSeconds = 900,
    [string]$RetopoFbx = "",
    [string]$BlenderExe = "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe"
)

$ErrorActionPreference = "Stop"

$ScriptPath = "C:\UE\T66\Model Generation\Scripts\Core\QuadRetro\t66_quad_retro_character_pipeline.py"

function Convert-ToBoolArg {
    param(
        [object]$Value,
        [string]$Name
    )

    if ($null -eq $Value) {
        return $false
    }
    if ($Value -is [bool]) {
        return [bool]$Value
    }
    if ($Value -is [int]) {
        return ([int]$Value) -ne 0
    }

    $text = $Value.ToString().Trim().ToLowerInvariant()
    switch ($text) {
        "true" { return $true }
        "1" { return $true }
        "yes" { return $true }
        "y" { return $true }
        "on" { return $true }
        "false" { return $false }
        "0" { return $false }
        "no" { return $false }
        "n" { return $false }
        "off" { return $false }
        default { throw "Cannot convert $Name value '$Value' to a boolean." }
    }
}

if (!(Test-Path -LiteralPath $BlenderExe)) {
    throw "Blender executable not found: $BlenderExe"
}
if (!(Test-Path -LiteralPath $ScriptPath)) {
    throw "Pipeline script not found: $ScriptPath"
}
if (!(Test-Path -LiteralPath $InputModel)) {
    throw "Input model not found: $InputModel"
}

$RenderQABool = Convert-ToBoolArg -Value $RenderQA -Name "RenderQA"
$BackgroundBool = Convert-ToBoolArg -Value $Background -Name "Background"

$BlenderArgs = @()
if ($BackgroundBool) {
    $BlenderArgs += "--background"
}
$BlenderArgs += @("--python", $ScriptPath, "--")

$PipelineArgs = @(
    "--input", $InputModel,
    "--output-dir", $OutputDir,
    "--label", $Label
)

if (![string]::IsNullOrWhiteSpace($RetopoFbx)) {
    $PipelineArgs += @("--retopo-fbx", $RetopoFbx)
}

$PipelineArgs += @(
    "--target-quads", $TargetQuads,
    "--adaptive-size", $AdaptiveSize,
    "--adapt-quad-count", $AdaptQuadCount.ToString().ToLowerInvariant(),
    "--use-materials", $UseMaterials.ToString().ToLowerInvariant(),
    "--use-normals", $UseNormals.ToString().ToLowerInvariant(),
    "--autodetect-hard-edges", $AutodetectHardEdges.ToString().ToLowerInvariant(),
    "--qremesh-source-target-tris", $QRemeshSourceTargetTris,
    "--bake-size", $BakeSize,
    "--texture-size", $TextureSize,
    "--palette-mode", $PaletteMode,
    "--palette-size", $PaletteSize,
    "--palette-steps", $PaletteSteps,
    "--dither-type", $DitherType,
    "--dither-strength", $DitherStrength,
    "--render-qa", $RenderQABool.ToString().ToLowerInvariant(),
    "--timeout-seconds", $TimeoutSeconds,
    "--quit-when-done", "true"
)

& $BlenderExe @BlenderArgs @PipelineArgs

if ($LASTEXITCODE -ne 0) {
    throw "Blender pipeline failed with exit code $LASTEXITCODE"
}
