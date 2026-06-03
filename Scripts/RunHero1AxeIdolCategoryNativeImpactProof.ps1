<#
.SYNOPSIS
    Category-native idol impact proof runner for the Hero 1 AOE weapon impact point.

.DESCRIPTION
    Generalizes RunHero1AxeAOEWaterIdolImpactProof.ps1 from Water/Earth-only to the
    category-native proof idols driven from the official Hero_1_black_aoe weapon impact
    context:
        Idol_Electricity_Pierce    -> Pierce  (line read off the impact point, idol-owned falloff)
        Idol_Electricity_Bounce -> Bounce  (chain read off the impact point, idol-owned per link)
        Idol_Nature_DOT   -> DOT     (idol-owned ticking damage on the impact target)
        Idol_Ice_AOE    -> AOE     (preserved Water regression path)
        Idol_Nature_AOE    -> neutral control (no themed idol impact context / damage)

    All cases run through the existing proven capture mode
    `hero1axeaoewateridolimpact` with `-Hero1AxeProofIdol <id>`; the Water alias and
    behavior are preserved. Pattern checks are recorded into the per-case summary as
    PASS/FAIL instead of throwing, so a single mismatched anchor cannot abort the batch
    or destroy the other cases' evidence.
#>
param(
    [string]$OutputRoot,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [int]$FrameCount = 72,
    [int]$FrameRate = 12,
    [double]$CaptureIntervalSeconds = 0.08,
    [double]$DelaySeconds = 5.0,
    [double]$FireDelaySeconds = 7.6,
    [int]$TimeoutSeconds = 200,
    [string[]]$Only = @(),
    [switch]$PrintOnly
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
if (-not $OutputRoot) {
    $OutputRoot = Join-Path $repoRoot "Saved\VideoCaptures\Hero1AxeIdolCategoryNativeImpactProof_$timestamp"
}
$OutputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$captureScript = Join-Path $PSScriptRoot "CaptureT66GameplayVideo.ps1"

function New-CategoryNativeCase {
    param([string]$Name, [string]$Idol, [string]$Category)
    $weapon = "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe"
    return [pscustomobject]@{
        Name = $Name
        Idol = $Idol
        RequiredPatterns = @(
            $weapon,
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=$Idol ParentSourceID=Hero_1_black_aoe",
            "CombatVFXIdolProjectileLaneSuppressed SourceID=$Idol",
            "CombatIdolCategoryImpactResolved SourceID=$Idol Category=$Category ParentSourceID=Hero_1_black_aoe",
            "CombatImpactChainDiagnostic SourceID=$Idol",
            "ParentSourceID=Hero_1_black_aoe ContextParity=PASS",
            "DamageBySource SourceID=$Idol",
            "RestoredIdolState"
        )
        ForbiddenPatterns = @(
            "Result=FAIL"
        )
    }
}

$allCases = @(
    (New-CategoryNativeCase -Name "LightPierce" -Idol "Idol_Electricity_Pierce" -Category "Pierce"),
    (New-CategoryNativeCase -Name "ElectricBounce" -Idol "Idol_Electricity_Bounce" -Category "Bounce"),
    (New-CategoryNativeCase -Name "PoisonDOT" -Idol "Idol_Nature_DOT" -Category "DOT"),
    [pscustomobject]@{
        Name = "WaterAOERegression"
        Idol = "Idol_Ice_AOE"
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Ice_AOE ParentSourceID=Hero_1_black_aoe",
            "CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Ice_AOE",
            "CombatIdolWaterImpactResolved SourceID=Idol_Ice_AOE",
            "RadiusSource=FIdolData.AoeRadius",
            "CombatImpactChainDiagnostic SourceID=Idol_Ice_AOE",
            "ParentSourceID=Hero_1_black_aoe ContextParity=PASS",
            "CombatIdolImpactDiagnostic SourceID=Idol_Ice_AOE",
            "WaterIdolContextParity=PASS",
            "DamageBySource SourceID=Idol_Ice_AOE",
            "RestoredIdolState"
        )
        ForbiddenPatterns = @(
            "Result=FAIL"
        )
    },
    [pscustomobject]@{
        Name = "EarthNeutral"
        Idol = "Idol_Nature_AOE"
        RequiredPatterns = @(
            "CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe",
            "DamageBySource SourceID=Idol_Nature_AOE",
            "RestoredIdolState"
        )
        ForbiddenPatterns = @(
            "Result=FAIL",
            "CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Ice_AOE",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Electricity_Pierce",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Electricity_Bounce",
            "CombatIdolCategoryImpactResolved SourceID=Idol_Nature_DOT",
            "DamageBySource SourceID=Idol_Ice_AOE",
            "DamageBySource SourceID=Idol_Electricity_Pierce",
            "DamageBySource SourceID=Idol_Electricity_Bounce",
            "DamageBySource SourceID=Idol_Nature_DOT"
        )
    }
)

