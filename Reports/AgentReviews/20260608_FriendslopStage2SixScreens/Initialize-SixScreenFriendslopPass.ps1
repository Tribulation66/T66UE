param(
    [string]$ProjectRoot = "C:\UE\T66"
)

$ErrorActionPreference = "Stop"

$screens = @(
    [pscustomobject]@{
        Name = "Overview"
        Title = "Account Overview"
        Baseline = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\Overview\baseline_20260608\Overview_baseline.png"
        Dump = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\Overview\baseline_20260608\Overview_baseline_dump.json"
        Families = @(
            "AccountTabs|Overview and History sub-tabs plus info icons; excludes shared frontend top bar.",
            "LeftAccountPanels|Player block, account status panel, account progress panel, avatar, progress tracks, and status messaging.",
            "RightRecordsPanels|Personal best filters, highest score panel, best speed run panel, table rows, dividers, and value cells."
        )
        Source = "Source/T66/UI/Screens/T66AccountStatusScreen.cpp"
    },
    [pscustomobject]@{
        Name = "History"
        Title = "Account History"
        Baseline = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\History\baseline_20260608\History_baseline.png"
        Dump = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\History\baseline_20260608\History_baseline_dump.json"
        Families = @(
            "AccountTabs|Overview and History sub-tabs plus info icons; excludes shared frontend top bar.",
            "HistoryFilterPanel|Hero, difficulty, party size, status filters, daily descent checkbox, labels, and dropdown surfaces.",
            "HistoryTable|Run history panel, table header, row surfaces, sort controls, rank selector, dividers, and empty state."
        )
        Source = "Source/T66/UI/Screens/T66AccountStatusScreen.cpp"
    },
    [pscustomobject]@{
        Name = "Diplomas"
        Title = "Permanent Powerups"
        Baseline = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\Diplomas\baseline_20260608\Diplomas_baseline.png"
        Dump = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\Diplomas\baseline_20260608\Diplomas_baseline_dump.json"
        Families = @(
            "PowerUpTabs|Relics/Permanent and Steroids/One Run Use sub-tabs; excludes shared frontend top bar.",
            "PermanentInfoStrip|Permanent powerup instruction strip and coupon balance context.",
            "RelicCardGrid|Relic card shells, art wells, buy/owned buttons, cost pills, and card row spacing.",
            "RelicScrollSurface|Scrollable body, scrollbar, card-grid backing, and vertical containment."
        )
        Source = "Source/T66/UI/Screens/T66PowerUpScreen.cpp"
    },
    [pscustomobject]@{
        Name = "Drugs"
        Title = "Temporary Powerups"
        Baseline = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\Drugs\baseline_20260608\Drugs_baseline.png"
        Dump = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\Drugs\baseline_20260608\Drugs_baseline_dump.json"
        Families = @(
            "PowerUpTabs|Relics/Permanent and Steroids/One Run Use sub-tabs; excludes shared frontend top bar.",
            "TemporaryInfoStrip|One-run-use instruction strip and current-selection context.",
            "SteroidRows|Primary stat row shells, row labels, and row backing surfaces.",
            "TemporaryCardGrid|Temporary buff card surfaces, icon wells, buy/equip buttons, coupon/cost areas, and card spacing.",
            "TemporaryScrollSurface|Scrollable body, scrollbar, row stacking, and vertical containment."
        )
        Source = "Source/T66/UI/Screens/T66PowerUpScreen.cpp"
    },
    [pscustomobject]@{
        Name = "SteamAchievements"
        Title = "Steam Achievements"
        Baseline = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\SteamAchievements\baseline_20260608\SteamAchievements_baseline.png"
        Dump = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\SteamAchievements\baseline_20260608\SteamAchievements_baseline_dump.json"
        Families = @(
            "AchievementTabs|Steam and Secret sub-tabs plus info icons; excludes shared frontend top bar.",
            "SteamSummaryPanel|Steam logo, summary panel shell, progress track/fill, count area, and header containment.",
            "AchievementList|Achievement list shell, row surfaces, claim buttons, favorite buttons, reward icons, table dividers, and live row text/data slots."
        )
        Source = "Source/T66/UI/Screens/T66AchievementsScreen.cpp"
    },
    [pscustomobject]@{
        Name = "SecretAchievements"
        Title = "Secret Achievements"
        Baseline = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\SecretAchievements\baseline_20260608\SecretAchievements_baseline.png"
        Dump = "$ProjectRoot\Saved\Codex\UI\FriendslopStyle\SecretAchievements\baseline_20260608\SecretAchievements_baseline_dump.json"
        Families = @(
            "AchievementTabs|Steam and Secret sub-tabs plus info icons; excludes shared frontend top bar.",
            "SecretSummaryPanel|Secret logo, summary panel shell, masked count, progress track/fill, and header containment.",
            "SecretList|Secret list shell, masked row surfaces, claim buttons, favorite buttons, reward icons, table dividers, and live row text/data slots."
        )
        Source = "Source/T66/UI/Screens/T66AchievementsScreen.cpp"
    }
)

