# T66 UI Flat Redesign — Master Plan

This document is the single source of truth for the T66 UI flat redesign initiative. It covers the locked design system, the three-stage rollout plan, and all reference data needed to execute the work. It is meant to be consumed by Codex (or any equivalent coding agent) as the primary brief, alongside the existing technical audit and the V3 screen reference images.

The companion documents:

- `C:\UE\T66\UI\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md` — the existing technical audit. Read this first for codebase architecture (how T66 builds Slate UI, where files live, what helpers exist, what the conventions are).
- `C:\UE\T66\UI\Screen References\` — the V3 mockup folder. Each major screen has at least one PNG reference rendered in the target flat style. These are visual targets, not pixel-perfect specs.

This document complements the audit by specifying the *what* of the redesign. The audit specifies the *how* of the existing system.

---

## 1. Purpose and Scope

### 1.1 What this document is

A plan to migrate the T66 frontend UI from its current PNG-composited chrome (the Ultrakill reference plate library + glow material + chrome retainer) to a flat Slate-native chrome (axis-aligned rectangles with solid colors, no glow, no decorative chrome), screen by screen, while preserving runtime architecture, screen routing, navigation, localization, font access, data subsystems, and gameplay HUDs.

### 1.2 What this document is not

- Not a per-screen pixel spec. Visual targets are the V3 reference images.
- Not a replacement for the technical audit. Architecture context lives there.
- Not a list of art assets to author. The redesign explicitly removes PNG chrome as a load-bearing system.

### 1.3 The three-stage plan at a glance

- **Stage 1 — Foundation + Pilot.** Build `FT66FlatStyle` alongside the existing `FT66Style`, register design-system constants, disable glow paths for flat widgets, audit icons against the V3 specs (Codex generates any missing icons via image generation), and migrate the Hero Selection screen as the pilot to validate the system end-to-end. Other screens untouched. Delivered by Codex against this doc.
- **Stage 2 — Per-Screen Migration.** Iterative, screen by screen, with Pablo in the loop for content reconciliation. Each screen: examine existing class, compare to V3 reference, reconcile content differences, migrate to `FT66FlatStyle`, screenshot, iterate. Runs until every frontend screen has been migrated.
- **Stage 3 — Cleanup and Full Migration.** After every screen is on flat style, deprecate and remove the PNG chrome path: reference asset roots, master plate texture loaders, `M_UI_Glow`, chrome retainer pass for chrome surfaces, the chrome-relevant `MakeReference*AssetPath` helpers. Gameplay HUD, retro FX for world post-process, leaderboard/icon/portrait artwork, and core architecture stay.

### 1.4 How Pablo and Codex use this document

Pablo references this doc when initiating any Stage 1/2/3 session. Each session prompt to Codex includes a pointer to this doc plus a stage-specific scope statement. Codex reads this doc plus the technical audit plus relevant V3 references before acting.

---

## 2. Context

### 2.1 Current state

T66's frontend UI today is **Slate-native in structure, PNG-composited in chrome**. Widget trees are built in C++ using `SBorder`, `SOverlay`, `SButton`, etc., but the visible panel borders, button plates, decorative corners, and neon glow come from baked PNG art in the Ultrakill reference plate library. Brushes are typically tinted white so baked colors pass through. State changes (default → selected) swap a different PNG plate rather than changing a color. Glow comes from three contributing sources: baked into the PNGs, the runtime `M_UI_Glow` material applied per button, and the chrome retainer (`M_UI_RetroRetainer`) wrapping chrome and text surfaces. Icons are individual UTexture/PNG assets. The locked UI font is `Jersey10-Regular.ttf` (a pixel/display-style face) — no condensed sans-serif is registered, and we are intentionally keeping it that way.

### 2.2 Target state

A flat chrome system that:

- Renders panel borders, button borders, dropdown borders, and dividers as **pure Slate primitives** — `SBorder` with `FSlateColorBrush` or equivalents, no PNG plates required.
- Uses a small fixed palette (defined below), applied via code constants, not via baked art.
- Has **no glow, no halo, no bloom, no luminance falloff** on chrome. The chrome retainer pass is bypassed for flat surfaces. `M_UI_Glow` is not applied to flat buttons.
- Uses **Jersey10** consistently for all text via existing `T66RuntimeUIFontAccess`.
- Uses **individual icon textures** for content glyphs (trophy, stopwatch, helmet, etc.), with missing icons generated via Codex's image generation pipeline.

### 2.3 Why this change

The previous PNG-composited direction (Ultrakill reference library, baked glow, decorative corners) was layered, hard to keep visually consistent across screens, and produced rendering artifacts at certain sizes (whiteward fade on long edges, asymmetric glow). The new flat direction is simpler to maintain, renders predictably at any size, and scales cleanly with `T66ReferenceLayout` because there are no slice-stretch dependencies on chrome.

### 2.4 What does not change

- Screen routing (`ET66ScreenType`, `UT66UIManager`).
- Navigation (`NavigateTo`, `NavigateBack`, `ShowModal`, `CloseModal`).
- The base `UT66ScreenBase` lifecycle (`OnScreenActivated`, `RefreshScreen`, etc.).
- The reference layout system (`FT66ReferenceRect`, `FT66ReferenceTransform`).
- Font access (`T66RuntimeUIFontAccess`).
- Localization (`FText`, `NSLOCTEXT`, `TAttribute<FText>`).
- Subsystem data sources (`UT66GameInstance`, save data, leaderboards, settings).
- Gameplay HUD, in-run overlays, and minigame UIs (those have their own contracts; see audit).
- The world-facing retro FX subsystem for post-process effects on game rendering (separate from UI chrome).
- Content artwork: hero portraits, companion portraits, item icons, diploma art, drug art, minigame screenshots, 3D character renders, leaderboard icons, etc.

---

## 3. Locked Design System

This section is the canonical specification of the visual language. Every helper in `FT66FlatStyle` and every screen migration must conform to it.

### 3.1 Palette

All structural chrome and text uses these exact values. Define them as `FLinearColor` constants in `FT66FlatStyle` (or a shared palette header it includes). UI fidelity checklists should use the symbol column instead of literal hex; `Scripts/VerifyUIFidelity.py` resolves those symbols from the `FT66FlatStyle.cpp` palette functions.

| Role | Symbol | Hex | RGB | Notes |
|---|---|---|---|---|
| Background | `BackgroundColor` | `#08080C` | (8, 8, 12) | Near-black with a barely-perceptible cool tint. Screen base color. |
| Disabled fill | `DisabledFill` | `#14141C` | (20, 20, 28) | Used inside disabled controls. |
| Disabled border | `DisabledBorder` | `#34343C` | (52, 52, 60) | Dim neutral gray for disabled outlines. |
| Disabled text | `DisabledText` | `#5A5569` | (90, 85, 105) | |
| Default fill | `DefaultFill` | `#17171E` | (23, 23, 30) | Inside default-state controls and panels. Lifted above the page background so global chrome fill pixelation has visible value range. |
| Default border (structural) | `DefaultBorder` | `#4A4A55` | (74, 74, 85) | Neutral gray. Used for every structural panel border in default state. |
| Default text on default fill | `DefaultText` | `#DCD7EB` | (220, 215, 235) | Soft off-white for default controls. |
| Purple text accent | `PurpleAccent` | `#8A8A95` | (138, 138, 149) | Legacy API name for label-style accent text (column headers, filter labels, sub-headers, status text, accent markers). Never used for panel borders. |
| Selected fill | `SelectedFill` | `#1C0E10` | (28, 14, 16) | Inside selected-state controls. |
| Selected border | `SelectedBorder` | `#E1232D` | (225, 35, 45) | Red. Used for every interactive element in selected state. |
| Selected text | `SelectedText` | `#FF505F` | (255, 80, 95) | |
| Progress bar fill | `ProgressFill` | `#E1232D` | (225, 35, 45) | Solid red flat fill. Same red as selected border. |
| Ready / good standing text | `GoodStandingGreen` / `ReadyBorder` | `#1FB358` | (31, 179, 88) | Green. Used for positive status text ("GOOD STANDING", "READY" badge on local player Steam slot). |
| Hover border | `HoverBorder` | `#1FB358` | (31, 179, 88) | Transient hover alias. Same hue as ready/good standing, applied only while enabled interactive helpers are hovered. |
| Hover text | `HoverText` | `#4FD088` | (79, 208, 136) | Transient hover text for enabled interactive helpers. |
| Hover fill | `HoverFill` | `#0E140E` | (14, 20, 14) | Transient hover fill for enabled interactive helpers. |
| Primary text | `PrimaryText` | `#F0F0F5` | (240, 240, 245) | Near-white. Default text color on dark backgrounds. |
| Secondary text | `SecondaryText` | `#A7A7B0` | (167, 167, 176) | Muted neutral gray. Used for descriptions, captions, and secondary content. |
| Data accent (cyan) | `DataAccent` | `#3CDCF0` | (60, 220, 240) | Reserved for data-viz contexts (stat bars in hero detail panels). Not used for chrome. |
| Yellow accent (tickets) | preserve existing | preserve existing | - | Yellow ticket icon color. Preserve current value. |

### 3.2 Stroke

Single uniform border width: **2 px at 1920×1080 reference resolution**. The same stroke is used everywhere — outer containers, inner sub-panels, buttons, dropdowns, dividers. No variable-thickness emphasis, no double lines.

