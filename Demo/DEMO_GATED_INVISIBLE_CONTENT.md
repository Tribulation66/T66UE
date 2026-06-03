# Demo-Gated Invisible Content

Tracks content that is intentionally unavailable or hidden in the Steam **demo**
build after Demo Gating Visibility Phase 1. Each entry names the controlling
seam so the item can be restored to the demo (or moved to "available") with a
single, well-understood change.

This inventory is **demo gating only**. Content that is removed/disabled as a
feature (not demo-scoped) lives in `Demo/DEPRECATED_CONTENT.md`. An item appears
here only when its unavailability is driven by the demo/release-variant gate.

## Central Seam (read this first)

All demo gating routes through one release-variant gate. Restoring any item
below is "allow it through this gate."

| Layer | File | Role |
|---|---|---|
| Demo config (data) | `Config/DefaultDemoMode.ini` `[/Script/T66.T66DemoModeSettings]` | Allow-lists and demo flags. |
| Settings class | `Source/T66/Core/T66ReleaseVariantSubsystem.h` (`UT66DemoModeSettings`) | Property definitions for the config above. |
| Gate subsystem | `Source/T66/Core/T66ReleaseVariantSubsystem.cpp` (`UT66ReleaseVariantSubsystem`) | `IsSteamDemoBuild()` + per-content `Is*Allowed()` checks. Gating only diverges from the full game when `IsSteamDemoBuild()` is true. |
| GameInstance wrappers | `Source/T66/Core/T66GameInstance.cpp` (`IsHeroPlayable`, `IsDifficultyPlayable`, `IsRunCategoryPlayable`, `IsCompanionPlayable`, `IsCollectorPlayable`) | UI-facing convenience wrappers over the gate subsystem. |
| Screen gate | `Source/T66/UI/T66UIManagerReleaseVariant.cpp` (`CanShowScreenForReleaseVariant`) | Blocks whole screens in demo mode. |
| Overlay helper | `Source/T66/UI/T66DemoModeUIUtils.cpp` (`WrapWithComingSoonOverlay`, `GetUnavailableContentText`) | Renders the "COMING SOON" overlay over a gated widget. After Phase 3, visible non-Mini UI **hides** demo-gated entries instead of overlaying them, so this helper is no longer wired into those surfaces; it remains available for any case that should still intentionally show a label. |

Demo activation: `UT66ReleaseVariantSubsystem::GetEffectiveReleaseVariant()`
returns `SteamDemo` when `bForceDemoMode=true` (current state), a demo
command-line flag is passed, or the active Steam AppID equals `DemoSteamAppId`.

---

## Still Demo-Gated (invisible/unavailable in demo)

### 1. Extra heroes (hero carousel boxes beyond the demo roster)
- **What:** Any hero not in the demo allow-list is **hidden** — it is omitted from the hero carousel and the hero grid (Phase 3). Backend/navigation guards (`IsHeroPlayable`) remain.
- **Seam:** `Config/DefaultDemoMode.ini` `+AllowedHeroIDs=` (currently `Hero_1`..`Hero_5`) → `UT66ReleaseVariantSubsystem::IsHeroAllowed` / `FilterHeroIDs` → `UT66GameInstance::IsHeroPlayable`.
- **UI consumer:** Hero carousel/grid source lists use `GetPlayableHeroIDs()` (`T66HeroSelectionScreen_Build.cpp`, `T66HeroSelectionScreen_Party.cpp::RefreshHeroList`, `T66HeroGridScreen.cpp`), so non-allowed heroes never enter the visible list.
- **Restore:** Add the hero's row name to `AllowedHeroIDs` (an empty list allows all).

### 2. Extra companions (beyond the demo roster)
- **What:** Companions not in the demo allow-list are **hidden** — omitted from the companion carousel/grid and the hero-selection companion wheel (Phase 3). Progression-locked-but-demo-allowed companions still appear as locked tiles (the demo gate, not the unlock state, drives hiding). Backend guard (`IsCompanionPlayable`) remains.
- **Seam:** `Config/DefaultDemoMode.ini` `+AllowedCompanionIDs=` (currently `Companion_01`..`Companion_04`) → `IsCompanionAllowed` / `FilterCompanionIDs` → `UT66GameInstance::IsCompanionPlayable`.
- **UI consumer:** Companion source lists use `GetPlayableCompanionIDs()` (`T66CompanionSelectionScreen.cpp::RefreshCompanionList`, `T66CompanionGridScreen.cpp`, `T66HeroSelectionScreen_Party.cpp::RefreshCompanionList`).
- **Restore:** Add the companion row name to `AllowedCompanionIDs` (an empty list allows all).

