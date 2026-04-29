# T66 Packaging Cleanup Tracker

Last updated: 2026-04-05

## Goal

Sanitize the runtime codebase before the packaged parity refactor so the standalone `.exe` can be shipped from a clean, deterministic runtime path.

This cleanup phase is intentionally behavior-preserving. It is not a redesign pass. The purpose is to remove ambiguity, legacy baggage, and editor/runtime mixing so later packaging fixes are mechanical and verifiable.

## Working Rules

- Preserve current visuals and gameplay behavior during cleanup.
- Treat non-editor runtime raw-disk file loading as a defect unless explicitly justified.
- Treat editor-only asset generation mixed into runtime orchestration as a defect.
- Centralize runtime asset ownership before broad asset migration.
- Build after each bounded remediation slice.
- Stop on unrelated compile failures and report them instead of folding them into the cleanup pass.

## Preflight

| Item | Purpose | Status |
| --- | --- | --- |
| Baseline screenshots/video/log capture | Preserve a visual and mechanical reference before behavior-changing work starts | Log baseline captured |
| Cleanup tracker | Single source of truth for pass scope, status, and findings | Complete |
| Pass 1 hotspot inventory | Identify highest-risk runtime files before extraction/remediation | Complete |

## Pass Tracker

| Pass | Scope | Primary Outputs | Exit Criteria | Status |
| --- | --- | --- | --- | --- |
| 1 | Hotspot inventory and runtime/editor boundary map | File inventory, defect-class map, initial priorities | Highest-risk runtime files identified and grouped by cleanup class | Complete |
| 2 | `T66GameMode` sanitation map | Extraction plan for lighting/theme/runtime orchestration seams | Deprecated or editor-only runtime lighting paths isolated and ownership mapped | Complete |
| 3 | Runtime/editor boundary cleanup | Separate legitimate editor helpers from shipping runtime logic | Shipping runtime path no longer depends on editor-only generation | Complete |
| 4 | Asset reference centralization design | Shared runtime asset catalog / path-owner layer | Scattered high-risk asset strings have a clear future owner | In progress |
| 5 | Raw file-system dependency quarantine | Explicit inventory of raw-disk runtime dependencies with owners | Every raw-disk runtime dependency is isolated, classified, or scheduled for removal | In progress |
| 6 | Runtime data determinism cleanup | CSV/DataTable override and loose data-path inventory | Import-time data flow separated from runtime data flow | In progress |
| 7 | Dynamic discovery inventory | Asset registry / folder-discovery risk map | Dynamic discovery systems have documented cook assumptions | Pending |
| 8 | Config and module hygiene | Packaging-relevant config and dependency audit | Stale packaging assumptions and accidental runtime/editor coupling documented | Pending |
| 9 | Logging and fallback cleanup | Missing-asset/missing-data observability plan | Runtime fallbacks are explicit and diagnosable | Pending |
| 10 | Cleanup guardrails | Audit script / grep rules and guideline update inputs | Cleanup regressions can be detected automatically | Pending |

## Defect Classes

Use these labels throughout the cleanup program.

- `god-file`
- `editor/runtime mixing`
- `stringly asset loads`
- `raw disk dependency`
- `runtime data override`
- `deprecated path`
- `dynamic discovery risk`
- `config risk`
- `staging ambiguity`
- `silent fallback`

## Pass 1 Results

### Summary

Pass 1 confirms that the packaging-risk surface is concentrated rather than random:

- The raw-disk runtime dependency surface is concentrated in 13 files.
- The runtime/editor boundary surface is concentrated in 6 files.
- `T66GameMode.cpp` is the dominant cleanup hotspot because it combines `god-file`, `editor/runtime mixing`, `stringly asset loads`, and `deprecated path` risk in one runtime-critical file.
- Broad `/Game/...` string loading exists across the program. Not all of it is wrong, but it is scattered enough that a central asset ownership layer is warranted before parity remediation begins.

### Highest-Priority Cleanup Hotspots

| File | Why It Is A Hotspot | Defect Classes | Pass Priority |
| --- | --- | --- | --- |
| `Source/T66/Gameplay/T66GameMode.cpp` | Runtime orchestration, shared lighting, theme/material loads, editor-only HDRI generation, many hardcoded asset strings | `god-file`, `editor/runtime mixing`, `stringly asset loads`, `deprecated path`, `silent fallback` | P0 |
| `Source/T66/Core/T66GameInstance.cpp` | Runtime CSV override path and large runtime asset preload surface | `runtime data override`, `stringly asset loads` | P0 |
| `Source/T66/Core/T66LeaderboardSubsystem.cpp` | Runtime CSV fallback for gameplay-facing data | `runtime data override`, `raw disk dependency` | P0 |
| `Source/T66/Gameplay/T66MiasmaBoundary.cpp` | World texture fallback from raw source file | `raw disk dependency`, `silent fallback` | P1 |
| `Source/T66/UI/Style/T66Style.cpp` | Shared UI generated-texture fallback and raw font path logic | `raw disk dependency`, `staging ambiguity`, `silent fallback` | P1 |
| `Source/T66/UI/Style/T66ButtonVisuals.cpp` | Main menu/button visual fallback to raw source textures | `raw disk dependency`, `silent fallback` | P1 |
| `Source/T66/UI/Screens/T66MainMenuScreen.cpp` | Main menu backgrounds and icons can prefer raw disk files | `raw disk dependency`, `silent fallback` | P1 |
| `Source/T66/UI/T66FrontendTopBarWidget.cpp` | Top bar icons and generated visuals can use loose files | `raw disk dependency`, `staging ambiguity` | P1 |
| `Source/T66/UI/T66GameplayHUDWidget.cpp` | Gameplay HUD imports icons/atlas from raw files | `raw disk dependency`, `silent fallback` | P1 |
| `Source/T66/Gameplay/T66PreviewMaterials.cpp` | Legitimate editor helper, but still a boundary file because it creates assets via C++ if missing | `editor/runtime mixing`, `deprecated path` | P2 |

### Raw-Disk Runtime Dependency Inventory

The following runtime files contain `ProjectDir`, `ProjectContentDir`, `ImportFileAsTexture2D`, or equivalent file-path-based loading:

