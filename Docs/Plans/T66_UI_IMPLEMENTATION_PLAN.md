# T66 UI Implementation Plan

Date: 2026-04-02
Workspace: `C:\UE\T66`
Companion audit: `T66_UI_AUDIT.md`

## Objective

Rebuild the T66 Slate UI into a resolution-independent system that:

- is fully playable and readable at `1280x720`
- works cleanly on Steam Deck at `1280x800`
- scales up cleanly to `1920x1080`, `2560x1440`, `3840x2160`, and ultrawide displays
- eliminates button and panel text clipping
- removes bespoke per-screen scaling math
- centralizes layout, typography, spacing, and icon sizing into shared tokens and primitives

This document is an implementation plan only. It does not perform the implementation.

Visual-direction note:

- The reconstruction-first workflow now supersedes older style-direction assumptions in this file.
- Use this plan for layout/scaling/primitives/plumbing guidance.
- Use `Docs/UI/UI_Reconstruction_Sprite_Sheet_Workflow.md`, `Docs/UI/MasterStyle.md`, `Docs/UI/MASTER.md`, `Docs/UI/SCREEN_REVIEW_GATE.md`, and the `reconstruction-ui` skill for the active visual and screen-rebuild policy.

## Locked Decisions

- Minimum supported safe layout: `1280x720`
- Primary handheld target: Steam Deck `1280x800`
- Primary laptop target: `1366x768`
- Standard desktop target: `1920x1080`
- Large-display validation targets: `2560x1440`, `3440x1440`, `3840x2160`
- DPI rule: `Shortest Side`
- Layout strategy: design at the minimum supported layout size and scale up
- Asset strategy: author icons/textures at `2x` to `4x` of displayed size with mipmaps
- Scaling model: UE DPI scaling plus player UI scale multiplier
- Critical layout strategy on ultrawide: keep core UI inside a centered safe frame; use side gutters only for decorative or secondary content
- No new screen should introduce bespoke viewport scale math
- No new button or panel should hardcode text-unsafe fixed dimensions

## High-Level Architecture Target

The target end state has five core layers:

1. A single scale policy driven by UE user interface settings and one player-controlled UI scale multiplier.
2. A shared safe-frame layout helper that computes the usable core region for menus, HUD clusters, and modals.
3. A shared token layer for spacing, typography, icon sizes, button sizing, panel sizing, and screen chrome.
4. Reusable primitives:
   - `ST66Button`
   - `ST66Panel`
   - shared text helper/policy functions
5. Screen-level migrations that replace rigid sizes with tokenized or content-driven layout rules.

## Safe-Frame Strategy

The core UI should not expand indefinitely across wide monitors.

Use three layout zones:

- Core safe frame:
  - centered on screen
  - derived from the baseline aspect ratio (`16:9`)
  - width should be `min(ViewportWidth, ViewportHeight * 16 / 9)`
  - this is where primary menus, CTA flows, confirmation modals, critical HUD, and readable text blocks live
- Gutter space:
  - left/right areas outside the core safe frame on ultrawide
  - reserved for decorative framing, background art, ambient widgets, or optional low-priority information
- Free overlays:
  - full-screen loading scrims, damage flashes, scoped overlays, and other intentional screen-covering effects

This keeps the game usable on small displays while still looking intentional on large and curved ultrawide monitors.

## Global Rules To Enforce During Implementation

- Test in Standalone Game, not arbitrary PIE editor panel sizes.
- Button text must never rely on `SScaleBox` shrink-to-fit to remain readable.
- Panels containing variable-height content must wrap or scroll instead of clipping.
- Text behavior must be explicit:
  - labels/buttons: grow first, ellipsize only where explicitly intended
  - paragraphs/tooltips/descriptions: wrap at controlled widths
- Layout-facing numbers must come from shared tokens unless they are true one-off art geometry with clear justification.
- Paint-only constants inside custom `SLeafWidget` drawing code do not need to be externalized unless they affect layout or readability.
- All new layout should assume controller/gamepad focus remains viable.

## Validation Matrix

Every major migration phase must be checked at:

- `1280x720`
- `1280x800`
- `1366x768`
- `1920x1080`
- `2560x1440`
- `3440x1440`
- `3840x2160`

Additional checks:

- UI scale minimum
- UI scale default
- UI scale maximum
- mouse/keyboard focus
- gamepad focus

## Phase 0: Baseline Policy And Test Harness

### Goal

