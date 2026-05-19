# Section 5 - UI Performance

## Global Invalidation

No config or source hit was found for:

- `Slate.EnableGlobalInvalidation`
- `Slate.EnableGlobalInvalidationLists`
- equivalent global invalidation toggle

No explicit `SInvalidationPanel` construction was found. There are stale-looking includes in:

- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66TD/Private/UI/Screens/T66TDMainMenuScreen.cpp`

Frontend root does use a retainer:

- `UT66FrontendUIRootWidget` wraps main screen/top bar/modal/loading/popup layers in `SRetainerWidget`.
- The retainer uses `RenderOnInvalidation(true)`.

## Live Widget Composition

Cannot count live Slate widgets without running `stat slate` or Widget Reflector.

Static structure:

- `UT66UIManager` caches screens.
- It switches the active screen into the frontend root.
- It separately manages top bar, modal, loading, and popup layers.
- Gameplay HUD is added directly to viewport by `SetupGameplayHUD()`.
- HUD has delegate and timer refresh paths.

Likely live gameplay UI surfaces:

- Gameplay HUD root.
- HUD resource/health/timer/inventory/map panels depending on state.
- Interaction prompt overlays.
- Media/retro overlays depending on settings.
- Popup/modal layers if active.

## Scaling State

There are multiple scaling systems:

- Unreal DPI curve via `UIScaleRule=ShortestSide`.
- Player/UI scale helpers.
- `FT66Style` / `T66ScreenSlateHelpers` responsive metrics.
- Many screen-local fixed canvas systems.

Remediation is partial.

Evidence of remaining fixed roots:

- Settings Graphics still declares a 1920x1080 canvas reference.
- Main Menu uses a fixed 1920x1080 reference root.
- Several screens use `SConstraintCanvas` with fixed pixel offsets or normalized-to-1920 conversions.

`FT66Style::MakeResponsiveRoot()` currently applies global DPI/player UI scale through `SDPIScaler`; it does not fully apply the `ComputeResponsiveScale()` reference-fit path across all screens.

## 1280x800 / Steam Deck

Support is not proven.

Positive signs:

- UI instructions require 1280x720 among validation sizes.
- Helper metrics include compact and stacked thresholds.
- Deck runtime profile exists.

Risks:

- Many native Slate screens still use fixed 1920x1080 roots.
- No current 1280x800 capture gate was found.
- Existing pending issue says frontend screens lack a central controller-focus contract.

Required validation:

- Packaged standalone launch at 1280x800.
- Screenshots of core frontend, settings, hero selection, gameplay HUD, minigames.
- Controller-only navigation pass.
- `stat slate` and invalidation stats.

## Settings Screen

Settings screen is native Slate, not WBP.

Tabs:

- Gameplay
- Graphics
- Controls
- HUD
- Media Viewer
- Audio
- Crashing
- Retro FX

Graphics controls:

- Resolution
- Window mode
- Display mode
- UI style
- Quality/scalability
- FPS cap
- Fog

Missing:

- VSync toggle/control.
- Confirmed Deck-specific auto-application of UI scale/scalability/FPS cap.

## UI Performance Risks

- No global invalidation rollout.
- Retainer exists only at frontend root; gameplay HUD path needs separate profiling.
- Timer/delegate refresh paths may still repaint large HUD sections.
- 1920 reference layouts can create expensive layout and poor Deck fit.
- Lack of controller-focus contract blocks reliable Deck validation.

