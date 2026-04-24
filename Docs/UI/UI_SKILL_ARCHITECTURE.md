# UI Skill Architecture

## Purpose

This document defines the recommended skill split for T66 screen reconstruction work.

The current `reconstruction-ui` skill is carrying too many responsibilities at once:

- style continuation
- layout measurement
- runtime art family generation
- Unreal reconstruction
- packaged validation

Those stages fail for different reasons. Splitting them by artifact stage makes failures easier to diagnose and handoff cleaner.

## Recommended Shape

The mental model is two production phases:

1. **Make assets.** Produce scene plates, foreground families, slices, states, and validation notes.
2. **Place assets.** Assemble those assets in Unreal with one coordinate system, semantic anchors, real controls, live text, and dynamic content wells.

The individual skills below are routing aids for those phases, not extra architectural layers. If a skill makes the task feel like reference-overlay alignment or multi-system style plumbing, collapse back to the two-phase contract.

Hard asset rule across all stages: generated UI assets must come correct from image generation. No stage may manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair generated assets after the fact. Deterministic slicing/cropping from an accepted generated board, deterministic export steps, packaged-review masks, and runtime placement/text overlay are allowed. If generated pixels are wrong, route back to regeneration.

Image-generation rule across all stages: use Codex-native `image_gen` only. Do not route UI work through legacy browser-automation generation tooling or request manifests. If native generation cannot produce a required artifact, mark the target blocked.

Completion rule across all stages: reference generation is not completion. A target screen is complete only after accepted component assets are generated, runtime implementation is updated, packaged capture is taken, and packaged review is recorded, unless a concrete blocker stops progress.

Keep one thin orchestrator, five specialist stage skills, and two shared helpers/prerequisites:

- `ui-reference-prep`
- `content ownership audit`

1. `ui-style-reference`
2. `ui-layout-manifest`
3. `ui-sprite-families`
4. `ui-runtime-reconstruction`
5. `ui-packaged-review`

Do not split down to one full skill per widget type. Buttons, panels, top bar, toggles, and icons should be sub-workflows inside `ui-sprite-families`, not separate top-level skills.

Operational mapping:

- asset phase: `ui-style-reference`, `ui-layout-manifest`, `ui-sprite-families`
- placement phase: `ui-runtime-reconstruction`
- review phase: `ui-packaged-review`
- optional helper only: `ui-reference-prep`

Strict baseline:
- the main menu is the golden calibration screen before this process is generalized
- normal 16:9 packaged acceptance targets are `1920x1080`
- every screen, modal, HUD overlay, tab, and mini-game UI must have `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png` generated from the canonical main-menu anchor, the current target screenshot, and the target layout list before sprite generation, runtime styling, or review
- no chat or agent may report completion after producing only the reference image
- no `1672x941` or other non-canonical generated output may be promoted as a production reference, sprite sheet, scene plate, slice, or runtime asset
- full reference screenshots are offline targets only, never shipped runtime backgrounds
- production composition uses a UI-free scene/background plate plus foreground component families
- the main menu title wordmark may be baked display art
- the main menu tagline and all other runtime labels, values, and list content remain live/localizable
- every stage preserves shell/control/live-content rect separation

Repo-managed mirror:
- `C:\UE\T66\CodexSkills\UI`
- `C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1`

Installed skill paths after bootstrap:
- `C:\Users\DoPra\.codex\skills\ui-reconstruction-orchestrator\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-reference-prep\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-style-reference\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-layout-manifest\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-sprite-families\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-runtime-reconstruction\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-packaged-review\SKILL.md`

Bootstrap command:

`powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1" -Validate`

## Orchestrator

### `ui-reconstruction-orchestrator`

Purpose:
- decide which stage to run next
- skip stages when required artifacts already exist
- enforce handoff contracts between skills

Use when:
- the user asks to reconstruct a screen from a reference
- the work spans multiple stages and should stay consistent

Must do:
- identify what artifacts already exist
- verify whether the main menu calibration artifacts are trusted before routing similar screens
- identify whether the active blocker is reference usability before routing to a production stage
- identify whether a trusted `content_ownership.json` exists before routing into generation, manifesting, or packaged diffing
- call the fewest stage skills needed
- keep the screen moving from reference to packaged validation

Must not do:
- become the place where all real work happens
- absorb stage-specific rules that belong in the specialist skills

## Stage Skills

### `ui-reference-prep`

Purpose:
- prepare higher-utility helper references before reconstruction work begins

Use when:
- the source reference is too low-resolution to inspect comfortably
- the team needs deterministic `2x` or `4x` exports
- the team needs helper-only AI upscales for inspection or prompting

Outputs:
- helper reference exports
- classification labels such as:
  - `layout-export`
  - `helper-only`
  - `runtime-safe`
  - `do-not-slice`

Must do:
- choose between plain resample and AI upscale deliberately
- preserve the canonical composition
- keep output classification explicit

