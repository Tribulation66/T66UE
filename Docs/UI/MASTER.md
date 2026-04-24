# UI Master

## Purpose

This folder is the source-of-truth documentation area for T66 UI reconstruction work.

The current goal is not open-ended UI exploration. The goal is:

1. make the assets
2. place those assets on the screen with real widgets and live text
3. validate the packaged build against the reference

All helper skills and scripts exist only to support those two production contracts. If a step makes it harder to answer "which asset owns this pixel?" or "which anchor places this widget?", simplify it before continuing.

## Canonical Locations

- Canonical UI workspace: `C:\UE\T66\UI`
- Per-screen packs: `C:\UE\T66\UI\screens\<screen_slug>`
- Canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Runtime UI code: `C:\UE\T66\Source\T66\UI`
- Main menu runtime component pack: `C:\UE\T66\SourceAssets\UI\MainMenuReference`
- Reconstruction scripts: `C:\UE\T66\Scripts`
- UI docs: `C:\UE\T66\Docs\UI`
- Content ownership audit: `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- Image generation policy: `C:\UE\T66\Docs\UI\UI_IMAGE_BACKEND_POLICY.md`
- Repo-managed skill mirror: `C:\UE\T66\CodexSkills\UI`
- Skill bootstrap script: `C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1`
- Reconstruction orchestrator: `C:\Users\DoPra\.codex\skills\ui-reconstruction-orchestrator\SKILL.md`
- Reference prep helper: `C:\Users\DoPra\.codex\skills\ui-reference-prep\SKILL.md`

The repo mirror is the shareable source of truth for the custom UI skills. Reinstall the live local copies with:

`powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1" -Validate`

## Core Rules

### -1. Deploy agents for multi-screen work

When a UI assignment spans more than one independent screen, modal, HUD overlay, tab, asset family, or review task, start subagents immediately with explicit ownership. This is part of the standard process because capture, reference prep, checklist validation, and packaged review can run in parallel.

Good parallel splits:

- current screenshot capture, source inventory, live-content audit, and layout list creation
- screen-specific reference prompt/input preparation and reference-gate validation
- asset/component checklist and sprite-family validation
- packaged screenshot review, blocker classification, and diff/mask notes

Do not assign two agents to edit the same source file or asset folder. Each agent must be told the canonical anchor path, its target `UI\screens\<screen_slug>` folder, its owned files, and the rule that runtime styling is blocked until `reference\canonical_reference_1920x1080.png` exists for that screen.

Reference generation is only the first gate. Agents must not finish their answer after creating references alone. Unless a concrete blocker prevents progress, each assigned screen must continue through sprite/component generation, slicing/import or staging, runtime implementation, packaged capture, and packaged review before the agent reports the screen as complete.

### 0. Keep the process split in two

There are only two production phases:

- **Asset phase:** produce the UI-free scene plate and clean foreground families. This phase owns image generation, slicing, state boards, alpha checks, and dimensions.
- **Placement phase:** assemble those assets in Unreal. This phase owns anchors, sizing, real buttons, live `FText`, and dynamic content wells.

Do not mix the two phases. Runtime code should not align widgets against a full-screen reference image, and asset generation should not solve runtime placement by baking whole-screen composites.

Hard asset rule: generated UI assets must come correct from image generation. Do not manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair a generated asset after the fact. If an asset is contaminated, missing pixels, wrong-size, or structurally wrong, regenerate the relevant board or family. Deterministic slicing/cropping from an accepted generated board and runtime placement/text overlay are allowed.

Every screen starts with an explicit element checklist in the screen intake. List each required scene plate, family board, control plate, icon, shell, and live-content aperture, and mark the required states for that element: `normal`, `hover`, `pressed`, `disabled`, and `selected` where applicable. Keep generating or regenerating assets until every checklist item and required state is present, correctly proportioned, and cleanly sliceable. Only then start placement in Unreal.

### 0A. Generate a screen-specific reference first

Every screen, modal, HUD overlay, tab, and mini-game UI must have its own generated full-screen reference before sprite generation or runtime placement starts. This is a blocking gate, not an optional style helper.

For each target screen, feed these three inputs to image generation:

- the canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- the current runtime screenshot of the exact target screen or modal
- a layout list for the exact target screen, including regions, controls, live-data wells, tab/modal variants, and required states

Save the result as:

`C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`

Do not proceed if this reference is missing. Do not accept a screen that only has a vague main-menu-inspired style pass. If the generated reference does not preserve the target layout or does not match the canonical main-menu visual language, regenerate it before continuing.

### 0B. Complete the whole screen pipeline

A UI task is not complete when the reference image is generated. The minimum complete pass is:

1. screen-specific reference generated and approved
2. content ownership recorded
3. element/state checklist complete
4. sprite/component families generated and validated
5. accepted assets sliced or staged
6. Unreal runtime rebuilt with real controls and live text/data
7. packaged `1920x1080` capture produced through the display-1 launcher
8. packaged review notes classify remaining deltas

If a chat or agent cannot continue, it must report `blocked`, name the missing artifact or failing command, and state the next required step. Do not report `done`, `complete`, or final status after only a reference-generation pass.

### 1. Calibrate on the main menu first

The main menu is the golden calibration screen for this reconstruction process. Do not generalize the workflow from a new screen until the main menu can pass the same strict packaged target, mask, ownership, and asset-validation rules that future screens must use.

