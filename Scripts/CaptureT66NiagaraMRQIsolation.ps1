param(
    [string]$Project = "C:\UE\T66\T66.uproject",
    [string]$EditorCmd = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    [string]$SetupScript = "C:\UE\T66\Scripts\SetupT66NiagaraMRQIsolation.py",
    [string]$SystemPath = "/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash",
    [string]$OutputDir,
    [string]$TempName = "T66NiagaraMRQIsolation",
    [int]$ResX = 1400,
    [int]$ResY = 1400,
    [double]$OrthoWidth = 1250.0,
    [double]$CameraX = 0.0,
    [double]$CameraY = 0.0,
    [double]$CameraZ = 900.0,
    [int]$WarmupTicks = 45,
    [double]$WarmupDeltaSeconds = 0.0166667,
    [double]$NonBlackThreshold = 12.0,
    [int]$MinimumFrameMarginPixels = 24,
    [int]$TimeoutSeconds = 300,
    [switch]$KeepTempAssets,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

function Quote-Arg {
    param([string]$Value)
    if ($Value -match '[\s"]') {
        return '"' + ($Value -replace '"', '\"') + '"'
    }
    return $Value
}

function Convert-PngToOpaqueBlackBackground {
    param(
        [Parameter(Mandatory = $true)][string]$Path
    )

    Add-Type -AssemblyName System.Drawing
    $source = [System.Drawing.Bitmap]::FromFile($Path)
    try {
        $output = New-Object System.Drawing.Bitmap($source.Width, $source.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            for ($y = 0; $y -lt $source.Height; $y++) {
                for ($x = 0; $x -lt $source.Width; $x++) {
                    $pixel = $source.GetPixel($x, $y)
                    if ($pixel.R -eq 0 -and $pixel.G -eq 0 -and $pixel.B -eq 0) {
                        $output.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, 0, 0, 0))
                    } else {
                        # MRQ can write useful VFX RGB with very low alpha. For review
                        # images, keep the rendered color and make the background opaque.
                        $output.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $pixel.R, $pixel.G, $pixel.B))
                    }
                }
            }

            $tempPath = "$Path.opaque.tmp.png"
            $output.Save($tempPath, [System.Drawing.Imaging.ImageFormat]::Png)
        } finally {
            $output.Dispose()
        }
    } finally {
        $source.Dispose()
    }

    Move-Item -LiteralPath $tempPath -Destination $Path -Force
}

function Invoke-LoggedProcess {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StdoutPath,
        [string]$StderrPath,
        [int]$Timeout
    )

    $argumentLine = ($Arguments | ForEach-Object { Quote-Arg $_ }) -join " "
    $process = Start-Process -FilePath $FilePath -ArgumentList $argumentLine -NoNewWindow -PassThru -RedirectStandardOutput $StdoutPath -RedirectStandardError $StderrPath
    if (-not $process.WaitForExit($Timeout * 1000)) {
        try {
            $process.Kill()
        } catch {
        }
        throw "Timed out after $Timeout seconds: $FilePath $argumentLine"
    }
    $process.Refresh()
    $exitCode = $process.ExitCode
    if ($null -eq $exitCode -and $process.HasExited -and (Test-Path -LiteralPath $StdoutPath -PathType Leaf)) {
        $stdoutText = Get-Content -LiteralPath $StdoutPath -Raw
        if (
            $stdoutText -match "Success - 0 error\(s\)" -or
            $stdoutText -match "Python script executed successfully" -or
            ($stdoutText -match "Movie Pipeline completed" -and $stdoutText -match "RequestExitWithStatus\(0, 0")
        ) {
            $exitCode = 0
        }
    }
    return [pscustomobject]@{
        ExitCode = $exitCode
        Command = "$FilePath $argumentLine"
    }
}

