[CmdletBinding()]
param(
    [string]$BlenderExe = "",
    [int]$Port = 9876,
    [switch]$Visible
)

$ErrorActionPreference = "Stop"

$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$StartScript = Join-Path $ScriptRoot "start_blender_lab_mcp.py"
$LogPath = Join-Path $ScriptRoot "launch_blender_lab_mcp.log"

function Write-LaunchLog {
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

function Wait-ForBlenderSocket {
    param(
        [int]$TargetPort,
        [int]$TimeoutSeconds = 45
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        $client = New-Object System.Net.Sockets.TcpClient
        try {
            $connect = $client.BeginConnect("127.0.0.1", $TargetPort, $null, $null)
            if ($connect.AsyncWaitHandle.WaitOne(1000, $false)) {
                $client.EndConnect($connect)
                Write-LaunchLog "Official Blender Lab MCP socket is accepting connections on 127.0.0.1:$TargetPort"
                return $true
            }
        }
        catch {
        }
        finally {
            $client.Close()
        }

        Start-Sleep -Seconds 1
    }

    Write-LaunchLog "Timed out waiting for official Blender Lab MCP socket on 127.0.0.1:$TargetPort"
    return $false
}

New-Item -ItemType Directory -Force -Path $ScriptRoot | Out-Null
Set-Content -Path $LogPath -Value "" -Encoding UTF8

$ResolvedBlenderExe = Resolve-BlenderExe -Candidate $BlenderExe
$env:BLENDER_MCP_HOST = "localhost"
$env:BLENDER_MCP_PORT = "$Port"

$windowStyle = "Hidden"
if ($Visible) {
    $windowStyle = "Normal"
}

Write-LaunchLog "Starting Blender Lab MCP through: $ResolvedBlenderExe"
Write-LaunchLog "Using start script: $StartScript"
$args = @("--online-mode", "--python", $StartScript)
$process = Start-Process -FilePath $ResolvedBlenderExe -ArgumentList $args -WindowStyle $windowStyle -PassThru
Write-LaunchLog "Started Blender process $($process.Id) with window style $windowStyle"

$connected = Wait-ForBlenderSocket -TargetPort $Port
if (-not $connected) {
    exit 1
}