Must not do:
- become a mandatory stage for every screen
- pretend that a larger image automatically becomes runtime-safe
- replace proper family regeneration when proportions or ownership are wrong

### `ui-style-reference`

Purpose:
- create the canonical offline reference image set for a new screen in the already-approved T66 visual style

Use when:
- there is an approved anchor screen such as the main menu
- a new screen such as hero select, settings, or loadout must match that style
- the target screen structure is known or mostly known

Inputs:
- approved anchor screen(s)
- target screen skeleton or component map
- target canvas size
- `content_ownership.json` or an equivalent code-first ownership note
- known live or dynamic regions
- explicit bans such as no baked localizable text

Outputs:
- `screen_master.png` as the offline comparison target
- `screen_master_2x.png` as helper/reference art
- optional `screen_master_nobuttons.png`, `screen_master_notext.png`, and `screen_master_nodynamic.png` for analysis or prompting only
- scene/background plate requirement notes for `$ui-sprite-families`
- short note on what was inferred vs held fixed

Must do:
- preserve style fidelity to the anchor screen
- preserve the target screen structure
- keep localizable and runtime-owned content removable from runtime art
- state clearly that no full-screen reference variant is a production runtime background
- keep anchor-only scenic motifs out of utilitarian screens unless the target structure explicitly calls for them

Must not do:
- invent new layout hierarchy
- solve sprite slicing
- solve Unreal implementation

Success bar:
- the new reference clearly belongs to the same product family as the anchor screen

### `ui-layout-manifest`

Purpose:
- convert the approved reference into measured runtime geometry

Use when:
- a reference image already exists
- runtime placement needs one coordinate system

Inputs:
- approved offline comparison target, normally `screen_master.png`
- naming rules for regions and controls
- `content_ownership.json`
- optional exclusions for live or dynamic zones

Outputs:
- `reference_layout.json`
- generated layout header
- optional labeled debug overlay
- optional crop index

Must do:
- create named rects for all important screen regions
- create distinct rects for the UI-free scene plate, foreground component families, and live regions
- distinguish visual crop rects from actual runtime control rects
- distinguish shell rects from live-content rects when ownership is mixed
- record canonical canvas and packaged target size
- act as the single placement source of truth

Must not do:
- generate new art
- manually nudge placements outside the manifest
- hide guesswork behind “looks right” adjustments

Success bar:
- runtime placement can be derived entirely from the manifest

### `ui-sprite-families`

Purpose:
- generate clean runtime-ready scene plates and foreground art families from the approved reference and measured layout

Use when:
- runtime needs real button, panel, tab, toggle, or icon art
- screenshot cleanup is not an allowed production method

Inputs:
- approved reference image set
- `reference_layout.json`
- `content_ownership.json`
- region list to reconstruct
- state matrix for each family:
  - normal
  - hover
  - pressed
  - disabled
  - selected, when needed

Outputs:
- UI-free scene/background plate when the screen needs full-screen background art
- family boards
- sliced runtime assets
- prompt packs
- asset manifest or import list with dimensions, alpha expectations, nine-slice margins, and state anchors

Recommended internal sub-workflows:
- `scene-plate`
- `topbar-family`
- `cta-family`
- `panel-shell-family`
- `leaderboard-chrome-family`
- `toggle-family`
- `icon-family`

Must do:
- match measured target proportions
- keep the full reference screenshot and buttonless/textless masters out of runtime composition
- generate scene/background plates with all UI chrome, text, controls, panels, leaderboard rows, and live content removed
- avoid baked localizable text
- generate shell-only, socket-frame, empty-backplate, or open-aperture assets when runtime owns the interior
- generate assets that fit the manifest slots without visible distortion
- validate dimensions, clean alpha, nine-slice margins, and state anchor alignment before runtime handoff
- keep family ownership clean

Must not do:
- ask one generic board to solve all widget classes
- rely on post-hoc pixel cleanup as any production method
- stretch fixed plate art into unrelated boxes

Success bar:
- the scene plate and foreground assets can be dropped into runtime slots with no obvious proportion drift or duplicate ownership

### `ui-runtime-reconstruction`

Purpose:
- build the real Unreal screen so it matches the reference as closely as possible

Use when:
- the reference, manifest, and required art families exist

Inputs:
- runtime screen files
- reference layout header
- sliced runtime assets
- `content_ownership.json`
- region ownership rules
- live-data requirements

Outputs:
- code changes
- real visible controls wired to visible plates
- packaged screenshot path
- note on which regions were strict-diffed vs manually validated

Must do:
- use the manifest as placement truth
- compose the screen from the UI-free scene plate plus foreground components, not from a full-screen screenshot background
- keep labels as real `FText`
- keep the visible control as the actual button
- preserve live content only where appropriate
- assign exactly one visual owner per region
- preserve runtime-owned apertures, sockets, and live-content wells instead of filling them with shell art