Lock the baseline rules and create a repeatable validation workflow before code migrations begin.

### Scope

- resolution policy
- manual test matrix
- acceptance screenshots/checklist
- implementation sequencing

### Primary Files

- `Config/DefaultEngine.ini`
- `T66_UI_AUDIT.md`
- `T66_UI_IMPLEMENTATION_PLAN.md`

### Tasks

- Record `1280x720` as the minimum safe layout in project UI documentation.
- Record the required validation matrix above.
- Define default screenshot checkpoints for frontend, overlay, and gameplay validation.
- Decide whether Steam Deck gets a slightly elevated default UI scale at runtime or via user setting defaults.
- Decide whether the legacy theme wrapper corner-radius staying at `0.0f` is intentional or a bug; this affects the primitive rollout.

### Acceptance Criteria

- The team uses one agreed baseline layout size: `1280x720`.
- There is no remaining ambiguity about whether layouts are authored for `1080p`, `4K`, or arbitrary editor viewport sizes.
- The rollout order is approved and implementation can begin without re-deciding policy.

## Phase 1: Unify Scaling And Viewport Policy

### Goal

Replace the current mixed scaling system with one authoritative path.

### Scope

- UE DPI settings
- shared application/player scale path
- removal of bespoke per-screen scaling

### Primary Files

- `Config/DefaultEngine.ini`
- `Source/T66/UI/Style/T66Style.h`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/T66ScreenBase.cpp`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`
- `Source/T66/UI/Screens/T66SettingsScreen.cpp`

### Tasks

- Configure `/Script/Engine.UserInterfaceSettings` with a real DPI rule and DPI curve based on the `1280x720` baseline.
- Introduce one shared UI scale multiplier that can be saved per-user and applied consistently on top of DPI.
- Decide whether the project keeps a thin shared `SDPIScaler` wrapper or fully defers to engine DPI; if a wrapper remains, it must only apply the player scale and safe-frame policy, not a second viewport heuristic.
- Remove custom viewport scale logic from:
  - `UT66MainMenuScreen`
  - `UT66FrontendTopBarWidget`
  - any other screen that bypasses the common path
- Make settings modal sizing depend on current viewport/safe frame, not physical desktop display metrics.

### Acceptance Criteria

- There is one scale policy in the codebase.
- Main menu, top bar, and standard screens no longer compute their own independent scale formulas.
- The same UI surface produces predictable size relationships across all test resolutions.

## Phase 2: Create Shared Layout Tokens

### Goal

Centralize layout-facing numbers so screen code stops tuning dimensions ad hoc.

### Scope

- spacing tokens
- font-size tokens
- icon-size tokens
- button/panel tokens
- safe-frame and modal size tokens

### Primary Files

- `Source/T66/UI/Style/T66Style.h`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/Dota/T66DotaTheme.h`
- `Source/T66/UI/Dota/T66DotaTheme.cpp`

### Tasks

- Introduce `FT66UIConstants` or equivalent shared layout token struct.
- Move common values into tokens:
  - button horizontal/vertical padding
  - button min/max width
  - standard button heights
  - panel padding
  - modal max width/max height
  - top bar reserved height
  - icon sizes
  - spacing scale
  - title/body/caption font sizes
  - readable line widths
- Keep legacy style/theme wrappers isolated from visual direction:
  - Legacy style/theme wrappers must not dictate the approved visual direction for reconstructed screens.
  - Shared tokens and primitives should stay style-neutral or be fed by the reconstruction-derived UI kit.
- Reconcile duplicated or contradictory theme/layout values, especially corner-radius behavior.

### Acceptance Criteria

- New layout code can use named tokens instead of raw numeric literals.
- At least the most common repeated dimensions have one canonical source.
- Theme and layout responsibilities are clearly separated.

## Phase 3: Build The Shared Primitives

### Goal

Create reusable button, panel, and text primitives that solve clipping at the source.

### Scope

- `ST66Button`
- `ST66Panel`
- shared text helpers
- adapter layer for current call sites

### Primary Files

- `Source/T66/UI/Style/T66Style.h`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/Components/T66Button.h`
- `Source/T66/UI/Components/T66Button.cpp`
- new Slate primitive files under `Source/T66/UI/Components/`

### Tasks

- Build `ST66Button` with:
  - content-driven sizing
  - configurable min/max width
  - configurable min height
  - icon-plus-text support
  - stacked-text support where needed
  - explicit text behavior
  - theme-driven padding/font/brush selection