function Analyze-PngBounds {
    param(
        [string]$Path,
        [string]$CropPath,
        [double]$Threshold
    )

    Add-Type -AssemblyName System.Drawing
    $bitmap = [System.Drawing.Bitmap]::FromFile($Path)
    try {
        $width = $bitmap.Width
        $height = $bitmap.Height
        $minX = $width
        $minY = $height
        $maxX = -1
        $maxY = -1

        for ($y = 0; $y -lt $height; $y++) {
            for ($x = 0; $x -lt $width; $x++) {
                $pixel = $bitmap.GetPixel($x, $y)
                if (($pixel.R -gt $Threshold) -or ($pixel.G -gt $Threshold) -or ($pixel.B -gt $Threshold)) {
                    if ($x -lt $minX) { $minX = $x }
                    if ($y -lt $minY) { $minY = $y }
                    if ($x -gt $maxX) { $maxX = $x }
                    if ($y -gt $maxY) { $maxY = $y }
                }
            }
        }

        if ($maxX -lt 0) {
            Copy-Item -LiteralPath $Path -Destination $CropPath -Force
            return [pscustomobject]@{
                Width = $width
                Height = $height
                HasNonBlackPixels = $false
                BoundingBox = $null
                Margins = $null
                NonBlackAreaPixels = 0
            }
        }

        $cropMargin = 18
        $cropX = [Math]::Max(0, $minX - $cropMargin)
        $cropY = [Math]::Max(0, $minY - $cropMargin)
        $cropMaxX = [Math]::Min($width - 1, $maxX + $cropMargin)
        $cropMaxY = [Math]::Min($height - 1, $maxY + $cropMargin)
        $cropRect = New-Object System.Drawing.Rectangle($cropX, $cropY, ($cropMaxX - $cropX + 1), ($cropMaxY - $cropY + 1))
        $crop = $bitmap.Clone($cropRect, $bitmap.PixelFormat)
        try {
            $crop.Save($CropPath, [System.Drawing.Imaging.ImageFormat]::Png)
        } finally {
            $crop.Dispose()
        }

        return [pscustomobject]@{
            Width = $width
            Height = $height
            HasNonBlackPixels = $true
            BoundingBox = [pscustomobject]@{
                MinX = $minX
                MinY = $minY
                MaxX = $maxX
                MaxY = $maxY
            }
            Margins = [pscustomobject]@{
                Left = $minX
                Top = $minY
                Right = $width - 1 - $maxX
                Bottom = $height - 1 - $maxY
            }
            NonBlackAreaPixels = ($maxX - $minX + 1) * ($maxY - $minY + 1)
        }
    } finally {
        $bitmap.Dispose()
    }
}

$repoRoot = Get-RepoRoot
if (-not $OutputDir) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $OutputDir = Join-Path $repoRoot "Saved\VFXResearch\Hero1Axe\AOE_AmericanFlagVisualTarget\EditorIsolation\$timestamp"
}

$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)
$framesDir = Join-Path $OutputDir "frames"
New-Item -ItemType Directory -Force -Path $framesDir | Out-Null

$setupStdout = Join-Path $OutputDir "setup.stdout.log"
$setupStderr = Join-Path $OutputDir "setup.stderr.log"
$renderStdout = Join-Path $OutputDir "render.stdout.log"
$renderStderr = Join-Path $OutputDir "render.stderr.log"
$cleanupStdout = Join-Path $OutputDir "cleanup.stdout.log"
$cleanupStderr = Join-Path $OutputDir "cleanup.stderr.log"

$setupArgs = @(
    $Project,
    "-run=pythonscript",
    "-script=$SetupScript",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-T66MRQIsolationOutput=$OutputDir",
    "-T66MRQIsolationTempName=$TempName",
    "-T66MRQIsolationResX=$ResX",
    "-T66MRQIsolationResY=$ResY",
    "-T66MRQIsolationCameraX=$CameraX",
    "-T66MRQIsolationCameraY=$CameraY",
    "-T66MRQIsolationCameraZ=$CameraZ",
    "-T66MRQIsolationOrthoWidth=$OrthoWidth"
)

$setupCommand = "$EditorCmd " + (($setupArgs | ForEach-Object { Quote-Arg $_ }) -join " ")
if ($PrintOnly) {
    Write-Host "[CaptureT66NiagaraMRQIsolation] setup: $setupCommand"
}

if (-not $PrintOnly) {
    $setupResult = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments $setupArgs -StdoutPath $setupStdout -StderrPath $setupStderr -Timeout $TimeoutSeconds
    if ($setupResult.ExitCode -ne 0) {
        throw "MRQ isolation setup failed with exit code $($setupResult.ExitCode). See $setupStdout and $setupStderr"
    }
}

$setupManifestPath = Join-Path $OutputDir "setup_manifest.json"
if (-not $PrintOnly -and -not (Test-Path -LiteralPath $setupManifestPath -PathType Leaf)) {
    throw "Missing setup manifest: $setupManifestPath"
}

