# T66 UI Fidelity Loop

This document defines the operational loop for migrating any T66 UI screen to the flat redesign with reference-image fidelity. It is the procedural counterpart to the master plan (`UI_FLAT_REDESIGN_REFERENCE.md`) and the technical handoff (`Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md`).

Root `ART_DIRECTION.md` declares FriendSlop as the active 3D/world direction and future rubber material target. This loop remains a 2D UI fidelity process; it does not apply the 3D rubber material contract to Slate chrome, FriendslopStyle raster chrome, icons, or content artwork.

Codex follows this loop for every Stage 1 and Stage 2 screen migration. `AGENTS.md` enforces the loop as the acceptance gate.

---

## 1. Purpose

A UI screen migration is **not complete** when:

- The code compiles.
- A screenshot exists.
- The implementation "generally matches" the reference.

A UI screen migration **is complete** when:

- All legacy PNG-composited chrome paths reachable in the screen are removed.
- The captured screenshot at 1920×1080 matches the V3 reference per the screen's verification checklist, with zero Fail items.
- All Unsure items have been reviewed and either auto-resolved or accepted as content deltas by Pablo.
- Every interactive element listed in the per-screen Interactivity spec has a bound handler, correct toggle-group metadata, and passes manual interaction verification.
- A pass log documents every iteration and the final state.

The loop is the mechanism that takes us from "compiles and looks roughly right" to that complete state.

---

## 2. Core Concepts

### 2.1 Chrome vs content artwork

**Chrome** is the visual structure of the UI: panels, borders, button plates, dividers, tab plates, dropdown shells, input field frames, decorative framing, scrollbars, progress bar tracks, separator lines.

In the flat redesign, **all chrome is pure Slate** built via `FT66FlatStyle` helpers. No PNG plates, no chrome retainer wrapping, no glow material, no decorative chrome artwork.

Flat chrome surfaces include two global retro visual treatments: subtle fill pixelation and deterministic stepped edge distortion. Both are driven by `FT66RetroFXSettings` and applied inside `FT66FlatStyle`; they affect chrome fill/border rendering only and do not change layout bounds, hit rects, semantic state, text, icons, or content artwork. Settings infrastructure is in place; user-facing controls for these values are deferred.


Content artwork **remains PNG-driven**. The migration does not touch how content is rendered, only how it's framed.

### 2.2 Legacy cleanup precondition

Migration is destructive of legacy paths, not additive on top of them. Before any flat construction begins for a screen, all legacy chrome paths reachable in that screen must be removed or routed away. See Section 5 for the audit baseline and Section 7 for the cleanup step.

This is the most common failure mode in the current state: building flat chrome alongside legacy chrome, leaving the legacy paths active, and surfacing rendering artifacts (e.g., the magenta scrollbar block) whose root cause is in unaudited helper code.

### 2.3 Structural verification is necessary but not sufficient

LLM visual comparison is unreliable for fine-grained chrome deviations (slightly wrong color, slightly wrong panel width, missing border). LLM structural verification against a tagged widget dump is reliable: "panel `HeroSelection.RightPanel` border color is `DefaultBorder`" is a string compare, not a visual judgment.

The loop converts visual claims into structural claims wherever possible. The Slate dump JSON is the source of truth for structural claims. The captured screenshot is the source of truth for purely visual judgments (content artwork resemblance, font character, anti-aliasing quality) that the dump can't capture.

