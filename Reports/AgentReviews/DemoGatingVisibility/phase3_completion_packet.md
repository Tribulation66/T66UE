# Demo Gating Visibility — Phase 3 Completion Packet

## Revision note (post-REVISE)

Validator found that the grid screens' **initial** population paths
(`OnScreenActivated_Implementation`) still called `GI->GetAllHeroIDs()` /
`GI->GetAllCompanionIDs()`, while only the `BuildSlateUI` fallback used the
playable lists. This revision changes those initial paths to
`GetPlayableHeroIDs()` / `GetPlayableCompanionIDs()` so that **both** populate
paths in each grid use the playable lists. Exact anchors corrected:

- `Source/T66/UI/Screens/T66HeroGridScreen.cpp:101`
  → `AllHeroIDs = GI->GetPlayableHeroIDs();`
- `Source/T66/UI/Screens/T66CompanionGridScreen.cpp:103`
  → `AllCompanionIDs = GI->GetPlayableCompanionIDs();`

Focused T66Editor compile re-run after this revision: `Result: Succeeded`.

## Outcome

Demo-gated, non-deprecated, non-Mini UI entries are now **hidden** from visible
lists/buttons instead of being shown with a `COMING SOON` overlay. Backend and
navigation guards are unchanged. The shared overlay helper
(`T66DemoModeUI::WrapWithComingSoonOverlay` / `GetUnavailableContentText`) is left
intact for code that still intentionally uses it. The two demo docs were updated
to describe the hidden-entry model. Focused T66Editor compile passed.

## Files changed (only approved files)

Code:
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Party.cpp`
- `Source/T66/UI/Screens/T66HeroGridScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionGridScreen.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`
- `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Build.cpp` (untracked file from prior work; edited on disk and compiled into the module)

Docs:
- `Demo/DEMO_RELEASE_INSTRUCTIONS.md`
- `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` (untracked file from Phase 1; edited on disk)

`Source/T66/UI/T66CasinoGamblerTabWidget.cpp` was in the approved set but did not
need edits (its `IsCasinoGameAllowed` / `SetStatus` remain a backend guard).

Not touched: all other modified files in the working tree (e.g. `T66PowerUpScreen.cpp`,
`T66AchievementsScreen.cpp`, HUD widgets) are pre-existing user changes and were
preserved.

## Key behavior changes (hide, don't overlay)

- **Heroes (carousel + grid):** source lists now come from `GetPlayableHeroIDs()`
  instead of `GetAllHeroIDs()`, so non-allowed heroes never enter the visible list;
  the now-inert carousel/grid overlay wraps were removed.
- **Companions (selection + grid + hero-selection wheel):** source lists now come
  from `GetPlayableCompanionIDs()`; demo-gated companions are omitted. Progression-
  locked-but-demo-allowed companions remain visible as locked/disabled tiles
  (unlock state still drives tile enablement, not hiding).
- **Difficulties (hero-selection, companion-selection, leaderboard dropdowns):**
  lists now built from `GetPlayableDifficulties()` (leaderboard skips non-playable),
  so only `Easy` is exposed in demo; harder difficulties no longer appear.
- **Lab button:** the LAB slot is only added when `bLabPlayable`, otherwise omitted.
- **Daily Descent CTA:** the main-menu CTA slot is only added when
  `bDailyDescentAvailable`; the screen navigation guard
  (`CanShowScreenForReleaseVariant`) and `IsDailyDescentAvailable` are unchanged.
- **Casino games:** gated game cards return `SNullWidget` (hidden); the "More Games"
  button collapses when no more-games card is available, so it cannot lead to an
  empty sub-view. The per-game open handlers / `SetStatus` backend guard remain.

## Verification commands + markers

Focused compile:
```
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
```
Result marker: `Result: Succeeded` (Total execution time: 55.47 seconds).
Build steps: `[6/7] Link [x64] UnrealEditor-T66.dll`, `[7/7] WriteMetadata T66Editor.target`.

Post-REVISE re-run (after correcting the grid initial-population paths):
Result marker: `Result: Succeeded` (Total execution time: 20.78 seconds).
Build steps: `[3/4] Link [x64] UnrealEditor-T66.dll`, `[4/4] WriteMetadata T66Editor.target`.

## Code-level proof

Heroes — `T66HeroSelectionScreen_Build.cpp`:
```cpp
AllHeroIDs = T66GI->GetPlayableHeroIDs();
...
HSAddCanvasSlot(CarouselCanvas, X, Index < 5 ? 1.f : 2.f, 67.f, Index < 5 ? 72.f : 71.f,
    HeroCarouselButton);            // overlay wrap removed
