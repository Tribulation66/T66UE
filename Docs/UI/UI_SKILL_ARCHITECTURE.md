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

Keep one thin orchestrator, five production stage skills, and one optional shared helper:

- `ui-reference-prep`

1. `ui-style-reference`
2. `ui-layout-manifest`
3. `ui-sprite-families`
4. `ui-runtime-reconstruction`
5. `ui-packaged-review`

Do not split down to one full skill per widget type. Buttons, panels, top bar, toggles, and icons should be sub-workflows inside `ui-sprite-families`, not separate top-level skills.

Installed skill paths:
- `C:\Users\DoPra\.codex\skills\ui-reconstruction-orchestrator\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-reference-prep\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-style-reference\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-layout-manifest\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-sprite-families\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-runtime-reconstruction\SKILL.md`
- `C:\Users\DoPra\.codex\skills\ui-packaged-review\SKILL.md`

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
- identify whether the active blocker is reference usability before routing to a production stage
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
- create the canonical reference image set for a new screen in the already-approved T66 visual style

Use when:
- there is an approved anchor screen such as the main menu
- a new screen such as hero select, settings, or loadout must match that style
- the target screen structure is known or mostly known

Inputs:
- approved anchor screen(s)
- target screen skeleton or component map
- target canvas size
- known live or dynamic regions
- explicit bans such as no baked localizable text

Outputs:
- `screen_master.png`
- `screen_master_2x.png`
- `screen_master_nobuttons.png`
- `screen_master_notext.png`
- optional `screen_master_nodynamic.png`
- short note on what was inferred vs held fixed

Must do:
- preserve style fidelity to the anchor screen
- preserve the target screen structure
- keep localizable and live content removable from runtime art

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
- approved `screen_master.png`
- naming rules for regions and controls
- optional exclusions for live or dynamic zones

Outputs:
- `reference_layout.json`
- generated layout header
- optional labeled debug overlay
- optional crop index

Must do:
- create named rects for all important screen regions
- distinguish visual crop rects from actual runtime control rects
- act as the single placement source of truth

Must not do:
- generate new art
- manually nudge placements outside the manifest
- hide guesswork behind “looks right” adjustments

Success bar:
- runtime placement can be derived entirely from the manifest

### `ui-sprite-families`

Purpose:
- generate clean runtime-ready art families from the approved reference and measured layout

Use when:
- runtime needs real button, panel, tab, toggle, or icon art
- screenshot cleanup is no longer sufficient

Inputs:
- approved reference image set
- `reference_layout.json`
- region list to reconstruct
- state matrix for each family:
  - normal
  - hover
  - pressed
  - disabled
  - selected, when needed

Outputs:
- family boards
- sliced runtime assets
- prompt packs
- optional asset manifest or import list

Recommended internal sub-workflows:
- `topbar-family`
- `cta-family`
- `panel-shell-family`
- `toggle-family`
- `icon-family`

Must do:
- match measured target proportions
- avoid baked localizable text
- generate assets that fit the manifest slots without visible distortion
- keep family ownership clean

Must not do:
- ask one generic board to solve all widget classes
- rely on post-hoc pixel cleanup as the main method
- stretch fixed plate art into unrelated boxes

Success bar:
- assets can be dropped into runtime slots with no obvious proportion drift

### `ui-runtime-reconstruction`

Purpose:
- build the real Unreal screen so it matches the reference as closely as possible

Use when:
- the reference, manifest, and required art families exist

Inputs:
- runtime screen files
- reference layout header
- sliced runtime assets
- region ownership rules
- live-data requirements

Outputs:
- code changes
- real visible controls wired to visible plates
- packaged screenshot path
- note on which regions were strict-diffed vs manually validated

Must do:
- use the manifest as placement truth
- keep labels as real `FText`
- keep the visible control as the actual button
- preserve live content only where appropriate
- assign exactly one visual owner per region

Must not do:
- ship hotspot overlays as the interaction model
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
- approved reference
- packaged screenshot
- optional region masks or priority list

Outputs:
- ranked blocker list
- diff overlays
- per-region status
- root-cause classification

Root-cause classes:
- ownership
- layout
- asset
- text-fit
- live-data mismatch

Must do:
- review packaged output, not just editor behavior
- separate static-region failures from live-region differences
- classify the cause before proposing the next fix

Must not do:
- hide regressions inside vague summaries
- prescribe random fixes without identifying the failure type

Success bar:
- the next pass has a short, unambiguous target list

## Handoff Contract

Each skill should hand off concrete files, not just prose.

### After `ui-style-reference`

- master image set
- canvas size
- region list
- note on inferred areas

### After `ui-layout-manifest`

- `reference_layout.json`
- generated header
- labeled debug overlay

### After `ui-sprite-families`

- family boards
- final slices
- state list
- prompt pack

### After `ui-runtime-reconstruction`

- changed runtime files
- packaged screenshot path
- ownership decisions

### After `ui-packaged-review`

- blocker list
- diff artifact paths
- next recommended stage

## Trigger Rules

### Helper insertion rule

Invoke `ui-reference-prep` only when the blocker is reference usability:

- low source resolution
- deterministic `2x/4x` export need
- helper-only AI upscale need for inspection or prompting

Do not insert it after runtime work starts.

### Existing approved reference already exists

Run:
1. `ui-layout-manifest`
2. `ui-sprite-families`
3. `ui-runtime-reconstruction`
4. `ui-packaged-review`

Example:
- current main menu work

Optional insertion:
- run `ui-reference-prep` first only if a better `layout-export` or helper-only hi-res companion is needed

### New screen in an existing approved style

Run:
1. `ui-style-reference`
2. `ui-layout-manifest`
3. `ui-sprite-families`
4. `ui-runtime-reconstruction`
5. `ui-packaged-review`

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
  - owns top bar plates, CTA plates, panel shells, toggles, and icon families
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