A clean structural report does not mean a screen visually matches the reference.
For FriendslopStyle work, `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
owns the screen-specific exception: Codex produces capture/dump/contact evidence
for user visual review and reports only the wiring/functionality gate as
PASS/FAIL. `VerifyUIFidelity.py` may still be used as a structural or wiring
helper, but it is not the Friendslop final gate.

### 2.4 Content stubs via imagegen

When the V3 reference shows content artwork (portrait, character art, illustration) and the production content pipeline doesn't yet provide it, Codex generates a stub via imagegen that closely reproduces the reference imagery. Stubs go in a designated path and are flagged in code for later replacement. See Section 9 for the policy.

For FriendslopStyle work, the Friendslop authority file owns the stricter
imagegen route: generation must run through separate local Codex CLI workers
using account-backed built-in imagegen, not through the main Codex app chat and
not through `OPENAI_API_KEY` API scripts.
After each CLI worker result is collected, close/archive that worker thread so
it does not remain as open background work in Codex.

This means: a "missing skin portrait" never becomes a Slate-rendered colored box with initials. It becomes a generated PNG that visually matches the reference closely enough that both structural verification ("portrait slot exists, right size, right position") and visual sanity check pass.

### 2.5 Pass log

Every iteration produces a pass log entry: what failed, what was tried, what changed, what remains. The log is consulted by the iteration cap and escalation logic. It also makes Pablo's review possible without requiring him to reconstruct what happened from screenshots alone.

---

## 3. Required Infrastructure

These three pieces of infrastructure must exist before iteration begins on any screen. They are built once and used across all screens. Stage 1 (Hero Selection pilot) includes building them as part of its scope.

### 3.1 `T66.UI.DumpScreen` runtime console command

A console command that walks the current `UT66ScreenBase` widget tree and writes JSON to a specified path.

**Invocation:**
```
T66.UI.DumpScreen Path=<output_path.json>
```

**Output JSON schema** (per node):

```jsonc
{
  "screen": "HeroSelection",
  "capture_timestamp": "2026-05-11T14:32:00Z",
  "viewport": { "width": 1920, "height": 1080 },
  "widgets": [
    {
      "tag": "HeroSelection.TopRow.BackButton",          // null if untagged
      "type": "FT66FlatButton",                          // C++ type
      "source": "T66HeroSelectionScreen.cpp:412",        // source file:line of construction
      "parent_tag": "HeroSelection.TopRow",
      "child_index": 0,
      "child_count": 2,
      "geometry": {
        "absolute_x": 32,
        "absolute_y": 24,
        "width": 120,
        "height": 48,
        "normalized": { "x": 0.0167, "y": 0.0222, "w": 0.0625, "h": 0.0444 },
        "desired_size": { "width": 120, "height": 48 },
        "layer_id": 12,
        "visibility": "Visible",
        "enabled": true,
        "hovered": false,
        "pressed": false
      },
      "border": {                                        // if SBorder or has a border brush
        "brush_resource": "(flat)",                      // "(flat)" for FSlateColorBrush, else asset path
        "brush_draw_type": "Box",
        "tint": "#E1232D",
        "background_color": "#1C0E10",
        "margin": { "left": 2, "top": 2, "right": 2, "bottom": 2 }
      },
      "text": {                                          // if STextBlock
        "content": "BACK",
        "font_path": "Jersey10-Regular.ttf",
        "font_size": 20,
        "color": "#FF505F",
        "letter_spacing": 0
      },
      "button_state": "Selected",                        // FT66FlatStyle button state when set
      "interactivity": {
        "has_click_handler": true,                        // true when OnClicked/OnPressed/equivalent is bound
        "hover_capable": true,                             // true when enabled flat helper hover visuals are supported
        "toggle_group": "GenderToggle"                    // empty when not in a toggle group
      },
      "image": null,                                     // populated for SImage with brush_resource etc.
      "t66_metadata": {                                  // optional, attached by FT66FlatStyle helpers
        "intended_role": "back_button",
        "intended_state": "Selected",
        "has_click_handler": true,
        "hover_capable": true,
        "toggle_group": "GenderToggle",
        "is_label": false
      }
    }
    // ... more widgets
  ]
}
```

**Coverage requirements:**

- All widgets in the active screen's tree (recursive from the screen root).
- Tagged widgets (those with `t66_metadata.tag`) are required; untagged widgets are included but flagged.
- Geometry in both absolute pixel coordinates and normalized 1920×1080 coordinates.
- Brush resource paths when the brush is asset-backed; `"(flat)"` when it's a Slate-native color brush.
- Hex color values for every color (tint, background, text, border).
- Text content verbatim including any localized substitution result.
- Interaction metadata for every tagged button or interactive widget: `has_click_handler`, `hover_capable`, and `toggle_group`.
- Label metadata for every tagged text widget: `is_label` is true only for widgets constructed through `FT66FlatStyle::MakeFlatLabel` or a label-only wrapper.

**Implementation hooks:**

- `UT66ScreenBase::DumpToJson(const FString& OutputPath)` virtual method.
- A `FT66WidgetTreeWalker` helper that recursively descends and emits per-widget records.
- `FT66FlatStyle::MakeFlat*` helpers attach `t66_metadata` to the constructed widget so the walker can read it.

**Fallback:** `Slate.Debug.LogAllWidgets File=...` exists in UE5.7 but does not cover brush resource, tint, text, font, or button state. Use it only as a sanity check that the tree exists; the structural verification depends on `T66.UI.DumpScreen`.

### 3.1.1 `T66.UI.DumpWidget` runtime console command

Runtime HUD, overlay, component, and in-world UI often live outside the `UT66ScreenBase`/`UT66UIManager` screen pipeline. These widgets use the same structural-preservation procedure as no-reference frontend screens, but their dump root is acquired with `T66.UI.DumpWidget` instead of `T66.UI.DumpScreen`.

**Invocation:**
```
T66.UI.DumpWidget Class=<UClassOrSlateType> Path=<output_path.json>
T66.UI.DumpWidget Tag=<FNameTag> Path=<output_path.json>
T66.UI.DumpWidget ViewportIndex=<n> Path=<output_path.json>
T66.UI.DumpWidget Actor=<ActorName> Path=<output_path.json>
```

**Automation flag:**
```
-T66AutoDumpWidget=<Target>:<output_path.json>
```

`<Target>` is one of:

- `Class=<UClassName>`: finds the first active viewport `UUserWidget` instance matching that class or one of its superclasses, for example `Class=UT66GameplayHUDWidget`.
- `Class=<SlateType>`: if no active `UUserWidget` class matches, searches active viewport roots for the first Slate descendant whose `GetTypeAsString()` matches, for example `Class=ST66LeaderboardPanel`.
- `Tag=<FNameTag>`: searches active viewport roots for the first Slate descendant whose Slate tag or `FT66FlatStyle` metadata tag matches.
- `ViewportIndex=<n>`: dumps the nth active viewport `UUserWidget` root, sorted by viewport z-order and then class/name for deterministic automation.
- `Actor=<ActorName>`: finds the actor in the current world, dumps the first `UWidgetComponent` widget tree attached to it, and uses the same JSON schema. Current `AT66WorldInteractableBase` prompts are HUD-rendered rather than widget-component-rendered, so this target falls back to the active `UT66GameplayHUDWidget` when the actor inherits from `AT66WorldInteractableBase`.

`T66.UI.DumpWidget` calls the same `FT66WidgetTreeWalker` as `T66.UI.DumpScreen`. The output JSON schema is intentionally identical: `screen`, `capture_timestamp`, `viewport`, and `widgets` are present so `Scripts\VerifyUIFidelity.py` can consume widget dumps without modification.

If the selected dump root has no Slate tag and no `FT66FlatWidgetMetadata` tag, `T66.UI.DumpWidget` assigns a transient synthetic root tag of `<DumpName>.Root` before walking the tree. This keeps legacy HUD/overlay baselines verifier-compatible without requiring verifier changes or widget migration first.

**Capture helper:**
```
.\Scripts\CaptureT66UIWidget.ps1 -Target "Class=UT66GameplayHUDWidget" -Output <baseline_capture.png> -Dump <baseline_dump.json> -CaptureMode hudreview
.\Scripts\CaptureT66UIWidget.ps1 -Target "Tag=MainMenu.Right.LeaderboardPanel" -FrontendScreen MainMenu -Output <baseline_capture.png> -Dump <baseline_dump.json>
```

### 3.1.2 Frontend tag-click automation

Frontend interaction proof should use tagged Slate widgets instead of OS-level mouse injection or hardcoded pixel coordinates. The runtime flag is:

```
-T66AutoClickTag=<FNameTag>
```

Optional delay:

```
-T66AutoClickDelay=<seconds>
```

`-T66AutoClickTag` resolves the requested tag with the same active-viewport Slate/FlatStyle metadata resolver used by `T66.UI.DumpWidget`. In non-shipping builds it finds the tagged `SButton` at or below the resolved widget, validates that the button is visible, enabled, and laid out, then simulates the click through Slate.

The frontend capture helper exposes this as:

```
.\Scripts\CaptureT66UIScreen.ps1 -Screen MainMenu -Modal QuitConfirmation -ClickTag "QuitConfirmation.QuitButton" -ClickDelaySeconds 2.5 -WaitForExit
```

Use `-WaitForExit` for Quit Game proof because a successful quit exits the process before a post-click screenshot exists. For visual interaction proof, capture or dump before the click, or set explicit delays so dump/screenshot happen before the tagged click.

For gameplay HUD and overlay captures, the helper launches `/Game/Maps/GameplayLevel` by default and reuses the existing gameplay automation modes through `-T66GameplayAutoCapture=<mode>`. For frontend-embedded components, pass `-FrontendScreen <ScreenName>` and the helper uses the frontend capture path.

**HUD/overlay/in-world distinctions:**

- **HUD widgets** are persistent gameplay viewport widgets such as `UT66GameplayHUDWidget`. Use `Class=<WidgetClass>` or a tagged descendant. The dump should be taken after any automation state setup has made the desired HUD mode visible.
- **In-world UI** may be `UWidgetComponent`-backed or HUD-routed. For widget components, use `Actor=<ActorName>`. For current T66 interactable prompts, the actor target falls back to the HUD-rendered prompt path because `AT66WorldInteractableBase` drives `UT66GameplayHUDWidget::ShowInteractionPrompt()` rather than owning a widget component.

**Known overlay trigger modes for automation:**

- Gameplay HUD review: `-T66GameplayAutoCapture=hudreview`, target `Class=UT66GameplayHUDWidget`.
- Inventory inspect HUD: `-T66GameplayAutoCapture=inventory`, target `Class=UT66GameplayHUDWidget`.
- Full map HUD: `-T66GameplayAutoCapture=fullmap`, target `Class=UT66GameplayHUDWidget`.
- Idol altar overlay: `-T66GameplayAutoCapture=idol`, target `Class=UT66IdolAltarOverlayWidget`.
- Lab overlay: `-T66GameplayAutoCapture=lab`, target `Class=UT66LabOverlayWidget`.
- Crate overlay: `-T66GameplayAutoCapture=crate`, target `Class=UT66CrateOverlayWidget`.
- Collector overlay: `-T66GameplayAutoCapture=collector`, target `Class=UT66CollectorOverlayWidget`.
- Casino/gambler overlays: `-T66GameplayAutoCapture=casinoshop` or `casinogambling`, target the active casino/gambler class. Casino Alchemy is retired and should not be captured as an active UI surface.
- World interactable prompt: `-T66GameplayAutoCapture=worldprompt`, target `Actor=T66WidgetDump_WorldInteractablePrompt`.
- Cowardice prompt: use a natural cowardice-gate trigger or a debug trigger that calls `OpenCowardicePrompt`; then target `Class=UT66CowardicePromptWidget`.
- Loading screen: `-T66GameplayAutoCapture=loading`, target `Class=UT66LoadingScreenWidget`; natural gameplay/frontend transitions can also be captured while the widget is active.

### 3.2 `Scripts\VerifyUIFidelity.py`

A Python script that compares a captured screen state against its verification checklist and produces a structured report plus an annotated contact sheet.

**Invocation:**
```
python Scripts\VerifyUIFidelity.py ^
  --reference C:\UE\T66\UI\Screen References\hero_selection.png ^
  --capture C:\UE\T66\Saved\Codex\UI\HeroSelection\pass5.png ^
  --dump C:\UE\T66\Saved\Codex\UI\HeroSelection\pass5_dump.json ^
  --checklist C:\UE\T66\UI\Checklists\hero_selection.md ^
  --output C:\UE\T66\Saved\Codex\UI\HeroSelection\pass5_report.md ^
  --contact-sheet C:\UE\T66\Saved\Codex\UI\HeroSelection\pass5_contact.png