$setupManifest = $null
if (-not $PrintOnly) {
    $setupManifest = Get-Content -LiteralPath $setupManifestPath -Raw | ConvertFrom-Json
    if (-not $setupManifest.success) {
        throw "MRQ isolation setup manifest reported failure: $($setupManifest.failure_mode)"
    }
}

$mapPath = if ($setupManifest) { $setupManifest.map_path } else { "/Game/VFXLab/Temp/MRQ/L_$TempName" }
$sequencePath = if ($setupManifest) { $setupManifest.sequence_object_path } else { "/Game/VFXLab/Temp/MRQ/LS_$TempName.LS_$TempName" }
$configPath = if ($setupManifest) { $setupManifest.config_object_path } else { "/Game/VFXLab/Temp/MRQ/PC_$TempName.PC_$TempName" }

$renderArgs = @(
    $Project,
    $mapPath,
    "-game",
    "-LevelSequence=$sequencePath",
    "-MoviePipelineConfig=$configPath",
    "-windowed",
    "-resx=$ResX",
    "-resy=$ResY",
    "-notexturestreaming",
    "-unattended",
    "-nop4",
    "-nosplash",
    "-log",
    "-stdout",
    "-FullStdOutLogOutput",
    "-T66Hero1AxeAOEOverrideNiagara=$SystemPath",
    "-T66Hero1AxeAOEManualWarmupTicks=$WarmupTicks",
    "-T66Hero1AxeAOEManualWarmupDelta=$WarmupDeltaSeconds"
)

$renderCommand = "$EditorCmd " + (($renderArgs | ForEach-Object { Quote-Arg $_ }) -join " ")
if ($PrintOnly) {
    Write-Host "[CaptureT66NiagaraMRQIsolation] render: $renderCommand"
    return
}

$renderResult = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments $renderArgs -StdoutPath $renderStdout -StderrPath $renderStderr -Timeout $TimeoutSeconds
if ($renderResult.ExitCode -ne 0) {
    throw "MRQ isolation render failed with exit code $($renderResult.ExitCode). See $renderStdout and $renderStderr"
}

$firstFrame = Get-ChildItem -LiteralPath $framesDir -Filter "*.png" | Sort-Object Name | Select-Object -First 1
if (-not $firstFrame) {
    throw "MRQ isolation render produced no PNG frames in $framesDir"
}

$actualPath = Join-Path $OutputDir "actual.png"
$cropPath = Join-Path $OutputDir "actual_crop.png"
$contactSheetPath = Join-Path $OutputDir "contact_sheet.png"
Copy-Item -LiteralPath $firstFrame.FullName -Destination $actualPath -Force
Convert-PngToOpaqueBlackBackground -Path $actualPath
$analysis = Analyze-PngBounds -Path $actualPath -CropPath $cropPath -Threshold $NonBlackThreshold
Copy-Item -LiteralPath $actualPath -Destination $contactSheetPath -Force

$renderLogText = ""
if (Test-Path -LiteralPath $renderStdout -PathType Leaf) {
    $renderLogText += Get-Content -LiteralPath $renderStdout -Raw
}
if (Test-Path -LiteralPath $renderStderr -PathType Leaf) {
    $renderLogText += "`n" + (Get-Content -LiteralPath $renderStderr -Raw)
}
$hasParticleEvidence = $renderLogText -match "Hero1AxeAOEDiag" -and $renderLogText -match "particleCount=[1-9]"
$hasNoBlackPixels = [bool]$analysis.HasNonBlackPixels
$isSquare = $analysis.Width -eq $analysis.Height
$marginsPass = $false
if ($analysis.Margins) {
    $marginsPass =
        $analysis.Margins.Left -ge $MinimumFrameMarginPixels -and
        $analysis.Margins.Top -ge $MinimumFrameMarginPixels -and
        $analysis.Margins.Right -ge $MinimumFrameMarginPixels -and
        $analysis.Margins.Bottom -ge $MinimumFrameMarginPixels
}
$renderSuccess = $isSquare -and $hasNoBlackPixels -and $marginsPass -and $hasParticleEvidence

$mismatchPath = Join-Path $OutputDir "mismatch_notes.md"
@"
# MRQ Niagara Isolation Mismatch Notes

