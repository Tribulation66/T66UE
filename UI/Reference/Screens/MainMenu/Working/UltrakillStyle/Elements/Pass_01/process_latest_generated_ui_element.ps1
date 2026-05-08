param(
    [Parameter(Mandatory=$true)]
    [string]$Name,

    [string]$GeneratedRoot = 'C:\Users\DoPra\.codex\generated_images',
    [string]$PassRoot = 'C:\UE\T66\UI\Reference\Screens\MainMenu\Working\UltrakillStyle\Elements\Pass_01',
    [int]$Pad = 24
)

$ErrorActionPreference = 'Stop'

$generatedDir = Join-Path $PassRoot 'Generated'
$alphaDir = Join-Path $PassRoot 'Alpha'
New-Item -ItemType Directory -Force -Path $generatedDir, $alphaDir | Out-Null

$latest = Get-ChildItem -Path $GeneratedRoot -Recurse -File -Include *.png,*.jpg,*.jpeg,*.webp |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $latest) {
    throw "No generated image found under $GeneratedRoot"
}

$sourcePath = Join-Path $generatedDir "$($Name)_source.png"
$alphaPath = Join-Path $alphaDir "$Name.png"

Copy-Item -LiteralPath $latest.FullName -Destination $sourcePath -Force

& python 'C:\Users\DoPra\.codex\skills\.system\imagegen\scripts\remove_chroma_key.py' `
    --input $sourcePath `
    --out $alphaPath `
    --auto-key border `
    --soft-matte `
    --transparent-threshold 12 `
    --opaque-threshold 220 `
    --despill `
    --force
if ($LASTEXITCODE -ne 0) {
    throw "remove_chroma_key.py failed for $Name with exit code $LASTEXITCODE"
}

$env:T66_UI_ELEMENT_ALPHA = $alphaPath
$env:T66_UI_ELEMENT_SOURCE = $sourcePath
$env:T66_UI_ELEMENT_PAD = "$Pad"
@'
import os
from pathlib import Path
from PIL import Image

path = Path(os.environ["T66_UI_ELEMENT_ALPHA"])
pad = int(os.environ.get("T66_UI_ELEMENT_PAD", "24"))

im = Image.open(path).convert("RGBA")
alpha = im.getchannel("A")
bbox = alpha.getbbox()
if bbox is None:
    raise SystemExit(f"{path} has no non-transparent pixels")

left = max(0, bbox[0] - pad)
top = max(0, bbox[1] - pad)
right = min(im.width, bbox[2] + pad)
bottom = min(im.height, bbox[3] + pad)
trimmed = im.crop((left, top, right, bottom))
trimmed.save(path)

check = Image.open(path).convert("RGBA")
a = check.getchannel("A")
corner_alpha = [
    a.getpixel((0, 0)),
    a.getpixel((check.width - 1, 0)),
    a.getpixel((0, check.height - 1)),
    a.getpixel((check.width - 1, check.height - 1)),
]
if any(v != 0 for v in corner_alpha):
    raise SystemExit(f"{path} failed transparent-corner validation: {corner_alpha}")

print({
    "name": path.stem,
    "source": str(Path(os.environ.get("T66_UI_ELEMENT_SOURCE", ""))),
    "alpha": str(path),
    "size": check.size,
    "bbox": a.getbbox(),
    "corner_alpha": corner_alpha,
})
'@ | python -

Get-Item -LiteralPath $sourcePath, $alphaPath |
    Select-Object FullName,Length,LastWriteTime |
    Format-List
