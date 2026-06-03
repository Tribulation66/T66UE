param(
    [string]$OutputRoot,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$FrameCount = 72,
    [int]$FrameRate = 12,
    [double]$CaptureIntervalSeconds = 0.08,
    [double]$DelaySeconds = 5.0,
    [double]$FireDelaySeconds = 7.6,
    [string]$EvidenceSelectedFrames = "start=50,mid=56,impact=64,dissipate=68",
    [switch]$EvidenceAutoSelectFrames,
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
if (-not $OutputRoot) {
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_$timestamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"
$cases = @(
    [pscustomobject]@{
        Name = "WaterImpact"
        Idol = "Idol_Ice_AOE"
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Ice_AOE",
            "CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Ice_AOE",
            "CombatIdolWaterImpactResolved SourceID=Idol_Ice_AOE",
            "RadiusSource=FIdolData.AoeRadius",
            "AoeDelay=0.150 DelayApplied=false Reason=LegacyImmediatePreserved",
            "CombatVFXIdolImpactBindingLookup SourceType=IdolModifier SourceID=Idol_Ice_AOE",
            "CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Ice_AOE",
            "CombatIdolImpactDiagnostic SourceID=Idol_Ice_AOE",
            "WaterIdolContextParity=PASS",
            "WaterSkippedNoWeaponContext=0",
            "WaterSkippedInvalidImpactPoint=0",
            "WaterLegacyFallbacks=0",
            "DamageBySource SourceID=Idol_Ice_AOE",
            "RestoredIdolState"
        )
        RequiredRegexPatterns = @(
            "Target=WaterOnlyInnerHollow ExpectedHit=1 ActualHit=1 .* Result=PASS",
            "WeaponImpactContexts=[1-9][0-9]*",
            "EligibleWeaponImpactContexts=[1-9][0-9]*",
            "ImpactPresentationIdolSlots=[1-9][0-9]*",
            "ExpectedWaterIdolImpactContexts=[1-9][0-9]*",
            "WaterIdolImpactContexts=[1-9][0-9]*"
        )
        ForbiddenPatterns = @(
            "Result=FAIL"
        )
    },
    [pscustomobject]@{
        Name = "EarthNeutral"
        Idol = "Idol_Nature_AOE"
        RequiredPatterns = @(
            "DamageBySource SourceID=Idol_Nature_AOE",
            "RestoredIdolState"
        )
        RequiredRegexPatterns = @(
            "Target=InnerHollow ExpectedHit=0 ActualHit=0 .* Result=PASS"
        )
        ForbiddenPatterns = @(
            "Result=FAIL",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Ice_AOE",
            "CombatIdolWaterImpactResolved",
            "CombatIdolImpactDiagnostic SourceID=Idol_Ice_AOE",
            "CombatVFXIdolImpactPlaceholderSpawned"
        )
    }
)

$summary = @(
    "# Hero 1 Axe AOE Water Idol Impact Proof",
    "",
    "- Output root: $OutputRoot",
    "- Mode: hero1axeaoewateridolimpact",
    "- Fire delay: $FireDelaySeconds",
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
        CaptureMode = "hero1axeaoewateridolimpact"
        Output = $videoPath
        FrameDir = $frameDir
        ResX = $ResX
        ResY = $ResY
        FrameCount = $FrameCount
        FrameRate = $FrameRate
        CaptureIntervalSeconds = $CaptureIntervalSeconds
        DelaySeconds = $DelaySeconds
        Hero1AxeHitboxFireDelay = $FireDelaySeconds
        Hero1AxeProofIdol = $case.Idol
        UseHero1AxePreviewStaging = $true
        EvidenceBundle = $true
        EvidenceRoot = $evidenceRoot
        EvidenceLabel = "Hero1AxeAOEWaterIdolImpactProof_$($case.Name)"
        TimeoutSeconds = 180
    }
    if ($EvidenceAutoSelectFrames) {
        $captureArgs["EvidenceAutoSelectFrames"] = $true
    } else {
        $captureArgs["EvidenceSelectedFrames"] = $EvidenceSelectedFrames
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
    if (-not (Test-Path -LiteralPath $latestLog)) {
        throw "Saved\Logs\T66.log was not present after $($case.Name)."
    }

    Copy-Item -LiteralPath $latestLog -Destination $caseLogPath -Force
    $patterns = @(
        "Hero1AxeAOEIdolImpactProof",
        "Hero1AxeAOEHitboxProof",
        "CombatImpactContext",
        "CombatIdolWaterImpactResolved",
        "CombatIdolImpactDiagnostic",
        "CombatIdolImpactDiagnosticSkip",
        "CombatVFXIdolProjectileLaneSuppressed",
        "CombatVFXIdolImpactBindingLookup",
        "CombatVFXIdolImpactPlaceholderSpawned",
        "DamageBySource"
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

    foreach ($requiredPattern in $case.RequiredPatterns) {
        if (-not (Select-String -LiteralPath $caseLogPath -Pattern $requiredPattern -SimpleMatch -Quiet)) {
            throw "Proof $($case.Name) missing required log pattern: $requiredPattern"
        }
    }
    foreach ($requiredRegex in $case.RequiredRegexPatterns) {
        if (-not (Select-String -LiteralPath $caseLogPath -Pattern $requiredRegex -Quiet)) {
            throw "Proof $($case.Name) missing required log regex: $requiredRegex"
        }
    }
    foreach ($forbiddenPattern in $case.ForbiddenPatterns) {
        if (Select-String -LiteralPath $caseLogPath -Pattern $forbiddenPattern -SimpleMatch -Quiet) {
            throw "Proof $($case.Name) found forbidden log pattern: $forbiddenPattern"
        }
    }

    $summary += @(
        "## $($case.Name)",
        "",
        "- Idol: $($case.Idol)",
        "- Video: $videoPath",
        "- Evidence: $evidenceRoot",
        "- Log excerpt: $caseLogExcerptPath",
        "- Required patterns checked: $($case.RequiredPatterns.Count)",
        "- Required regex patterns checked: $($case.RequiredRegexPatterns.Count)",
        "- Forbidden patterns checked: $($case.ForbiddenPatterns.Count)",
        ""
    )
}

$summaryPath = Join-Path $OutputRoot "Hero1AxeAOEWaterIdolImpactProofSummary.md"
$summary | Set-Content -LiteralPath $summaryPath -Encoding UTF8
Write-Host "Hero 1 axe AOE Water idol impact proof summary: $summaryPath"
