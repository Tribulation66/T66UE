$ErrorActionPreference = "Stop"

$sourceDir = "C:\UE\T66\SourceAssets"
$outDir = Join-Path $sourceDir "Extracted"

New-Item -ItemType Directory -Force -Path $outDir | Out-Null

Get-ChildItem -Path $sourceDir -Filter "*.zip" | ForEach-Object {
    $zip = $_
    $dest = Join-Path $outDir $zip.BaseName
    New-Item -ItemType Directory -Force -Path $dest | Out-Null
    Write-Host ("Extracting {0} -> {1}" -f $zip.Name, $dest)
    Expand-Archive -Path $zip.FullName -DestinationPath $dest -Force
}

Write-Host "Done."