```

For checklists that include a visual gate:

```
python Scripts\VerifyUIFidelity.py ^
  --reference <reference.png> ^
  --capture <capture.png> ^
  --dump <dump.json> ^
  --checklist <checklist.md> ^
  --output <report.md> ^
  --contact-sheet <contact.png> ^
  --visual-scorecard <visual_scorecard.md>
```

**Outputs:**

1. **Report** (markdown). One line per checklist item with verdict: `PASS` / `FAIL` / `UNSURE`. For FAIL, the line includes the expected and actual values from the dump. For UNSURE, a reason ("requires visual judgment", "tag not found in dump", "dimension comparison ambiguous").

2. **Contact sheet** (PNG). Side-by-side reference and capture, normalized to the same height. Failed checklist items are annotated as overlay boxes on the capture with brief labels ("border DefaultBorder expected, got DisabledBorder"). Unsure items get a different color overlay.

3. **Summary**: counts of PASS/FAIL/UNSURE, plus the screen name, capture filename, and timestamp.

**Comparison logic:**

- Structural items (color, text content, widget presence, button state, border brush type): compare dump value to checklist expected value. Binary PASS/FAIL.
- Geometric items (position, size, proportion): compare dump value to checklist expected range (with tolerance). PASS if within tolerance, FAIL with deviation amount otherwise.
- Containment items use `contained_in=<ParentTag>` and optional pixel tolerance
  to verify a child absolute rect stays inside its parent absolute rect.
  Checklist authors may specify parent insets with
  `contained_in=<ParentTag> inset=<left>,<top>,<right>,<bottom>` when a row or
  control must fit inside a panel's content area rather than the outer border.
- Visual gate items use `visual_gate=PASS` and require a scorecard file passed
  through `--visual-scorecard`. The scorecard must include `Result: PASS` to
  pass. Missing scorecard, missing verdict, or `Result: FAIL` blocks the report
  even when all structured widget items pass.
- Visual items (artwork resemblance, font character, overall composition feel): mark UNSURE if the dump can't verify and a visual judgment is required. Codex reviews these qualitatively per iteration; persistent UNSUREs escalate to Pablo.

**Normalization:**

- The reference image is normalized to the capture's resolution (typically scaling 1672×941 reference → 1920×1080 capture) before contact sheet generation.
- All geometric comparisons happen in normalized 1920×1080 coordinates derived from the dump, not in raw pixel coordinates from the screenshot.

### 3.3 Widget tagging convention in `FT66FlatStyle`

Every `FT66FlatStyle::MakeFlat*` helper accepts an optional `Tag` parameter (an `FName` or `FString`). When provided, the tag is attached as widget metadata that `T66.UI.DumpScreen` reads back out.

**Tag naming convention:**

- Format: `<ScreenName>.<RegionPath>.<ElementName>`
- Examples:
  - `HeroSelection.TopRow.BackButton`
  - `HeroSelection.TopRow.HeroCarousel`
  - `HeroSelection.TopRow.HeroCarousel.Portrait.3` (numeric suffix for repeated children)
  - `HeroSelection.LeftColumn.SkinsPanel`
  - `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default`
  - `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer`
  - `HeroSelection.MiddleColumn.CharacterPreview`
  - `HeroSelection.RightColumn.OuterPanel.StatsSubPanel`
  - `HeroSelection.BottomRow.SteamParty.Slot.1`
  - `HeroSelection.BottomRow.SteamParty.Slot.1.ReadyBadge`

**Tagging requirement:**

Every named element in a screen's per-screen spec (from master plan Section 7.2) **must have a tag**. The tag is what allows the verification checklist to reference it. Untagged widgets exist but can't be verified beyond their parent's expectations.

**Mechanism:**

- `FT66FlatStyle::MakeFlatPanel(Params, Children)` where `Params` includes `Tag`.
- `FT66FlatStyle::MakeFlatButton(Params)` where `Params` includes `Tag`.
- Tag is stored on the widget via `SetTag()` (`FName`) for native Slate widgets and via a `t66_metadata` payload for FT66 widgets.
- `FT66WidgetTreeWalker` reads the tag back during dump.

### 3.4 `FT66ButtonParams::bUseGlow` default

Codex reported `bUseGlow` defaults to `true` in the existing `FT66ButtonParams`. Two paths to fix this:

1. **Preferred:** Add a parallel `FT66FlatButtonParams` struct used only by `FT66FlatStyle`. It defaults `bUseGlow = false` and doesn't expose a way to enable glow. The legacy `FT66ButtonParams` stays as-is for non-migrated screens.

2. **Alternative:** Flip the existing default to `false` and audit every caller that depended on glow. Higher risk of breaking unmigrated screens.

The loop assumes option 1.

### 3.5 Interactivity Specification and Verification

Every per-screen spec in master plan Section 7.2 must include an **Interactivity** subsection. The subsection is authored from the reference and the expected UX, not from the current implementation.

The Interactivity subsection must list:

- **Toggle groups:** group name, member tag list, whether the group is mutually exclusive, initial selection tag as shown in the mockup, and the state variable it drives. Example: `GenderToggle` members `ChadButton`, `StacyButton`, mutually exclusive `true`, initial `ChadButton`, drives `GenderSelection`.
- **Single-action buttons:** tag and action description. Examples: `StartRun`, `NavigateBack`, `OpenChallenges`, `OpenMods`.
- **Dropdowns:** tag, options source, and current selection state variable. Example: Difficulty dropdown, options source `ET66Difficulty`, drives `SelectedDifficulty`.

State assignments in the spec text mean **initial state in the mockup**, not a permanent visual property. If the user clicks another mutually-exclusive option, the selected visual state must move with the state variable.

The verification checklist gains an **Interactivity** section. It can assert:

```
- [ ] <Tag> | has_click_handler=true
- [ ] <Tag> | hover_capable=true
- [ ] <Tag> | toggle_group=<GroupName>
```

`FT66FlatStyle` provides toggle-group-aware helpers. Toggle groups attach `toggle_group` metadata and are wired so a clicked member drives one selected state at a time through the screen's state variable. Custom-content toggles may use `MakeFlatToggleGroupButton`; simple label/icon toggles may use `MakeFlatToggleGroup`.

Flat helper hover is a transient visual overlay only. Enabled interactive helpers render hover with `HoverBorder #1FB358`, `HoverText #4FD088`, and `HoverFill #0E140E`, then return to their intended semantic state on unhover. Disabled controls ignore hover and must not report `hover_capable=true`.

After the automated verifier passes, a **Manual Interaction Verification** step runs before DONE. The interactivity spec drives the manual test list. Pablo records `Works`, `Doesn't Work`, or `N/A` for each item. Any `Doesn't Work` result becomes a FAIL and the loop continues.

---

## 4. The Iteration Loop

The loop has nine steps plus Manual Interaction Verification. Step 0 (legacy cleanup) and Step 0.5 (reference geometry extraction) are preconditions that run once per screen before construction begins. Steps 1–7 iterate until the automated verifier passes, then Manual Interaction Verification runs before DONE.

Before capture, the target screen must be resolvable by automation. `-T66FrontendScreen=<ScreenName>` must either resolve directly to the target screen, or resolve to the parent screen and drive the documented tab/category state through the screen's command-line override (for example `Overview` -> `AccountStatus` with the Overview tab active). Unknown screen names must fail loudly; `CaptureT66UIScreen.ps1` must return nonzero rather than producing a misleading fallback capture.

### Step 0: Legacy chrome cleanup (precondition, runs once per screen)

This must complete before any flat construction begins for the screen.

**0.1 Audit.** Run the validation regex from Section 5 against the screen's source files and any helpers it references. Produce a removal checklist:

```
rg -n "SourceAssets/UI/Reference|RuntimeDependencies/T66/UI/Reference|MakeReference|Get.*ReferenceScrollBarStyle|ST66Reference|ST66RetroUIRetainedSurface|M_UI_Glow|M_UI_RetroRetainer|MakeRetroUIChromeSurface|MakeRetroUIChromeOverlay" Source\T66\UI\Screens\<ScreenFolder>
```

