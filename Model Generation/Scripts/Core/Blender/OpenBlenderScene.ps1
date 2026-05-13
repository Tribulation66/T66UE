[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $BlendFile,

    [string] $BlenderPath = "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe",

    [switch] $SkipBackgroundVerify,

    [int] $LoadTimeoutSeconds = 30
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $BlenderPath)) {
    throw "Blender executable was not found: $BlenderPath"
}

$resolvedBlend = Resolve-Path -LiteralPath $BlendFile
$blendPath = $resolvedBlend.ProviderPath
$blendLeaf = Split-Path -Leaf $blendPath
$blendDir = Split-Path -Parent $blendPath

if (-not $SkipBackgroundVerify) {
    $verifyExpr = @"
import bpy
print("T66_OPEN_BLEND=" + bpy.data.filepath)
print("T66_OBJECT_COUNT=" + str(len(bpy.data.objects)))
print("T66_OBJECT_NAMES=" + ",".join(sorted(obj.name for obj in bpy.data.objects)[:24]))
"@

    $verifyOutput = & $BlenderPath --background $blendPath --python-expr $verifyExpr 2>&1
    if ($LASTEXITCODE -ne 0) {
        $verifyText = ($verifyOutput | Out-String).Trim()
        throw "Blender failed to load '$blendPath' in background verification.`n$verifyText"
    }

    $loadedLine = $verifyOutput | Where-Object { $_ -like "T66_OPEN_BLEND=*" } | Select-Object -Last 1
    if (-not $loadedLine -or ($loadedLine -ne "T66_OPEN_BLEND=$blendPath")) {
        $verifyText = ($verifyOutput | Out-String).Trim()
        throw "Background verification did not load the expected file '$blendPath'.`n$verifyText"
    }
}

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $BlenderPath
$startInfo.WorkingDirectory = $blendDir
$null = $startInfo.ArgumentList.Add($blendPath)

$process = [System.Diagnostics.Process]::Start($startInfo)
if (-not $process) {
    throw "Failed to start Blender."
}

$deadline = (Get-Date).AddSeconds($LoadTimeoutSeconds)
$title = ""
do {
    Start-Sleep -Milliseconds 500
    $process.Refresh()
    $title = $process.MainWindowTitle
    if ($title -like "*$blendLeaf*") {
        break
    }
} while (-not $process.HasExited -and (Get-Date) -lt $deadline)

if ($process.HasExited) {
    throw "Blender exited before loading '$blendPath'."
}

if ($title -notlike "*$blendLeaf*") {
    Write-Warning "Blender started as PID $($process.Id), but the window title did not confirm '$blendLeaf' within $LoadTimeoutSeconds seconds. Current title: '$title'"
} else {
    Write-Host "Opened '$blendPath' in Blender PID $($process.Id)."
    Write-Host "Window title confirmed: $title"
}
