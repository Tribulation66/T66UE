# Completion Packet — Undeprecate Minigames → Demo-Gated Invisible

**Operator:** Claude (FullOperator)
**Date:** 2026-05-30
**Approval:** `codex_operator_approval.md` (APPROVE)

## Outcome

Minigames (top-bar tab + Mini/TD/Idle/Deck/Versus screen family) are no longer
part of the deprecated-feature category. The `bDisableMinigames` flag and the
`AreMinigamesDisabled()` accessor were removed entirely. Screen visibility is now
driven by the release-variant demo gate, so the current forced-demo build still
hides Minigames, while the full (non-demo) build exposes the Minigames top-bar
tab and makes the minigame screens reachable. Arcade remains deprecated and
untouched.

## Files Changed (approved scope)

| File | Change |
|---|---|
| `Config/DefaultGame.ini` | Removed `bDisableMinigames=true`. Only the two arcade flags remain under `[/Script/T66.T66DeprecatedFeatureSettings]`. |
| `Source/T66/Core/T66DeprecatedFeatureSettings.h` | Removed `bDisableMinigames` UPROPERTY and `AreMinigamesDisabled()` declaration. Arcade settings/accessors retained. |
| `Source/T66/Core/T66DeprecatedFeatureSettings.cpp` | Removed `AreMinigamesDisabled()` body. |
| `Source/T66/UI/T66UIManagerReleaseVariant.cpp` | Dropped DeprecatedFeatureSettings include + deprecation early-return; renamed helper `T66IsDeprecatedMinigameScreenType` → `T66IsDemoGatedMinigameScreenType`; minigame screen types now blocked only when `IsDemoModeActive()`. |
| `Source/T66/UI/T66FrontendTopBarWidget.cpp` | Removed deprecation guard from `HandleMiniGamesClicked` and the DeprecatedFeatureSettings include; added `T66DemoModeUIUtils.h`; added `bShowMinigamesTab = !IsDemoModeActive(this)` driving a conditional Minigames category tab + dual rect layout (demo path = shipped layout unchanged; non-demo path = 5-tab layout incl. Minigames). |
| `Source/T66/UI/Screens/T66MainMenuScreen.cpp` | Removed `AreMinigamesDisabled()` guard in `OnMinigamesClicked` and the now-unused include. |
| `Source/T66/UI/Screens/T66MinigamesScreen.cpp` | Removed minigame-disabled lock; card `bDemoLocked` and click handlers now keyed off arcade-disabled / demo-play-lock only. |
| `Source/T66/UI/WidgetGames/T66WidgetGameRegistry.cpp` | Removed the `FrontendMinigameLocked && AreMinigamesDisabled()` early-out; gate kind now resolves to `!IsDemoModeActive()`. Arcade gate retained. |
| `Source/T66/Core/T66DirectEntry.cpp` | Renamed `TryResolveDeprecatedMinigameScreen` → `ResolveMinigameScreen`, dropped the disabled early-return, removed the include, updated all call sites. |
| `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | Removed `ResolveScreenClass` deprecation guard + now-unused `T66IsDeprecatedMinigameScreenType`; renamed `TryResolveDeprecatedMinigameScreen` → `ResolveMinigameScreen`, dropped early-return, removed include, updated all call sites. |
| `Demo/DEPRECATED_CONTENT.md` | Removed the Minigames feature-gate row, the Minigames DEPRECATED section, and the `AreMinigamesDisabled()` accessor mention; added a pointer to the demo-gated inventory. |
| `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` | Added entry #9 (Minigames) naming the screen gate, conditional top-bar tab, and registry seam, plus the restore path. |

## Files Changed (outside listed scope — disclosed for validator ratification)

| File | Change | Justification |
|---|---|---|
| `Source/T66/UI/Screens/T66VersusArcadeScreen.cpp` | `OnBackClicked()` previously branched on `AreMinigamesDisabled()` (→ MainMenu) vs Minigames hub. With the accessor removed this was a hard compile blocker. Simplified to navigate to `ET66ScreenType::Minigames` unconditionally. | Forced compile-blocker fix anticipated by approval Process Rules ("compile blocker proves one adjacent fix is required" and "where a Minigames container still references an arcade/Versus card"). Arcade availability unchanged; only the back-navigation target changed. |

## Verification

| Check | Command | Result |
|---|---|---|
| Removed-symbol sweep (source) | grep `AreMinigamesDisabled|bDisableMinigames|T66IsDeprecatedMinigameScreenType|TryResolveDeprecatedMinigameScreen` over `Source/` | **PASS** — no matches |
| Residual deprecated-feature uses are arcade-only | grep `T66DeprecatedFeatures::` over `Source/` | **PASS** — all remaining calls are `AreArcadeGamesDisabled()` / `AreArcadeInteractablesDisabled()` |
| Config state | grep `bDisable*` in `Config/DefaultGame.ini` | **PASS** — only `bDisableArcadeGames=true`, `bDisableArcadeInteractables=true` |
| Focused C++ compile | `Build.bat T66Editor Win64 Development -project=T66.uproject -waitmutex` | **PASS** — exit 0, full T66 module relink, `UnrealEditor-T66.dll` linked, UHT ran with `-WarningsAsErrors`. 55.6s. |

## Pending / Recommended (not yet run)

- **Staged standalone build:** `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` — not run this session (heavy full cook+package; deferred pending user go-ahead to avoid tying up the machine). Recommended before release sign-off since config + UI gating changed.
- **Runtime capture proofs:** (a) demo Main Menu/top bar shows no Minigames tab; (b) demo direct `T66.Screen Minigames` is blocked/absent; (c) if practical, `-T66FullGame` direct-entry proof that the Minigames screen/top-bar entry exists outside demo. Deferred with the staged build.

## Caveats

- The non-demo top-bar layout uses a newly-derived 5-tab rect set (Account/HOME/PowerUp/Achievements/MiniGames) since the shipped demo layout had no Minigames slot. The demo path keeps the shipped rects byte-for-byte, so the demo top-bar screenshot acceptance is unaffected. The non-demo 5-tab layout has been compile-verified but not yet visually captured (see pending proofs).
- `ResolveMinigameScreen` in both `T66DirectEntry.cpp` and `T66PlayerController_Frontend.cpp` is now a trivial passthrough (`OutScreenType = ScreenType; return true;`). Kept as a named resolver to preserve the per-key call-site structure with minimal diff.
- Token ledger: helper manifest not available this session; not recorded.