**0.2 Remove or route away.** For each match in the screen's reachable code:

- Delete the call if it's only chrome (e.g., `MakeReferenceSlicedPlateButton` for a button frame).
- Route to `FT66FlatStyle` equivalent if there's a clear flat replacement.
- Flag with `// TODO(content-stub)` if it's content artwork that needs imagegen stub treatment (Step 0.4).
- Leave alone only if it's verified non-chrome (e.g., a portrait UTexture load, a hero render brush).

**0.3 Confirm `bUseGlow = false`.** Every flat-style construction in the screen must use `FT66FlatButtonParams` (or equivalent flat-only struct). Legacy `FT66ButtonParams` calls with `bUseGlow = true` are removed.

**0.4 Inventory content stubs needed.** From the V3 reference, identify content artwork that requires imagegen stubs. List them in the pass log under "Content Stubs Needed" with target paths under `SourceAssets/UI/ContentStubs/<ScreenName>/`. See Section 9 for the imagegen workflow and record every stub in `UI/content_stubs_registry.md`.

**0.6 Compile.** After cleanup, the project must compile cleanly. Compile failures here indicate the cleanup removed something a non-chrome path depended on — investigate before continuing.

**Acceptance for Step 0:** validation regex returns no chrome-related matches in the screen's reachable code, project compiles, content stub inventory exists in the pass log.

### Step 0.5: Reference geometry extraction (pre-implementation)

This must complete before implementation begins for the screen.

Measure the screen's reference image visually at its native resolution and normalize every measured rectangle to the 1920x1080 basis used by the dump and verifier. Extract bounding boxes for every named region in the per-screen spec: panels, sub-panels, buttons, dropdowns, portrait slots, icon containers, content slots, and named labels when their placement drives composition.

Save the output table at:

```
C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md
```

Render a geometry sanity overlay after the raw extraction. Draw every extracted bounding box over the reference image with the tag label and save it beside the table:

```
C:\UE\T66\UI\Geometry\<screen>_reference_geometry_overlay.png
```

Reusable overlay generation script:

```
python Scripts\GenerateUIGeometryOverlay.py --geometry C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md --output C:\UE\T66\UI\Geometry\<screen>_reference_geometry_overlay.png
```

Visually inspect the overlay before implementation. Confirm that boxes match the visual regions intended, not just the tightest visible pixels:

- Columns must not claim empty padding as content unless that padding is part of the perceived region.
- Rows must not compress whitespace that the reference uses for visual breathing room.
- Cluster boundaries must match the perceived visual group, not a tight box around only the interior content.

If any box is too tight, expand it to include the intended breathing room. The corrected table becomes canonical; the raw extraction is only a measurement draft.

The geometry table is the source of truth for checklist geometry expectations. Populate checklist coordinate ranges and tolerances from this table instead of estimating positions while authoring the checklist. If a reference element is ambiguous, mark the row as approximate and include the visual reason.

**Acceptance for Step 0.5:** geometry table exists, overlay exists, both were visually sanity-checked, the table covers every named element from the per-screen spec, records the reference image path and native resolution, and gives normalized 1920x1080 bounding boxes with an explicit measurement tolerance.

### Step 1: Implementation pass (build per spec)

Using `FT66FlatStyle` helpers, construct the screen's `BuildSlateUI()` per the per-screen spec in master plan Section 7.2 and the Step 0.5 geometry table.

**1.1 Tag every named element.** Every panel, sub-panel, button, dropdown, label, icon container, and slot specified in the per-screen spec gets a tag per Section 3.3 conventions.

**1.2 Generate content stubs and missing icons.** For each item from the Step 0.4 inventory, generate via imagegen per Section 9 and save to the target path. For every icon generated via imagegen, first crop the exact icon region from the screen reference image and use that crop as visual context. Wire the stubs/icons into the screen via the existing runtime texture access pattern.

**1.3 Wire live data.** Hook the screen's dynamic content (player name, stat values, scores, dates, ticket counts, etc.) to the existing subsystems. Placeholder content from the spec is used only when no live value is available.

**1.4 Compile.**

### Step 2: Capture

```
.\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenName> -Output <pass_N_capture.png> -DelaySeconds 6 -ExtraArgs @("-T66AutoDumpScreen=<pass_N_dump.json>")
```

**Standardized invocation.** The flag is `-Output`, not `-OutputPath`. Captures go to `Saved\Codex\UI\<ScreenName>\pass_<N>_capture.png`. Always pass `-T66AutoDumpScreen=<dump_path>` through `-ExtraArgs` so capture and dump come from the same scripted run.

If the screen is a tab/category inside a parent screen, use the canonical Stage 2 automation name from the master plan and resolver audit. Examples: `Overview`, `History`, `Diplomas`, `Drugs`, `SteamAchievements`, `SettingsRetroFX`, `LoadGame`.

For HUD, overlay, in-world, or shared component structural-preservation work, use the widget capture helper instead:

```
.\Scripts\CaptureT66UIWidget.ps1 -Target "Class=<WidgetClassOrSlateType>" -Output <baseline_capture.png> -Dump <baseline_dump.json> -CaptureMode <mode>
```

Use `-FrontendScreen <ScreenName>` when the component is embedded in a frontend screen. Otherwise the helper launches `/Game/Maps/GameplayLevel` and uses `-T66GameplayAutoCapture=<mode>` to trigger the HUD or overlay state.

### Step 3: Slate dump

```
T66.UI.DumpScreen Path=Saved\Codex\UI\<ScreenName>\pass_<N>_dump.json
```

The recommended path is the automated `-T66AutoDumpScreen` flag from Step 2. Manual console invocation is only a fallback for interactive debugging. The dump represents the same widget state that the capture rendered.

For non-screen widgets, use:

```
T66.UI.DumpWidget Class=<WidgetClassOrSlateType> Path=Saved\Codex\UI\<WidgetName>\baseline_dump.json
T66.UI.DumpWidget Tag=<FNameTag> Path=Saved\Codex\UI\<WidgetName>\baseline_dump.json
T66.UI.DumpWidget Actor=<ActorName> Path=Saved\Codex\UI\<WidgetName>\baseline_dump.json
```

The recommended scripted path is `-T66AutoDumpWidget=<Target>:<dump_path>` through `CaptureT66UIWidget.ps1`, so the screenshot and dump come from the same automation run.

### Step 4: Verify

```
python Scripts\VerifyUIFidelity.py ^
  --reference C:\UE\T66\UI\Screen References\<screen_ref>.png ^
  --capture Saved\Codex\UI\<ScreenName>\pass_<N>_capture.png ^
  --dump Saved\Codex\UI\<ScreenName>\pass_<N>_dump.json ^
  --checklist C:\UE\T66\UI\Checklists\<screen>_checklist.md ^
  --output Saved\Codex\UI\<ScreenName>\pass_<N>_report.md ^
  --contact-sheet Saved\Codex\UI\<ScreenName>\pass_<N>_contact.png ^
  --visual-scorecard Saved\Codex\UI\<ScreenName>\pass_<N>_visual_scorecard.md
```

Pass `--visual-scorecard` when the checklist contains a `visual_gate=PASS`
item. Outputs: report + contact sheet. Counts of PASS/FAIL/UNSURE and the
scorecard verdict are recorded in the pass log.

### Step 5: Triage

Codex reads the report and categorizes each non-PASS item:

- **Auto-fixable FAIL:** the deviation is objective and the fix is mechanical (wrong hex color, wrong text content, missing widget, wrong button state, wrong tag attachment). Auto-fix.
- **Code-fixable FAIL with judgment:** the deviation is objective but the fix requires choice (wrong panel width by 50px — adjust to what value exactly? wrong child count — add or remove which?). Codex makes a reasoned fix and notes the reasoning in the pass log.
- **UNSURE requiring visual review:** the verification can't determine if it's correct (font character, artwork match, subjective proportion). Note in the pass log; do not block iteration on these.
- **Visual gate FAIL:** the structured assertions may pass, but the
  reference/capture/contact sheet does not meet the scorecard. Treat this as a
  blocking FAIL and continue visual correction.