### 3. Non-Easy difficulties (Medium, Hard, Very Hard, Impossible)
- **What:** Only `Easy` is playable in demo; harder difficulties are **hidden** from difficulty dropdowns (Phase 3). The hero-selection, companion-selection, and leaderboard difficulty lists are built from `GetPlayableDifficulties()` (Easy-only in demo) instead of `GetVisibleDifficulties()`, so harder difficulties no longer appear as overlaid entries.
- **Seam:** `Config/DefaultDemoMode.ini` `+AllowedDifficultyIDs=Easy` → `UT66ReleaseVariantSubsystem::GetPlayableDifficulties` / `IsDifficultyAllowed` → `UT66GameInstance::IsDifficultyPlayable`.
- **UI consumer:** `T66HeroSelectionScreen_Build.cpp`, `T66CompanionSelectionScreen.cpp`, and `T66FlatLeaderboardPanel.cpp::BuildDifficultyMenu` (skips non-playable difficulties).
- **Restore:** Add difficulty IDs (e.g. `Medium`, `Hard`, `VeryHard`, `Impossible`) to `AllowedDifficultyIDs` (an empty list allows all).

### 4. Daily Descent
- **What:** The Daily Descent CTA is **hidden** (omitted) from the main menu when unavailable (Phase 3), and the Daily Descent screen is still blocked entirely in demo mode by the navigation guard.
- **Seam:** `Source/T66/UI/T66UIManagerReleaseVariant.cpp:62` (`CanShowScreenForReleaseVariant` returns `false` for `ET66ScreenType::DailyDescent` when demo is active) and `Source/T66/UI/Screens/T66MainMenuScreen.cpp` (`IsDailyDescentAvailable` returns false when `IsDemoModeActive()`), consumed in `MakeCtaStack` — the CTA slot is only added when `bDailyDescentAvailable`.
- **Restore:** Remove the `DailyDescent` case from `CanShowScreenForReleaseVariant` and make `IsDailyDescentAvailable` not depend on demo mode. (No demo config flag currently controls Daily Descent — it is hard-gated to demo mode.)

### 5. Lab run
- **What:** The LAB button is **hidden** (omitted) in demo (Lab run category disallowed) (Phase 3). Backend guard (`IsRunCategoryPlayable`) remains.
- **Seam:** `Config/DefaultDemoMode.ini` `bAllowLabRun=false` → `UT66ReleaseVariantSubsystem::IsRunCategoryAllowed(ET66RunCategory::Lab)` → `UT66GameInstance::IsRunCategoryPlayable`.
- **UI consumer:** `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp` (Lab button slot is only added when `bLabPlayable`).
- **Restore:** Set `bAllowLabRun=true` in the demo config.

### 6. Collector
- **What:** Collector interaction is disallowed in demo.
- **Seam:** `Config/DefaultDemoMode.ini` `bAllowCollector=false` → `UT66ReleaseVariantSubsystem::IsCollectorAllowed` → `UT66GameInstance::IsCollectorPlayable`.
- **Restore:** Set `bAllowCollector=true` in the demo config.

### 7. Extra casino games (beyond the demo set)
- **What:** Casino games not in the demo allow-list are **hidden** — gated game cards are omitted from the gambler tab (Phase 3), and the "More Games" button collapses when no more-games card is available so it does not lead to an empty sub-view. Backend guard (`IsCasinoGameAllowed`, plus the per-game open handlers) remains.
- **Seam:** `Config/DefaultDemoMode.ini` `+AllowedCasinoGameIDs=` (currently `CoinFlip`, `RockPaperScissors`, `BlackJack`) → `UT66ReleaseVariantSubsystem::IsCasinoGameAllowed` (via `T66WidgetGames::Registry::IsAvailable`).
- **UI consumer:** `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Build.cpp` (`MakeGameCard` returns `SNullWidget` when not available; `bAnyMoreGamesAvailable` gates the More Games button).
- **Restore:** Add the casino game ID to `AllowedCasinoGameIDs` (an empty list allows all).

