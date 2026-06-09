param(
    [Parameter(Mandatory = $true)]
    [string]$DestinationDirectory,

    [Parameter(Mandatory = $true)]
    [string]$BaseName,

    [Parameter(Mandatory = $true)]
    [string]$Prompt,

    [Parameter(Mandatory = $true)]
    [string]$Category,

    [Parameter(Mandatory = $true)]
    [string]$Target,

    [string]$Notes = ""
)

$ErrorActionPreference = "Stop"

$generatedRoot = Join-Path $env:USERPROFILE ".codex\generated_images"
$latest = Get-ChildItem -Path $generatedRoot -Recurse -Filter "*.png" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $latest) {
    throw "No generated PNG found under $generatedRoot"
}

New-Item -ItemType Directory -Path $DestinationDirectory -Force | Out-Null

$imagePath = Join-Path $DestinationDirectory "$BaseName.png"
$promptPath = Join-Path $DestinationDirectory "${BaseName}_prompt.txt"
$manifestPath = Join-Path $DestinationDirectory "${BaseName}_manifest.md"

Copy-Item -LiteralPath $latest.FullName -Destination $imagePath -Force
Set-Content -LiteralPath $promptPath -Value $Prompt -Encoding UTF8

$manifest = @(
    "# $BaseName",
    "",
    "- Category: $Category",
    "- Target: $Target",
    "- Image: $imagePath",
    "- Prompt: $promptPath",
    "- Source generated image: $($latest.FullName)",
    "- Generated at: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz')",
    "- Style references: locked FriendSlop Hero 1 male/female rose-pink references only",
    "- Notes: $Notes"
)

Set-Content -LiteralPath $manifestPath -Value $manifest -Encoding UTF8

Write-Output $imagePath