The stroke must render as a solid line of exactly the specified hex color along its full length. **The line color is not affected by any glow, halo, or luminance pass.** This is a hard requirement; see the forbidden elements section.

### 3.3 Panel separation rule (critical)

Every panel is a **complete, self-contained rectangle with its border drawn on all four sides**. No two panels share an edge. Between every pair of adjacent panels there is a **visible gap of background color** — roughly **8–14 px at 1920×1080**. The gap shows the near-black background between panel borders.

This rule applies recursively:
- Sub-panels inside an outer container have gaps between themselves and the outer container's border.
- Adjacent sub-panels inside the same outer container have gaps between each other.

**Forbidden failure modes:**
- Panels sharing border edges
- A row of buttons embedded inside a strip such that their top/bottom edges touch the strip's border
- A column of sub-panels merged into one continuous tower with internal dividers (must be separate rectangles with gaps)
- Any "merged mesh" of connected panels covering a region

Implement this in `FT66FlatStyle` via a constant `FlatGap` (e.g., 12 px at reference) and helper composition utilities that auto-insert gaps between vertically/horizontally stacked panels.

### 3.4 Panel containment hierarchy rule (critical)

**Outer container panels are used only where the V3 reference for a given screen explicitly shows them.** Do not create additional outer containers, and do not flatten ones that are specified. Per-screen specs (Stage 2) call out the containment tree explicitly.

Common patterns:
- A row of related buttons (top bar, sub-tab row) may have a single outer default-border container wrapping loose buttons inside, OR be a flat row of loose buttons with no outer container. Each screen specifies which.
- A logical grouping of related sub-panels (e.g., a filter row + the data it filters) commonly has a single outer container wrapping the group.
- Standalone panels do not nest.

When a container wraps loose elements, the loose elements maintain their own complete borders and have gaps both between each other and to the container's inner border.

### 3.5 State system

Every interactive element is in exactly one of these states:

- **Disabled** — dim fill, dim border, dim text. No glow. Color values per palette table above.
- **Default** — dark fill, neutral gray structural border, light text. No glow. The "normal" state of any interactive element.
- **Selected** — dark-with-red-tint fill, red border, red text. No glow. Used for currently active tabs, active filter dropdowns, primary action buttons (CTAs).
- **Ready** (specialized; green) — green border. Used for the local player's Steam party slot in Hero Selection and equivalent "positive ready" indicators. Not a general-purpose state.

Enabled interactive elements also have a transient hover overlay: `HoverBorder #1FB358`, `HoverText #4FD088`, and `HoverFill #0E140E`. Hover must never write or reinterpret the element's semantic state; Default, Selected, and Ready return to their authored state immediately on unhover. Disabled controls ignore hover and report `hover_capable=false` in UI dumps.