### 8. "COMING SOON" presentation string
- **What:** Not gated content itself; this is the label the shared overlay helper renders.
- **Seam:** `Config/DefaultDemoMode.ini` `UnavailableContentText=COMING SOON` → `UT66ReleaseVariantSubsystem::GetUnavailableContentText` → `T66DemoModeUI::GetUnavailableContentText`.
- **Note:** After Phase 3, visible non-Mini UI **hides** gated entries instead of overlaying this label, so the overlay is no longer wired into those surfaces. The string and helper are retained for any case that should still intentionally show a label.

### 9. Minigames (Minigames tab + Mini / TD / Idle / Deck / Versus screen family)
- **What:** The Minigames top-bar tab and every minigame screen are **present in the full game** but **hidden/blocked in demo**. Previously deprecated in all builds; now demo-gated-invisible only. In demo the top-bar Minigames tab is omitted and direct navigation to any minigame screen is blocked; in the full game the tab is visible and the screens are reachable.
- **Seams:**
  - **Screen gate:** `Source/T66/UI/T66UIManagerReleaseVariant.cpp` (`CanShowScreenForReleaseVariant` → `T66IsDemoGatedMinigameScreenType`) returns `false` in demo mode for: `Minigames`, `VersusMainMenu`, `MiniMainMenu`, `MiniSaveSlots`, `MiniCharacterSelect`, `MiniCompanionSelect`, `MiniDifficultySelect`, `MiniIdolSelect`, `MiniShop`, `MiniRunSummary`, `MiniBattle`, `TDMainMenu`, `TDDifficultySelect`, `TDBattle`, `IdleMainMenu`, `DeckMainMenu`.
  - **Top-bar tab:** `Source/T66/UI/T66FrontendTopBarWidget.cpp` adds the Minigames category tab (and its layout slot) only when `bShowMinigamesTab = !T66DemoModeUI::IsDemoModeActive(this)`. Demo path keeps the shipped non-Minigames layout unchanged.
  - **Widget-game registry:** `Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp` (`FrontendMinigameLocked` demo-gate kind → `!ReleaseVariant->IsDemoModeActive()`).
- **Restore (to demo):** Allow the minigame screen types through `CanShowScreenForReleaseVariant` (drop them from `T66IsDemoGatedMinigameScreenType`), force `bShowMinigamesTab` true, and relax the `FrontendMinigameLocked` registry gate. No demo config flag currently controls minigames — they are hard-gated to demo mode (mirrors Daily Descent).

---

## Moved to Available by Phase 1 (no longer demo-gated)

These were previously demo-gated and were intentionally unlocked in Phase 1.
Kept here for traceability; they are **not** currently gated.

### Drugs / single-use buff purchases — AVAILABLE
- **Seam:** `Config/DefaultDemoMode.ini` `bAllowDrugPurchases=true` → `UT66ReleaseVariantSubsystem::AreDrugPurchasesAllowed` → `UT66BuffSubsystem::AreSingleUseBuffPurchasesAllowed` (`Source/T66/Core/T66BuffSubsystem.cpp`, delegates to the release-variant gate).
- **UI:** Buy buttons in `T66HeroSelectionScreen_Build.cpp:520` and `T66PowerUpScreen.cpp:2426` only overlay when `bDemoDrugPurchasesBlocked`, which is now false in demo.
- **Re-gate (if ever needed):** Set `bAllowDrugPurchases=false`.

### Diploma upgrades (full fill-step cap) — AVAILABLE
- **Seam:** `Config/DefaultDemoMode.ini` `MaxDiplomaUpgradesPerStat=4` (equals `UT66BuffSubsystem::MaxFillStepsPerStat = 4`) → `UT66ReleaseVariantSubsystem::IsDiplomaUpgradeAllowed`.
- **UI:** Graduate buttons in `T66PowerUpScreen.cpp:1969` overlay only when `bDemoDiplomaLocked`; with the cap at the full 4 steps, no step is gated.
- **Re-gate (if ever needed):** Lower `MaxDiplomaUpgradesPerStat`.

### Steam and Secret achievements — AVAILABLE
- **Seam:** `Source/T66/UI/Screens/T66AchievementsScreen.cpp` — `bDemoAchievementRowsLocked` is hard-set to `false`, neutralizing both the Steam (~line 1385) and Secret (~line 1765) "COMING SOON" overlay loops.
- **Re-gate (if ever needed):** Restore `bDemoAchievementRowsLocked = T66DemoModeUI::IsDemoModeActive(this)`.