For the main menu specifically:

- canonical comparison target is a `1920x1080` packaged capture
- the active main menu pack must be created natively at `1920x1080`, not converted from another canvas
- no `1672x941` or other non-canonical generated image may be promoted as a production reference, sprite sheet, scene plate, slice, or runtime asset
- old wrong-resolution generated assets should be deleted and rebuilt, not rescued
- the approved reference pack is the visual authority
- the title wordmark may be baked display art
- the tagline and all navigational/social/leaderboard labels remain live, localizable text

### 2. Treat the reference as the offline target

Do not redesign layout, hierarchy, spacing, or shell logic unless explicitly requested.

The full reference screenshot is an offline target for measurement, style matching, and packaged comparison only. It must never ship as a runtime background layer, and a buttonless or textless full-screen master is not a production background by default.

### 3. One coordinate system

The measured reference layout manifest is the placement source of truth. Runtime widgets, crops, and art families should all derive from the same measured boxes.

All normal 16:9 reconstruction work targets a canonical `1920x1080` packaged viewport unless the screen intake explicitly locks a different size. Helper exports may be larger, but they do not replace the canonical target.

For runtime placement, prefer semantic anchors over pixel nudging:

- center stack, title, and tagline use center anchors
- left-side panels use left/bottom or left/top anchors
- right-side panels use right/bottom or right/top anchors
- control interiors use local coordinates inside their owning component

### 4. Keep visible controls real

Do not ship invisible hotspot overlays as the real interaction model. The visible plate should be the actual button. Labels should stay as real `FText`.

### 5. Separate layout from asset generation

One flattened screenshot should not be both:

- the layout spec
- the runtime art source

Use:

- a canonical offline comparison target
- a UI-free scene/background plate for only the non-UI environment/backdrop
- clean foreground component families for recurring chrome
- runtime state assets with no baked localizable text

### 6. Fix large ownership problems before polish

Solve region ownership first:

- which layer owns the scene/background plate
- which layer owns the top bar shell
- which layer owns the CTA stack shell
- which layer owns panel shells
- which layer owns leaderboard chrome and live leaderboard content

Do not stack speculative overlays onto contaminated backgrounds and call that progress.

### 7. Audit runtime-owned interiors before generation

Before style-reference generation, family-board generation, or strict diffing:

- inspect the screen code
- identify runtime text, localized strings, values, portraits, icons, avatars, media, screenshots, store/social/leaderboard entries, timers, prices, progress values, or 3D previews
- record them in `content_ownership.json`
- keep shell rects, visible control rects, and live-content rects separate

If a region is runtime-owned, prompts and runtime assets must preserve the shell and leave the interior open, socketed, or neutral.

### 8. Use native Codex image generation only

Use Codex-native `image_gen` for UI references, scene plates, sprite-family boards, icon families, and regenerated components.

Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling for this UI reconstruction process. If native image generation cannot produce the required artifact, mark the screen blocked and describe the missing image-generation capability instead of routing to another generator.

### 9. Validate in packaged runtime

Editor-only correctness is not sufficient. Every meaningful pass should be judged in the packaged build.

Packaged review uses the canonical offline target frame and an ownership-aware mask set:

- compare the composed packaged screen, not an individual layer, against the offline target
- strict-diff static shell regions
- strict-diff the UI-free scene plate only where it is actually visible behind UI
- mask or manually validate live/runtime-owned interiors
- record capture path, diff artifacts, and masked regions with the review

## Standard Workflow

### 1. Make the assets

- create or verify the screen pack under `C:\UE\T66\UI\screens\<screen_slug>`
- capture the current target runtime screenshot under that screen pack
- write the target layout list under that screen pack
- generate the screen-specific offline reference from the canonical main-menu anchor, current screenshot, and layout list
- identify the approved screen-specific offline reference
- audit runtime-owned content before generation
- fill the element checklist, including required `normal`, `hover`, `pressed`, `disabled`, and `selected` states
- produce or refresh the UI-free scene/background plate
- produce foreground component families for panels, top bar, CTA stack, controls, toggles, and icons
- validate asset dimensions, alpha, and state alignment
- treat the asset phase as incomplete until the element checklist is complete
- reject and regenerate bad generated assets instead of manually repairing pixels
- do not stop or final-answer here; the pass must continue into runtime placement and packaged review unless blocked

Optional helper step:

- if the active blocker is reference usability rather than layout, use `ui-reference-prep` to create deterministic `2x/4x` exports or helper-only AI upscales before manifesting or family generation

### 2. Place the assets

- assemble a single fixed reference canvas in Unreal
- place major regions by semantic anchors, not manual nudges over the full reference
- keep visible controls as real buttons
- keep localizable labels, values, social rows, leaderboard rows, portraits, avatars, and dynamic data live
- keep local coordinates inside each component family
- do not report completion until the rebuilt packaged screen has been captured

### 3. Review through the gate

Every pass should clear `C:\UE\T66\Docs\UI\SCREEN_REVIEW_GATE.md` before it is treated as a meaningful improvement.

## Current Priority

The first active screen under this process is the main menu. See:

- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference` for runtime component assets only

## Useful Carryover From Old Frontend Docs

The older `FrontendDesign/MASTER.md` was still useful in three ways and that guidance is intentionally preserved here:

- preserve structure first, skin second
- judge passes by mechanical craft before taste
- keep a blunt review language instead of calling unfinished work done

