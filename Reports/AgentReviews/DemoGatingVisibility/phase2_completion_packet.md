# Demo Gating Visibility — Phase 2 Completion Packet

Operator: Claude (FullOperator)
Validator: Codex
Date: 2026-05-30
Approval: `Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase2.md`

## Outcome

PASS (Operator work artifact — not a greenlight). Phase 2 is docs-only. Two
separate Markdown inventories were created under `Demo/`, anchored to current
source/config seams: one for demo-gated invisible content and one for deprecated
content. The deprecated inventory keeps feature gates separate from
compatibility-retained data fields, and includes centrally declared
arcade/minigame deprecated items as documentation-only entries per the user's
decision. No code, config, or assets were changed.

## Docs Changed

1. `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` (new)
   - Central seam table (config → settings class → gate subsystem → GameInstance
     wrappers → screen gate → overlay helper).
   - "Still Demo-Gated" entries with controlling seam + restore step: extra
     heroes, extra companions, non-Easy difficulties, Daily Descent, Lab run,
     Collector, extra casino games, and the "COMING SOON" presentation string.
   - "Moved to Available by Phase 1" section clearly marking drugs, diploma
     upgrades, and Steam/Secret achievements as no longer demo-gated, with re-gate
     seams.
2. `Demo/DEPRECATED_CONTENT.md` (new)
   - Feature-gate section for the central `UT66DeprecatedFeatureSettings`
     (arcade games, arcade interactables, minigames), documentation-only.
   - Cross-reference noting demo config's `AllowedArcadeGameIDs` is a
     compatibility/forward field, not a re-enable, because arcade is centrally
     deprecated.
   - Separate "Compatibility-Retained" section for deprecated-but-kept data
     fields exposed by source comments, so they are not confused with feature
     gates.
   - "Out of Scope / Pending" pointer to existing pending-issue cleanup items.

## Anchors Used

Config:
- `Config/DefaultDemoMode.ini` `[/Script/T66.T66DemoModeSettings]` — `AllowedHeroIDs`, `AllowedCompanionIDs`, `AllowedDifficultyIDs`, `AllowedArcadeGameIDs`, `AllowedCasinoGameIDs`, `bAllowLabRun=false`, `bAllowCollector=false`, `MaxDiplomaUpgradesPerStat=4`, `bAllowDrugPurchases=true`, `UnavailableContentText=COMING SOON`.
- `Config/DefaultGame.ini:64` `[/Script/T66.T66DeprecatedFeatureSettings]` — `bDisableArcadeGames=true`, `bDisableArcadeInteractables=true`, `bDisableMinigames=true`.

Core gate:
- `Source/T66/Core/T66ReleaseVariantSubsystem.h` — `UT66DemoModeSettings` properties; gate API.
- `Source/T66/Core/T66ReleaseVariantSubsystem.cpp` — `GetEffectiveReleaseVariant` (84), `IsSteamDemoBuild` (135), `IsHeroAllowed` (145), `IsCompanionAllowed` (161), `IsDifficultyAllowed` (177), `IsCasinoGameAllowed` (204), `IsRunCategoryAllowed` (220), `IsCollectorAllowed` (236), `GetPlayableDifficulties` (297), `GetVisibleDifficulties` (324), `IsDiplomaUpgradeAllowed` (345), `AreDrugPurchasesAllowed` (357), `GetUnavailableContentText` (368).
- `Source/T66/Core/T66GameInstance.cpp` — `IsHeroPlayable` (1035), `IsDifficultyPlayable` (1087), `IsRunCategoryPlayable` (1105), `IsCollectorPlayable` (1123), `IsCompanionPlayable` (1148).
- `Source/T66/Core/T66DeprecatedFeatureSettings.h` / `.cpp` — settings class + `T66DeprecatedFeatures` accessors.

UI consumers:
- `Source/T66/UI/T66UIManagerReleaseVariant.cpp` — `CanShowScreenForReleaseVariant` (39), `T66IsDeprecatedMinigameScreenType` (12), DailyDescent block (62).
- `Source/T66/UI/T66DemoModeUIUtils.h` / `.cpp` — `WrapWithComingSoonOverlay`, `GetUnavailableContentText`.
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp` — hero carousel overlay (445), drugs buy overlay (520), Lab button overlay (646), difficulty dropdown overlay (757).
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp` — `IsDailyDescentAvailable` (573), Daily Descent CTA overlay (1204).
- `Source/T66/UI/Screens/T66PowerUpScreen.cpp` — diploma graduate overlay (1969), drugs buy overlay (2426).
- `Source/T66/UI/Screens/T66AchievementsScreen.cpp` — `bDemoAchievementRowsLocked=false`; Steam (~1385) and Secret (~1765) overlay loops.

Compatibility-retained data fields:
- `Source/T66/Data/T66DataTypes.h` — deprecated per-level gain ranges (265), random gain range (713, 744), `T66IsDeprecatedSecondaryStatType` (926), legacy item rows (985, 991), legacy per-idol max level (1622).
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h:17`.
- `Source/T66/Public/UI/WidgetGames/T66WidgetGameDescriptor.h:16`, `T66WidgetGameRegistry.h:16`.

## Confirmation: No Code/Config/Asset Changes

No code, config, or asset files were intentionally edited in Phase 2. The only
files written are the two approved Markdown docs under `Demo/` and this
completion packet under `Reports/`. No Unreal binary asset folders were
inspected; no Git/LFS scans were run; only focused text searches over `Config`,
`Source/T66`, and `Demo`/`Reports` were used. No Mini/minigame runtime files
were inspected or changed (arcade/minigame entries are documentation-only).

## Token Ledger

- Claude token count: Unavailable (no helper manifest exposing a Claude token
  count was used for this run).

## Caveats

- Docs-only phase: no compile or capture was performed, per the approval. The
  inventories are anchored to current source/config but were not behavior-verified
  in a running build.
- Line numbers for `T66AchievementsScreen.cpp` overlay loops (~1385 Steam, ~1765
  Secret) are approximate and may drift; the stable anchor is the
  `bDemoAchievementRowsLocked` constant established in Phase 1.
- `GetVisibleDifficulties()` currently returns all difficulties unconditionally,
  so non-Easy difficulties are still listed (with a COMING SOON overlay) rather
  than hidden; full hiding is Phase 3 scope and is noted in the demo-gated doc.
- Daily Descent is hard-gated to demo mode (no dedicated demo config flag);
  restoring it requires a code change, not a config toggle — documented as such.
- This artifact is an Operator work product; Codex validates the actual files and
  authors the final user-facing report.
