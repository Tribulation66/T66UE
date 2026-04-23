param(
    [string]$InstallRoot = (Join-Path $PSScriptRoot "..\Tools\Temp\UIUpscaleStack"),
    [string]$RealEsrganVersion = "v0.2.5.0",
    [string]$UpscaylVersion = "v2.15.0",
    [string]$ChaiNNerVersion = "v0.25.1"
)

$ErrorActionPreference = "Stop"

function Ensure-Directory {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }
}

function Download-File {
    param(
        [string]$Url,
        [string]$OutFile
    )
    if (Test-Path -LiteralPath $OutFile) {
        $existing = Get-Item -LiteralPath $OutFile
        if ($existing.Length -gt 0) {
            Write-Host "Using cached download: $OutFile"
            return
        }

        Write-Host "Removing zero-byte partial download: $OutFile"
        Remove-Item -Force -LiteralPath $OutFile
    }

    Write-Host "Downloading $Url"
    $curl = Get-Command curl.exe -ErrorAction SilentlyContinue
    if ($curl) {
        & $curl.Source -L --fail --silent --show-error --output $OutFile $Url
        if ($LASTEXITCODE -ne 0) {
            throw "curl.exe failed to download $Url"
        }
    }
    else {
        Invoke-WebRequest -Uri $Url -OutFile $OutFile
    }

    $downloaded = Get-Item -LiteralPath $OutFile
    if ($downloaded.Length -le 0) {
        throw "Downloaded file is empty: $OutFile"
    }
}

function Extract-Zip {
    param(
        [string]$ZipPath,
        [string]$Destination
    )
    if (Test-Path -LiteralPath $Destination) {
        Remove-Item -Recurse -Force -LiteralPath $Destination
    }
    Ensure-Directory -Path $Destination
    Expand-Archive -LiteralPath $ZipPath -DestinationPath $Destination -Force
}

$InstallRoot = [System.IO.Path]::GetFullPath($InstallRoot)
$DownloadsRoot = Join-Path $InstallRoot "downloads"
$RealEsrganRoot = Join-Path $InstallRoot "RealESRGAN"
$UpscaylRoot = Join-Path $InstallRoot "Upscayl"
$ChaiNNerRoot = Join-Path $InstallRoot "chaiNNer"

Ensure-Directory -Path $InstallRoot
Ensure-Directory -Path $DownloadsRoot

$realEsrganAssetName = "realesrgan-ncnn-vulkan-20220424-windows.zip"
$realEsrganZip = Join-Path $DownloadsRoot $realEsrganAssetName
$realEsrganUrl = "https://github.com/xinntao/Real-ESRGAN/releases/download/$RealEsrganVersion/$realEsrganAssetName"
Download-File -Url $realEsrganUrl -OutFile $realEsrganZip
Extract-Zip -ZipPath $realEsrganZip -Destination $RealEsrganRoot

$upscaylZip = Join-Path $DownloadsRoot ("upscayl-{0}-win.zip" -f $UpscaylVersion.TrimStart("v"))
$upscaylUrl = "https://github.com/upscayl/upscayl/releases/download/$UpscaylVersion/upscayl-$($UpscaylVersion.TrimStart('v'))-win.zip"
Download-File -Url $upscaylUrl -OutFile $upscaylZip
Extract-Zip -ZipPath $upscaylZip -Destination $UpscaylRoot

$chaiNNerZip = Join-Path $DownloadsRoot ("chaiNNer-windows-x64-{0}-portable.zip" -f $ChaiNNerVersion.TrimStart("v"))
$chaiNNerUrl = "https://github.com/chaiNNer-org/chaiNNer/releases/download/$ChaiNNerVersion/chaiNNer-windows-x64-$($ChaiNNerVersion.TrimStart('v'))-portable.zip"
Download-File -Url $chaiNNerUrl -OutFile $chaiNNerZip
Extract-Zip -ZipPath $chaiNNerZip -Destination $ChaiNNerRoot

$realEsrganExe = Get-ChildItem -LiteralPath $RealEsrganRoot -Recurse -Filter "realesrgan-ncnn-vulkan.exe" | Select-Object -First 1 -ExpandProperty FullName
$upscaylExe = Get-ChildItem -LiteralPath $UpscaylRoot -Recurse -Filter "Upscayl.exe" | Select-Object -First 1 -ExpandProperty FullName
$chaiNNerExe = Get-ChildItem -LiteralPath $ChaiNNerRoot -Recurse -Filter "chaiNNer.exe" | Select-Object -First 1 -ExpandProperty FullName

if (-not $realEsrganExe) {
    throw "Could not locate realesrgan-ncnn-vulkan.exe after extraction."
}
if (-not $upscaylExe) {
    throw "Could not locate Upscayl.exe after extraction."
}
if (-not $chaiNNerExe) {
    throw "Could not locate chaiNNer.exe after extraction."
}

$manifest = [ordered]@{
    installed_at = (Get-Date).ToString("s")
    install_root = $InstallRoot
    tools = [ordered]@{
        realesrgan_ncnn_vulkan = [ordered]@{
            version = $RealEsrganVersion
            exe = $realEsrganExe
        }
        upscayl = [ordered]@{
            version = $UpscaylVersion
            exe = $upscaylExe
        }
        chainner = [ordered]@{
            version = $ChaiNNerVersion
            exe = $chaiNNerExe
        }
    }
}

$manifestPath = Join-Path $InstallRoot "stack_manifest.json"
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8

Write-Host ""
Write-Host "Free UI upscale stack installed."
Write-Host "Manifest: $manifestPath"
Write-Host "Real-ESRGAN: $realEsrganExe"
Write-Host "Upscayl: $upscaylExe"
Write-Host "chaiNNer: $chaiNNerExe"
