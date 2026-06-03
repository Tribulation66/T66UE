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
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\Hero1AxeAOE_CategoryIdolImpactProof_$timestamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"

# Each proof idol drives a category-native downstream behaviour from the official
# Hero_1_black_aoe weapon impact point. The AOE weapon is always the upstream trigger
# (ParentSourceID=Hero_1_black_aoe); only the equipped idol changes the category lane.
$cases = @(
    [pscustomobject]@{
        Name = "LightPierce"
        Idol = "Idol_Electricity_Pierce"
        TimeoutSeconds = 200
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Electricity_Pierce",
            "CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Electricity_Pierce",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Electricity_Pierce Category=Pierce",
            "CombatVFXIdolImpactBindingLookup SourceType=IdolModifier SourceID=Idol_Electricity_Pierce",
            "CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Electricity_Pierce",
            "DamageBySource SourceID=Idol_Electricity_Pierce",
            "RestoredIdolState"
        )
        RequiredRegexPatterns = @(
            "CombatImpactChainDiagnostic SourceID=Idol_Electricity_Pierce .*ParentSourceID=Hero_1_black_aoe ContextParity=PASS",
            "CombatImpactChainDiagnostic SourceID=Idol_Electricity_Pierce .*DamageByDownstreamSource=PASS",
            "Target=Primary ExpectedHit=1 ActualHit=1 .* Result=PASS",
            "Target=PierceInLineSecond ExpectedHit=1 ActualHit=1 .* Result=PASS"
        )
        ForbiddenPatterns = @(
            "Result=FAIL",
            "CombatImpactChainDiagnostic SourceID=Idol_Electricity_Pierce .*ContextParity=FAIL"
        )
    },
    [pscustomobject]@{
        Name = "ElectricBounce"
        Idol = "Idol_Electricity_Bounce"
        TimeoutSeconds = 200
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Electricity_Bounce",
            "CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Electricity_Bounce",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Electricity_Bounce Category=Bounce",
            "CombatVFXIdolImpactBindingLookup SourceType=IdolModifier SourceID=Idol_Electricity_Bounce",
            "CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Electricity_Bounce",
            "DamageBySource SourceID=Idol_Electricity_Bounce",
            "RestoredIdolState"
        )
        RequiredRegexPatterns = @(
            "CombatImpactChainDiagnostic SourceID=Idol_Electricity_Bounce .*ParentSourceID=Hero_1_black_aoe ContextParity=PASS",
            "CombatImpactChainDiagnostic SourceID=Idol_Electricity_Bounce .*DamageByDownstreamSource=PASS",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Electricity_Bounce Category=Bounce .*LinkCount=[1-9]",
            "Target=Primary ExpectedHit=1 ActualHit=1 .* Result=PASS",
            "Target=ChainSecond ExpectedHit=1 ActualHit=1 .* Result=PASS"
        )
        ForbiddenPatterns = @(
            "Result=FAIL",
            "CombatImpactChainDiagnostic SourceID=Idol_Electricity_Bounce .*ContextParity=FAIL"
        )
    },
    [pscustomobject]@{
        Name = "PoisonDOT"
        Idol = "Idol_Nature_DOT"
        TimeoutSeconds = 300
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Nature_DOT",
            "CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Nature_DOT",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Nature_DOT Category=DOT",
            "CombatVFXIdolImpactBindingLookup SourceType=IdolModifier SourceID=Idol_Nature_DOT",
            "CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Nature_DOT",
            "DamageBySource SourceID=Idol_Nature_DOT",
            "RestoredIdolState"
        )
        RequiredRegexPatterns = @(
            "CombatImpactChainDiagnostic SourceID=Idol_Nature_DOT .*ParentSourceID=Hero_1_black_aoe ContextParity=PASS",
            "CombatImpactChainDiagnostic SourceID=Idol_Nature_DOT .*DamageByDownstreamSource=PASS",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Nature_DOT Category=DOT .*DamagePerTick=[0-9]",
            "Target=Primary ExpectedHit=1 ActualHit=1 .* Result=PASS"
        )
        ForbiddenPatterns = @(
            "Result=FAIL",
            "CombatImpactChainDiagnostic SourceID=Idol_Nature_DOT .*ContextParity=FAIL"
        )
    },
    [pscustomobject]@{
        Name = "EarthNeutral"
        Idol = "Idol_Nature_AOE"
        TimeoutSeconds = 200
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase",
            "RestoredIdolState"
        )
        RequiredRegexPatterns = @(
            "Target=Primary ExpectedHit=1 ActualHit=1 .* Result=PASS"
        )
        ForbiddenPatterns = @(
            "Result=FAIL",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Nature_AOE",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Nature_AOE",
            "CombatVFXIdolImpactPlaceholderSpawned SourceType=IdolModifier SourceID=Idol_Nature_AOE",
            "CombatImpactChainDiagnostic SourceID=Idol_Nature_AOE"
        )
    }
)

$summary = @(
    "# Hero 1 Axe AOE Category-Native Idol Impact Proof",
    "",
    "- Output root: $OutputRoot",
    "- Mode: hero1axeaoewateridolimpact (category dispatch by equipped idol)",
    "- Fire delay: $FireDelaySeconds",
    "- Proof idols: Idol_Electricity_Pierce (Pierce), Idol_Electricity_Bounce (Bounce), Idol_Nature_DOT (DOT), Idol_Nature_AOE (neutral control)",
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
        EvidenceLabel = "Hero1AxeAOECategoryIdolImpactProof_$($case.Name)"
        TimeoutSeconds = $case.TimeoutSeconds
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
        "CombatImpactChainDiagnostic",
        "CombatIdolCategoryImpactResolved",
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
    $logMatches = Select-String -LiteralPath $caseLogPath -Pattern $patterns -SimpleMatch
    if ($logMatches) {
        $excerptLines += $logMatches | ForEach-Object { $_.Line }
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
        if (Select-String -LiteralPath $caseLogPath -Pattern $forbiddenPattern -Quiet) {
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

$summaryPath = Join-Path $OutputRoot "Hero1AxeAOECategoryIdolImpactProofSummary.md"
$summary | Set-Content -LiteralPath $summaryPath -Encoding UTF8
Write-Host "Hero 1 axe AOE category-native idol impact proof summary: $summaryPath"