- **Unfixable in this pass:** the deviation requires asset generation, backend wiring, or external decision. Note in the pass log; flag for Pablo if it blocks fidelity.

For generated-raster visual screens such as FriendslopStyle, do not collapse the
screen judgment into this verifier item list. The verifier still reports
`PASS` / `FAIL` / `UNSURE` for structured checklist rows, but the visual
iteration uses a separate ordered process:

1. Check the screen's small set of visual families and mark each family visual
   `PASS` or visual `FAIL`.
2. For every visual `FAIL` family, check all elements inside that family and
   mark each element visual `PASS` or visual `FAIL`.
3. Launch one approved imagegen worker per visual `FAIL` family. That worker
   generates the sheet/assets for all visual `FAIL` elements in that family.
   After collecting the worker result and artifacts, close/archive the worker
   thread before launching additional unrelated work.
4. Implement every regenerated family element onto the screen.
5. Run a layout `PASS`/`FAIL` pass for the same families and correct layout
   failures until fixed or blocked.
6. Run a wiring `PASS`/`FAIL` pass for the same families and correct wiring
   failures until fixed or blocked.

Visual `PASS` means no image regeneration is needed. Visual `FAIL` means image
regeneration is required in that iteration. Layout and wiring have their own
`PASS`/`FAIL` gates after generated assets are implemented. A clean structured
verifier report is not a visual screen pass when a generated-raster family
scorecard still fails.

### Step 6: Apply fixes

Implement the auto-fixable and judgment fixes from Step 5. Commit the changes.

### Step 7: Re-capture and re-verify

Loop back to Step 2 with iteration counter incremented.

### Manual Interaction Verification

Run after the latest automated verifier report has zero FAIL and zero UNSURE, or all remaining UNSURE items have already been accepted as content deltas, and any required visual scorecard is `Result: PASS`.

Create or update:

```
Saved\Codex\UI\<ScreenName>\manual_interaction_checklist.md
```

The checklist must enumerate every toggle group, single-action button, and dropdown from the per-screen Interactivity spec. Pablo fills each item with `Works`, `Doesn't Work`, or `N/A`. If any item returns `Doesn't Work`, treat it as a FAIL, log it in the pass log, fix the behavior, and repeat capture/dump/verify/manual review as needed.

### Termination

After each iteration, check termination conditions before continuing:

**Terminate as DONE if:** zero FAIL items and zero UNSURE items in the latest report, any required visual scorecard has `Result: PASS`, every generated-raster visual family is visual `PASS`, every regenerated family asset is implemented, layout and wiring are `PASS`, and Manual Interaction Verification has no `Doesn't Work` items.

**Terminate as ESCALATE if any of these:**
- Iteration counter reaches 5.
- The latest report's FAIL set is identical to the previous iteration's FAIL set (a stuck loop — the fixes are not landing).
- The latest report has zero FAIL but non-zero UNSURE that require visual review.
- The latest report has zero structured FAIL but the visual scorecard is missing
  or has `Result: FAIL`.
- A generated-raster screen has an unreviewed visual family, a visual `FAIL`
  family without a worker, generated assets not implemented, layout `FAIL`, or
  wiring `FAIL`.