if ($Only.Count -gt 0) {
    $allCases = $allCases | Where-Object { $Only -contains $_.Name }
}

$logPatterns = @(
    "Hero1AxeAOEIdolImpactProof",
    "Hero1AxeAOEHitboxProof",
    "CombatImpactContext",
    "CombatIdolCategoryImpactResolved",
    "CombatIdolWaterImpactResolved",
    "CombatImpactChainDiagnostic",
    "CombatIdolImpactDiagnostic",
    "CombatIdolImpactDiagnosticSkip",
    "CombatVFXIdolProjectileLaneSuppressed",
    "CombatVFXIdolImpactBindingLookup",
    "CombatVFXIdolImpactPlaceholderSpawned",
    "DamageBySource"
)

$summary = @(
    "# Hero 1 Axe Idol Category-Native Impact Proof",
    "",
    "- Output root: $OutputRoot",
    "- Mode: hero1axeaoewateridolimpact (Water alias preserved)",
    "- Fire delay: $FireDelaySeconds",
    "- Generated: $timestamp",
    ""
)

foreach ($case in $allCases) {
    $caseRoot = Join-Path $OutputRoot $case.Name
    $videoPath = Join-Path $caseRoot "$($case.Name).mp4"
    $frameDir = Join-Path $caseRoot "frames"
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
        TimeoutSeconds = $TimeoutSeconds
    }
    if ($PrintOnly) {
        $captureArgs["PrintOnly"] = $true
    }

    Write-Host "=== Idol category-native proof case: $($case.Name) ($($case.Idol)) ==="
    & $captureScript @captureArgs
    $captureExitCode = if ($null -eq $LASTEXITCODE) { 0 } else { $LASTEXITCODE }

    if ($PrintOnly) {
        continue
    }

    $caseStatus = if ($captureExitCode -eq 0) { "CaptureOK" } else { "CaptureFAILED(exit=$captureExitCode)" }

    $latestLog = Join-Path $repoRoot "Saved\Logs\T66.log"
    $requiredResults = @()
    $forbiddenResults = @()
    if (Test-Path -LiteralPath $latestLog) {
        Copy-Item -LiteralPath $latestLog -Destination $caseLogPath -Force

        $excerptLines = @(
            "# $($case.Name) Proof Log Excerpt",
            "",
            "- Source log: $caseLogPath",
            "- Capture status: $caseStatus",
            ""
        )
        $matched = Select-String -LiteralPath $caseLogPath -Pattern $logPatterns -SimpleMatch
        if ($matched) {
            $excerptLines += $matched | ForEach-Object { $_.Line }
        } else {
            $excerptLines += "_No proof log lines matched the expected patterns._"
        }
        $excerptLines | Set-Content -LiteralPath $caseLogExcerptPath -Encoding UTF8

        foreach ($p in $case.RequiredPatterns) {
            $hit = Select-String -LiteralPath $caseLogPath -Pattern $p -SimpleMatch -Quiet
            $requiredResults += [pscustomobject]@{ Pattern = $p; Result = if ($hit) { "PASS" } else { "FAIL" } }
        }
        foreach ($p in $case.ForbiddenPatterns) {
            $hit = Select-String -LiteralPath $caseLogPath -Pattern $p -SimpleMatch -Quiet
            $forbiddenResults += [pscustomobject]@{ Pattern = $p; Result = if ($hit) { "FAIL(present)" } else { "PASS(absent)" } }
        }
    } else {
        $caseStatus += "; MissingLog"
    }

    $summary += @("## $($case.Name)", "", "- Idol: $($case.Idol)", "- Capture status: $caseStatus", "- Video: $videoPath", "- Frames: $frameDir", "- Log: $caseLogPath", "- Log excerpt: $caseLogExcerptPath", "", "Required patterns:")
    foreach ($r in $requiredResults) { $summary += "- [$($r.Result)] $($r.Pattern)" }
    $summary += @("", "Forbidden patterns:")
    foreach ($r in $forbiddenResults) { $summary += "- [$($r.Result)] $($r.Pattern)" }
    $summary += ""
}

$summaryPath = Join-Path $OutputRoot "Hero1AxeIdolCategoryNativeImpactProofSummary.md"
$summary | Set-Content -LiteralPath $summaryPath -Encoding UTF8
Write-Host "Idol category-native impact proof summary: $summaryPath"
