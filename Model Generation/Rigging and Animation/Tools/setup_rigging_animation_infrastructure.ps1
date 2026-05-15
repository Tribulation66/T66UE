param(
    [string]$ProjectRoot = "C:\UE\T66",
    [string]$DownloadsRoot = "$env:USERPROFILE\Downloads",
    [string]$BlenderExe = "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe",
    [switch]$SkipBlenderInstall
)

$ErrorActionPreference = "Stop"

$RiggingRoot = Join-Path $ProjectRoot "Model Generation\Rigging and Animation"
$ExternalRoot = Join-Path $RiggingRoot "External"
$QuaterniusRoot = Join-Path $ExternalRoot "Quaternius"
$RigodotifyRoot = Join-Path $ExternalRoot "Rigodotify"
$RigodotifyZip = Join-Path $ExternalRoot "Rigodotify.zip"

New-Item -ItemType Directory -Force -Path $ExternalRoot, $QuaterniusRoot | Out-Null

if (Test-Path -LiteralPath $RigodotifyRoot) {
    git -C $RigodotifyRoot pull --ff-only
} else {
    git clone https://github.com/catprisbrey/Rigodotify.git $RigodotifyRoot
}

git -C $RigodotifyRoot archive --format=zip --prefix=Rigodotify/ --output=$RigodotifyZip HEAD

$Packages = @(
    @{
        Zip = "Universal Animation Library[Standard].zip"
        Target = "Universal Animation Library Standard"
    },
    @{
        Zip = "Universal Animation Library 2[Standard].zip"
        Target = "Universal Animation Library 2 Standard"
    },
    @{
        Zip = "Universal Base Characters[Standard].zip"
        Target = "Universal Base Characters Standard"
    },
    @{
        Zip = "Universal Animation Library[Source].zip"
        Target = "Universal Animation Library Source"
    },
    @{
        Zip = "Universal Animation Library 2[Source].zip"
        Target = "Universal Animation Library 2 Source"
    },
    @{
        Zip = "Universal Base Characters[Source].zip"
        Target = "Universal Base Characters Source"
    }
)

$FoundQuaterniusPackage = $false
foreach ($Package in $Packages) {
    $ZipPath = Join-Path $DownloadsRoot $Package.Zip
    $TargetPath = Join-Path $QuaterniusRoot $Package.Target

    if (!(Test-Path -LiteralPath $ZipPath)) {
        Write-Warning "Missing package zip: $ZipPath"
        continue
    }

    $FoundQuaterniusPackage = $true
    if (Test-Path -LiteralPath $TargetPath) {
        $Existing = Get-ChildItem -LiteralPath $TargetPath -Force -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($Existing) {
            Write-Host "Skipping existing extract: $TargetPath"
            continue
        }
    } else {
        New-Item -ItemType Directory -Force -Path $TargetPath | Out-Null
    }

    Write-Host "Extracting $ZipPath -> $TargetPath"
    Expand-Archive -LiteralPath $ZipPath -DestinationPath $TargetPath -Force
}

if (!$FoundQuaterniusPackage) {
    throw "No Quaternius package zips were found under $DownloadsRoot"
}

if (!$SkipBlenderInstall) {
    if (!(Test-Path -LiteralPath $BlenderExe)) {
        throw "Blender executable not found: $BlenderExe"
    }

    & $BlenderExe --background --python-expr "import bpy; bpy.ops.preferences.addon_enable(module='rigify'); bpy.ops.preferences.addon_install(filepath=r'$RigodotifyZip', overwrite=True); bpy.ops.preferences.addon_enable(module='Rigodotify'); bpy.ops.wm.save_userpref()"
}

Write-Host "Rigging and animation infrastructure is ready: $RiggingRoot"