Some elements are always rendered in Selected state regardless of interaction (e.g., the filter dropdowns on Overview and History — they're always-active inputs). Per-screen specs note these.

### 3.6 Typography (Jersey10)

The locked font is `Jersey10-Regular.ttf` for all text. Resolve via existing `T66RuntimeUIFontAccess::MakeFontFromAbsoluteFile(...)` paths. No new font registration is part of this redesign.

Role conventions (all using Jersey10 at different sizes; the visual hierarchy comes from size and weight, not from face changes):

- **Screen titles** (e.g., "MINIGAMES", "CHALLENGES", "RUN SUMMARY", "LOAD GAME") — large size, primary white. Float above content with no panel.
- **Section headers within panels** (e.g., "RUN OUTCOME", "STATS", "INVENTORY", "ACCOUNT STATUS") — medium-large size, primary white, ALL CAPS where shown that way.
- **Sub-headers and label-style accents** (e.g., "MODIFIERS", "WEAPON", "ULTIMATE", "DIFFICULTY", "DATE") — small-medium size, `PurpleAccent`, ALL CAPS. The symbol name is retained for compatibility even though the current value is neutral grey.
- **Body text and descriptions** — regular size, primary white or `SecondaryText` depending on prominence, sentence case.
- **Stat values, scores, dates, times** — primary white, monospaced/tabular alignment. Right-align in columns where it improves readability.
- **Button labels** — primary white (default state), red (selected state), dim (disabled state). Match button border state.

Note on Jersey10's character: it's a pixel/display-style face. Headers will read as blocky/cyber rather than as condensed sans-serif. This is intentional and matches the project's existing locked font direction. The V3 mockups were rendered with a different font for design speed; final in-game presentation uses Jersey10 throughout.

### 3.7 Forbidden elements (universal)

These never appear on flat chrome. Apply across every panel, button, dropdown, and divider rendered via `FT66FlatStyle`:

- Inner glow, outer glow, halo, bloom, or any luminance effect on chrome
- Bevels, embossing, drop shadows, 3D effects
- Corner brackets, L-shapes, decorative corner pieces
- Decorative skull badges, decorative dashes, "+" marker decorations around headers
- Authored geometry notches, tabs, angular cuts on panel edges. The deterministic edge distortion applied globally by `FT66FlatStyle` is allowed because it is a render treatment that does not alter layout, hit rects, or panel geometry.
- Rounded corners (sharp 90° only)
- Diagonal cuts, chamfers, asymmetric shapes
- Scanlines, CRT/glitch effects, chromatic aberration on chrome (note: the world-facing retro FX subsystem can still apply effects to the game world; flat UI chrome is excluded from that pass via the bypass mechanism, see Stage 1 helper spec). The global chrome edge distortion treatment is not a CRT/glitch pass; it is a deterministic stepped border render treatment.
- Ad hoc background noise, grain, texture, or pattern fills inside chrome. The settings-backed fill pixelation applied globally by `FT66FlatStyle` is explicitly allowed; content artwork — hero portraits, 3D renders, diploma art — retains its native rendering.
- Gradient fills inside chrome (solid flat colors only)
- Decorative arrow flourishes (e.g., `<<<<` or `>>>>` around buttons)
- Watermarks, branding, or logos outside the screen's actual content (the Steam logo on party slots, the Steam Achievements logo on the achievements panel — those are preserved content)
- Scrollbars (use pagination indicators instead where horizontal navigation is needed; for vertical content, fit-in-viewport or paginate)
- Pixel-art display fonts other than Jersey10
- Tinted glow/luminance on borders that causes whiteward fade on long edges (this was a recurring failure mode in V3 iteration; the flat system avoids it by removing glow entirely)
- "Chadpocalypse" branding strip at screen bottom

### 3.8 Tooltip-icon-on-tab pattern

The previous "description band" pattern (a thin horizontal panel below the tab row containing one sentence describing the screen) is **replaced** by a small "i" info tooltip icon embedded inside each tab button.

For each tab in a tab row:
- Render the tab as normal (Default neutral gray or Selected red)
- Add a small lowercase-"i" glyph inside the tab button, positioned to the right of the tab label, sized roughly to the tab text's cap height, colored matching the tab's state (red for selected, accent gray for default)

The tooltip content (the description that used to be in the description band) lives in localization and is shown via Slate tooltip on hover. The mockup does not render the tooltip popup itself.

**Per-screen exceptions:**
- **Minigames**: keeps the description band (no tooltip icons on Minigames sub-tabs). Locked replacement text for the band: `Earn Chad Coupons and compete with friends and the world in the minigames.`
- **Challenges**: keeps a status notification line ("Community catalog refreshed (3 entries).") which is functionally different from a description band — it's a live state indicator, not a static description.

### 3.9 Locked content rewrites

These are content changes from the pre-redesign screens, locked in as part of the redesign:

- **Minigames description band**: `Earn Chad Coupons and compete with friends and the world in the minigames.` (replaces previous copy)
- **Overview Account Status warning paragraph**: `Your account is eligible for the leaderboard. If you cheat or manipulate runs or submissions, your account will be flagged and eternally removed from the leaderboard.`
- **Overview screen**: removes the bottom "CHADPOCALYPSE" branding strip; renames "Local Player" panel to use the player's actual name (placeholder "RandomChad" in V3); compacts the Account Status header to a single inline row with the status value in green.
- **Settings (Retro FX)**: removes the bottom warning text ("Adjust Retro FX sliders and press APPLY to save pending changes."), moves light/dark mode toggles from the right side of the sub-tab row to the left side.

Additional content reconciliation happens per screen during Stage 2.

### 3.10 Panel hierarchy by screen-family pattern

These are recurring structural patterns observed across the V3 mockups. `FT66FlatStyle` helpers should make them trivial to compose.

**Top bar pattern (meta-progression screens)** — outer default-border container wrapping loose buttons across the top: settings cog, globe, ACCOUNT, profile icon, POWER UP, ACHIEVEMENTS, MINIGAMES, ticket counter badge, power button. The currently active top-level section gets its tab in Selected red state. The settings cog is selected when on a settings screen. The power button is always red-bordered.

**Slim top bar pattern (leaf screens)** — Daily Descent and similar leaf screens have a reduced top bar: settings, globe, BACK TO MAIN MENU (centered, wider), power. Same outer container pattern, fewer elements.

**No top bar pattern (deep leaf screens)** — Hero Selection, Run Summary, Load Game, Challenges have no top bar at all. They have a BACK button + title in the top row instead.

**Sub-tab row pattern** — two or three loose tab buttons (no outer container) below the top bar, with tooltip "i" icons embedded in each tab.

**Two-column body** — Overview, Settings, Hero Selection variants. Left column with stacked panels, right column with stacked panels, no outer wrapper around the columns themselves. Per-screen specs note where outer containers wrap a column's contents.

**Three-column body** — Daily Descent, Run Summary, Hero Selection. Left/middle/right columns, often with outer containers on the column groups that need them.

**Card row pattern** — Diplomas, Drugs, Minigames. A row of self-contained cards inside an outer default-border container, each card containing artwork + name + description + a red-bordered action button.

**Bottom action row** — BACK + CONFIRM, BACK + ENTER + secondary, PREV + NEXT, etc. Loose buttons positioned at the screen bottom with no outer container.

---

## 4. Stage 1 — Visual System Foundation and Pilot

### 4.1 Goal

Deliver the `FT66FlatStyle` helper surface, register the design-system constants, audit the icon library, generate any missing icons, and migrate **Hero Selection** as the pilot screen to validate that the system can produce a complete screen matching its V3 reference. All other screens remain unchanged.

Hero Selection is the pilot because it is the project's most complex frontend screen (no top bar, three-column body with multiple sub-panels per column, hero portrait carousel, skin rows with portraits, drug equip slots, 2×2 action button grid, gender toggle, Steam party panel with ready state, difficulty dropdown, prominent CTA button, leaderboard-style sub-panels), and because validating the flat system against the hardest target ensures it can handle every simpler screen. It also exercises every helper pattern that Stage 2 screens will need.

### 4.2 Deliverable: `FT66FlatStyle` helpers

A new namespace/file (`FT66FlatStyle` in `Source/T66/UI/Style/T66FlatStyle.h` and `.cpp`) providing the following helper surface. The signatures below are illustrative — adapt to existing T66 parameter-struct conventions (`FT66FlatPanelParams`, `FT66FlatButtonParams`, etc., modeled on `FT66ButtonParams` and `FT66PanelParams`).

**Constants:**
- `FT66FlatStyle::FlatStroke` — border thickness at reference resolution (2 px).
- `FT66FlatStyle::FlatGap` — visible background gap between adjacent panels (12 px at reference, tunable).
- Palette constants (`FT66FlatStyle::BackgroundColor`, `DefaultBorder`, `SelectedBorder`, `ReadyBorder`, `DisabledBorder`, `PrimaryText`, `SecondaryText`, `PurpleAccent`, `ProgressFill`, `GoodStandingGreen`, etc.) returning `FLinearColor` per Section 3.1.
- An enum `ET66FlatState { Disabled, Default, Selected, Ready }` selecting the state for any control.

**Panel builders:**
- `MakeFlatPanel(state, padding, content)` — basic axis-aligned rectangle with solid colored border + dark fill. The fundamental building block.
- `MakeFlatInteractivePanel(state, padding, content, is_enabled, tag, intended_role)` — hover-capable panel shell for editable text boxes and custom input surfaces whose child widget owns focus/input instead of an `SButton` click handler.
- `MakeFlatOuterContainer(state, gap, children)` — auto-stacks child widgets with `FlatGap` separation and wraps the result in a flat panel.
- `MakeFlatSubPanel(state, padding, content)` — semantic alias of `MakeFlatPanel`, used inside outer containers for clarity.
- `MakeFlatHeaderedPanel(state, header_text, body_content, optional_icon, optional_header_accent)` — flat panel with a header row + body section. Used for "RUN OUTCOME", "STATS", "ACCOUNT PROGRESS"–style panels.

**Button builders:**
- `MakeFlatButton(state, label, click_handler, optional_left_icon, optional_right_icon, padding)` — text button with optional icons on either side of the label.
- `MakeFlatIconButton(state, icon, click_handler, optional_size_hint)` — square icon-only button.
- `MakeFlatTabButton(state, label, click_handler, optional_left_icon, tooltip_text)` — tab button with optional left icon and a mandatory tooltip "i" icon on the right; tooltip text is set as the Slate tooltip on the icon.
- `MakeFlatActionRow(buttons, alignment, gap)` — utility to lay out a row of action buttons with proper spacing.

**Input builders:**
- `MakeFlatDropdown(state, current_value_text, options_provider, on_select, force_selected_state_bool)` — dropdown control. The `force_selected_state_bool` parameter renders the dropdown in red even when not actively focused, for the always-active filter pattern (Overview, History).
- `MakeFlatSlider(state, min, max, current, on_change, optional_value_display)` — horizontal slider with thin track + small handle. Track has tick marks. Selected handle is red.
- `MakeFlatCheckbox(state, checked, on_toggle, optional_label)` — square checkbox.
- `MakeFlatToggleButton(state, label, is_active, on_toggle)` — Default/Selected toggle (used for CHAD/STACY-style gender toggles).

**Display builders:**
- `MakeFlatLabel(text, role)` where `role` is an enum like `Header | SubHeader | Body | Caption | PurpleAccent | StatLabel | StatValue` — returns a properly-sized/colored `STextBlock` using Jersey10.
- `MakeFlatStatRow(label_text, value_text, label_role, value_role)` — single label/value row with thin bottom divider in dim neutral gray.
- `MakeFlatStatsTable(rows, columns)` — multi-column stat grid with thin row dividers and no full cell borders.
- `MakeFlatProgressBar(percent, optional_color)` — horizontal bar with neutral gray track, red fill (default) or green fill (when used for Ready/positive). No glow.
- `MakeFlatDivider(orientation, length, color)` — thin solid line.
- `MakeFlatPaginationIndicator(total, current_index, current_color)` — segmented row of small rectangles showing position.

**Specialized layout builders:**
- `MakeFlatPortraitSlot(state, portrait_texture, optional_role_icon, optional_size)` — small bordered square containing a portrait + optional role icon corner. Reused by save slot character portraits, hero selection skin rows, Steam party slots (with Ready state for green border).
- `MakeFlatRankingRow(rank, name, score, optional_state)` — leaderboard-style row used in Daily Descent.
- `MakeFlatTopBar(buttons, optional_outer)` — flat panel wrapping loose top-bar buttons. Used by all meta-progression screens.
- `MakeFlatSlimTopBar(left_buttons, center_button, right_buttons)` — variant for Daily Descent–style slim top bar.

**Glow and retainer bypass:**
- All `FT66FlatStyle::MakeFlat*` widgets render with **`bUseGlow = false`** internally so `M_UI_Glow` is never applied.
- The chrome retainer (`M_UI_RetroRetainer` wrapping in `ST66RetroUIRetainedSurface`) is **bypassed for flat surfaces**. Provide an `FT66FlatStyle::WrapWithoutRetainer(widget)` helper or have all `MakeFlat*` helpers wrap their output in a non-retainer surface by default.
- The world-facing retro FX subsystem (`UT66RetroFXSubsystem`) for game-world post-process is **unaffected**. Only the chrome retainer pass is bypassed for flat UI.
- Flat chrome surfaces also receive settings-backed retro visual treatments through `FT66RetroFXSettings`: subtle fill pixelation on chrome fills and deterministic stepped edge distortion on borders. These treatments are applied by `FT66FlatStyle` helpers, affect chrome only, and keep text, icons, content artwork, layout bounds, and hit rects crisp and unchanged.

**Composition:**
- All helpers compose freely. Per-screen `BuildSlateUI()` calls a sequence of these helpers and assembles the final tree, identical to current `FT66Style` usage patterns.
- Helpers must accept `TAttribute<FText>` for any text that may update dynamically (player name, stat values, scores, dates, status, etc.) — preserve the existing live-data wiring contract.

### 4.3 Deliverable: Icon audit

Codex inventories every icon referenced across the V3 specs and produces an icon manifest. The V3 specs reference the following icons (non-exhaustive list — Codex should cross-check against the V3 reference images in `C:\UE\T66\UI\Screen References\`):

Top bar / system: settings cog, globe, profile icon, ticket, power button.

Action / navigation: BACK chevron (←), forward chevron (→), pagination chevrons (<, >), refresh / circular arrow, play / triangle, home, save / floppy disk, copy / clipboard, info "i", warning triangle, target / crosshair, pencil / edit, link / chain, crossed-X, broadcast / antenna, X-mark (X-crossed icon), gear icons.

Content / stat / role icons: trophy / laurel wreath, stopwatch / timer, helmet, shield, fist, book, skull (multiple variants), starburst / explosion, gauge / speedometer, dice, loot bag, plus / cross / heal, chevrons (rank), flame, moon, mountain triangle, wings, eye, eye with wings, lightning bolt, clover (4-leaf), check shield, chalice / grail, diamond / spiral idol shape, horned skull, cube / box, bar chart, log / clipboard.

Brand / partner glyphs (preserve, do not redesign): Steam logo, gender male/female icons (preserve blue/pink colors).

For each icon, Codex categorizes as:
- **Present** — an existing UTexture/PNG asset matches the spec. Note the asset path.
- **Partial** — a similar icon exists but does not match the V3 spec (wrong shape, wrong content). Note the existing asset path and what differs.
- **Missing** — no matching asset. Generate via Codex's image generation pipeline. Produce a single-glyph PNG asset, monochrome (`PurpleAccent` for default tint, or as a white/grayscale glyph that can be tinted at runtime), no background, no decorative chrome, sized for icon use (suggested 64×64 to 256×256 PNG with transparency).

Output: a manifest file at `C:\UE\T66\UI\icon_manifest.md` listing every icon, its category, and either its existing path or the generated file path.

Missing icons should be generated and saved under `RuntimeDependencies/T66/UI/Icons/Flat/<icon_name>.png`. The icon manifest must reference these paths. The runtime icon access path (`T66RuntimeUITextureAccess` or equivalent) should resolve these files. During migration, legacy `SourceAssets/UI/Icons/Flat/` requests should remap to the runtime dependency path rather than making ignored source-art folders a required runtime dependency.

### 4.4 Deliverable: Hero Selection pilot

After helpers exist and the icon manifest is complete, migrate the Hero Selection screen:

- Existing class: `T66HeroSelectionScreen` (and any auxiliary files under `Source/T66/UI/Screens/HeroSelection/` per the audit).
- Update `BuildSlateUI()` to compose the screen using `FT66FlatStyle` helpers exclusively for chrome. The 3D character render, hero portraits, skin portraits, weapon icon, ultimate icon, and Steam logo glyphs remain as content artwork (loaded via existing texture helpers, displayed inside flat-bordered slots).
- Match the Hero Selection V3 reference image at `C:\UE\T66\UI\Screen References\<hero_selection_v3_filename>.png` visually. Use the per-screen spec authored for Hero Selection (see Section 7.2 below) as the authoritative panel hierarchy and content list.
- Apply the size guidance from the Hero Selection V3 spec: ARTHUR name sized like a screen title (not a giant block); skin row portraits sized as small thumbnails; Steam party slots compact; ENTER button standard height (not a tall block).
- Live data: hook name, level, XP, stats, ticket counts, skin ownership, and Steam party state into the existing subsystems. Placeholder content ("RandomChad", "Hero_14", "0/100 XP TO LEVEL 2", "10" tickets, etc.) from the V3 stays as placeholder only if the live subsystem returns no value — otherwise live values win.
- Validate by capturing a screenshot using the existing capture script (per audit: `.\Scripts\CaptureT66UIScreen.ps1 -Screen HeroSelection -DelaySeconds 6 -CloseAfter` or equivalent). Compare visually against the V3 reference.

### 4.5 Acceptance criteria for Stage 1

All of the following must be true to consider Stage 1 complete:

1. `FT66FlatStyle.h` and `FT66FlatStyle.cpp` exist under `Source/T66/UI/Style/` with the helper surface specified in Section 4.2.
2. Palette constants and the `ET66FlatState` enum are defined and accessible.
3. The chrome retainer bypass and `bUseGlow = false` defaults are wired so no flat widget renders with glow or retainer chrome effects.
4. Icon manifest exists at `C:\UE\T66\UI\icon_manifest.md` covering every icon referenced in the V3 specs. Missing icons are generated and saved under `RuntimeDependencies/T66/UI/Icons/Flat/`. Runtime path resolution for these icons works.
5. The Hero Selection screen renders in flat style matching its V3 reference (modulo content differences, which are logged for Stage 2 follow-up).
6. The project compiles cleanly:
   `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
7. Hero Selection screenshot captured and saved alongside the V3 reference for visual comparison.
8. Every other frontend screen still renders without crash or visual regression (spot-check a few major screens: Main Menu, Overview, Run Summary).

### 4.6 Out of scope for Stage 1

- Migration of any screen other than Hero Selection.
- Deletion of any PNG asset, helper, or material from the existing `FT66Style` PNG-composited path. That path continues to function for non-migrated screens.
- Content reconciliation beyond the Hero Selection pilot (other screens' content stays as-is until their Stage 2 cycle).
- Performance optimization beyond what's natural in the helper implementation.
- New font registration. Jersey10 is the locked font; no new face is added.

---

## 5. Stage 2 — Per-Screen Migration

### 5.1 Goal

Migrate every remaining frontend screen from the PNG-composited path to `FT66FlatStyle`, one screen per session, with Pablo in the loop for content reconciliation. Each session produces one fully-migrated screen.

### 5.2 Per-screen workflow

Each Stage 2 session follows this loop:

1. **Codex examines** the existing screen class (locate under `Source/T66/UI/Screens/...`).
2. **Codex examines** the V3 reference image for the screen at `C:\UE\T66\UI\Screen References\<screen_name>.png`.
3. **Codex produces a content delta report**: a bullet list of every difference between the existing screen's content/structure and the V3 reference. Examples: "Existing class has a 'PARTY' panel showing 3 companion slots; V3 shows a 4-slot Steam party panel." Or: "Existing class shows 5 stat rows; V3 shows 9 stat rows." Or: "Existing class shows a 'BUY' button; V3 shows 'BUY' + 'CLEAR' buttons."
4. **Pablo reviews** the content delta, clarifies any ambiguity, and decides per item: keep existing, replace with V3, or adjust further.
5. **Codex implements** the migration: rewrites `BuildSlateUI()` using `FT66FlatStyle` helpers; reconciles content per Pablo's decisions; wires any new data hookups required (subsystem calls, save data references); preserves all navigation, refresh, and lifecycle logic.
6. **Codex captures a screenshot** using the per-screen script and compares visually against the V3 reference.
7. **Pablo confirms** the result or requests adjustments. Loop until accepted.

### 5.3 Screen list (in suggested migration order)

The order is informed by complexity, dependencies, and what validates the system most usefully early.

1. **Hero Selection** — completed in Stage 1 as pilot.
2. **Overview** (Account → Overview tab) — V3 ref ready, 5+ rounds of iteration, simplest hierarchy. Validates the standard meta-progression screen pattern with top bar + sub-tab row + two-column body.
3. **History** (Account → History tab) — V3 ref ready, same family as Overview, reuses everything.
4. **Diplomas** (Power Up → Diplomas) — card row pattern with horizontal pagination.
5. **Drugs** (Power Up → Drugs) — category-grouped card grid pattern.
6. **Steam Achievements** (Achievements → Steam) — list-with-progress-summary pattern.
7. **Minigames** — card row with locked description band (no tooltip).
8. **Settings → Retro FX** — settings sections with slider sub-panels.
9. **Daily Descent** — slim top bar, three-column body, leaderboard.
10. **Challenges** — no top bar, centered sub-tabs, two-column body, BACK/CONFIRM bottom row.
11. **Load Game** — no top bar, BACK + title top row, 2×2 save slot grid.
12. **Run Summary** — no top bar, dense three-column body with many sub-panels.

Additional screens (Main Menu, Pause Menu, Quit Confirmation, Report Bug, Hero Grid, Companion Grid, Language Select, Party Invite, Account Status, Player Summary Picker, Save Preview, Companion Selection, minigame-specific screens) get migrated as their V3 references are produced. They are out of scope until their references exist.

### 5.4 Per-screen handoff template

Each Stage 2 session uses this prompt template to Codex:

```
Reference docs:
- C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md (this master plan)
- C:\UE\T66\UI\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md (technical audit)

Target screen: <ScreenName>
V3 reference: C:\UE\T66\UI\Screen References\<screen_name>.png
Existing class: Source\T66\UI\Screens\<existing_class_location>

Task:
1. Read the master plan section 5 for the per-screen workflow.
2. Examine the existing class and the V3 reference.
3. Produce a content delta report (do not modify code yet).
4. Wait for Pablo's reconciliation decisions.
5. After clarification, migrate the screen to FT66FlatStyle using the per-screen spec.
6. Compile, capture screenshot, present for visual comparison.

Constraints:
- Use FT66FlatStyle helpers only (no FT66Style PNG path for chrome on this screen).
- Preserve content artwork (portraits, 3D renders, item images) as content.
- Preserve navigation, refresh, and lifecycle logic.
- Wire live data via existing subsystems; placeholder content only if no live value.
- Conform to the locked design system (master plan section 3).
```

### 5.5 Per-screen spec format

For each screen, the per-screen spec (already drafted during the V3 iteration sessions; collected in this master plan in Section 7.2) defines:

- Reference image path
- Existing class location
- Panel containment hierarchy (the tree)
- Content text (verbatim where authored)
- Icons used per element
- State assignments (which buttons are Selected red, which are Default neutral gray, etc.)
- Specific size guidance (where the V3 had proportion issues to correct)
- Per-screen exceptions (e.g., Minigames keeps description band)
- Live data hookups (which subsystem provides each dynamic value)

### 5.6 Acceptance criteria for each screen migration

Per screen:
1. Existing class's `BuildSlateUI()` uses `FT66FlatStyle` helpers for chrome.
2. Content delta has been reconciled with Pablo.
3. Live data flows correctly; refresh and lifecycle behavior is preserved.
4. Project compiles cleanly.
5. Screenshot matches the V3 reference modulo agreed content variations.
6. No regression on other screens.

### 5.7 Stage 2 completion

Stage 2 is complete when every screen listed in Section 5.3 has been migrated and accepted. Additional screens may follow later as their V3 references become available, but the gating step for Stage 3 is that the listed twelve screens are migrated.

---

## 6. Stage 3 — Cleanup and Full Migration

### 6.1 Goal

After every screen is on `FT66FlatStyle`, deprecate and remove the PNG-composited chrome path so the codebase has a single, consistent UI chrome system.

### 6.2 Triggers

Begin Stage 3 only when:
1. Every screen in the Stage 2 list is on `FT66FlatStyle` and accepted.
2. No remaining code path constructs the PNG-composited chrome path through `FT66Style::MakeButton/MakePanel` (PNG variant) or `T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton/MakeReferenceSharedBorder` for chrome.
3. Gameplay HUD and overlays are confirmed unaffected (they may use chrome differently — verify per the audit before deprecating shared helpers).

### 6.3 Deprecation list (chrome-only — do not touch outside this scope)

To remove:
- `SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/` — the Ultrakill reference plate library (chrome PNG roots).
- Master button/panel texture loaders in `FT66Style` that resolved chrome PNGs.
- Most `MakeReference*AssetPath` helpers that pointed at chrome PNG paths (audit each — some may serve non-chrome content and should remain).
- `M_UI_Glow` material application from button paths.
- Chrome retainer pass (`M_UI_RetroRetainer` via `ST66RetroUIRetainedSurface`) for chrome surfaces. Keep the retro retainer where it is used for non-chrome purposes (verify each usage).
- The PNG-composited code branches in `FT66Style::MakeButton/MakePanel` (collapse to flat-only behavior).

To keep:
- `UT66UIManager`, `UT66ScreenBase`, navigation system.
- `T66ReferenceLayout` and reference rects.
- `T66RuntimeUIFontAccess` and Jersey10.
- `T66RuntimeUITextureAccess` (still needed for icons, portraits, content artwork).
- All gameplay HUD, in-run overlay, and minigame-specific UI helpers.
- `UT66RetroFXSubsystem` for world-facing post-process (separate from UI chrome).
- All content artwork: hero portraits, companion portraits, item icons, diploma art, drug art, minigame screenshots, 3D character renders, leaderboard icons, etc.

### 6.4 Removal sequence

1. Audit: grep for every usage of the to-be-removed helpers and materials. Confirm every usage is either inside a migrated screen (and ready to be removed) or part of a non-chrome path (and to be preserved).
2. Update `FT66Style::MakeButton/MakePanel` to remove the PNG branches; keep the procedural fallback path or redirect to `FT66FlatStyle` if `FT66Style` is deprecated entirely.
3. Remove `M_UI_Glow` application from button construction.
4. Remove chrome retainer wrapping from chrome surfaces (keep elsewhere if needed).
5. Move the Ultrakill plate library out of active paths (initially to an `Archive/` folder; deletion can wait for confidence).
6. Compile, full regression sweep, capture screenshots of every migrated screen, compare against the Stage 2 baseline.
7. Confirm no visual regression. Delete archived assets if confident.

### 6.5 Acceptance criteria for Stage 3

1. No active code path resolves chrome from a PNG plate.
2. No active code path applies `M_UI_Glow` or the chrome retainer to chrome surfaces.
3. Project compiles cleanly.
4. Every migrated screen still renders identically to its Stage 2 final.
5. Gameplay HUD and overlays unaffected.
6. Optionally: archived PNG plate library deleted from active paths (with confidence).

### 6.6 Stage 3 risk

The main risk is incidentally breaking a non-chrome usage of a shared helper (e.g., a `MakeReference*AssetPath` that's used for both chrome and a content icon). Mitigation: audit every usage before removal, preserve helpers that serve dual purposes, and remove only the chrome-specific paths within helpers that branch.

---

## 7. Reference Data

### 7.1 V3 screen reference inventory

All V3 reference images live in `C:\UE\T66\UI\Screen References\`. Screens with mockups already produced (as of this plan's authoring):

- Overview (Account → Overview)
- History (Account → History)
- Diplomas (Power Up → Diplomas)
- Drugs (Power Up → Drugs)
- Steam Achievements (Achievements → Steam)
- Minigames
- Daily Descent
- Challenges
- Settings (Retro FX tab specifically; other settings tabs derive from this pattern)
- Load Game
- Run Summary
- Hero Selection

Screens still pending V3 reference creation (not in current scope; added to Stage 2 as their references are produced):

- Main Menu
- Pause Menu
- Quit Confirmation
- Report Bug Modal
- Hero Grid
- Companion Grid
- Language Select
- Party Invite Modal
- Account Status Modal
- Player Summary Picker
- Save Preview
- Companion Selection
- Additional Settings tabs (Gameplay, Graphics, Controls, Media Viewer, Audio)
- Additional Achievements tabs (Secret)
- Additional History filter views
- Minigame-specific screens (Mini, TD, Idle, Deck, Versus main menus + sub-screens)

### 7.2 Per-screen specifications

Per-screen specs were authored during the V3 iteration sessions and are reproduced below in compact form. Each screen's spec defines its panel hierarchy, content, state assignments, and exceptions. These are the authoritative specs for Stage 2 migrations.

**Hero Selection** (Stage 1 pilot)
- No top bar.
- Top row: BACK button (Selected) on far left; hero portrait carousel (no outer container, left arrow + 7 portrait squares + right arrow) centered; ARTHUR hero name (sized as a screen title, not oversized) + LAB button (Selected) on far right.
- Three-column main body:
  - LEFT: two stacked panels — SKINS panel (header with ticket badge, Default skin row Selected with EQUIPPED badge, Beachgoer row Default with PREVIEW button and 50-ticket cost; skin portrait thumbnails small) + DRUGS panel (header, 4 empty equip slots row, BUY (Selected) + CLEAR (Default) action row).
  - MIDDLE: character preview panel containing the 3D Arthur render and circular platform (preserve native rendering).
  - RIGHT: outer default-border container with subtitle "A KING. A CRUSADE. AN APOCALYPSE.", RANK sub-panel (info icon + label + lock + "--" value), MASTERY sub-panel (info icon + label + red progress bar + "LV 1" + "0 / 100 XP"), STATS sub-panel (two-column stat grid with 8 stats — Damage 4/99, ATT Speed 2/99, ATT Scale 2/99, Accuracy 2/99 / Armor 7/99, Evasion 1/99, Luck 2/99, Speed 2/99), WEAPON / ULTIMATE sub-panel (two columns, weapon icon + ultimate icon).
- Bottom row (compact — see size guidance): Steam party panel (4 small slots, slot 1 Ready green, slots 2-4 Default neutral gray with Steam logo + character silhouette); CHAD (Selected) / STACY (Default) toggle + CHOOSE COMPANION (Default) button stacked; DIFFICULTY label + dropdown ("Easy") + ENTER button (Selected, standard height, with skull icon); CHALLENGES MODS button (Default, compact) far right.
- Size guidance: ARTHUR name like screen titles; skin portraits small thumbnails; Steam party compact; ENTER standard height; carousel portraits same size as reference or slightly smaller.
- Interactivity:
  - Toggle group `GenderToggle`: members `HeroSelection.BottomRow.CompanionPanel.ChadButton`, `HeroSelection.BottomRow.CompanionPanel.StacyButton`; mutually exclusive `true`; initial selection `HeroSelection.BottomRow.CompanionPanel.ChadButton`; drives `GenderSelection` / `SelectedBodyType`.
  - Toggle group `SkinSelection`: members `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default`, `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer`, `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Crusader`, `HeroSelection.LeftColumn.SkinsPanel.SkinRow.GoldenPaladin`; mutually exclusive `true`; initial selection `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default`; drives `EquippedSkin` / `SelectedHeroSkinID`. PREVIEW on non-equipped rows is a separate action, not part of the toggle.
  - Toggle group `HeroCarousel`: members `HeroSelection.TopRow.HeroCarousel.Portrait01` through `HeroSelection.TopRow.HeroCarousel.Portrait07`; mutually exclusive `true`; initial selection `HeroSelection.TopRow.HeroCarousel.Portrait04`; drives `PreviewedHeroID`. `HeroSelection.TopRow.HeroCarousel.LeftArrow` and `HeroSelection.TopRow.HeroCarousel.RightArrow` scroll the carousel and are not toggle members.
  - Single-action buttons: `HeroSelection.TopRow.BackButton` -> `NavigateBack`; `HeroSelection.RightColumn.HeaderRow.LabButton` -> `OpenLab` placeholder until backend exists; `HeroSelection.BottomRow.DifficultyPanel.EnterButton` -> `StartRun`; `HeroSelection.BottomRow.CompanionPanel.ChooseCompanionButton` -> `OpenCompanionPicker`; `HeroSelection.LeftColumn.DrugsPanel.BuyButton` -> `BuyCurrentSelectedDrugOrSkin`; `HeroSelection.LeftColumn.DrugsPanel.ClearButton` -> `ClearDrugSlots`; `HeroSelection.BottomRow.ChallengesButton` -> `OpenChallenges`; `HeroSelection.BottomRow.ModsButton` -> `OpenMods`; `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.PreviewButton` -> `PreviewSkin` placeholder until backend exists.
  - Dropdowns: `HeroSelection.BottomRow.DifficultyPanel.Dropdown`; options source `ET66Difficulty` playable difficulty list; drives `SelectedDifficulty`.
  - Any V3-listed single-action element without backend infrastructure must still bind a placeholder logging handler so the control is visibly responsive and reports `has_click_handler=true`.

**Overview** (most refined V3, 5 rounds of iteration)
- Top bar (outer container, loose buttons): settings, globe, ACCOUNT (Default neutral gray because the account sub-tab carries Selected state), profile, POWER UP, ACHIEVEMENTS, MINIGAMES, ticket badge (10), power button.
  - Note: ACCOUNT button is Default state when the user is on an account-section page; sub-tabs handle Selected state.
- Sub-tab row (no outer container): OVERVIEW (Selected, with "i" tooltip icon) + HISTORY (Default, with "i" tooltip icon).
- No description band (tooltip handles it).
- Two-column body:
  - LEFT (no outer container): three stacked sub-panels.
    - Player block (default-border): borderless avatar slot left + right stack of [RandomChad name / LEVEL 1/100 in red text / EXPERIENCE label with XP value right-aligned / red progress bar].
    - Account Status (default-border): shield icon left + inline header row "ACCOUNT STATUS — GOOD STANDING" (GOOD STANDING in green `#1FB358`) + warning paragraph in secondary text.
    - Account Progress (default-border): bar-chart icon + "ACCOUNT PROGRESS" header + 5 red progress bars (Achievements Unlocked, Permanent Buffs, Heroes, Companions, Challenges) with monospaced count labels right-aligned.
  - RIGHT (outer default-border container wrapping):
    - Filter row: PERSONAL BEST + SOLO dropdowns, both forced Selected (red) — always-active filters.
    - HIGHEST SCORE sub-panel (trophy icon + header + 5-row table: Easy/Medium/Hard/Very Hard/Impossible × columns Difficulty/Hero/Date/Global Rank/Score).
    - BEST SPEED RUN sub-panel (stopwatch icon + header + 5-row table with same difficulties × Difficulty/Hero/Date/Global Rank/Time).
- No CHADPOCALYPSE branding strip. No scrollbar.
- Interactivity:
  - Top bar single-actions: settings cog -> `OpenSettings`; globe -> `OpenLanguageSelect`; ACCOUNT category button -> no-op/current section; profile -> `OpenPlayerSummary`; POWER UP -> `NavigatePowerUp`; ACHIEVEMENTS -> `NavigateAchievements`; MINIGAMES -> `NavigateMinigames`; ticket badge -> `OpenCouponInfo` or placeholder; power -> `OpenQuitConfirmation`.
  - Sub-tab toggle group `AccountTabs`: OVERVIEW Selected, HISTORY Default; mutually exclusive; drives `ActiveAccountTab`. Info icons on each sub-tab open/hover the tooltip and must have handler or tooltip metadata.
  - Dropdowns: PERSONAL BEST and SOLO; options from leaderboard/account filters; both render forced Selected because they are active controls.
  - Body panels are informational; table rows are non-interactive unless the implementation has an existing details action, in which case bind and document it during migration.
- Label vs button:
  - Labels: player name, LEVEL value, EXPERIENCE label/value, ACCOUNT STATUS header/value, warning paragraph, ACCOUNT PROGRESS header, progress row names/counts, HIGHEST SCORE/BEST SPEED RUN headers, table headers, difficulty/date/rank/score/time values.
  - Buttons/dropdowns: top bar controls, OVERVIEW/HISTORY sub-tabs, sub-tab info tooltip controls, PERSONAL BEST dropdown, SOLO dropdown.
- Icon manifest:
  - Existing flat icons: `gear.png`, `globe`/language icon if present or generate via M1, `ticket.png`, `shield.png`, `bar_chart.png`, `trophy_laurel.png`, `stopwatch.png`, `power` icon if present or generate via M1.
  - Needs M1 reference crop if exact style is missing: account/profile glyph, POWER UP icon, ACHIEVEMENTS icon, MINIGAMES icon, sub-tab info icon.
  - Content art: player avatar is content; use live profile avatar if available, otherwise stub under `SourceAssets\UI\ContentStubs\Overview\`.

**History**
- Top bar same as Overview, ACCOUNT button Default.
- Sub-tab row: OVERVIEW (Default, tooltip) + HISTORY (Selected, tooltip).
- No description band.
- Filter row outer container: 4 dropdowns (HERO, DIFFICULTY, PARTY SIZE, STATUS) showing "ALL" each + DAILY DESCENT checkbox (Default neutral gray square). All dropdowns forced Selected.
- Run history panel (no section header): column header row with 6 columns (HERO PLAYED, DATE, STATUS, SCORE, DURATION, RANK), each sortable column with small sort-arrow indicator; RANK column header is a dropdown selector (not sortable). Empty state row: "No runs have been recorded yet." in secondary text.
- Interactivity:
  - Top bar single-actions same as Overview; ACCOUNT remains current section.
  - Sub-tab toggle group `AccountTabs`: OVERVIEW Default, HISTORY Selected; mutually exclusive; drives `ActiveAccountTab`.
  - Filter controls: HERO, DIFFICULTY, PARTY SIZE, STATUS dropdowns; DAILY DESCENT checkbox drives `bHistoryDailyDescentOnly`.
  - Sortable column headers: HERO PLAYED, DATE, STATUS, SCORE, DURATION are buttons/toggles that drive sort column/direction. RANK is a dropdown selector.
- Label vs button:
  - Labels: empty state text, row values, table captions, non-clickable descriptive copy.
  - Buttons/dropdowns/checkboxes: top bar controls, account sub-tabs, info tooltip controls, four filter dropdowns, DAILY DESCENT checkbox, sortable column headers, RANK selector.
- Icon manifest:
  - Reuse Overview top bar icons.
  - Existing or M1-needed icons: info icon, sort arrows, checkbox check/empty mark, rank selector chevron/dropdown.

**Diplomas** (Power Up → Diplomas)
- Top bar with POWER UP Selected (red).
- Sub-tab row: DIPLOMAS (PERMANENT) Selected with tooltip + DRUGS (ONE TIME USE / ONE RUN USE — verify final wording) Default with tooltip.
- No description band.
- Main content outer default-border container with: left navigation arrow button + 4 diploma cards horizontally + right navigation arrow button + pagination indicator at bottom.
- Each diploma card: diploma artwork preserved at top (each card's distinct parchment imagery) + stat upgrade text in accent gray (`PurpleAccent`) (+0 DAMAGE, +1 ATTACK SPEED, +2 ATTACK SCALE, +3 ACCURACY) + GRADUATE button (Selected red, full-width, "GRADUATE" label + 10-ticket cost).
- Pagination: 4 visible diplomas of many; current position red, others dim neutral gray. No "SCROLL FOR MORE DIPLOMAS" text.
- Interactivity:
  - Top bar single-actions as standard; POWER UP is current/selected.
  - Sub-tab toggle group `PowerUpTabs`: DIPLOMAS Selected, DRUGS Default; mutually exclusive; drives `bShowingSingleUse=false`.
  - Left/right navigation arrows page the diploma carousel.
  - Pagination dots reflect current page; if clickable, each dot drives page index.
  - Each GRADUATE button purchases/upgrades the card's primary stat and refreshes counts.
- Label vs button:
  - Labels: card stat names, upgrade values, ticket cost text if not independently clickable, pagination status text if any.
  - Buttons: top bar controls, DIPLOMAS/DRUGS tabs, nav arrows, pagination dots if clickable, GRADUATE controls.
- Icon manifest:
  - Existing flat icons: `ticket.png`, `pagination_left.png`, `pagination_right.png`.
  - Needs M1 reference crop if absent: sub-tab info icons and any diploma-specific stat glyphs.
  - Diploma parchment/card artwork is content art and remains PNG-driven; stub only if production diploma art is missing.

**Drugs** (Power Up → Drugs)
- Top bar with POWER UP Selected.
- Sub-tab row: DIPLOMAS Default + DRUGS Selected.
- No description band.
- Main content outer default-border container with two stacked category rows. Each row: left vertical category label panel (icon + multi-line category name in accent gray) + 4 drug cards horizontally.
- Category 1 Damage Drugs (target icon): OXYMETHOLONE, METHANDROSTENOLONE, FLUOXYMESTERONE, NANDROLONE DECANOATE with respective drug artwork preserved + "+10% AOE/Bounce/Pierce/DOT Damage" stat text + BUY button (Selected red, 1-ticket cost).
- Category 2 Attack Speed Drugs (speedometer icon): CAFFEINE CITRATE, MODAFINIL, EPHEDRINE HCL, SALBUTAMOL SULFATE + analogous stat text + BUY buttons.
- No scrollbar; additional categories exist in implementation but mockup shows only these two.
- Interactivity:
  - Top bar single-actions as standard; POWER UP is current/selected.
  - Sub-tab toggle group `PowerUpTabs`: DIPLOMAS Default, DRUGS Selected; mutually exclusive; drives `bShowingSingleUse=true`.
  - Each BUY button purchases the drug/secondary buff represented by that card.
  - Drug cards themselves are informational unless the existing implementation treats the whole card as clickable; if so, bind the card to the same buy/select action and document it in the checklist.
- Label vs button:
  - Labels: category names, drug names, stat effect text, ticket cost text if not separately clickable.
  - Buttons: top bar controls, DIPLOMAS/DRUGS tabs, BUY controls.
- Icon manifest:
  - Existing flat icons: `target_crosshair.png`, `gauge_speedometer.png`, `ticket.png`.
  - Drug bottle/card artwork is content art and remains PNG-driven; generate content stubs via Section 9 only for missing production art.

**Steam Achievements** (Achievements → Steam)
- Top bar with ACHIEVEMENTS Selected.
- Sub-tab row: STEAM Selected with tooltip + SECRET Default with tooltip.
- No description band.
- Steam summary panel (wide horizontal, default-border): Steam logo left + "STEAM ACHIEVEMENTS 0/100" header (0 in red, /100 in secondary) + red progress bar.
- Achievement list panel (no section header): rows with row number + name + bracketed description (in secondary text) + progress value (0/1) + reward value (5 CC) + outlined star icon (favorite indicator). Three rows visible: Collector 1, Field Notes 1, Token Rank 1. Thin dividers between rows.
- Interactivity:
  - Top bar single-actions as standard; ACHIEVEMENTS is current/selected.
  - Sub-tab toggle group `AchievementTabs`: STEAM Selected, SECRET Default; mutually exclusive; drives `ActiveAchievementTab`.
  - Favorite star per row toggles favorite/pinned state if backend supports it; otherwise bind a placeholder logging handler.
  - Achievement rows are informational unless current implementation opens details; if so, row click opens details.
- Label vs button:
  - Labels: summary header, count values, row number/name/description/progress/reward text, progress bar labels.
  - Buttons: top bar controls, STEAM/SECRET tabs, sub-tab info controls, favorite star toggles, row detail actions if present.
- Icon manifest:
  - Brand asset placeholder: Steam logo requires approved Steam brand asset or live platform asset; do not invent a production logo.
  - Existing flat icons: `starburst.png` or generate favorite outline via M1 if the reference star style is distinct; `ticket.png` for CC if shown as icon.

**Minigames**
- Top bar with MINIGAMES Selected.
- No sub-tab row (leaf-level section).
- Large floating "MINIGAMES" screen title centered.
- **Description band kept** (this is the locked exception): "Earn Chad Coupons and compete with friends and the world in the minigames." in primary white, regular sans-serif, sentence case, centered.
- Main content outer default-border container with: left nav arrow + 4 minigame cards + right nav arrow + pagination at bottom.
- Each minigame card: screenshot preserved + card title (CHADPOCALYPSE MINI / TOWER DEFENSE / DECKBUILDER / IDLE) + description + PLAY GAME button (Selected red, full-width).
- Interactivity:
  - Top bar single-actions as standard; MINIGAMES is current/selected.
  - Left/right nav arrows page the minigame carousel if more than four entries exist.
  - Pagination dots reflect the current page; if clickable, each dot drives page index.
  - Each PLAY GAME button navigates to that minigame's main menu.
  - Cards themselves are informational unless implementation already treats card click as play/select; if so, bind to the same route and document.
- Label vs button:
  - Labels: MINIGAMES screen title, description band text, card titles, card descriptions.
  - Buttons: top bar controls, nav arrows, pagination dots if clickable, PLAY GAME buttons.
- Icon manifest:
  - Existing flat icons: `pagination_left.png`, `pagination_right.png`, top bar icons.
  - Minigame screenshots are content art and remain PNG-driven; generate stubs only for missing screenshots.

**Daily Descent**
- Slim top bar (outer container, 4 loose buttons): settings cog + globe + BACK TO MAIN MENU (centered, wider) + power button (Selected red).
- Three-column body:
  - LEFT (outer default-border container): "RULES OF THE DAY" header + intro row sub-panel (i icon + intro text) + 4 stat row sub-panels (Hero Selected → Hero_14, Companion Selected → None, Difficulty → Hard, Continue Save → "No saved Daily run") + "MODIFIERS" sub-header with /// marker + 3 modifier row sub-panels (Pocket Draft + dice icon + description, Iron Parade + shield icon + description, Double Drop + bag icon + description).
  - MIDDLE (outer default-border container): hero artwork (gold idol on pyramid with fiery halo, preserved) + "DAILY DESCENT" title (standard cyber typography, not pixel art) + subtitle "One seed. One attempt. Same puzzle for everyone." in accent gray + START DESCENT (Selected red, large primary CTA with chevron decorations) + CONTINUE DESCENT (Default neutral gray, secondary with chevrons).
  - RIGHT: 3 small icon-only tabs at top (globe Selected, people Default, broadcast Default) + Daily Global Chad Rankings panel (default-border) with header + 9-row leaderboard + red separator + player's row (#42 DOPRA 118700).
- Interactivity:
  - Slim top bar single-actions: settings -> `OpenSettings`; globe -> `OpenLanguageSelect`; BACK TO MAIN MENU -> `NavigateMainMenu`; power -> `OpenQuitConfirmation`.
  - Leaderboard scope toggle group `DailyLeaderboardTabs`: globe Selected, people Default, broadcast Default; mutually exclusive; drives leaderboard source/scope.
  - START DESCENT starts the daily run. CONTINUE DESCENT resumes saved daily run if available; disabled or placeholder if no save.
  - Rules/stat/modifier rows are informational.
- Label vs button:
  - Labels: RULES OF THE DAY header, intro text, stat labels/values, MODIFIERS header, modifier names/descriptions, DAILY DESCENT title/subtitle, leaderboard header, leaderboard row text.
  - Buttons: slim top bar controls, leaderboard scope icon tabs, START DESCENT, CONTINUE DESCENT.
- Icon manifest:
  - Existing flat icons: `gear.png`, globe/language icon, power icon if present or M1, `info.png`, `dice.png`, `shield.png`, `loot_bag.png`, `people.png`, `broadcast_antenna.png`.
  - Hero artwork is content art and remains PNG-driven.

**Challenges**
- **No top bar.**
- Large floating "CHALLENGES" screen title centered.
- 3 centered sub-tab buttons (no outer container): OFFICIAL (Selected with target icon) + COMMUNITY (Default with people icon) + CREATE CHALLENGE (Default with pencil icon).
- Status notification panel (thin, default-border): "Community catalog refreshed (3 entries)." in accent gray.
- Two-column body:
  - LEFT (outer default-border container): GLASS ROUTE challenge card (Selected red, with starburst icon, OFFICIAL badge, 40 CHAD COUPONS, TRIBULATION 66 author) + PRESSURE RUN challenge card (Default, gauge icon, 30 CHAD COUPONS) + pagination indicator (4 dots, first red) at bottom.
  - RIGHT (outer default-border container): "GLASS ROUTE" title block + "Official by Tribulation 66" subtitle (no + decorations) + description sub-panel ("Clear the run without taking a single hit.") + "RULES AND REQUIREMENTS" sub-header (no + decorations) + rules sub-panel with diamond bullets (◆ Challenge only completes on a full clear. ◆ Take no damage for the run.).
- Bottom row: BACK (Default, left arrow, far-left) + CONFIRM (Selected, right arrow, far-right). No central skull decoration.
- Interactivity:
  - Sub-tab toggle group `ChallengeTabs`: OFFICIAL Selected, COMMUNITY Default, CREATE CHALLENGE Default; mutually exclusive; drives challenge catalog mode/editor route.
  - Challenge card selection toggle group `ChallengeSelection`: GLASS ROUTE Selected, PRESSURE RUN Default; drives selected challenge details.
  - Pagination dots/page controls update visible challenge page.
  - BACK navigates back; CONFIRM starts/accepts selected challenge.
  - Status notification panel is informational.
- Label vs button:
  - Labels: CHALLENGES screen title, status notification text, challenge card titles/badges/rewards/authors, detail title/subtitle/description, rules header, rules text.
  - Buttons: OFFICIAL/COMMUNITY/CREATE CHALLENGE tabs, challenge cards if selectable, pagination controls, BACK, CONFIRM.
- Icon manifest:
  - Existing flat icons: `target_crosshair.png`, `people.png`, `pencil_edit.png`, `starburst.png`, `gauge_speedometer.png`, `back_chevron.png`, `forward_chevron.png`.
  - Needs M1 if exact reference style is absent: diamond bullet glyph for rules.

**Settings (Retro FX tab)**
- Top bar with settings cog Selected (this is the only screen where the cog is the selected indicator).
- Sub-tab row (no outer container): Sun (Default) + Moon (Selected) light/dark toggles on the LEFT, then GAMEPLAY / GRAPHICS / CONTROLS / MEDIA VIEWER / AUDIO / RETRO FX (Selected) tabs. (Light/dark moved from right to left per locked content change.)
- No description band.
- Three stacked settings panels:
  - RETRO FX MASTER ENABLE (default-border): left side header + description + status note; right side ON (Selected) / OFF (Default) buttons row + APPLY button (Selected, no arrow flourishes).
  - UI (default-border): text-only panel with header + multi-sentence description.
  - UI CHROME (default-border outer): header + 2 slider sub-panels (CHROME PIXELATION + CHROME DITHERING, each with left label/description + right slider with handle at 0 + value display + caption).
- **No bottom warning notification** (locked removal). No decorative arrow flourishes.
- Interactivity:
  - Top bar single-actions as standard; settings cog is current/selected.
  - Theme toggle group `ThemeMode`: Sun Default, Moon Selected; mutually exclusive; drives light/dark UI mode if supported, otherwise placeholder.
  - Settings tab group: GAMEPLAY, GRAPHICS, CONTROLS, MEDIA VIEWER, AUDIO, RETRO FX Selected; mutually exclusive; drives `CurrentTab`.
  - RETRO FX MASTER ENABLE toggle group: ON Selected, OFF Default; drives master enable setting.
  - APPLY commits Retro FX changes.
  - CHROME PIXELATION and CHROME DITHERING sliders drive numeric Retro FX settings and update value labels.
- Label vs button:
  - Labels: panel headers, descriptions, status note, slider labels/descriptions/value captions.
  - Buttons/toggles/sliders: top bar controls, Sun/Moon toggles, settings tabs, ON/OFF toggles, APPLY, slider handles/tracks.
- Icon manifest:
  - Existing flat icons: `gear.png`, `sun` if present or generate via M1, `moon.png`.
  - Needs M1 if exact reference style is absent: any tab icons, slider handle glyph if not pure Slate.

**Load Game**
- **No top bar.**
- Top row: BACK button (Selected, far-left, with left chevron) + "LOAD GAME" title (centered, primary white, no gold tint).
- Filter row: Solo dropdown (Selected red, left) + "Showing Solo saves stored on this machine." status text (accent gray, middle) + "Page 1 / 1" indicator (right).
- 2×2 grid of save slot panels (each default-border):
  - Header "SAVE SLOT X" in accent gray.
  - Two-column body: 4 character portrait slots (silhouettes + role icons) on left + 5 metadata rows (Date / Difficulty / Stage / Score / Time, labels accent gray, values white) on right.
  - Bottom action row: PREVIEW (Default) + LOAD (Selected) + DELETE (Selected, with trash icon).
- Save data: Slot 1 VIOLENT difficulty 1,234,560 score; Slot 2 STANDARD 876,430; Slot 3 BRUTAL 654,210; Slot 4 HARMLESS 245,980 — with full date/stage/time data per V3.
- Bottom row: PREV (Default, far-left) + NEXT (Default, far-right) pagination buttons. No central skull.
- Interactivity:
  - BACK navigates back.
  - Solo dropdown filters save slots by party/mode source and drives `SaveModeFilter`.
  - Each save slot panel may be selectable; PREVIEW opens the save preview modal, LOAD loads the save, DELETE opens/executes delete confirmation.
  - PREV/NEXT page the save slot grid.
- Label vs button:
  - Labels: LOAD GAME title, status text, Page indicator, SAVE SLOT headers, metadata labels/values, role captions if any.
  - Buttons/dropdowns: BACK, Solo dropdown, PREVIEW, LOAD, DELETE, PREV, NEXT, clickable portrait/slot controls only if implementation supports them.
- Icon manifest:
  - Existing flat icons: `back_chevron.png`, `trash.png`, `pagination_left.png`, `pagination_right.png`.
  - Needs M1 if exact reference style is absent: role icons inside portrait silhouettes and dropdown chevrons.
  - Save character portraits/silhouettes are content art; use stubs if live save data lacks portraits.

**Run Summary**
- **No top bar.**
- Top row: "RUN SUMMARY" title (far-left, large) + 3 mini-stat panels centered (CHAG COUPONS 0 / ACHIEVEMENTS 0 / SECRET ACH. 0, each with icon and big value) + EVENT LOG button (Selected, far-right, with clipboard icon).
- Three-column body:
  - LEFT: stacked panels — RUN OUTCOME (flag icon + 3 stat rows: Stage Reached 1 / Score 0 / Time 00:00) + skull progression row (5 skull squares) + WEEKLY RANK / ALL TIME RANK split panel (Score N/A, Speed Run N/A in each column) + SEED LUCK (clover icon + "65 / 100 (Fortunate)") + 2×2 action button grid (GO AGAIN! Selected with refresh icon / CONTINUE Default with play icon / MAIN MENU Default with home icon / SAVE AND QUIT Default with save icon).
  - MIDDLE: character preview panel (3D character render preserved) + IDOLS panel (4 idol icons row) + INVENTORY panel (header with cube icon + GOLD 1,275 / DEBT 320 / NET WORTH 955 right side + 2×7 empty inventory slot grid).
  - RIGHT: stat-view tab row (STATS Selected / DAMAGE DEALT Default / DAMAGE RECEIVED Default) + Stats panel (9 stat rows: LEVEL 1 / Damage 1 / Attack Speed 1 / Attack Scale 1 / Accuracy 1 / Armor 1 / Evasion 1 / Luck 1 / Speed 1) + PROOF OF RUN panel (chain icon + "youtube.com/watch?v=run-proof-001" URL field + copy button) + SUBMIT SUSPICION OF CHEATING button (Default, with warning triangle icon).
- Interactivity:
  - EVENT LOG opens/toggles event log view.
  - Action button grid: GO AGAIN starts a new run; CONTINUE resumes/continues when allowed; MAIN MENU navigates to Main Menu; SAVE AND QUIT saves then exits to appropriate frontend state.
  - Stat-view toggle group `RunSummaryStatTabs`: STATS Selected, DAMAGE DEALT Default, DAMAGE RECEIVED Default; mutually exclusive; drives right-panel table.
  - Copy button copies proof URL. SUBMIT SUSPICION OF CHEATING opens report/submission flow.
  - Inventory/idol slots are informational unless the implementation exposes tooltips/details; if so, bind and document those handlers.
- Label vs button:
  - Labels: RUN SUMMARY title, mini-stat names/values, RUN OUTCOME header/stat labels/values, rank labels/values, SEED LUCK text, IDOLS/INVENTORY headers, currency labels/values, right-panel stat labels/values, proof URL field text when read-only.
  - Buttons/toggles: EVENT LOG, GO AGAIN, CONTINUE, MAIN MENU, SAVE AND QUIT, STATS/DAMAGE DEALT/DAMAGE RECEIVED tabs, proof copy button, SUBMIT SUSPICION OF CHEATING.
- Icon manifest:
  - Existing flat icons: `log_clipboard.png`, `refresh.png`, `play_triangle.png`, `home.png`, `save_floppy.png`, `cube_box.png`, `clover.png`, `link_chain.png`, `copy_clipboard.png`, `warning_triangle.png`.
  - Needs M1 if exact reference style is absent: coupons/achievement/secret-achievement mini-stat icons, skull progression squares.
  - Character render, idol icons, and inventory item art are content art and remain PNG-driven.

### 7.3 Icon manifest format

Output of Stage 1 icon audit at `C:\UE\T66\UI\icon_manifest.md`. Format suggested:

```markdown
# T66 UI Flat Redesign — Icon Manifest

## Top bar / system

| Icon | Status | Path |
|---|---|---|
| settings cog | present | C:\UE\T66\RuntimeDependencies\T66\UI\MainMenu\settings_gear_icon.png |
| globe | present | C:\UE\T66\RuntimeDependencies\T66\UI\MainMenu\language_globe_icon.png |
| ticket | present | ... |
| ... | ... | ... |

## Content / stat / role icons

| Icon | Status | Path / Generation Notes |
|---|---|---|
| trophy / laurel wreath | missing → generated | RuntimeDependencies\T66\UI\Icons\Flat\trophy_laurel.png |
| stopwatch / timer | missing → generated | RuntimeDependencies\T66\UI\Icons\Flat\stopwatch.png |
| helmet | partial | Existing helmet icon at <path>; differs in <how>; replaced by generated RuntimeDependencies\T66\UI\Icons\Flat\helmet.png |
| ... | ... | ... |
```

### 7.4 Helper API surface (FT66FlatStyle)

The full helper list is specified in Section 4.2. Reproduce as a quick-reference table in code as Doxygen comments or a `T66FlatStyle.h` summary block when implementing.

### 7.5 File paths

- This master plan: `C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md`
- Technical audit: `C:\UE\T66\UI\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md`
- V3 reference images: `C:\UE\T66\UI\Screen References\*.png`
- Icon manifest (Stage 1 output): `C:\UE\T66\UI\icon_manifest.md`
- Generated icons (Stage 1 output): `C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\*.png`
- New flat style code: `C:\UE\T66\Source\T66\UI\Style\T66FlatStyle.h` and `T66FlatStyle.cpp`
- Existing audit-referenced process docs now live under `C:\UE\T66\UI\Instructions\*.md`.

---

## 8. Quality Gates

### 8.1 Universal compile gate

Every stage delivery requires a clean compile:
```
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex
```

### 8.2 Screenshot verification

Use the existing capture script per audit:
```
.\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenName> -DelaySeconds 6 -CloseAfter
```

For Stage 1: capture Hero Selection. For Stage 2: capture each migrated screen. For Stage 3: capture all migrated screens for regression comparison.

### 8.3 Regression checks

After any change, spot-check that non-target screens still render. Suggested baseline checks: Main Menu opens; Overview opens; a minigame screen opens; gameplay HUD renders during a brief in-run capture.

### 8.4 Stage acceptance summary

- **Stage 1**: helpers exist + icon manifest produced + Hero Selection matches V3 reference + compile clean + no regression.
- **Stage 2 per screen**: helpers used + content reconciled + screenshot matches reference + compile clean + no regression on others.
- **Stage 3**: PNG chrome path removed + every migrated screen renders identical to its Stage 2 final + gameplay HUD unaffected + compile clean.

### 8.5 What does not count as verification

Per the audit: "compile success is not visual verification." Always include screenshot capture and visual diff against the V3 reference (Stage 1, 2) or the Stage 2 baseline (Stage 3).

---

## 9. Open Items and Future Work

These are not blockers for the staged work but should be tracked.

- V3 reference creation for screens still pending (Section 7.1 second list). When references are produced, add corresponding per-screen specs to Section 7.2.
- Decision on whether to deprecate `FT66Style` entirely after Stage 3 or keep it as a minimal procedural fallback. Defer until Stage 3 retrospective.
- Decision on font policy if localization requires non-Latin script support beyond Jersey10's coverage. Engine fallbacks already handle this per audit; revisit if any localization session reveals coverage gaps.
- Whether the chrome retainer can be fully removed or just bypassed for chrome (some non-chrome usages may still benefit from it). Audit each usage during Stage 3.
- Performance characterization of flat helpers vs PNG helpers. Not expected to be a problem, but worth measuring once Stage 1 is delivered.

---

End of master plan.

