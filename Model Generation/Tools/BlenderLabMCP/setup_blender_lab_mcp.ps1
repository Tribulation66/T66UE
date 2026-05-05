[CmdletBinding()]
param(
    [string]$BlenderExe = "",
    [string]$InstallRoot = "$env:USERPROFILE\.codex\tools\blender_mcp_official",
    [string]$Version = "v1.0.0",
    [switch]$SkipCodexConfig
)

$ErrorActionPreference = "Stop"

$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$LogPath = Join-Path $ScriptRoot "setup_blender_lab_mcp.log"
$AddonUrl = "https://projects.blender.org/lab/blender_mcp/releases/download/v1.0.0/mcp-1.0.0.zip?repository=https%3A%2F%2Flab.blender.org%2F&blender_version_min=5.1.0"
$RepoUrl = "https://projects.blender.org/lab/blender_mcp.git"
$LegacyAddonFiles = @(
    "$env:APPDATA\Blender Foundation\Blender\5.0\scripts\addons\blender_mcp_addon.py",
    "$env:APPDATA\Blender Foundation\Blender\5.1\scripts\addons\blender_mcp_addon.py"
)

function Write-SetupLog {
    param([string]$Message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $line = "[$timestamp] $Message"
    Write-Host $line
    Add-Content -Path $LogPath -Value $line -Encoding UTF8
}

function Resolve-BlenderExe {
    param([string]$Candidate)

    if ($Candidate -and (Test-Path -LiteralPath $Candidate)) {
        return (Resolve-Path -LiteralPath $Candidate).Path
    }

    $root = "C:\Program Files\Blender Foundation"
    if (Test-Path -LiteralPath $root) {
        $match = Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object {
                $exe = Join-Path $_.FullName "blender.exe"
                if (Test-Path -LiteralPath $exe) {
                    $exe
                }
            } |
            Select-Object -First 1

        if ($match) {
            return $match
        }
    }

    throw "Could not find blender.exe. Pass -BlenderExe with the full path."
}

function Remove-LegacyAddon {
    foreach ($path in $LegacyAddonFiles) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
            Write-SetupLog "Removed legacy third-party add-on file: $path"
        }
    }

    Get-ChildItem -Path "$env:APPDATA\Blender Foundation\Blender" -Recurse -Filter "blender_mcp_addon*" -ErrorAction SilentlyContinue |
        Remove-Item -Force -ErrorAction SilentlyContinue
}

function Sync-OfficialRepo {
    if (Test-Path -LiteralPath $InstallRoot) {
        Write-SetupLog "Updating official Blender Lab MCP clone: $InstallRoot"
        git -C $InstallRoot fetch --tags origin | Out-Host
        git -C $InstallRoot checkout $Version | Out-Host
    }
    else {
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $InstallRoot) | Out-Null
        Write-SetupLog "Cloning official Blender Lab MCP repo to $InstallRoot"
        git clone --branch $Version $RepoUrl $InstallRoot | Out-Host
    }

    $head = git -C $InstallRoot rev-parse HEAD
    Write-SetupLog "Official Blender Lab MCP revision: $head"
}

function Sync-OfficialMcpVenv {
    $project = Join-Path $InstallRoot "mcp"
    Write-SetupLog "Syncing official MCP server venv: $project"
    uv sync --project $project | Out-Host
}

function Install-OfficialBlenderExtension {
    param([string]$ResolvedBlenderExe)

    $addonZip = Join-Path $env:TEMP "blender_lab_mcp_1.0.0.zip"
    Write-SetupLog "Downloading official Blender Lab MCP extension"
    Invoke-WebRequest -Uri $AddonUrl -OutFile $addonZip

    $installScript = Join-Path $env:TEMP "install_blender_lab_mcp.py"
    @"
import bpy

zip_path = r"$addonZip"
print("Installing official Blender Lab MCP extension:", zip_path)
bpy.ops.extensions.package_install_files(
    filepath=zip_path,
    repo="user_default",
    enable_on_install=True,
    overwrite=True,
)
bpy.ops.wm.save_userpref()
print("Enabled MCP add-ons:", [k for k in bpy.context.preferences.addons.keys() if "mcp" in k.lower()])
"@ | Set-Content -Path $installScript -Encoding UTF8

    & $ResolvedBlenderExe --background --factory-startup --online-mode --python $installScript | Out-Host
}

function Update-CodexConfig {
    param([string]$ResolvedBlenderExe)

    if ($SkipCodexConfig) {
        Write-SetupLog "Skipping Codex config update."
        return
    }

    $configPath = Join-Path $env:USERPROFILE ".codex\config.toml"
    $serverExe = Join-Path $InstallRoot "mcp\.venv\Scripts\blender-mcp.exe"
    $newBlock = @"
[mcp_servers.blender]
command = '$serverExe'
args = []
env = { BLENDER_MCP_HOST = "localhost", BLENDER_MCP_PORT = "9876", BLENDER_PATH = '$ResolvedBlenderExe' }

"@

    if (-not (Test-Path -LiteralPath $configPath)) {
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $configPath) | Out-Null
        Set-Content -Path $configPath -Value $newBlock -Encoding UTF8
        Write-SetupLog "Created Codex config with official Blender Lab MCP server."
        return
    }

    $content = Get-Content -Path $configPath -Raw
    $pattern = "(?ms)^\[mcp_servers\.blender\]\r?\n.*?(?=^\[|\z)"
    if ($content -match $pattern) {
        $content = [regex]::Replace($content, $pattern, $newBlock)
    }
    else {
        if (-not $content.EndsWith("`n")) {
            $content += "`n"
        }
        $content += "`n$newBlock"
    }

    Set-Content -Path $configPath -Value $content -Encoding UTF8
    Write-SetupLog "Updated Codex config to use official Blender Lab MCP server: $serverExe"
}

New-Item -ItemType Directory -Force -Path $ScriptRoot | Out-Null
Set-Content -Path $LogPath -Value "" -Encoding UTF8

$ResolvedBlenderExe = Resolve-BlenderExe -Candidate $BlenderExe
Write-SetupLog "Using Blender: $ResolvedBlenderExe"

Remove-LegacyAddon
Sync-OfficialRepo
Sync-OfficialMcpVenv
Install-OfficialBlenderExtension -ResolvedBlenderExe $ResolvedBlenderExe
Update-CodexConfig -ResolvedBlenderExe $ResolvedBlenderExe