- Remove `SScaleBox(ScaleToFit, DownOnly)` from default button text safety paths.
- Build `ST66Panel` with:
  - theme-driven shell styling
  - padding tokens
  - optional header/body/footer slots
  - optional scrolling body
  - viewport-safe max-size behavior
- Add shared text helper functions for:
  - section titles
  - body copy
  - compact labels
  - data rows
  - CTA labels
- Convert `FT66Style::MakeButton()` into an adapter over the new primitive so existing screens can migrate gradually.
- Decide whether existing `UT66Button` remains only as a Blueprint/action wrapper or becomes a convenience wrapper around the new Slate primitive.

### Acceptance Criteria

- The project has one canonical button primitive and one canonical panel shell.
- Default button behavior is text-safe without per-screen hacks.
- Existing screens can adopt the primitives incrementally through the adapter layer.

## Phase 4: Fix The Highest-Risk Clipping Screens First

### Goal

Resolve the most visible and user-reported text clipping failures immediately after the primitives exist.

### Scope

- hero selection
- companion selection
- adjacent simple CTA screens that share the same failure mode

### Primary Files

- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
- `Source/T66/UI/Screens/T66LobbyScreen.cpp`

### Tasks

- Replace fixed `90x36` CTA shells in hero/companion flows with content-safe button composition.
- Replace stacked buy/equip CTA rows with primitive-backed layouts using min/max width and safe padding.
- Convert adjacent fixed-width CTA rows in save slots and lobby flows where they would fail at `1280x720`.
- Ensure these screens use wrapping, ellipsis, or scrolling intentionally rather than by accident.

### Acceptance Criteria

- `EQUIPPED`, `BUY 250 AC`, and related labels do not clip at any required resolution.
- Hero/companion selection remains visually stable at `1280x720`, `1280x800`, and `3440x1440`.
- No screen in this phase depends on shrink-to-fit button text.

## Phase 5: Rebuild The Frontend Shell

### Goal

Stabilize the overall frontend structure so all menus inherit the same scaling, safe-frame, and primitive rules.

### Scope

- top bar
- main menu
- settings
- leaderboard and modal shell behavior

### Primary Files

- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/Screens/T66SettingsScreen.cpp`
- `Source/T66/UI/Components/T66LeaderboardPanel.cpp`
- `Source/T66/UI/T66UIManager.cpp`

### Tasks

- Move top bar layout into the shared safe-frame rules.
- Keep top bar controls within comfortable reach on ultrawide rather than pushing them to extreme edges.
- Rebuild main menu central column and surrounding content around tokenized widths and safe-frame layout.
- Replace display-metrics-based settings modal sizing with viewport-bound panel sizing plus scroll support where needed.
- Audit leaderboard filter controls and dropdowns for minimum width, text overflow policy, and scroll behavior.

### Acceptance Criteria

- Main menu and top bar no longer use bespoke scale systems.
- Frontend screens preserve hierarchy and readability from `1280x720` to `3840x2160`.
- Ultrawide layouts look intentional rather than stretched.

## Phase 6: Rebuild Merchant And Overlay Screens

### Goal

Remove the densest cluster of rigid card and panel layouts.

### Scope

- vendor
- gambler
- casino
- circus
- wheel
- collector
- lab
- crate

### Primary Files

- `Source/T66/UI/T66VendorOverlayWidget.cpp`
- `Source/T66/UI/T66GamblerOverlayWidget.cpp`
- `Source/T66/UI/T66CasinoOverlayWidget.cpp`
- `Source/T66/UI/T66CircusOverlayWidget.cpp`
- `Source/T66/UI/T66WheelOverlayWidget.cpp`
- `Source/T66/UI/T66CollectorOverlayWidget.cpp`
- `Source/T66/UI/T66LabOverlayWidget.cpp`
- `Source/T66/UI/T66CrateOverlayWidget.cpp`

### Tasks

- Replace fixed card heights and rigid CTA rows with panel primitives and tokenized slot/card sizing.
- Separate art-box size from text-container size so long names or prices do not compete with icon dimensions.
- Introduce scrolling for variable-length item/stat/detail sections where vertical density exceeds safe limits.
- Standardize bank/spinbox/button clusters through shared component sizing.
- Keep critical interaction clusters inside the safe frame even on ultrawide.

### Acceptance Criteria

- Shop cards, price labels, and action buttons do not clip at `1280x720`.
- Overlay screens do not collapse awkwardly when width is limited.
- Overlay screens do not spread essential interactions excessively on ultrawide.

## Phase 7: Rebuild The Gameplay HUD For Safe-Frame And Ultrawide

### Goal

Make gameplay UI readable and ergonomic across handheld, laptop, standard, and ultrawide displays.

### Scope

- persistent HUD clusters
- enemy widgets
- prompts
- map/minimap
- scoped and full-screen overlays

### Primary Files

- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/T66EnemyHealthBarWidget.cpp`
- `Source/T66/UI/T66EnemyLockWidget.cpp`
- `Source/T66/UI/T66HeroCooldownBarWidget.cpp`
- `Source/T66/UI/T66CowardicePromptWidget.cpp`
- `Source/T66/UI/T66LoadPreviewOverlayWidget.cpp`
- `Source/T66/UI/T66IdolAltarOverlayWidget.cpp`

