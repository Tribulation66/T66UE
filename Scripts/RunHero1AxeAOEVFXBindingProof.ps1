param(
    [string]$OutputRoot,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$FrameCount = 72,
    [int]$FrameRate = 12,
    [double]$CaptureIntervalSeconds = 0.08,
    [double]$DelaySeconds = 5.0,
    [double]$FireDelaySeconds = 7.6,
    [int]$ProofLine1 = 8,
    [int]$ProofSecondary = 1,
    [string]$EvidenceSelectedFrames = "start=50,mid=54,impact=56,dissipate=64",
    [switch]$EvidenceAutoSelectFrames,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
if (-not $OutputRoot) {
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\Hero1AxeAOE_VFXBindingProof_$timestamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"
$cases = @(
    [pscustomobject]@{ Name = "Baseline"; Items = ""; Notes = "No proof items; validates bound production VFX at base item scaling." },
    [pscustomobject]@{ Name = "AoeScale"; Items = "Item_AoeScale"; Notes = "Grants deterministic AOE scale item; validates hitbox/visual scale response." },
    [pscustomobject]@{ Name = "AoeSpeed"; Items = "Item_AoeSpeed"; Notes = "Grants deterministic AOE speed item; validates fire interval and VFX playback response." },
    [pscustomobject]@{ Name = "AoeDamage"; Items = "Item_AoeDamage"; Notes = "Grants deterministic AOE damage item; validates damage-number/HP delta response." }
)

$summary = @(
    "# Hero 1 Axe AOE VFX Binding Proof",
    "",
    "- Output root: $OutputRoot",
    "- Mode: hero1axeaoevfxbinding",
    "- Proof roll: Line1=$ProofLine1, Secondary=$ProofSecondary",
    ""
)

foreach ($case in $cases) {
    $caseRoot = Join-Path $OutputRoot $case.Name
    $videoPath = Join-Path $caseRoot "$($case.Name).mp4"
    $frameDir = Join-Path $caseRoot "frames"
    $evidenceRoot = Join-Path $caseRoot "evidence"
    $caseLogPath = Join-Path $caseRoot "T66.log"
    $caseLogExcerptPath = Join-Path $caseRoot "proof_log_excerpt.md"
    New-Item -ItemType Directory -Force -Path $caseRoot | Out-Null

    $captureArgs = @{
        CaptureMode = "hero1axeaoevfxbinding"
        Output = $videoPath
        FrameDir = $frameDir
        ResX = $ResX
        ResY = $ResY
        FrameCount = $FrameCount
        FrameRate = $FrameRate
        CaptureIntervalSeconds = $CaptureIntervalSeconds
        DelaySeconds = $DelaySeconds
        Hero1AxeHitboxFireDelay = $FireDelaySeconds
        Hero1AxeProofLine1 = $ProofLine1
        Hero1AxeProofSecondary = $ProofSecondary
        UseHero1AxePreviewStaging = $true
        EvidenceBundle = $true
        EvidenceRoot = $evidenceRoot
        EvidenceLabel = "Hero1AxeAOEVFXBindingProof_$($case.Name)"
        TimeoutSeconds = 180
    }
    if ($EvidenceAutoSelectFrames) {
        $captureArgs["EvidenceAutoSelectFrames"] = $true
    } else {
        $captureArgs["EvidenceSelectedFrames"] = $EvidenceSelectedFrames
    }
    if ($case.Items) {
        $captureArgs["Hero1AxeProofItems"] = $case.Items
    }
    if ($PrintOnly) {
        $captureArgs["PrintOnly"] = $true
    }

    & $captureScript @captureArgs
    $captureExitCode = if ($null -eq $LASTEXITCODE) { 0 } else { $LASTEXITCODE }
    if ($captureExitCode -ne 0) {
        throw "Capture failed for $($case.Name) with exit code $captureExitCode."
    }

    $latestLog = Join-Path $repoRoot "Saved\Logs\T66.log"
    if (Test-Path -LiteralPath $latestLog) {
        Copy-Item -LiteralPath $latestLog -Destination $caseLogPath -Force
        $patterns = @(
            "Hero1AxeAOEVFXBindingProof",
            "CombatVFXProductionSpawned",
            "CombatItemCategoryTuning",
            "Hero1AxeAOEHitboxProof",
            "DamageNumber"
        )
        $excerptLines = @(
            "# $($case.Name) Proof Log Excerpt",
            "",
            "- Source log: $caseLogPath",
            "- Patterns: $($patterns -join ', ')",
            ""
        )
        $matches = Select-String -LiteralPath $caseLogPath -Pattern $patterns -SimpleMatch
        if ($matches) {
            $excerptLines += $matches | ForEach-Object { $_.Line }
        } else {
            $excerptLines += "_No proof log lines matched the expected patterns._"
        }
        $excerptLines | Set-Content -LiteralPath $caseLogExcerptPath -Encoding UTF8
    } else {
        @(
            "# $($case.Name) Proof Log Excerpt",
            "",
            "_Saved\\Logs\\T66.log was not present after capture._"
        ) | Set-Content -LiteralPath $caseLogExcerptPath -Encoding UTF8
    }

    $summary += @(
        "## $($case.Name)",
        "",
        "- Items: $($case.Items)",
        "- Video: $videoPath",
        "- Evidence: $evidenceRoot",
        "- Log excerpt: $caseLogExcerptPath",
        "- Notes: $($case.Notes)",
        ""
    )
}

$summaryPath = Join-Path $OutputRoot "Hero1AxeAOEVFXBindingProofSummary.md"
$summary | Set-Content -LiteralPath $summaryPath -Encoding UTF8
Write-Host "Hero 1 axe AOE VFX binding proof summary: $summaryPath"