```
`T66HeroSelectionScreen_Party.cpp::RefreshHeroList` already used `GetPlayableHeroIDs()`.
`T66HeroGridScreen.cpp` both populate paths now use `AllHeroIDs = GI->GetPlayableHeroIDs();`
— the initial `OnScreenActivated_Implementation` path (line 101) and the
`BuildSlateUI` fallback (line 112) — and the tile `AddSlot(... Tile);` (overlay wrap removed).

Companions:
- `T66CompanionSelectionScreen.cpp::RefreshCompanionList`: `AllCompanionIDs = GI->GetPlayableCompanionIDs();`
  and carousel slot now adds `CompanionButton` directly (overlay wrap removed).
- `T66CompanionGridScreen.cpp` both populate paths now use
  `AllCompanionIDs = GI->GetPlayableCompanionIDs();` — the initial
  `OnScreenActivated_Implementation` path (line 103) and the `BuildSlateUI`
  fallback (line 113) — and tile `AddSlot(... Tile);` (overlay wrap removed);
  `bEnabled` still gates on `bUnlocked && bCompanionPlayable` so progression-locked
  tiles remain visible.
- `T66HeroSelectionScreen_Party.cpp::RefreshCompanionList`: now `GetPlayableCompanionIDs()`.

Difficulties:
- `T66HeroSelectionScreen_Build.cpp`: `Difficulties = T66GI->GetPlayableDifficulties();`
- `T66CompanionSelectionScreen.cpp`: `Difficulties = T66GI->GetPlayableDifficulties();`
- `T66FlatLeaderboardPanel.cpp::BuildDifficultyMenu`: `if (!bPlayable) { continue; }`
  before building the option.

Lab — `T66HeroSelectionScreen_Build.cpp`:
```cpp
if (bLabPlayable)
{
    HSAddCanvasSlot(RightContent, 455.f, 24.f, 119.f, 50.f, MakeButton(...));
}
```

Daily Descent — `T66MainMenuScreen.cpp::MakeCtaStack`:
```cpp
if (bDailyDescentAvailable)
{
    CtaCanvas->AddSlot()....[ MakeCtaButton( ... "DAILY DESCENT" ... ) ];
}
```

Casino — `T66CasinoGamblerTabWidget_Build.cpp`:
```cpp
const bool bGameAllowed = T66WidgetGames::Registry::IsAvailable(this, *Descriptor);
if (!bGameAllowed) { return SNullWidget::NullWidget; }
...
// More Games button gated:
.Visibility(bAnyMoreGamesAvailable ? EVisibility::Visible : EVisibility::Collapsed)
```

## Remaining overlay usages (classification)

Available-now no-op (overlay condition is always false given current demo config /
Phase 1 unlocks, or now-filtered source list):
- `T66HeroSelectionScreen_Build.cpp:516` — single-use buff / "drug" buy button,
  gated by `bDemoDrugPurchasesBlocked` (false since Phase 1 set `bAllowDrugPurchases=true`).
- `T66HeroSelectionScreen_Build.cpp:752` — difficulty dropdown option flag; menu now
  iterates `GetPlayableDifficulties()`, so `bDifficultyPlayable` is always true → inert.
- `T66CompanionSelectionScreen.cpp:871` — same difficulty-dropdown flag, now inert.
- `T66FlatLeaderboardPanel.cpp:761` — `bShowUnavailableOverlay = false` set by this change.
- `T66PowerUpScreen.cpp:1969, 2426, 2579, 2778` — diploma/drug overlays; inert since
  Phase 1 unlocked drug purchases and set the diploma cap to full. Not in approved
  scope; untouched.
- `T66AchievementsScreen.cpp:1385, 1765` — `bDemoAchievementRowsLocked` hard-set false
  (Phase 1). Inert. Not in approved scope; untouched.

Backend guard / still-intentional label (kept on purpose):
- `T66CasinoGamblerTabWidget.cpp:196` — `SetStatus(COMING SOON)` if a gated casino
  game is opened. Defensive backend guard; now unreachable via UI because the cards
  are hidden, but retained.
- `T66DemoModeUIUtils.cpp/.h` — helper definitions, retained intact.

Deprecated / Mini — excluded from scope (tracked in `Demo/DEPRECATED_CONTENT.md`):
- `T66MinigamesScreen.cpp:280`
- `T66ArcadeSelectionWidget.cpp:462`
- `T66VersusArcadeScreen.cpp:277`

Still needing follow-up on approved non-Mini surfaces: none. All targeted entries
(heroes, companions, non-Easy difficulties, Lab, Daily Descent, extra casino games)
are now hidden rather than overlaid.

## Token ledger

Unavailable (no precise Claude token count exposed to the operator run).

## Caveats

- **Hero carousel with a small roster:** the hero carousel is a 7-slot modulo wheel
  and the difficulty/companion carousels are similar. With a 5-hero demo roster the
  wheel wraps and can repeat a hero at the edges — this is the existing wrap-around
  behavior already produced by `RefreshHeroList()` (which already used
  `GetPlayableHeroIDs()`), not a new defect. No empty-slot logic was added, to stay
  within approved files (`..._Preview.cpp`, which recomputes carousel portraits with
  the same modulo, is not in the approved set and was not edited; it consumes the
  now-filtered `AllHeroIDs`/`AllCompanionIDs` members).
- **Grid empty slots:** hero/companion grids have fixed slot counts; hidden entries
  leave disabled/empty tiles rather than reflowing. This matches the "hide" intent.
- **Casino "More Games" in demo:** with the default (non-compact) layout all three
  more-games entries are demo-gated, so the sub-view is empty in demo; the More Games
  button is collapsed to avoid a dead-end. In full game the button and cards appear
  normally.
- **Leaderboard difficulty filter:** in demo the leaderboard difficulty dropdown
  exposes only `Easy`. Viewing other difficulties' leaderboards is therefore not
  available in demo via this dropdown (consistent with "only expose playable
  difficulties").
- The casino build file and `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` are currently
  untracked in git (new files from prior phases). Edits are on disk and compiled.
- Staged standalone refresh and combined staged/capture proof are intentionally
  deferred to Phase 4 per the approval. No git operations were performed.
