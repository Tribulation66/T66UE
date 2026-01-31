param(
  [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

$heroesCsv = Join-Path $ProjectRoot "Content\Data\Heroes.csv"
$bossesCsv = Join-Path $ProjectRoot "Content\Data\Bosses.csv"
$ostRoot  = Join-Path $ProjectRoot "Content\Audio\OSTS"

Write-Host "ProjectRoot: $ProjectRoot"
Write-Host "OST root:    $ostRoot"

function Ensure-Dir([string]$path) {
  New-Item -ItemType Directory -Force -Path $path | Out-Null
}

Ensure-Dir (Join-Path $ostRoot "Heroes")
Ensure-Dir (Join-Path $ostRoot "Bosses\Espadas")
Ensure-Dir (Join-Path $ostRoot "Bosses\Difficulty")
Ensure-Dir (Join-Path $ostRoot "Bosses\Special")

# Heroes: create one folder per row-name (first CSV column)
if (Test-Path $heroesCsv) {
  (Get-Content $heroesCsv | Select-Object -Skip 1) | ForEach-Object {
    $line = $_
    if ([string]::IsNullOrWhiteSpace($line)) { return }
    $name = $line.Split(',')[0].Trim()
    if ($name -and $name -ne "---") {
      Ensure-Dir (Join-Path $ostRoot ("Heroes\" + $name))
    }
  }
  Write-Host "Hero folders ensured from Heroes.csv"
} else {
  Write-Warning "Heroes.csv not found at $heroesCsv"
}

# Bosses: create Espadas + Difficulty folders per boss ID.
if (Test-Path $bossesCsv) {
  (Get-Content $bossesCsv | Select-Object -Skip 1) | ForEach-Object {
    $line = $_
    if ([string]::IsNullOrWhiteSpace($line)) { return }
    $boss = $line.Split(',')[0].Trim()
    if ($boss -and $boss -ne "---") {
      Ensure-Dir (Join-Path $ostRoot ("Bosses\Espadas\" + $boss))
      Ensure-Dir (Join-Path $ostRoot ("Bosses\Difficulty\" + $boss))
    }
  }
  Write-Host "Boss folders ensured from Bosses.csv"
} else {
  Write-Warning "Bosses.csv not found at $bossesCsv"
}

# Special boss folders (NPCs that can become bosses)
@("VendorBoss","GamblerBoss","OuroborosBoss") | ForEach-Object {
  Ensure-Dir (Join-Path $ostRoot ("Bosses\Special\" + $_))
}
Write-Host "Special boss folders ensured"

Write-Host "Done."