foreach ($screen in $screens) {
    $screenDir = Join-Path $ProjectRoot "UI\FriendslopStyle\Screens\$($screen.Name)"
    $referenceDir = Join-Path $ProjectRoot "UI\FriendslopStyle\Reference\$($screen.Name)\Current"
    $savedDir = Join-Path $ProjectRoot "Saved\Codex\UI\FriendslopStyle\$($screen.Name)"
    $sourceDir = Join-Path $ProjectRoot "SourceAssets\UI\FriendslopStyle\$($screen.Name)"
    $runtimeDir = Join-Path $ProjectRoot "RuntimeDependencies\T66\UI\FriendslopStyle\$($screen.Name)"
    foreach ($dir in @($screenDir, $referenceDir, $savedDir, $sourceDir, $runtimeDir)) {
        New-Item -ItemType Directory -Force -Path $dir | Out-Null
    }

    $familyRows = ($screen.Families | ForEach-Object {
        $parts = $_.Split("|", 2)
        "| ``$($parts[0])`` | $($parts[1]) | Visual FAIL | Generate one family worker after textless crop is available. |"
    }) -join "`r`n"

    $familyNames = ($screen.Families | ForEach-Object { "  - " + $_.Split("|", 2)[0] }) -join "`r`n"

    Set-Content -LiteralPath (Join-Path $screenDir "README.md") -Encoding UTF8 -Value @"
# FriendslopStyle $($screen.Title)

This folder owns the FriendslopStyle pass artifacts for `$($screen.Name)`.

## Live Baseline

- Baseline capture: `$($screen.Baseline)`
- Baseline dump: `$($screen.Dump)`
- Source owner: `$($screen.Source)`

## Declared Visual Families

The shared frontend top bar is intentionally excluded from this screen's generation queue.

$familyRows
"@

    Set-Content -LiteralPath (Join-Path $screenDir "element_manifest.md") -Encoding UTF8 -Value @"
# FriendslopStyle $($screen.Title) Element Manifest

Reference visual: `UI/FriendslopStyle/Reference/$($screen.Name)/Current/${($screen.Name).ToLowerInvariant()}_friendslop_reference_20260608.png`

Fresh baseline capture: `$($screen.Baseline)`

Fresh baseline dump: `$($screen.Dump)`

## Visual Family Ledger

The shared frontend top bar is excluded from regeneration by user instruction.

| Visual family | Owned region/elements | Visual PASS/FAIL | Next action |
|---|---|---|---|
$familyRows

## Family Names

$familyNames
"@

    Set-Content -LiteralPath (Join-Path $screenDir "component_contract_current.md") -Encoding UTF8 -Value @"
# $($screen.Title) Component Contract

- Keep all live Slate text, data, icons, handlers, dropdown/toggle behavior, tooltip metadata, and screen routing owned by `$($screen.Source)`.
- Replace screen-local chrome surfaces with FriendslopStyle runtime plates generated for this pass.
- Do not regenerate or replace the shared frontend top bar.
- Do not bake labels, player data, scores, counts, row names, prices, or localized strings into runtime chrome.
"@

    Set-Content -LiteralPath (Join-Path $screenDir "slice_specs.md") -Encoding UTF8 -Value @"
# $($screen.Title) Slice Specs

Pending generated family assets from the 2026-06-08 Friendslop pass.

Default starting margins for generated assets:

| Asset class | Slice mode | Initial margin |
|---|---|---|
| Panel shell | 9-slice | 0.16, 0.20, 0.16, 0.20 |
| Tab/button | 9-slice | 0.18, 0.26, 0.18, 0.26 |
| Row strip | 9-slice | 0.10, 0.30, 0.10, 0.30 |
| Progress track | 9-slice | 0.12, 0.30, 0.12, 0.30 |
| Icon/fixed art well | image | 0 |
"@

    Set-Content -LiteralPath (Join-Path $screenDir "checklist.md") -Encoding UTF8 -Value @"
# $($screen.Title) Friendslop Checklist

- [ ] Reference generated from current baseline capture and Main Menu Friendslop reference.
- [ ] Textless reference generated by a fresh CLI worker.
- [ ] Family crops created for every declared non-top-bar family.
- [ ] One runtime element worker launched for every declared family.
- [ ] Generated runtime elements copied to SourceAssets and RuntimeDependencies.
- [ ] Runtime screen uses FriendslopStyle plates with live Slate text/data.
- [ ] Focused compile passed.
- [ ] Fresh final capture and dump created.
- [ ] reference_vs_current sheet created.
- [ ] previous_vs_current sheet created.
- [ ] Wiring/functionality gate recorded.
- [ ] Responsive evidence recorded or skipped with reason.
"@

    $passLog = Join-Path $savedDir "pass_log.md"
    Set-Content -LiteralPath $passLog -Encoding UTF8 -Value @"
# $($screen.Title) FriendslopStyle Pass Log

## Pass 2026-06-08

PPF CHECK
Objective: Convert `$($screen.Name)` to the FriendslopStyle screen lane while preserving live content and layout.
Proven process: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md plus UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md Step 0.5 through verification loop.
My planned implementation: Use the fresh baseline capture/dump, generate a full-screen Friendslop reference from the baseline and Main Menu reference, generate a textless reference, crop declared non-top-bar families, generate runtime elements per family with extraction-only prompts, wire the generated surfaces through FriendslopStyle helpers while preserving live Slate content and handlers, then compile and capture/dump evidence.
Same method class: YES
If NO, why:
User approval required before proceeding: NO
Verification evidence: baseline capture/dump, worker records, generated assets, compile log, final capture/dump/contact sheets, wiring gate.

ARTIFACT PARITY GATE
Reference artifact/category: Approved full-screen visual reference
Role: Primary
Required: YES
Planned artifact/path: UI/FriendslopStyle/Reference/$($screen.Name)/Current/${($screen.Name).ToLowerInvariant()}_friendslop_reference_20260608.png
Status: PENDING
Evidence: reference worker record

ARTIFACT PARITY GATE
Reference artifact/category: Fresh live capture and dump
Role: Primary
Required: YES
Planned artifact/path: `$($screen.Baseline)` and `$($screen.Dump)`
Status: SAME
Evidence: baseline capture produced on 2026-06-08

ARTIFACT PARITY GATE
Reference artifact/category: Runtime transparent/family plates
Role: Primary
Required: YES
Planned artifact/path: RuntimeDependencies/T66/UI/FriendslopStyle/$($screen.Name)/
Status: PENDING
Evidence: runtime family worker records and copied assets

MECHANISM MANIFEST
Reference/source: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md
Required mechanisms:
  1. Mechanism: Layout preservation
     Required: YES
     Planned implementation: Keep current baseline geometry and live source ownership.
     Evidence needed: final capture/dump and comparison sheets
  2. Mechanism: Screen-local Friendslop runtime surfaces
     Required: YES
     Planned implementation: Generate non-top-bar family elements and wire through FriendslopStyle runtime brushes.
     Evidence needed: generated assets, source paths, final capture
  3. Mechanism: Live content preservation
     Required: YES
     Planned implementation: Keep all labels/data/icons as Slate widgets over generated surfaces.
     Evidence needed: dump tags/text and source review
  4. Mechanism: State behavior preservation
     Required: YES
     Planned implementation: Preserve existing buttons, dropdowns, checkboxes, toggles, hover/selected/disabled states, and handlers.
     Evidence needed: dump interactivity and wiring gate
  5. Mechanism: Responsive behavior
     Required: YES
     Planned implementation: Preserve existing screen layout scaling and record captures where practical.
     Evidence needed: responsive captures or skipped reason

## Declared Families

$familyNames
"@
}

Write-Host "Initialized Friendslop pass folders for $($screens.Count) screens."
