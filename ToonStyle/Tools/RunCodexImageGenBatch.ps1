[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Manifest,
    [int]$Concurrency = 3,
    [string]$LogDir,
    [switch]$Force,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$SingleRunner = Join-Path $PSScriptRoot "RunCodexImageGen.ps1"
if (-not (Test-Path -LiteralPath $SingleRunner)) {
    throw "Missing single imagegen runner: $SingleRunner"
}

$ManifestPath = if ([System.IO.Path]::IsPathRooted($Manifest)) {
    $Manifest
} else {
    Join-Path $RepoRoot $Manifest
}
$ManifestPath = [System.IO.Path]::GetFullPath($ManifestPath)
if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}

if ([string]::IsNullOrWhiteSpace($LogDir)) {
    $ManifestBase = [System.IO.Path]::GetFileNameWithoutExtension($ManifestPath)
    $LogDir = Join-Path $RepoRoot ("Saved\Codex\ToonStyle\ImageGenBatch\{0}" -f $ManifestBase)
} elseif (-not [System.IO.Path]::IsPathRooted($LogDir)) {
    $LogDir = Join-Path $RepoRoot $LogDir
}
$LogDir = [System.IO.Path]::GetFullPath($LogDir)
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

$ManifestData = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
$Jobs = @()
if ($ManifestData.jobs) {
    $Jobs = @($ManifestData.jobs)
} else {
    $Jobs = @($ManifestData)
}

if ($Jobs.Count -eq 0) {
    throw "Manifest contains no jobs: $ManifestPath"
}
if ($Concurrency -lt 1) {
    throw "Concurrency must be at least 1."
}

$Pending = [System.Collections.Queue]::new()
foreach ($JobSpec in $Jobs) {
    if ([string]::IsNullOrWhiteSpace($JobSpec.Name)) {
        throw "Every job must include Name."
    }
    $HasPrompt = -not [string]::IsNullOrWhiteSpace($JobSpec.Prompt)
    $HasPromptFile = -not [string]::IsNullOrWhiteSpace($JobSpec.PromptFile)
    if ($HasPrompt -eq $HasPromptFile) {
        throw "Job '$($JobSpec.Name)' must include exactly one of Prompt or PromptFile."
    }
    if ([string]::IsNullOrWhiteSpace($JobSpec.OutDir)) {
        throw "Job '$($JobSpec.Name)' must include OutDir."
    }
    $Pending.Enqueue($JobSpec)
}

$Running = @()
$Failures = 0
$Started = 0
$Completed = 0

while ($Pending.Count -gt 0 -or $Running.Count -gt 0) {
    while ($Pending.Count -gt 0 -and $Running.Count -lt $Concurrency) {
        $Spec = $Pending.Dequeue()
        $Started += 1

        $Job = Start-Job -Name $Spec.Name -ScriptBlock {
            param(
                [string]$Runner,
                [object]$Spec,
                [bool]$PassForce,
                [bool]$PassDryRun
            )

            $RunnerArgs = @(
                "-ExecutionPolicy", "Bypass",
                "-File", $Runner,
                "-Name", $Spec.Name,
                "-OutDir", $Spec.OutDir
            )

            if (-not [string]::IsNullOrWhiteSpace($Spec.Prompt)) {
                $RunnerArgs += @("-Prompt", $Spec.Prompt)
            }
            if (-not [string]::IsNullOrWhiteSpace($Spec.PromptFile)) {
                $RunnerArgs += @("-PromptFile", $Spec.PromptFile)
            }
            if (-not [string]::IsNullOrWhiteSpace($Spec.Size)) {
                $RunnerArgs += @("-Size", $Spec.Size)
            }
            if (-not [string]::IsNullOrWhiteSpace($Spec.Quality)) {
                $RunnerArgs += @("-Quality", $Spec.Quality)
            }
            if (-not [string]::IsNullOrWhiteSpace($Spec.Model)) {
                $RunnerArgs += @("-Model", $Spec.Model)
            }
            if ($PassForce) {
                $RunnerArgs += "-Force"
            }
            if ($PassDryRun) {
                $RunnerArgs += "-DryRun"
            }

            $Output = & powershell @RunnerArgs 2>&1
            Write-Output ("T66_IMAGEGEN_EXIT_CODE: {0}" -f $LASTEXITCODE)
            Write-Output $Output
        } -ArgumentList ([object[]]@($SingleRunner, $Spec, $Force.IsPresent, $DryRun.IsPresent))

        $Running += [pscustomobject]@{
            Spec = $Spec
            Job = $Job
        }
        Write-Output ("STARTED: {0}" -f $Spec.Name)
    }

    $Done = @($Running | Where-Object { $_.Job.State -in @("Completed", "Failed", "Stopped") })
    if ($Done.Count -eq 0) {
        Start-Sleep -Seconds 1
        continue
    }

    foreach ($Entry in $Done) {
        $Result = Receive-Job -Job $Entry.Job 2>&1
        $LogPath = Join-Path $LogDir ("{0}.log" -f (($Entry.Spec.Name -replace "[^A-Za-z0-9_.-]", "_").Trim("._-")))
        $OutputText = if ($Result) { ($Result | Out-String) } else { "" }
        Set-Content -Path $LogPath -Value $OutputText -Encoding UTF8

        $ExitCode = 1
        $ExitMatch = [regex]::Match($OutputText, "T66_IMAGEGEN_EXIT_CODE:\s*(-?\d+)")
        if ($ExitMatch.Success) {
            $ExitCode = [int]$ExitMatch.Groups[1].Value
        }
        if ($Entry.Job.State -ne "Completed" -or $ExitCode -ne 0) {
            $Failures += 1
            Write-Output ("FAILED: {0} exit={1} log={2}" -f $Entry.Spec.Name, $ExitCode, $LogPath)
        } else {
            $Completed += 1
            Write-Output ("DONE: {0} log={1}" -f $Entry.Spec.Name, $LogPath)
        }

        Remove-Job -Job $Entry.Job -Force
    }

    $Running = @($Running | Where-Object { $_.Job.State -notin @("Completed", "Failed", "Stopped") })
}

Write-Output ("SUMMARY: started={0} completed={1} failed={2} log_dir={3}" -f $Started, $Completed, $Failures, $LogDir)
if ($Failures -gt 0) {
    exit 1
}