- Codex encounters an unfixable deviation that blocks further fidelity (missing backend, missing asset Codex can't generate, ambiguous spec).

**On ESCALATE:** stop iterating, compile the pass log + final contact sheet + remaining FAIL/UNSURE list into a Pablo-review packet at `Saved\Codex\UI\<ScreenName>\pablo_review.md`, and request review before continuing.

---

## 5. Legacy Chrome Cleanup Reference

### 5.1 Validation regex

Run this against the screen's reachable code as the Step 0.1 audit:

```
rg -n "SourceAssets/UI/Reference|RuntimeDependencies/T66/UI/Reference|MakeReference|Get.*ReferenceScrollBarStyle|ST66Reference|ST66RetroUIRetainedSurface|M_UI_Glow|M_UI_RetroRetainer|MakeRetroUIChromeSurface|MakeRetroUIChromeOverlay" Source\T66\UI
```

Acceptance: no matches in the screen's reachable code after cleanup.

### 5.2 Known legacy chrome surface (per Codex audit)

These are the code paths classified as legacy PNG-composited chrome. Cleanup for a given screen consists of removing the reachable subset.

**Central shared helpers (`T66ScreenSlateHelpers`):**
- `ST66ReferenceHorizontalSlicedImage`
- `ST66ReferenceSlicedPlateButton`
- `ST66ReferenceProgressBar`
- `MakeReferenceHorizontalSlicedImage`
- `MakeReferenceMainMenuElementAssetPath`
- `MakeReferenceChromeElementAssetPath`
- `MakeReferenceLongPanelAssetPath`
- `MakeReferenceRedSquareButtonAssetPath`
- `MakeReferenceChromeButtonAssetPath`
- `MakeReferenceButtonAssetPath`
- `MakeReferenceSharedAssetPath`
- `GetReferenceSharedBrush`
- `MakeReferenceSharedBorder`
- `MakeReferenceSlicedPlateButton`
- `MakeReferenceProgressBar`
- `IsReferenceChrome*` helpers

**Master style (`T66Style.cpp`):**
- `LoadButtonTexturesOnce` (line 198 region) — loads red square button PNGs
- `LoadPanelTexturesOnce` — loads `main_panel_normal_square_variant.png`
- `MakeButton` PNG branch (when not using flat/no-bg/no-border params)
- `MakePanel` PNG branch
- `ST66RetroUIRetainedSurface` (line 768) and `MakeRetroUIChromeSurface`, `MakeRetroUIText`, `MakeRetroUIIcon`, `MakeRetroUIBackgroundImage`
- `M_UI_RetroRetainer` load (line 326)
- `M_UI_Glow` load (line 950)
- `FT66ButtonParams::bUseGlow` default — flat path uses `FT66FlatButtonParams` with `bUseGlow = false`

**Runtime texture/brush access:**
- `T66RuntimeUITextureAccess.cpp` line 317 — remap from `SourceAssets/UI/Reference/` to `RuntimeDependencies/T66/UI/Reference/`
- `T66RuntimeUIBrushAccess.cpp` line 191 — reference fallback resolution

**Overlay chrome:**
- `T66OverlayChromeStyle.cpp` line 25 — shared reference chrome lookup
- `T66OverlayChromeStyle.cpp` line 308 — `MakeReferenceHorizontalSlicedImage`

**Screen-private headers (per screen, only when migrating that screen):**
- `T66HeroSelectionScreen_Private.h` line 536 — `ResolveHeroSelectionSpriteBrush`, button/panel/row/carousel/dropdown/party-slot brush helpers
- `T66HeroSelectionScreen_Private.h` line 903 — `GetHeroSelectionReferenceScrollBarStyle`
- `T66SettingsScreen_Private.h` line 75 — `ResolveSettingsSpriteBrush`, settings button/panel/dropdown/progress/slider/scrollbar chrome
- `T66SettingsScreen_Private.h` line 1189 — `GetSettingsReferenceScrollBarStyle`

**Screen-level PNG chrome consumers** (replace during that screen's migration; full list in Codex's audit response):

### 5.3 What NOT to touch during cleanup

These are non-chrome and remain PNG-driven:

- Localization assets, font files, gameplay material instances unrelated to UI chrome.
- `UT66RetroFXSubsystem` for game-world post-process (separate from UI chrome retainer).

If a regex match falls in a non-chrome usage, leave it.

---

## 6. Verification Checklist Format

Each screen has a checklist file at `C:\UE\T66\UI\Checklists\<screen>_checklist.md`. The file is consumed by `VerifyUIFidelity.py` and is also human-readable.

### 6.1 Structure

The checklist has five core sections in order: **Structure**, **Geometry**, **Colors**, **Content**, **Interactivity**. Screen families may add stricter sections such as **Containment** and **Visual Gate** when the reference requires them.

Each item is a single line in this format:

```
- [ ] <Tag> | <property>=<expected_value> [| <tolerance>] [| #<note>]
```

The script parses each line, looks up `<Tag>` in the dump, reads `<property>`, compares to `<expected_value>` within `<tolerance>` if specified, and produces PASS/FAIL/UNSURE.

The **Geometry** section's expected coordinates come from `C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md`, produced in Step 0.5 after the visual overlay sanity check. Checklist authors should copy the normalized bounding boxes from the corrected table and apply explicit tolerances there, rather than estimating coordinates directly in the checklist.

The **Containment** section is required for lists, tables, rows, tabs, nested
panels, and any UI where a child can visually escape its parent while still
having a plausible absolute position. Use `contained_in=<ParentTag>` for outer
containment and `contained_in=<ParentTag> inset=<left>,<top>,<right>,<bottom>`
for content-area containment.

The **Visual Gate** section is required for visual-direction migrations where
the reference style matters beyond structure. It should include exactly one
blocking item such as:

```markdown
- [ ] <ScreenName>.VisualScorecard | visual_gate=PASS
```

That item is backed by a separate markdown scorecard, not by automated image
similarity. Codex and the Validator must inspect the reference/capture/contact
sheet before recording `Result: PASS`.

Non-interactive text elements must be labels. Screen titles, panel headers, subtitles, name displays, stat labels, stat values, descriptions, and captions are constructed with `FT66FlatStyle::MakeFlatLabel` or a clearly label-only wrapper. They must not use `MakeFlatButton`, `MakeFlatPanel`, or any helper that produces a border. The dump reports `is_label=true` only for label-only widgets. The checklist asserts `is_label=true` and `border_color=none` for every text tag that should render as plain text. `VerifyUIFidelity.py` fails a label check if the widget is not label metadata, or if the tagged label reports a visible border.

The **Interactivity** section's expected values come from the per-screen Interactivity spec in master plan Section 7.2. It verifies static wiring metadata (`has_click_handler`, `toggle_group`) through the dump. It does not replace Manual Interaction Verification, which confirms behavior after clicking.

### 6.2 Example (Hero Selection partial)

```markdown
# Hero Selection Verification Checklist

## Structure

- [ ] HeroSelection.TopRow.BackButton | exists=true
- [ ] HeroSelection.TopRow.BackButton | button_state=Selected
- [ ] HeroSelection.TopRow.HeroCarousel | exists=true
- [ ] HeroSelection.TopRow.HeroCarousel | child_count=9   # 7 portraits + 2 arrows
- [ ] HeroSelection.TopRow.HeroName | exists=true
- [ ] HeroSelection.TopRow.LabButton | button_state=Selected
- [ ] HeroSelection.LeftColumn.SkinsPanel | exists=true
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default | button_state=Selected
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer | button_state=Default
- [ ] HeroSelection.LeftColumn.DrugsPanel | exists=true
- [ ] HeroSelection.LeftColumn.DrugsPanel.BuyButton | button_state=Selected
- [ ] HeroSelection.LeftColumn.DrugsPanel.ClearButton | button_state=Default
- [ ] HeroSelection.MiddleColumn.CharacterPreview | exists=true
- [ ] HeroSelection.RightColumn.OuterPanel | exists=true
- [ ] HeroSelection.RightColumn.OuterPanel.RankSubPanel | exists=true
- [ ] HeroSelection.RightColumn.OuterPanel.MasterySubPanel | exists=true
- [ ] HeroSelection.RightColumn.OuterPanel.StatsSubPanel | exists=true
- [ ] HeroSelection.RightColumn.OuterPanel.WeaponUltimateSubPanel | exists=true
- [ ] HeroSelection.BottomRow.SteamParty | exists=true
- [ ] HeroSelection.BottomRow.SteamParty.Slot.1 | button_state=Ready
- [ ] HeroSelection.BottomRow.SteamParty.Slot.2 | button_state=Default
- [ ] HeroSelection.BottomRow.CHADButton | button_state=Selected
- [ ] HeroSelection.BottomRow.STACYButton | button_state=Default
- [ ] HeroSelection.BottomRow.EnterButton | button_state=Selected

## Geometry (normalized 1920x1080)

- [ ] HeroSelection.TopRow.BackButton | x=0.01..0.06 | y=0.02..0.06
- [ ] HeroSelection.LeftColumn.SkinsPanel | width=0.14..0.20 | height=0.18..0.24
- [ ] HeroSelection.LeftColumn.DrugsPanel | width=0.14..0.20 | height=0.14..0.20
- [ ] HeroSelection.MiddleColumn.CharacterPreview | width=0.40..0.52 | height=0.55..0.75
- [ ] HeroSelection.RightColumn.OuterPanel | width=0.20..0.28
- [ ] HeroSelection.BottomRow.SteamParty | height=0.08..0.12   # compact per size guidance
- [ ] HeroSelection.BottomRow.EnterButton | height=0.05..0.08  # standard, not tall
- [ ] HeroSelection.TopRow.HeroName | font_size<=48            # screen-title sized

## Colors

- [ ] HeroSelection.TopRow.BackButton | border_tint=SelectedBorder
- [ ] HeroSelection.TopRow.BackButton | text_color=SelectedText
- [ ] HeroSelection.TopRow.LabButton | border_tint=SelectedBorder
- [ ] HeroSelection.LeftColumn.SkinsPanel | border_tint=DefaultBorder
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default | border_tint=SelectedBorder
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer | border_tint=DefaultBorder
- [ ] HeroSelection.LeftColumn.DrugsPanel.BuyButton | border_tint=SelectedBorder
- [ ] HeroSelection.LeftColumn.DrugsPanel.ClearButton | border_tint=DefaultBorder
- [ ] HeroSelection.RightColumn.OuterPanel | border_tint=DefaultBorder
- [ ] HeroSelection.BottomRow.SteamParty.Slot.1 | border_tint=ReadyBorder
- [ ] HeroSelection.BottomRow.EnterButton | border_tint=SelectedBorder

## Content

- [ ] HeroSelection.TopRow.BackButton | text="BACK"
- [ ] HeroSelection.TopRow.LabButton | text="LAB"
- [ ] HeroSelection.TopRow.HeroName | text="ARTHUR"
- [ ] HeroSelection.RightColumn.OuterPanel.Subtitle | text="A KING. A CRUSADE. AN APOCALYPSE."
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default | text="Default"
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Badge | text="EQUIPPED"
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer | text="Beachgoer"
- [ ] HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Price | text="50"
- [ ] HeroSelection.LeftColumn.DrugsPanel.BuyButton | text="BUY"
- [ ] HeroSelection.LeftColumn.DrugsPanel.ClearButton | text="CLEAR"
- [ ] HeroSelection.BottomRow.CHADButton | text="CHAD"
- [ ] HeroSelection.BottomRow.STACYButton | text="STACY"
- [ ] HeroSelection.BottomRow.EnterButton | text="ENTER"
- [ ] HeroSelection.BottomRow.SteamParty.Slot.1.ReadyBadge | text="READY"

## Notes

- Skin row portraits (HeroSelection.LeftColumn.SkinsPanel.SkinRow.*.Portrait) require imagegen stubs.
- Character render in HeroSelection.MiddleColumn.CharacterPreview is content artwork, preserved from existing pipeline.
- Hero portrait carousel artwork is content; verify each portrait slot exists but do not verify portrait content.
```

### 6.3 Authoring

Hero Selection established the checklist format during the Stage 1 pilot. Later screen checklists may be hand-authored from the per-screen spec and Step 0.5 geometry table, or generated by a reviewed helper when such a helper exists. Do not assume checklist auto-generation exists unless the current repo contains the helper and the task explicitly uses it.

---

## 7. Pass Log Format

Each iteration produces an entry in `Saved\Codex\UI\<ScreenName>\pass_log.md`. The pass log is the running history of the migration.

### 7.1 Structure

```markdown
# <ScreenName> Migration Pass Log

## Pre-flight (Step 0)

### Legacy chrome audit
- Regex run: <timestamp>
- Matches found: <count>
- Helpers removed:
  - <helper_name> in <file>:<line>
  - ...
- Helpers retained as content artwork:
  - <helper_name> in <file>:<line> — reason: <portrait load, etc.>

### bUseGlow audit
- Calls flipped to FT66FlatButtonParams: <count>
- Legacy bUseGlow=true remaining in screen: <count> (target: 0)

### Content stubs identified
- HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Portrait — generate male protagonist headshot per reference
- HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Portrait — generate beachgoer skin headshot per reference
- ...

### Reference geometry table
- Path: C:\UE\T66\UI\Geometry\<screen>_reference_geometry.md
- Reference image: C:\UE\T66\UI\Screen References\<screen>.png
- Native resolution: <w>x<h>
- Named regions measured: <count>

### Step 0 acceptance
- Validation regex clean: YES
- Project compiles: YES
- Content stub inventory captured: YES
- Reference geometry table complete: YES

## Pass 1 — <timestamp>

### Implementation summary
- Constructed BuildSlateUI using FT66FlatStyle helpers
- Tags applied: <count>
- Content stubs generated: <count> (paths listed below)
- Live data wired: <list of subsystems>

### Capture
- Path: Saved\Codex\UI\HeroSelection\pass_1_capture.png
- Captured at: <timestamp>

### Dump
- Path: Saved\Codex\UI\HeroSelection\pass_1_dump.json
- Widget count: <n>
- Tagged widget count: <m>

### Verification report
- Path: Saved\Codex\UI\HeroSelection\pass_1_report.md
- PASS: 47
- FAIL: 8
- UNSURE: 5

### FAIL items
- HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Portrait | brush_resource | expected=stub_path.png, actual=null
- HeroSelection.RightColumn.OuterPanel | width | expected=0.20..0.28, actual=0.18
- ... (full list)

### UNSURE items
- HeroSelection.TopRow.HeroName | font character | requires visual review
- HeroSelection.MiddleColumn.CharacterPreview | artwork match | content artwork, accept

### Actions taken
- Generated SkinRow.Default.Portrait stub at SourceAssets\UI\ContentStubs\HeroSelection\skin_default.png
- Adjusted RightColumn.OuterPanel reference rect to widen
- (one bullet per fix)

### Termination check
- All PASS: NO
- Iteration cap reached: NO (1 / 5)
- FAIL set unchanged from previous pass: N/A (first pass)
- → Continue to Pass 2

## Pass 2 — <timestamp>

(...same structure...)

## Final state

- Termination reason: <DONE | ESCALATE>
- Final report path: ...
- Final contact sheet: ...
- Pablo review packet (if ESCALATE): Saved\Codex\UI\<ScreenName>\pablo_review.md
```

### 7.2 What the pass log enables

- **Iteration cap check.** After each pass, Codex reads the FAIL set and compares to the previous pass's FAIL set. If identical, escalate. For generated-raster screens, compare failed visual families, failed elements inside those families, generated worker coverage, layout failures, and wiring failures.
- **Pablo review.** Pablo reads the log to understand what was attempted and why.
- **Cross-screen learning.** Patterns that recur across multiple screens' pass logs become candidates for new `FT66FlatStyle` helpers, new checklist categories, or AGENTS.md updates.

---

## 8. Termination and Escalation

### 8.1 DONE termination

Conditions:
- Latest report has zero FAIL items.
- Latest report has zero UNSURE items, OR all UNSURE items are previously accepted as content deltas (logged in the pass log with Pablo's sign-off from a prior session).
- Any required visual scorecard exists and has `Result: PASS`.
- Manual Interaction Verification has no `Doesn't Work` items. If Pablo has not returned the manual checklist yet, the automated visual/data gate can be marked clean, but strict DONE is still pending.

Outputs:
- Final pass log entry marked DONE.
- Final contact sheet saved.
- Notify Pablo of completion with the path to the final contact sheet.

### 8.2 ESCALATE termination

Conditions (any):
- Iteration counter reaches 5.
- Latest report's FAIL set is identical to the previous report's FAIL set (stuck loop).
- Latest report has zero FAIL but UNSURE items remain that require visual review (and they're not previously accepted).
- Required visual scorecard is missing, not inspected by both agents when a
  Validator is available, or has `Result: FAIL`.
- A generated-raster screen has an unreviewed visual family, a visual `FAIL`
  family without a worker, generated assets not implemented, layout `FAIL`, or
  wiring `FAIL`.
- Codex encounters an unfixable deviation (missing backend, ambiguous spec, etc.).

Outputs:
- Final pass log entry marked ESCALATE with the trigger condition.
- Pablo review packet at `Saved\Codex\UI\<ScreenName>\pablo_review.md` containing:
  - Termination reason
  - Final contact sheet
  - Full FAIL and UNSURE lists with deviation details
  - Codex's recommendation for each unresolved item
  - Anything that requires Pablo decision (content delta acceptance, spec ambiguity, scope expansion)

### 8.3 Recovery after ESCALATE

Pablo reviews the packet, decides per item:
- Accept as content delta → updates the checklist to mark the item as accepted-delta and Codex re-verifies.
- Spec clarification → updates the per-screen spec in the master plan, Codex re-runs the relevant iteration step.
- Scope expansion → may require new helpers, new content stubs, or new infrastructure. New work added before resuming.

After Pablo's response, the loop resumes from Step 5 (triage) with the updated checklist and clarifications.

---

## 9. Content Stub Policy

### 9.1 When stubs are needed

A content stub is generated when **all** of these are true:
- The V3 reference shows content artwork (portrait, character art, illustration, decorative imagery) — not chrome.
- The production content pipeline doesn't yet provide that artwork (no existing UTexture, no asset path resolving).
- A pure-Slate placeholder (colored box, initials) would visually deviate from the reference enough to fail the checklist's geometric or visual check.

### 9.2 Imagegen workflow

For each stub:
1. Identify the target slot from the per-screen spec (e.g., `HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Portrait`).
2. Locate the corresponding region in the V3 reference image.
3. Generate via imagegen with a prompt that closely reproduces the reference imagery. Match style, framing, color palette, and content closely.
4. Save to `SourceAssets/UI/ContentStubs/<ScreenName>/<slot_name>.png` at a resolution appropriate for the slot (typical: 256×256 or 512×512 with transparency where appropriate).
5. Wire into the screen via the existing runtime texture access pattern (`T66RuntimeUITextureAccess::ImportFileTexture` or equivalent).
6. Flag the wire-up site with `// TODO(content-stub): replace with production asset when pipeline lands`.
7. Record the stub path in the pass log under "Content stubs generated."

For every icon generated via imagegen, use the reference-region extraction workflow instead of semantic-only prompting:

1. Open the screen's reference image at `C:\UE\T66\UI\Screen References\<screen>.png`.
2. Crop the specific icon region at native resolution and save the crop under `C:\UE\T66\UI\IconSourceCrops\<ScreenName>\<icon_name>_source_crop.png`.
3. Call imagegen with the crop as visual context and a text prompt that includes: "reproduce this specific icon's visual style and shape exactly. Match line weight, silhouette, fill style, and any decorative detail. Do not creatively interpret - replicate."
4. If the output diverges from the crop, regenerate with the same crop and a stricter "match the input image exactly" prompt. Do not accept a first output that merely shares semantic meaning.
5. Save the final approved PNG under `RuntimeDependencies\T66\UI\Icons\Flat\<icon_name>.png` unless the screen spec names a different runtime root.
6. Record the source crop path, final PNG path, and exact imagegen prompt in `C:\UE\T66\UI\icon_manifest.md`.

### 9.3 Stub registry

Maintain a registry file at `C:\UE\T66\UI\content_stubs_registry.md`. Format:

```markdown
# Content Stubs Registry

| Slot | Stub Path | Generated | Replaces |
|---|---|---|---|
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Portrait | SourceAssets/UI/ContentStubs/HeroSelection/skin_default.png | 2026-05-11 | TBD |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Portrait | SourceAssets/UI/ContentStubs/HeroSelection/skin_beachgoer.png | 2026-05-11 | TBD |
| ... | ... | ... | ... |
```

The registry is appended to during every Stage 1 / Stage 2 migration. It becomes the work list for the eventual content production pass.

### 9.4 What stubs are NOT

Stubs are not the answer to:
- Missing chrome (panels, borders, buttons) — that's flat Slate.
- Missing icons in the icon manifest — those are generated as icons under `RuntimeDependencies/T66/UI/Icons/Flat/`, not content stubs.
- Backend gaps where the data structure is missing (stats, names, scores) — those use placeholder text or `FText::GetEmpty()` per the screen's spec.

---

## 10. Common Failure Modes and Recovery

### 10.1 Legacy chrome leaking into a "migrated" screen

**Symptom:** A captured screen shows an artifact (magenta block, unexpected texture, glow halo) that doesn't appear in the V3 reference and isn't constructed by any `FT66FlatStyle` helper Codex called.

**Root cause:** A legacy chrome helper is still reachable via an `_Private.h` header or via `FT66Style::MakeButton/MakePanel` hitting the PNG branch.

**Recovery:**
1. Run the validation regex (Section 5.1) against the screen's source.
2. For each match, trace whether the call is on the active code path for the current screen state.
3. Remove or route the call.
4. Recompile, recapture, re-verify.

**Prevention:** Step 0 of the loop. Do not skip it.

### 10.2 Reference/capture dimension mismatch

**Symptom:** `VerifyUIFidelity.py` produces nonsense numeric comparisons or the contact sheet looks misaligned.

**Root cause:** V3 references are typically 1672×941 (or whatever GPT image output resolution was used); captures are 1920×1080.

**Recovery:** The script normalizes the reference to capture resolution before producing the contact sheet. Geometric comparisons happen in normalized 1920×1080 coordinates from the dump, not raw pixel coordinates from either image. If the script appears to be doing raw pixel comparison, it has a bug — fix the normalization.

### 10.3 Capture script parameter confusion

**Symptom:** `CaptureT66UIScreen.ps1` launches the game but doesn't produce a screenshot.

**Root cause:** The script's flag is `-Output`, not `-OutputPath`. A passed `-OutputPath` is ignored and the script falls through to its default behavior.

**Recovery:** Use `-Output` per Section 4 Step 2. This is the standardized invocation.

### 10.4 Resuming from existing tree without cleanup

**Symptom:** Codex starts a new session, sees a partial flat implementation from a previous session, continues on top of it, and inherits legacy chrome from the un-cleaned previous state.

**Root cause:** Migration was treated as additive ("add flat layer on top") instead of destructive ("replace legacy chrome with flat").

**Recovery:** Run Step 0 from scratch at the start of any resumed session. If Step 0's validation regex finds matches, the prior session didn't complete cleanup. Clean up before continuing implementation.

**Prevention:** Step 0 is a precondition, not a one-time gate. It runs at the start of every session for the screen.

### 10.5 Stuck on the same FAIL across iterations

**Symptom:** Pass 3 has the same FAIL items as Pass 2.

**Root cause:** The fix Codex attempted didn't land — either it was applied to the wrong location, didn't compile, or didn't affect the screen state.

**Recovery:** Escalate per Section 8.2. Show Pablo the pass log and the unchanged FAIL set. Pablo identifies whether it's a spec ambiguity, a deeper code issue, or a misdirected fix.

### 10.6 UNSURE items dominate after structural fixes

**Symptom:** After several passes, FAIL count drops to near zero, but UNSURE count remains high.

**Root cause:** The verification checklist may be over-relying on visual judgment items that the script can't auto-verify (font character, artwork match, composition feel).

**Recovery:** Two paths:
- **Convert UNSUREs to structural checks.** If "panel proportion looks off" recurs as UNSURE across multiple iterations, add bounding-box dimension fields to the checklist so the comparison becomes structural.
- **Escalate to Pablo for visual review.** For UNSUREs that genuinely require human judgment (does this generated stub portrait look enough like the reference?), produce the contact sheet and ask.

### 10.7 Checklist PASS count masks poor whole-screen fidelity

**Symptom:** `VerifyUIFidelity.py` reports zero FAIL items, but the contact sheet
does not match the approved reference at a glance.

**Root cause:** The checklist asserted selected structure/geometry/content
facts but did not assert load-bearing containment, scale relationships, or the
visual scorecard gate.

**Recovery:**
1. Do not report DONE.
2. Add or tighten missing structural assertions, especially `contained_in`
   checks for rows, tables, tabs, and nested panels.
3. Fill out the visual scorecard with `Result: FAIL` and concrete failed
   categories.
4. Continue the correction loop or escalate if the same visual failure survives
   two passes.

---

## 11. Stage 1 Specifics

The first execution of this loop is the Hero Selection pilot in Stage 1. During the pilot, expect the following to happen as part of building the loop infrastructure itself:

- Stage 1's first 2–3 passes will discover gaps in the infrastructure (dump fields missing, checklist format unclear, normalization edge cases). These get fixed as part of Stage 1 delivery — the loop is being built in parallel with being used.
- The Hero Selection checklist is hand-authored. The exercise of authoring it informs the format and tagging conventions, which then carry forward to Stage 2.
- The first stub generations exercise the imagegen workflow. Lessons there feed back into Section 9.

Acceptance for Stage 1 includes both:
- The Hero Selection screen passes its own verification checklist with zero FAIL.
- The infrastructure (`T66.UI.DumpScreen`, `VerifyUIFidelity.py`, widget tagging, content stub workflow) is functional and ready for Stage 2 screens to consume.

---

## 12. Stage 2 Transition

For each Stage 2 screen, the loop runs end-to-end:

1. Pablo identifies the next screen in the migration order (master plan Section 5.3).
2. Codex creates the screen's verification checklist from its per-screen spec in the master plan and the Step 0.5 geometry table. Use a generator only when a current reviewed helper exists; otherwise hand-author it.
3. Codex runs Step 0 (legacy cleanup) for that screen.
4. Pablo reviews the content delta report (per the master plan Section 5.2 per-screen workflow).
5. Codex runs Steps 1–7 to convergence.
6. Pablo reviews the final contact sheet and accepts or directs further work.

Each Stage 2 screen completion also updates:
- The content stub registry (Section 9.3).
- The icon manifest if new icons were needed.
- The pass log for the screen.
- Any AGENTS.md amendments that emerge from recurring patterns.

---

## 13. AGENTS.md Integration

`AGENTS.md` enforces this loop with a UI Reference Fidelity rule that references this document. The rule's text:

```
## UI Reference Fidelity Rule

When implementing or editing a UI screen from a reference image, follow
the loop defined in C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md.

Specifically:

1. Run Step 0 (legacy chrome cleanup) before any flat construction.
   The screen's reachable code must produce no matches against the
   validation regex in Section 5.1 of the loop doc.

2. Tag every named element constructed via FT66FlatStyle helpers per
   the convention in Section 3.3 of the loop doc.

3. Iterate Steps 1-7. The screen is not done until the verification
   report shows zero FAIL items and either zero UNSURE items or all
   UNSURE items previously accepted as content deltas. When the screen's
   checklist contains containment or visual-gate items, those must also pass.

4. Terminate per Section 8 of the loop doc. On ESCALATE, produce the
   Pablo review packet and stop.

5. Compile success and "looks roughly right" are necessary but not
   sufficient. For FlatStyle/reference-checklist work, the VerifyUIFidelity
   report is one required gate and visual direction work also requires the
   scorecard/contact-sheet gate. For FriendslopStyle work, the Friendslop
   authority file replaces this with capture/dump/contact evidence for user
   visual review plus a wiring/functionality PASS/FAIL gate.

Do not declare a UI migration complete without running the loop.
Do not skip Step 0. Do not resume an in-progress migration without
re-running Step 0's audit against current state.
```

This rule sits at the top of `AGENTS.md` under the UI section. Any conflict between this loop doc and AGENTS.md is resolved by updating AGENTS.md to match this doc — the loop doc is the canonical reference.

---

End of fidelity loop specification.