Must not do:
- ship hotspot overlays as the interaction model
- ship the full reference screenshot, buttonless master, or textless master as a runtime background
- let the background and runtime widget both own the same shell
- add manual placement nudges where manifest rects already exist

Success bar:
- the packaged build is structurally faithful, interactive, and localizable

### `ui-packaged-review`

Purpose:
- validate the packaged build and classify remaining deltas

Use when:
- a packaged screenshot exists
- the next pass needs a precise blocker list

Inputs:
- approved offline comparison target
- packaged screenshot
- `content_ownership.json`
- optional region masks or priority list

Outputs:
- ranked blocker list
- diff overlays
- strict-diff masks or mask notes
- per-region status
- root-cause classification

Root-cause classes:
- ownership
- layout
- asset
- scene-plate contamination
- text-fit
- live-data mismatch

Must do:
- review packaged output, not just editor behavior
- use the canonical packaged target frame, normally `1920x1080`
- compare the final layered composition against the offline target
- verify the scene plate gate separately: no baked foreground UI in the runtime background layer
- separate static-region failures from live-region differences
- strict-diff shell regions and mask or manually review runtime-owned interiors
- classify the cause before proposing the next fix

Must not do:
- hide regressions inside vague summaries
- prescribe random fixes without identifying the failure type

Success bar:
- the next pass has a short, unambiguous target list

## Handoff Contract

Each skill should hand off concrete files, not just prose.

Every stage that touches a mixed-ownership screen should preserve:

- `content_ownership.json`
- canonical canvas and packaged target size
- shell/control/live-content rect separation

### After `ui-style-reference`

- master image set
- canvas size
- region list
- statement that masters are offline/helper references, not runtime layers
- note on inferred areas

### After `ui-layout-manifest`

- `reference_layout.json`
- generated header
- labeled debug overlay

### After `ui-sprite-families`

- UI-free scene/background plate validation
- family boards
- final slices
- state list
- prompt pack
- asset validation notes for dimensions, alpha, nine-slice margins, and state anchors

### After `ui-runtime-reconstruction`

- changed runtime files
- packaged screenshot path
- ownership decisions

### After `ui-packaged-review`

- blocker list
- diff artifact paths
- mask artifact paths or mask notes
- next recommended stage

## Trigger Rules

### Helper insertion rule

Invoke `ui-reference-prep` only when the blocker is reference usability:

- low source resolution
- deterministic `2x/4x` export need
- helper-only AI upscale need for inspection or prompting

Do not insert it after runtime work starts.

### Existing approved reference already exists

The reference must be the screen-specific generated reference at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`, with proof that it was generated from the canonical main-menu anchor, current target screenshot, and layout list. If that proof is missing, route back to `ui-style-reference` before any manifest, sprite, runtime, or review work.

Run:
1. content ownership audit
2. `ui-layout-manifest`
3. `ui-sprite-families`
4. `ui-runtime-reconstruction`
5. `ui-packaged-review`

Example:
- current main menu work

Optional insertion:
- run `ui-reference-prep` first only if a better `layout-export` or helper-only hi-res companion is needed

### New screen in an existing approved style

Run:
1. content ownership audit
2. `ui-style-reference`
3. `ui-layout-manifest`
4. `ui-sprite-families`
5. `ui-runtime-reconstruction`
6. `ui-packaged-review`

Example:
- hero select built to match the main menu family

Optional insertion:
- run `ui-reference-prep` first if the anchor screen needs a better helper export before style continuation work

### Discovery work rather than production reconstruction

Do not use this stack as-is. Use the lighter discovery workflow first, then lock the winning direction before entering this pipeline.

## What Not To Split Further

Do not create separate top-level skills for:

- buttons
- panels
- top bar
- leaderboard rows
- icons

That level of fragmentation creates more handoff friction than value. Keep those as sub-workflows under `ui-sprite-families`.

## Current Mapping: Main Menu

For the current main menu effort:

- `ui-style-reference`
  - skipped, because the approved reference already exists
- `ui-layout-manifest`
  - owns `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_layout.json`
  - owns `C:\UE\T66\Source\T66\UI\Style\T66MainMenuReferenceLayout.generated.h`
- `ui-sprite-families`
  - owns the UI-free scene plate, top bar plates, CTA plates, panel shells, leaderboard chrome, toggles, and icon families
- `ui-runtime-reconstruction`
  - owns:
    - `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp`
    - `C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp`
    - `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardPanel.cpp`
- `ui-packaged-review`
  - owns packaged screenshot validation and blocker ranking

## Why This Matters

This split makes it much easier to diagnose failures.

Instead of saying “the reconstruction is off,” the team can say:

- the style reference drifted
- the manifest boxes are wrong
- the sprite family proportions are wrong
- the Unreal reconstruction is wrong
- the packaged build still diverges even though the editor looked correct

That clarity is the point.