### Tasks

- Anchor critical HUD clusters to the centered safe frame, not raw screen extremes.
- Review portrait, side-panel, minimap, boss bar, loot prompt, and media panel dimensions against the `1280x720` baseline.
- Let peripheral decorative elements expand into gutter space only when they do not hurt usability.
- Preserve full-screen overlays like scoped sniper matte as intentional free overlays.
- Ensure prompts and lock widgets remain readable without overwhelming the gameplay view at small resolutions.

### Acceptance Criteria

- HUD remains readable and comfortably scannable at `1280x720` and `1280x800`.
- Critical HUD elements do not require extreme eye travel on `3440x1440`.
- Full-screen overlays still behave as intended.

## Phase 8: Hardcoded-Size Cleanup And Final Regression Pass

### Goal

Use the audit inventory to remove the remaining layout-facing literals and close the rollout with a stable standard.

### Scope

- cleanup of remaining layout-facing hardcoded sizes
- final regression sweep
- coding standard update

### Primary Files

- all migrated UI files identified in `T66_UI_AUDIT.md`

### Tasks

- Triage the hardcoded-size inventory into three buckets:
  - must move to tokens
  - acceptable local constant
  - paint-only constant, leave as-is
- Remove any remaining bespoke viewport scale logic missed in earlier phases.
- Standardize text handling across remaining `STextBlock` usage.
- Update UI coding guidance so future code follows the new primitives and token system.
- Run a final screen-by-screen pass across the full validation matrix.

### Acceptance Criteria

- The major UI surfaces are no longer driven by scattered ad hoc layout literals.
- The project has a documented rule for when a raw constant is acceptable.
- The migrated UI passes the required validation matrix without known clipping or major alignment bugs.

## Migration Order Summary

Use this order unless implementation reveals a hard dependency:

1. Phase 1: scaling unification
2. Phase 2: token layer
3. Phase 3: primitives
4. Phase 4: hero/companion clipping fixes
5. Phase 5: top bar, main menu, settings, leaderboard
6. Phase 6: merchant and overlay screens
7. Phase 7: gameplay HUD
8. Phase 8: cleanup and regression

## Risks And Mitigations

### Risk: visual drift from the approved reconstruction reference

Mitigation:

- use the approved reference image and reference-derived UI kit as the visual authority
- migrate layout and interaction safely without preserving retired artistic directions as guidance

### Risk: button migration causes broad breakage because many screens use `FT66Style::MakeButton`

Mitigation:

- preserve `FT66Style::MakeButton()` as an adapter
- migrate call sites incrementally

### Risk: ultrawide support turns into stretched layouts

Mitigation:

- use the centered safe-frame/gutter model
- cap readable content widths
- do not fill the full screen width with primary interaction rows

### Risk: Steam Deck readability is still poor even with correct layout

Mitigation:

- support player UI scale
- consider a slightly higher default on Deck
- validate controller focus and handheld readability explicitly

### Risk: trying to externalize every numeric constant slows the project down

Mitigation:

- only externalize layout-facing numbers and widely reused values
- leave paint-only geometry local unless it causes a layout problem

## Definition Of Done

The rollout is complete when all of the following are true:

- The UI is functional and readable at `1280x720`.
- Steam Deck at `1280x800` is usable without clipping or tiny CTA text.
- Standard and high-end desktop displays scale cleanly.
- Ultrawide displays keep critical UI centered and comfortable to scan.
- Button and panel text clipping is eliminated in migrated screens.
- The project uses one scaling policy.
- Screen code primarily uses shared tokens and primitives rather than ad hoc width/height literals.