#### Core

- `Source/T66/Core/T66GameInstance.cpp`
- `Source/T66/Core/T66LeaderboardSubsystem.cpp`
- `Source/T66/Core/T66MediaViewerSubsystem.cpp`
- `Source/T66/Core/T66WebView2Host.cpp`

#### Gameplay

- `Source/T66/Gameplay/T66MiasmaBoundary.cpp`

#### UI

- `Source/T66/UI/Dota/T66DotaSlate.cpp`
- `Source/T66/UI/Dota/T66DotaTheme.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/Screens/T66PowerUpScreen.cpp`
- `Source/T66/UI/Style/T66ButtonVisuals.cpp`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`

### Runtime/Editor Boundary Inventory

The following files contain `WITH_EDITOR` or `WITH_EDITORONLY_DATA` logic:

| File | Boundary Assessment | Initial Cleanup Decision |
| --- | --- | --- |
| `Source/T66/Gameplay/T66GameMode.cpp` | High risk. Editor-only HDRI cubemap generation is mixed directly into the live shared-lighting path used by runtime worlds. | Extract and sanitize early in Pass 2/3. |
| `Source/T66/Gameplay/T66PreviewMaterials.cpp` | Mostly legitimate editor fallback for preview assets, but it still represents runtime/editor boundary complexity. | Keep as a contained helper unless later parity work proves it leaks into shipping behavior. |
| `Source/T66/Gameplay/T66PreviewMaterials.h` | Commentary/documentation of editor-only fallback behavior. | No immediate code action. |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | Low risk. `WITH_EDITOR` use is limited to development actor labels. | Leave alone unless broader cleanup touches the file. |
| `Source/T66/Gameplay/T66LavaPatch.cpp` | Low risk. `WITH_EDITORONLY_DATA` use is limited to mip generation settings on generated textures. | Leave alone for now. |
| `Source/T66/Gameplay/T66MiasmaManager.cpp` | Low risk. `WITH_EDITORONLY_DATA` use is limited to mip generation settings on generated textures. | Leave alone for now. |

### Broad String Asset-Load Surface

The following files participate in broad `LoadObject`, `StaticLoadObject`, or `FSoftObjectPath` runtime loading and should be considered when the asset catalog is introduced:

#### Core candidates

- `Source/T66/Core/T66GameInstance.cpp`
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
- `Source/T66/Core/T66MusicSubsystem.cpp`
- `Source/T66/Core/T66PropSubsystem.cpp`
- `Source/T66/Core/T66RetroFXSubsystem.cpp`

#### Gameplay candidates

- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/Gameplay/T66MainMapTerrain.cpp`
- `Source/T66/Gameplay/T66MiasmaBoundary.cpp`
- `Source/T66/Gameplay/T66PreviewStageEnvironment.cpp`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
- `Source/T66/Gameplay/T66LavaPatch.cpp`

#### UI candidates

- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/Style/T66ButtonVisuals.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`

### Key Pass 1 Observations

1. `T66GameMode.cpp` is the first cleanup target because it is carrying a deprecated-looking lighting path and editor-only asset generation inside a live runtime world-setup function.
2. Most editor macros outside `T66GameMode.cpp` are not severe packaging defects by themselves. The real cleanup concern is not the presence of `WITH_EDITOR`; it is whether editor-only behavior changes the shipping runtime path.
3. The raw-disk dependency surface is dominated by UI/frontend code plus a smaller number of gameplay/data outliers.
4. `T66WebView2Host.cpp` and `T66MediaViewerSubsystem.cpp` need separate classification later because they may represent intentional third-party/runtime-local file use rather than cooked-content misuse.
5. String-based asset loads are widespread enough that introducing a central runtime asset owner layer should happen before broad parity remediation, otherwise the cleanup will remain fragmented.

## Pass 2 Results

### Summary

Pass 2 confirms that `T66GameMode.cpp` should be sanitized by extraction, not by wholesale rewrite.

- The file contains 106 `AT66GameMode::` methods and remains the single largest cleanup hotspot.
- Two responsibilities inside the file are especially parity-sensitive:
  - terrain/theme asset resolution near the top of the file,
  - shared world lighting plus development-world setup near the bottom.
- `AT66FrontendGameMode` directly calls `AT66GameMode::EnsureSharedLightingForWorld`, `ApplyThemeToDirectionalLightsForWorld`, and `ApplyThemeToAtmosphereAndLightingForWorld`, so any extraction must preserve those entry points or provide a drop-in wrapper.
- `TC_HDRI_Studio` currently behaves like a legacy path, not a healthy shipping dependency:
  - no matching asset was found in `Content`,
  - no active script was found to generate/import it,
  - the only non-code reference is legacy documentation describing a manual import flow.

### Function Region Map

| Region | Approx. Lines | Responsibility | Cleanup Assessment |
| --- | --- | --- | --- |
| Anonymous namespace helpers | 1-512 | Terrain/theme helpers, placement helpers, cached asset loads | Mixed. Some are harmless helpers, but terrain/theme asset loading should not live in `GameMode`. |
| Constructor and startup | 513-838 | Bootstraps state and sets up runtime flow for gameplay stages | Keep in `GameMode`; orchestration is appropriate here. |
| Stage/world spawning | 839-5090 | Spawn gates, vendors, props, boss flow, terrain prep, interactables | Large but mostly orchestration; not the first extraction target. |
| Main map terrain entry | 5090-5235 | Runtime terrain generation orchestration | Keep entry here, but remove duplicated terrain asset knowledge from `GameMode`. |
| HDRI cubemap generation | 5236-5392 | Editor-only asset authoring mixed into runtime file | High-priority sanitation target. |
| Shared lighting setup | 5394-5979 | Finds/spawns sky actors, configures sky light, fog, post process, applies theme | High-priority extraction target. |
| Runtime settings reaction | 5979-6011 | Re-applies theme and retro FX after settings changes | Keep behavior, but likely route through extracted lighting helper. |
| Player/lab/runtime tail | 6012-6959 | Player spawn, terrain safety, lab systems, hero spawning | Not a first cleanup slice. |

### External Coupling Map

| Caller | Dependency | Why It Matters |
| --- | --- | --- |
| `Source/T66/Gameplay/T66FrontendGameMode.cpp` | Calls `AT66GameMode::EnsureSharedLightingForWorld` | Frontend preview lighting shares the gameplay lighting path. Extraction must preserve this behavior. |
| `Source/T66/Gameplay/T66FrontendGameMode.cpp` | Calls `AT66GameMode::ApplyThemeToDirectionalLightsForWorld` | Theme application is shared across gameplay and frontend. |
| `Source/T66/Gameplay/T66FrontendGameMode.cpp` | Calls `AT66GameMode::ApplyThemeToAtmosphereAndLightingForWorld` | Atmosphere/skylight theme behavior is shared, not gameplay-only. |
| `AT66GameMode::HandleSettingsChanged` | Calls in-file lighting/theme methods | Settings propagation needs to remain stable after extraction. |
| `AT66GameMode::ScheduleGameplayLightingRefresh` | Re-applies theme after terrain/prop registration | Lighting refresh sequencing is part of the runtime path and must not be broken. |

### Hardcoded Asset-Load Clusters In `T66GameMode.cpp`

| Cluster | Approx. Lines | Examples | Future Owner |
| --- | --- | --- | --- |
| Difficulty terrain textures/materials | 370-431 | `/Game/World/Terrain/TowerDungeon/*`, `/Game/World/Terrain/TowerForest/*`, `/Game/Materials/M_Environment_Unlit` | `T66TerrainThemeAssets` or `T66MainMapTerrain` asset layer |
| Cliff side material set | 507-510 | `/Game/World/Cliffs/MI_HillTile1-4` | Shared world asset catalog |
| Start-area decor specs | 3722-3724 | `/Game/World/Props/StartAreaDecor/*` | Shared world asset catalog or stage props catalog |
| Beacon/fallback materials | 4685-4686 | `/Game/Materials/M_Environment_Unlit`, engine white texture | Shared world asset catalog |
| Legacy HDRI lighting assets | 5238-5240, 5658 | `/Game/Lighting/T_HDRI_Studio`, `/Game/Lighting/TC_HDRI_Studio` | Remove or replace during lighting sanitation |

### Sanitation Seams

These are the extraction boundaries that keep risk controlled.

1. `World lighting setup`
Move the logic currently centered around `EnsureSharedLightingForWorld`, `SpawnLightingIfNeeded`, `ConfigureGameplayFogForWorld`, `ApplyThemeToDirectionalLightsForWorld`, and `ApplyThemeToAtmosphereAndLightingForWorld` into a dedicated helper layer. Keep static wrapper methods in `AT66GameMode` initially so frontend and gameplay callers do not need a broad signature change.

2. `Development-only world actor provisioning`
Inside the lighting region, separate:
- actor discovery/spawn for atmosphere, sun, moon, sky light, fog, and post process,
- theme application,
- any editor-only asset authoring.

3. `Terrain theme asset resolution`
Move the difficulty-theme texture/material helpers out of the anonymous namespace in `T66GameMode.cpp`. These belong with terrain/theme asset ownership, not game mode orchestration.

4. `Shared world asset ownership`
Cliff materials, decor mesh paths, beacon materials, and terrain/theme assets should converge on one shared runtime asset owner layer instead of remaining embedded in `GameMode`.

### `TC_HDRI_Studio` Verdict

Current assessment: treat `TC_HDRI_Studio` as a legacy/deprecated runtime path until proven otherwise.

Evidence:

- `Source/T66/Gameplay/T66GameMode.cpp` is the only live code using it.
- No `TC_HDRI_Studio` or `T_HDRI_Studio` asset exists under `Content`.
- No active HDRI import/generation script was found under `Scripts`.
- The only non-code reference is legacy documentation describing a manual workflow to create it.

Cleanup consequence:

- The editor-only cubemap generation path should not survive as part of a healthy shipping lighting pipeline.
- Pass 3 should remove or replace this dependency instead of preserving it as a hidden fallback.

### Pass 3 Target

Pass 3 should be the first behavioral cleanup slice:

- isolate shared lighting code behind a helper,
- remove editor-only cubemap authoring from the live runtime path,
- keep `AT66GameMode` and `AT66FrontendGameMode` behavior stable through wrapper methods,
- and leave the broader asset-catalog work for the following pass.

### Pass 3 Progress

Completed work:

- Added `Source/T66/Gameplay/T66WorldLightingSetup.h`
- Added `Source/T66/Gameplay/T66WorldLightingSetup.cpp`
- Converted the shared lighting, fog, and atmosphere/theme methods in `Source/T66/Gameplay/T66GameMode.cpp` into wrappers around the new helper layer
- Removed the editor-only HDRI cubemap authoring logic from the live `T66GameMode.cpp` runtime path

## Preflight Log Baseline

The user opted to use logs instead of screenshots/video for the cleanup baseline.

Observed from `Saved/Logs/T66.log` during the current frontend run:

- `LogT66FrontendTopBar` confirms the frontend top bar is still loading generated visuals from loose files under `Content/SourceAssets/UI/Dota/Generated`, with repeated `LoadedFrom=file` results.
- `LogSlate` confirms runtime fonts are still being loaded from raw source files:
  - `SourceAssets/Reaver-Bold.woff`
  - `SourceAssets/radiance.ttf`
- `LogStreaming` reports missing companion animation package loads under `/Game/Characters/Companions/...`. This is not part of the current cleanup slice, but it is now a tracked packaged-parity risk for later passes.

Cleanup consequence:

- The baseline already captures enough evidence to proceed with the sanitation program without screenshots.
- No additional user-driven capture is required before Pass 6.

## Pass 6 Audit Notes

Current runtime data determinism risks confirmed from code:

- `Source/T66/Core/T66GameInstance.cpp` still hot-overrides cooked DataTables from raw CSV files at runtime:
  - `Content/Data/Idols.csv`
  - `Content/Data/Bosses.csv`
  - `Content/Data/Stages.csv`
- The override helper uses `ProjectContentDir()` plus `FFileHelper::LoadFileToString`, so packaged parity depends on loose files being present.
- `Source/T66/Core/T66LeaderboardSubsystem.cpp` explicitly treats CSVs as the live runtime source of truth:
  - `Initialize()` calls `LoadTargetsFromCsv()` first
  - DataTables are only used as a fallback if CSV population leaves the target maps empty
- `Source/T66/Core/T66LeaderboardSubsystem.h` still points `ScoreTargetsDTPath` at `/Game/Data/DT_Leaderboard_ScoreTargets.DT_Leaderboard_ScoreTargets`, but the actual cooked asset in `Content/Data` is `Leaderboard_ScoreTargets.uasset`.
- `SpeedRunTargetsDTPath` matches a real asset: `DT_Leaderboard_SpeedrunTargets.uasset`.

Cleanup consequence:

- Data determinism cannot be fixed by a single search/replace.
- `GameInstance` and `LeaderboardSubsystem` currently disagree about what the runtime authority should be:
  - cooked DataTables with CSV hot-override for some gameplay tables,
  - CSV-first runtime loading for leaderboard targets.
- The next bounded cleanup slice should make runtime ownership explicit before any parity migration changes the source of truth.

### Pass 6 Progress

Completed sanitation in this slice:

- Added `Source/T66/Core/T66LegacyRuntimeDataAccess.h`
- Added `Source/T66/Core/T66LegacyRuntimeDataAccess.cpp`
- Moved legacy loose `Content/Data/*.csv` access behind a shared helper layer so the raw-disk dependency is explicit and centralized
- Added `LogT66RuntimeData` logging for:
  - loose CSV override attempts in `Source/T66/Core/T66GameInstance.cpp`
  - leaderboard target source selection in `Source/T66/Core/T66LeaderboardSubsystem.cpp`
  - missing loose CSV or cooked DataTable fallback cases
- Replaced `LogTemp` CSV-override warnings in `Source/T66/Core/T66GameInstance.cpp` with `LogT66RuntimeData`
- Fixed the broken leaderboard score target DataTable path in `Source/T66/Core/T66LeaderboardSubsystem.h`:
  - from `/Game/Data/DT_Leaderboard_ScoreTargets.DT_Leaderboard_ScoreTargets`
  - to `/Game/Data/Leaderboard_ScoreTargets.Leaderboard_ScoreTargets`
- Added explicit source tracking in `UT66LeaderboardSubsystem`:
  - `LooseCsv`
  - `CookedDataTable`
  - `None`

What this cleanup slice intentionally did not do:

- It did not flip gameplay tables or leaderboard targets from loose CSV to cooked DataTables yet.
- It did not remove the `GameInstance` CSV override behavior.
- It did not change the leaderboard subsystem's current CSV-first behavior.

Why:

- This cleanup phase is still behavior-preserving.
- The current repo state indicates runtime data authority is inconsistent, so the first safe step is to centralize and log the live source before changing precedence.

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

## Pass 5 Progress

Completed sanitation in this slice:

- Added `Source/T66/UI/Style/T66LegacyUIFontAccess.h`
- Added `Source/T66/UI/Style/T66LegacyUIFontAccess.cpp`
- Moved duplicated legacy Slate font path resolution out of:
  - `Source/T66/UI/Style/T66Style.cpp`
  - `Source/T66/UI/Dota/T66DotaTheme.cpp`
- Centralized the shared fallback chain for:
  - `SourceAssets/radiance.ttf`
  - `SourceAssets/Reaver-Bold.woff`
  - `Content/Slate/Fonts/...`
- Centralized the shared bold-weight detection logic used by both style layers
- Centralized the shared file-backed `FSlateFontInfo` creation path used by both style layers

What this cleanup slice intentionally did not do:

- It did not change the current font-source precedence.
- It did not migrate fonts to cooked `/Game` assets yet.
- It did not remove the file-backed Slate font dependency from runtime.

Why:

- This is still cleanup, not the parity-migration pass.
- The immediate goal is to give the raw font dependency a single owner so the later cooked-font migration is mechanical instead of duplicated.

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Additional sanitation completed in later slices:

- Added `Source/T66/UI/Style/T66LegacyUIBrushAccess.h`
- Added `Source/T66/UI/Style/T66LegacyUIBrushAccess.cpp`
- Moved the duplicated optional imported-texture / loose-file-fallback / nine-slice brush logic out of:
  - `Source/T66/UI/Style/T66Style.cpp`
  - `Source/T66/UI/Dota/T66DotaSlate.cpp`
- Centralized the shared Dota generated-source directory ownership and optional brush logging under `LogT66RuntimeUI`
- Added `Source/T66/UI/Style/T66LegacyUITextureAccess.h`
- Added `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp`
- Moved loose UI texture import/configuration into a shared helper and rewired:
  - `Source/T66/UI/Style/T66ButtonVisuals.cpp`
  - `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - `Source/T66/UI/Screens/T66PowerUpScreen.cpp`
  - `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- Removed direct `FImageUtils::ImportFileAsTexture2D(...)` configuration duplication from those files
- Converted the main-menu animated background layers from direct file-path `FSlateImageBrush` ownership to owned runtime textures:
  - `SkyBackgroundBrush`
  - `FireMoonBrush`
  - `PyramidChadBrush`
- Added `Source/T66/Core/T66LegacyRuntimeTextureAccess.h`
- Added `Source/T66/Core/T66LegacyRuntimeTextureAccess.cpp`
- Moved the gameplay-side loose wall texture import out of `Source/T66/Gameplay/T66MiasmaBoundary.cpp` into the shared runtime texture helper
- Added `LogT66RuntimeAssets` for gameplay-side loose texture fallback observability
- Moved the frontend top bar's custom loose image decode / mip generation / cooked asset load path behind `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp`
- Simplified `Source/T66/UI/T66FrontendTopBarWidget.cpp` so it now owns:
  - top-bar-specific cache policy
  - top-bar-specific brush wiring and diagnostics
  - but no longer owns the raw loose-image import implementation itself

What these cleanup slices intentionally did not do:

- They did not change the loose-vs-cooked precedence yet.
- They did not remove all remaining raw path-based UI loading; they centralized the most duplicated pieces first.

Build gate after these slices:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

## Exception Classification Notes

These remaining runtime file-path users are not the same class of defect as the loose source-art/UI paths:

- `Source/T66/Core/T66MediaViewerSubsystem.cpp`
  - Classification: intentional runtime-local data path
  - Why: it stores persistent WebView2 profile data under `Saved/TikTokWebView2/UserData` and deliberately keeps that state inside the game folder.
  - Cleanup decision: keep, but treat as an explicit runtime-local storage exception rather than cooked-content loading.

- `Source/T66/Core/T66WebView2Host.cpp`
  - Classification: runtime platform dependency / loader bootstrap
  - Why: it resolves `WebView2Loader.dll` from next to the executable first, then from the bundled third-party fallback path.
  - Cleanup decision: keep, but document it as a runtime binary dependency rather than a cooked-content loading path.

Resolved blocker:

- The unrelated unity-build collision between `Source/T66/Gameplay/T66LavaPatch.cpp` and `Source/T66/Gameplay/T66MiasmaManager.cpp` was fixed by introducing `Source/T66/Gameplay/T66LavaShared.h` and deduplicating the shared lava helper symbols there.

Build gate:

- `T66Editor Win64 Development` succeeded after the fix.
- `T66 Win64 Development` succeeded after the fix.

Exit assessment:

- Pass 3 is complete for the lighting/runtime-editor boundary slice.
- Shared lighting no longer depends on editor-only HDRI cubemap authoring in the live runtime path.

## Pass 4 Progress

Completed work:

- Added `Source/T66/Gameplay/T66TerrainThemeAssets.h`
- Added `Source/T66/Gameplay/T66TerrainThemeAssets.cpp`
- Moved `T66GameMode` difficulty terrain material/texture resolution behind `FT66TerrainThemeAssets`
- Moved default cliff-side material path ownership out of `T66GameMode` and into the same helper
- Rebuilt both `T66Editor Win64 Development` and `T66 Win64 Development` successfully

Current assessment:

- `T66GameMode.cpp` no longer owns the terrain-theme material resolution logic directly.
- `T66MainMapTerrain.cpp` still contains overlapping terrain asset knowledge, so Pass 4 is not complete yet; the ownership story is improved but not unified.

## Next Pass

Pass 4 should continue with the remaining asset-ownership cleanup:

- evaluate whether `T66MainMapTerrain.cpp` should consume `FT66TerrainThemeAssets` or a broader world asset catalog,
- identify the next shared asset clusters that still belong to a future central owner,
- and then move into Pass 6 runtime data determinism audit/remediation for `T66GameInstance` and `T66LeaderboardSubsystem`.

## Pass 6 Pre-Audit Notes

The next high-risk cleanup area is runtime data determinism.

Current findings:

- `Source/T66/Core/T66GameInstance.cpp` actively overrides cooked DataTables from raw CSV files at runtime for:
  - `Content/Data/Idols.csv`
  - `Content/Data/Bosses.csv`
  - `Content/Data/Stages.csv`
- `Source/T66/Core/T66LeaderboardSubsystem.cpp` currently loads CSV first in `Initialize()` and only falls back to DataTables if CSV population fails.
- `Source/T66/Core/T66LeaderboardSubsystem.h` points `ScoreTargetsDTPath` at `/Game/Data/DT_Leaderboard_ScoreTargets`, but the cooked asset currently present on disk is `Content/Data/Leaderboard_ScoreTargets.uasset`.

Cleanup consequence:

- Fixing this area is required for cooked determinism, but it can change gameplay behavior if the CSV and cooked DataTable assets are out of sync.
- Before removing the runtime CSV-first behavior, baseline validation is recommended so any mechanical differences can be attributed cleanly.

## Frontend Stability Follow-Up

New issue found during manual frontend exploration:

- Opening the account history hero filter dropdown could crash in `UT66AccountStatusScreen::BuildSlateUI()` from deferred Slate callbacks using stack-local captures after the screen build returned.

Fix applied:

- Updated `Source/T66/UI/Screens/T66AccountStatusScreen.cpp` so the history filter dropdown callbacks and their live value-text callbacks carry explicit value captures instead of `[&]`-capturing stack-local lambdas and arrays.
- The hero filter menu now captures a copy of `HistoryHeroFilterEntries`.
- The difficulty and status filter menus now capture stable copies of the helper callbacks they depend on.
- The reviewed-run button callback now resolves the leaderboard subsystem through a weak pointer instead of closing over a raw local subsystem pointer.

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Assessment:

- This was a real frontend lifetime bug, not a packaging-only issue.
- The account history filter row is now safe against the specific deferred-callback use-after-free that caused the crash.

## Log-Driven Cleanup Batch

Timestamp:

- `2026-04-05 09:26:46 -03:00`

Completed work:

- Removed live runtime CSV authority from `Source/T66/Core/T66GameInstance.cpp` for:
  - `Content/Data/Idols.csv`
  - `Content/Data/Bosses.csv`
  - `Content/Data/Stages.csv`
- Removed CSV-first leaderboard target loading from `Source/T66/Core/T66LeaderboardSubsystem.cpp`
- Kept leaderboard target loading on cooked DataTables only and fixed the score-target asset path to the actual cooked asset
- Staged the remaining required raw runtime files under `Content/...` and expanded `Config/DefaultGame.ini` non-UFS staging for:
  - `Content/Slate/Fonts`
  - `Content/SourceAssets/UI/MainMenuGenerated`
  - `Content/SourceAssets/UI/MainMenu/Generated`
  - `Content/SourceAssets/UI/MainMenuBlueTrim`
  - `Content/SourceAssets/UI/MainMenuBlueWoodFill`
  - `Content/SourceAssets/UI/MainMenuArcaneFill`
  - `Content/SourceAssets/UI/PowerUp/Statues/Generated`
  - `Content/SourceAssets/UI/Currency`
  - `Content/SourceAssets/UI/ChestRewards`
  - `Content/SourceAssets/Shikashi's Fantasy Icons Pack v2`
  - `Content/SourceAssets/World`
- Removed runtime UI font fallback to project-root `SourceAssets` and pointed legacy font resolution at staged `Content/Slate/Fonts`
- Switched main menu backgrounds to cooked-first loading and moved generated icon / CTA-fill loose fallback to staged `Content/...` paths
- Switched top bar plate / slot / glow / icon loading to cooked-first resolution with staged-file fallback
- Moved PowerUp statue fallback loading off `ProjectDir()` and onto cooked-first plus staged-content fallback
- Moved gameplay HUD loose runtime texture lookup off `ProjectDir()` and onto staged `Content/...` files
- Added cooked fallback for chest item sprites in `Source/T66/UI/T66GameplayHUDWidget.cpp` using existing `/Game/Items/Sprites/*` assets
- Moved `Source/T66/Gameplay/T66MiasmaBoundary.cpp` loose wall texture fallback off `ProjectDir()` and onto `Content/SourceAssets/World/OuterWallTexture.png`
- Guarded optional sound-class and eclipse material loads so missing deprecated/nonexistent assets no longer trigger invalid package load attempts
- Added character-visual row fallback logic so missing skin-specific rows (for example `Hero_1_TypeA_Beachgoer`) now fall back to the default row instead of erroring immediately
- Reframed `FT66WorldLightingSetup` fallback actor creation as intentional runtime bootstrap instead of “development” fallback, and reduced the related log noise

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Remaining intentional exceptions:

- `Source/T66/Core/T66MediaViewerSubsystem.cpp`
  - runtime-local storage under `Saved/...`
- `Source/T66/Core/T66WebView2Host.cpp`
  - runtime binary/bootstrap resolution for `WebView2Loader.dll`

Exit assessment:

- The cleanup phase is now code-complete enough for another runtime/log verification pass.
- Remaining unknowns are runtime-observed only; they need a fresh editor or standalone run against the new build.

## Staging Correction

Issue found:

- Copying loose fallback PNG / JSON / font files into `Content/...` caused the editor to detect them as import candidates on restart.
- This produced a bad cleanup regression:
  - auto-import prompts
  - attempted asset creation from files that were supposed to remain loose
  - direct editor pollution for runtime-only fallback files

Correction applied:

- Removed the copied loose fallback files from `Content/...`
- Removed the temporary `DirectoriesToAlwaysStageAsNonUFS` additions from `Config/DefaultGame.ini`
- Restored the affected fallback loaders to read from the real project-root `SourceAssets/...` locations instead of fake `Content/...` mirrors
- Added explicit `RuntimeDependencies` staging in `Source/T66/T66.Build.cs` for the remaining loose fallback files that still need to ship with the packaged build:
  - `SourceAssets/radiance.ttf`
  - `SourceAssets/Reaver-Bold.woff`
  - `SourceAssets/UI/MainMenuGenerated/...`
  - `SourceAssets/UI/MainMenu/Generated/...`
  - `SourceAssets/UI/MainMenuBlueTrim/...`
  - `SourceAssets/UI/MainMenuBlueWoodFill/...`
  - `SourceAssets/Archive/UI/MainMenuArcaneFill/...`
  - `SourceAssets/UI/PowerUp/Statues/Generated/...`
  - `SourceAssets/UI/Currency/...`
  - `SourceAssets/UI/ChestRewards/...`
  - `SourceAssets/Shikashi's Fantasy Icons Pack v2/...`
  - `SourceAssets/Archive/OuterWallTexture.png`
- Removed the stray imported assets created by the bad staging attempt:
  - `Content/Slate/Fonts/radiance.uasset`
  - `Content/Slate/Fonts/radiance_Font.uasset`
  - imported Shikashi icon asset(s)

Build gate after correction:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Rule learned:

- Loose runtime fallback files must not be mirrored into `Content/...` just to get them staged.
- If a file must remain loose, stage it explicitly from its real source location.

## Cleanup Pass: Runtime Log Noise + Media/Animation Hygiene

Log-driven cleanup applied after the fresh editor + tribulation verification run:

- Moved the remaining raw font fallback files into `Content/Slate/Fonts/T66/` and changed `Source/T66/UI/Style/T66LegacyUIFontAccess.cpp` to prefer those project-content Slate font files before the legacy `SourceAssets/...` path.
- Moved the remaining raw UI fallback art used by the current runtime into `Content/Slate/RuntimeUI/...`:
  - main menu CTA fill
  - HUD currency icons
  - chest reward open / closed icons
  - minimap atlas
  - PowerUp statue base images
- Added `Content/Slate/Fonts/T66/...` and `Content/Slate/RuntimeUI/...` to `RuntimeDependencies` in `Source/T66/T66.Build.cs` so the new raw-content fallback path is explicitly staged.
- Added legacy-source-to-`Content/Slate` remapping in `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp` and rewired:
  - `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
  - `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - `Source/T66/UI/Screens/T66PowerUpScreen.cpp`
  so they now prefer the project-content Slate fallback before falling back to the raw source tree.
- Fixed companion run-animation preload/path normalization in:
  - `Source/T66/Core/T66GameInstance.cpp`
  - `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
  by correcting legacy `_Anim` package/object name mismatches to the actual imported asset names.
- Removed the repeated `KnightClip` media-source validation error path by stopping the runtime from loading the stale `UFileMediaSource` asset when no actual movie file exists, and by building the preview source directly from a real movie file only if one exists:
  - `Source/T66/Core/T66GameInstance.cpp`
  - `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
  - `Source/T66/UI/Screens/T66HeroSelectionScreen.h`
- Downgraded the noisy diagnostic logging that was polluting normal runtime logs:
  - `Source/T66/Core/T66LagTrackerSubsystem.cpp`
  - `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
  - successful runtime UI cooked/loose texture loads in `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp`

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Expected outcome for next runtime verification:

- no `KnightClip` media validation spam when opening hero selection
- no companion `SkipPackage` spam for missing `_Running_Anim` packages
- no default-log spam from lag tracker / idol cache / mesh-alignment diagnostics
- loose fallback art, if still used, should resolve from `Content/Slate/...` instead of `SourceAssets/...`

## Cleanup Pass: Non-Content Staged Runtime Fallbacks + Residual Log Hygiene

Runtime verification showed that putting raw fallback files under `Content/Slate/...` was still wrong:

- the editor auto-imported those raw files into `.uasset`s
- that polluted the content tree and created another packaging ambiguity

Correction applied:

- Removed the bad raw fallback directories from `Content/Slate/...`:
  - `Content/Slate/Fonts/T66`
  - `Content/Slate/RuntimeUI`
- Created a dedicated non-content staged runtime directory:
  - `RuntimeDependencies/T66/Fonts/...`
  - `RuntimeDependencies/T66/UI/...`
- Copied the currently required loose fallback files into that runtime-only staging area:
  - `radiance.ttf`
  - `Reaver-Bold.woff`
  - main menu generated icons / CTA fill
  - main menu blue trim / blue wood / arcane fill
  - currency icons
  - chest reward icons
  - PowerUp statue generated art
  - minimap atlas
- Updated `Source/T66/T66.Build.cs` to stage the new runtime-only directory instead of the old `Content/Slate/...` mirror.
- Updated `Source/T66/UI/Style/T66LegacyUIFontAccess.cpp` to resolve staged loose fonts from `RuntimeDependencies/T66/Fonts/...` before falling back to `SourceAssets/...`.
- Updated `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp` to remap legacy loose texture paths into `RuntimeDependencies/T66/UI/...`, including:
  - `SourceAssets/UI/MainMenuGenerated/...`
  - `SourceAssets/UI/MainMenu/Generated/...`
  - `SourceAssets/UI/MainMenuBlueTrim/...`
  - `SourceAssets/UI/MainMenuBlueWoodFill/...`
  - `SourceAssets/Archive/UI/MainMenuArcaneFill/...`
  - `SourceAssets/UI/Currency/...`
  - `SourceAssets/UI/ChestRewards/...`
  - `SourceAssets/UI/PowerUp/Statues/Generated/...`
  - `SourceAssets/Shikashi's Fantasy Icons Pack v2/...`

Residual cleanup applied in the same pass:

- Fixed the gambler black-jack card-back soft path in `Source/T66/UI/T66GamblerOverlayWidget.cpp` to use the existing cooked `BJ/card_back` asset instead of the missing `/Game/UI/Sprites/Games/T_Game_CardBack`.
- Downgraded remaining noisy warning-level logs to `Verbose` in:
  - `Source/T66/UI/T66FrontendTopBarWidget.cpp`
  - `Source/T66/Gameplay/T66GameMode.cpp`
  - `Source/T66/Gameplay/T66PlayerController_Input.cpp`
  - `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
- Added broader glyph coverage for the language list itself by rendering language names and title text with the engine default font in `Source/T66/UI/Screens/T66LanguageSelectScreen.cpp`.

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Expected outcome for next runtime verification:

- no more raw fallback imports from `Content/Slate/T66` or `Content/Slate/RuntimeUI`
- no missing `/Game/UI/Sprites/Games/T_Game_CardBack` package warning
- no warning-level `[TopBarBrushLoad]`, `[LAG] Frame`, `[JUMP]`, or benign character-visual fallback spam during a normal run

## Cleanup Pass: Dedicated Lighting Extraction

To keep lighting work from flowing back into `GameMode`, the public runtime lighting ownership was moved into a dedicated world-scoped subsystem:

- Added `Source/T66/Gameplay/T66LightingSubsystem.h`
- Added `Source/T66/Gameplay/T66LightingSubsystem.cpp`

Design:

- `UT66LightingSubsystem` is the world-scoped entry point for runtime lighting orchestration.
- `FT66WorldLightingSetup` remains the low-level implementation layer for the actual actor/component lighting setup.
- `AT66GameMode` is no longer the lighting owner; it now delegates its instance lighting calls into the subsystem.
- Frontend now also uses the same subsystem directly, so gameplay and frontend share one runtime lighting entry path outside `GameMode`.

Packaging / cleanup benefit:

- lighting ownership is now explicit and world-scoped instead of buried in the gameplay god file
- frontend and gameplay now go through the same runtime lighting subsystem path
- future lighting features can be added without bloating `AT66GameMode`
- the lighting entry point is now easier to audit for PIE vs packaged parity

Code changes:

- Added `UT66LightingSubsystem` static world helpers:
  - `EnsureSharedLightingForWorld`
  - `ConfigureGameplayFogForWorld`
  - `ApplyThemeToDirectionalLightsForWorld`
  - `ApplyThemeToAtmosphereAndLightingForWorld`
- Scoped subsystem creation to game worlds via `ShouldCreateSubsystem`
- Rewired `Source/T66/Gameplay/T66GameMode.cpp` instance lighting methods to use `UT66LightingSubsystem`
- Removed the old static lighting doorway declarations from `Source/T66/Gameplay/T66GameMode.h`
- Rewired `Source/T66/Gameplay/T66FrontendGameMode.cpp` to use `UT66LightingSubsystem` directly

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Next verification target:

- confirm frontend and tribulation lighting still behave identically after the subsystem extraction
- verify no new lighting-specific warnings appear in `Saved/Logs/T66.log`

## Cleanup Pass: Combat VFX Extraction + Shared Dota Plate Brushes

Two architecture cleanup concerns were addressed in this pass to keep packaging work moving toward cleaner ownership boundaries instead of adding more logic to god files.

Combat VFX extraction:

- Added `Source/T66/Gameplay/T66CombatVFX.cpp`
- Moved the VFX-specific implementation out of `Source/T66/Gameplay/T66CombatComponent.cpp`, including:
  - Niagara/effect asset loading and caching helpers
  - pixel-particle spawn helpers
  - death / slash / pierce / bounce / DOT visual implementations
  - idol proc visual routing
  - hero-specific attack VFX variants
  - VFX warmup and active-system selection
- Kept `UT66CombatComponent` as the combat logic/orchestration owner and left the public API intact.
- Promoted `LogT66Combat` to a shared log category via `Source/T66/Gameplay/T66CombatComponent.h` so the new VFX translation unit can log through the same category.

Packaging / cleanup benefit:

- combat logic and combat visuals are no longer interleaved in the same large implementation block
- future VFX work can be isolated without re-bloating `UT66CombatComponent`
- packaged-asset audit for combat visuals now has one dedicated implementation file

Shared Dota plate brush dedup:

- Added `T66LegacyUIBrushAccess::ET66DotaPlateBrushKind`
- Added `T66LegacyUIBrushAccess::ResolveDotaButtonPlateBrush(...)`
- Removed the duplicated Dota button plate asset/fallback/brush state logic from:
  - `Source/T66/UI/Style/T66Style.cpp`
  - `Source/T66/UI/Dota/T66DotaSlate.cpp`
- Both style layers now resolve Dota button plates through the single shared helper in:
  - `Source/T66/UI/Style/T66LegacyUIBrushAccess.h`
  - `Source/T66/UI/Style/T66LegacyUIBrushAccess.cpp`

Packaging / cleanup benefit:

- the cooked asset path and loose fallback filename for Dota plates now live in one place
- frontend/shared UI no longer risk drifting because of copy-pasted brush ownership logic

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Next verification target:

- confirm combat attack VFX still fire normally after the translation-unit split
- confirm no new combat-VFX or Dota button-plate warnings appear in `Saved/Logs/T66.log`

## Cleanup Pass: Save/Load Resume Repair + Preview Removal

This pass removed the broken save-preview path and repaired the real save-resume path that was dropping players back into a fresh stage-one run.

Save/load fix:

- Root cause found in `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`:
  - `OnSaveAndQuitClicked()` wrote `StageReached` and metadata but never exported `RunSnapshot`
  - this allowed save cards to display a later stage while `Load Game` still resumed as a fresh run
- Fixed by exporting the full runtime snapshot during save-and-quit:
  - `LocalRunState->ExportSavedRunSnapshot(SaveObj->RunSnapshot);`

Preview removal:

- Removed the save-slot preview button and preview handlers from:
  - `Source/T66/UI/Screens/T66SaveSlotsScreen.h`
  - `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
- Removed the preview-only gameplay branch from:
  - `Source/T66/Gameplay/T66GameMode.cpp`
  - `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - `Source/T66/Core/T66GameInstance.h`
  - `Source/T66/Core/T66SessionSubsystem.cpp`
  - `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
  - `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- Deleted the now-dead preview overlay files:
  - `Source/T66/UI/T66LoadPreviewOverlayWidget.h`
  - `Source/T66/UI/T66LoadPreviewOverlayWidget.cpp`
- Removed the stale `AT66PlayerController` preview-overlay API and theme class plumbing.

Disk-cleanup follow-up:

- Verified `Source/T66/UI/Dota/T66DotaSlate.*` had no remaining references.
- Deleted:
  - `Source/T66/UI/Dota/T66DotaSlate.h`
  - `Source/T66/UI/Dota/T66DotaSlate.cpp`

Log cleanup folded into the pass:

- `GetIdolData(None)` noise is reduced by:
  - guarding `GetIdolData()` against `NAME_None`
  - skipping empty idol IDs in `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
- `Trickster` no longer probes `DT_HouseNPCs` for a missing row in `Source/T66/Gameplay/T66HouseNPCBase.cpp`

Packaging / cleanup benefit:

- only one gameplay resume path remains for saved runs
- packaged parity is easier to verify because the frozen-preview variant no longer exists
- dead frontend/theme files are removed from disk instead of lingering as legacy code

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Next verification target:

- save in-stage, quit, and confirm `Load Game` restores the saved stage instead of stage one
- confirm no save-slot preview UI is still exposed anywhere
- confirm `Saved/Logs/T66.log` no longer shows `GetIdolData(None)` or `GetHouseNPCData(Trickster)` warnings

## Cleanup Pass: Media Viewer Stabilization

This pass addressed a runtime stability issue in the TikTok / YouTube Shorts viewer path. The latest log showed the HUD repeatedly logging `Viewer CLOSED` during normal refresh and a WebView2 path that was doing too much work while hidden.

Issues found:

- `Source/T66/UI/T66GameplayHUDWidget.cpp` was calling `UpdateTikTokVisibility()` from the regular HUD refresh loop, which meant:
  - viewer open/closed logging could fire repeatedly during normal HUD refresh
  - open-state sync logic could be re-armed even when the viewer state had not changed
- `Source/T66/Core/T66MediaViewerSubsystem.cpp` prewarm was navigating the live TikTok/Shorts page while hidden, which increased background browser work and memory pressure before the player even opened the viewer
- `Source/T66/Core/T66WebView2Host.cpp` treated repeated overlay re-alignment like a fresh visibility transition, so re-positioning could re-run resume/play scripts unnecessarily

Fixes:

- Added a small toggle debounce to `UT66MediaViewerSubsystem` so repeated input can’t thrash the viewer state:
  - `ToggleDebounceSeconds`
  - `LastToggleRealtimeSeconds`
- Changed `PrewarmTikTok()` to prewarm the WebView2 host/session only, without navigating the live feed while hidden
- Made HUD media-viewer visibility application idempotent:
  - `UpdateTikTokVisibility(bool bForce = false)`
  - `bHasAppliedMediaViewerOpenState`
  - `bLastAppliedMediaViewerOpenState`
- Downgraded the HUD viewer-state log from `Log` to `Verbose`
- Added explicit visible-state tracking in `FT66WebView2Host`:
  - `bIsShowing`
- Reworked `ShowAtScreenRect()` so re-positioning an already-visible overlay no longer re-runs the resume/play script path

Packaging / cleanup benefit:

- runtime log churn from the media viewer is now controlled
- hidden/background browser work is reduced
- the browser overlay alignment path is less likely to cause memory growth through repeated resume-script execution
- the media viewer remains available, but with cleaner state ownership and less accidental work

Build gate:

- `T66Editor Win64 Development` succeeded
- `T66 Win64 Development` succeeded

Next verification target:

- open TikTok once in gameplay and confirm the game remains responsive
- switch to Shorts and confirm the same
- close and reopen once to verify the viewer no longer freezes the machine
- after that, retry the save/load verification that was blocked by the crash