Reference target: not generated for this same-view route yet.

Current actual frame:

- Square output: $isSquare
- Non-black VFX pixels present: $hasNoBlackPixels
- Full-effect margin pass: $marginsPass
- Niagara particle log evidence present: $hasParticleEvidence
- Visual acceptance status: NOT ACCEPTED. This frame is an isolation/capture proof only.

Known visual mismatch to solve next:

- Red, blue, and white bands are separated instead of forming one readable half-moon crescent.
- Current material read is still streak/checker-like rather than the intended shared aura slash material.
- Impact/white read is not yet attached to enemy contact in this editor-isolation frame.
"@ | Out-File -LiteralPath $mismatchPath -Encoding utf8

$manifestPath = Join-Path $OutputDir "manifest.json"
$manifest = [pscustomobject]@{
    tool = "CaptureT66NiagaraMRQIsolation"
    render_success = $renderSuccess
    failure_mode = if ($renderSuccess) { "" } else { "verification_failed" }
    output_dir = $OutputDir
    frames_dir = $framesDir
    actual_png = $actualPath
    actual_crop_png = $cropPath
    contact_sheet_png = $contactSheetPath
    mismatch_notes = $mismatchPath
    first_frame = $firstFrame.FullName
    setup_manifest = $setupManifestPath
    setup_command = if ($setupResult) { $setupResult.Command } else { $setupCommand }
    render_command = $renderResult.Command
    setup_stdout = $setupStdout
    setup_stderr = $setupStderr
    render_stdout = $renderStdout
    render_stderr = $renderStderr
    system_path = $SystemPath
    temp_name = $TempName
    temp_assets_are_regenerable = $true
    temp_assets_kept = [bool]$KeepTempAssets
    camera = [pscustomobject]@{
        projection_mode = "ORTHOGRAPHIC"
        location = @($CameraX, $CameraY, $CameraZ)
        rotation = @(-90.0, 0.0, 0.0)
        ortho_width = $OrthoWidth
    }
    resolution = @($ResX, $ResY)
    warmup = [pscustomobject]@{
        ticks = $WarmupTicks
        delta_seconds = $WarmupDeltaSeconds
        diagnostic_only = $true
    }
    verification = [pscustomobject]@{
        is_square = $isSquare
        has_non_black_pixels = $hasNoBlackPixels
        margins_pass = $marginsPass
        minimum_frame_margin_pixels = $MinimumFrameMarginPixels
        has_particle_log_evidence = $hasParticleEvidence
        review_background = "opaque_black_rgb_preserved"
        bounds = $analysis
    }
    limitations = @(
        "Editor-isolation visual gate only; does not prove gameplay timing.",
        "A single still cannot prove temporal slash mechanisms.",
        "Current Hero 1 AOE visual remains unaccepted until same-view imagegen target and gameplay evidence pass."
    )
}
$manifest | ConvertTo-Json -Depth 8 | Out-File -LiteralPath $manifestPath -Encoding utf8

if (-not $KeepTempAssets) {
    $cleanupArgs = @(
        $Project,
        "-run=pythonscript",
        "-script=$SetupScript",
        "-unattended",
        "-nop4",
        "-nosplash",
        "-T66MRQIsolationCleanup",
        "-T66MRQIsolationOutput=$OutputDir",
        "-T66MRQIsolationTempName=$TempName"
    )
    $cleanupResult = Invoke-LoggedProcess -FilePath $EditorCmd -Arguments $cleanupArgs -StdoutPath $cleanupStdout -StderrPath $cleanupStderr -Timeout $TimeoutSeconds
    if ($cleanupResult.ExitCode -ne 0) {
        throw "MRQ isolation cleanup failed with exit code $($cleanupResult.ExitCode). See $cleanupStdout and $cleanupStderr"
    }
}

if (-not $renderSuccess) {
    throw "MRQ isolation capture completed but verification failed. See $manifestPath and $actualPath"
}

Write-Host "[CaptureT66NiagaraMRQIsolation] OutputDir=$OutputDir"
Write-Host "[CaptureT66NiagaraMRQIsolation] actual.png=$actualPath"
Write-Host "[CaptureT66NiagaraMRQIsolation] actual_crop.png=$cropPath"
Write-Host "[CaptureT66NiagaraMRQIsolation] manifest.json=$manifestPath"
