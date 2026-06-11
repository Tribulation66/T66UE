# Copyright Tribulation 66. All Rights Reserved.
#
# Detached watcher for Pixal3D RunPod batches. Polls the remote DONE.json
# with per-call SSH timeouts (individual hangs/failures never kill the
# watch), auto-downloads outputs on completion, and prints exactly one
# machine-readable POLL_RESULT line at the end.
#
# Agent usage pattern (survives the 10-minute tool window):
#   1. Launch DETACHED with output redirected to a log:
#      Start-Process pwsh -WindowStyle Hidden `
#        -ArgumentList '-NoProfile','-File','<this script>',
#          '-LocalRunRoot','<local run>','-RemoteRunRoot','<remote run>' `
#        -RedirectStandardOutput '<local run>\poll.log'
#   2. Arm a persistent Monitor on the log:
#      tail -f '<local run>/poll.log' | grep --line-buffered "POLL_RESULT"
#   The Monitor notification fires the moment generation finishes.

param(
    [string]$PodIp = "194.68.245.3",
    [int]$PodPort = 22112,
    [string]$KeyPath = "$env:USERPROFILE\.ssh\id_ed25519",
    [Parameter(Mandatory = $true)][string]$LocalRunRoot,
    [Parameter(Mandatory = $true)][string]$RemoteRunRoot,
    [int]$PollSeconds = 45,
    [int]$TimeoutMinutes = 90
)

$ErrorActionPreference = "Continue"
$deadline = (Get-Date).AddMinutes($TimeoutMinutes)
$donePath = "$RemoteRunRoot/Logs/DONE.json"
$sshFailures = 0

Write-Output "[PixalPoll] watching $donePath every ${PollSeconds}s (timeout ${TimeoutMinutes}m)"

while ((Get-Date) -lt $deadline) {
    Start-Sleep -Seconds $PollSeconds
    $out = $null
    try {
        $out = ssh -o ConnectTimeout=20 -o BatchMode=yes -o StrictHostKeyChecking=no `
            -p $PodPort -i $KeyPath "root@$PodIp" "cat '$donePath' 2>/dev/null" 2>$null
    } catch {
        $sshFailures++
    }
    if (-not $out) {
        $sshFailures++
        if ($sshFailures % 5 -eq 0) {
            Write-Output "[PixalPoll] still waiting ($sshFailures transient ssh misses so far)"
        }
        continue
    }
    if (($out -join " ") -match "exit_code") {
        Write-Output "[PixalPoll] DONE.json found, downloading outputs..."
        python "$PSScriptRoot\run_pixal3d_batch.py" download `
            --pod-ip $PodIp --pod-port $PodPort --key-path $KeyPath `
            --local-run-root $LocalRunRoot --remote-run-root $RemoteRunRoot 2>&1 |
            ForEach-Object { Write-Output "[PixalPoll] $_" }
        $glbs = @(Get-ChildItem (Join-Path $LocalRunRoot "Outputs") -Filter *.glb -ErrorAction SilentlyContinue |
            Where-Object { $_.Length -gt 0 })
        if ($glbs.Count -eq 0) {
            # download subcommand can fail on a transient ssh hiccup — scp directly
            Write-Output "[PixalPoll] download fallback via scp"
            New-Item -ItemType Directory -Force -Path (Join-Path $LocalRunRoot "Outputs") | Out-Null
            scp -o ConnectTimeout=30 -P $PodPort -i $KeyPath `
                "root@${PodIp}:$RemoteRunRoot/Outputs/*.glb" (Join-Path $LocalRunRoot "Outputs") 2>&1 |
                ForEach-Object { Write-Output "[PixalPoll] $_" }
            $glbs = @(Get-ChildItem (Join-Path $LocalRunRoot "Outputs") -Filter *.glb -ErrorAction SilentlyContinue |
                Where-Object { $_.Length -gt 0 })
        }
        if ($glbs.Count -gt 0) {
            Write-Output "POLL_RESULT=DONE files=$($glbs.Count) names=$(($glbs.Name) -join ',')"
            exit 0
        }
        Write-Output "POLL_RESULT=FAILED reason=done-but-no-glbs"
        exit 1
    }
}

Write-Output "POLL_RESULT=TIMEOUT after ${TimeoutMinutes}m (remote job may still be running — re-arm the watcher)"
exit 2
