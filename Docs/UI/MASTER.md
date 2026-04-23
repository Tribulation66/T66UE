# UI Master

## Purpose

This folder is the source-of-truth documentation area for T66 UI reconstruction work.

The current goal is not open-ended UI exploration. The goal is:

1. take an approved reference image or locked mockup
2. turn it into a measured runtime layout
3. derive real runtime-ready art families from that layout
4. rebuild the screen in Unreal with actual widgets and live text
5. validate the packaged build against the reference

## Canonical Locations

- Runtime UI code: `C:\UE\T66\Source\T66\UI`
- Main menu reference pack: `C:\UE\T66\SourceAssets\UI\MainMenuReference`
- Reconstruction scripts: `C:\UE\T66\Scripts`
- UI docs: `C:\UE\T66\Docs\UI`
- Content ownership audit: `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- Image backend policy: `C:\UE\T66\Docs\UI\UI_IMAGE_BACKEND_POLICY.md`
- Repo-managed skill mirror: `C:\UE\T66\CodexSkills\UI`
- Skill bootstrap script: `C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1`
- Reconstruction orchestrator: `C:\Users\DoPra\.codex\skills\ui-reconstruction-orchestrator\SKILL.md`
- Reference prep helper: `C:\Users\DoPra\.codex\skills\ui-reference-prep\SKILL.md`

The repo mirror is the shareable source of truth for the custom UI skills. Reinstall the live local copies with:

`powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1" -Validate`

## Core Rules

### 1. Treat the reference as the spec

Do not redesign layout, hierarchy, spacing, or shell logic unless explicitly requested.

### 2. One coordinate system

The measured reference layout manifest is the placement source of truth. Runtime widgets, crops, and art families should all derive from the same measured boxes.

### 3. Keep visible controls real

Do not ship invisible hotspot overlays as the real interaction model. The visible plate should be the actual button. Labels should stay as real `FText`.

### 4. Separate layout from asset generation

One flattened screenshot should not be both:

- the layout spec
- the runtime art source

Use:

- a canonical layout master
- clean family boards for recurring chrome
- runtime state assets with no baked localizable text

### 5. Fix large ownership problems before polish

Solve region ownership first:

- which layer owns the top bar shell
- which layer owns the CTA stack shell
- which layer owns panel shells

Do not stack speculative overlays onto contaminated backgrounds and call that progress.

### 6. Audit runtime-owned interiors before generation

Before style-reference generation, family-board generation, or strict diffing:

- inspect the screen code
- identify runtime text, portraits, icons, avatars, media, or 3D previews
- record them in `content_ownership.json`
- keep shell rects and live-content rects separate

If a region is runtime-owned, prompts and runtime assets must preserve the shell and leave the interior open, socketed, or neutral.

### 7. Prefer native image generation first

Use native Codex `image_gen` first when prompt-only generation or thread-attached references are enough.

Use the ChatGPT bridge only as a fallback when the task needs:

- multiple repo-local reference attachments
- versioned request manifests or reproducible bridge requests
- explicit API-side controls not exposed in the local tool surface

### 8. Validate in packaged runtime

Editor-only correctness is not sufficient. Every meaningful pass should be judged in the packaged build.

## Standard Workflow

### 1. Lock the target frame

- identify the primary approved reference
- identify any clean variants
- capture the current packaged state at the same screen

Optional helper step:

- if the active blocker is reference usability rather than layout, use `ui-reference-prep` to create deterministic `2x/4x` exports or helper-only AI upscales before manifesting or family generation

### 2. Audit content ownership

- inspect the runtime screen code before generating art
- classify each mixed-ownership region in `content_ownership.json`
- decide whether each region needs a:
  - `shell-only`
  - `socket-frame`
  - `empty-backplate`
  - `open-aperture`

### 3. Partition the screen

Break the screen into stable regions:

- backdrop
- top bar
- center stage
- CTA stack
- left panel
- right panel

### 4. Derive reusable assets

Prefer clean families over screenshot cleanup:

- top bar plates
- CTA plates
- panel shells
- icons
- toggles

### 5. Rebuild from large to small

Work in this order:

1. shell ownership and region composition
2. measured placement and sizing
3. control plates and text fit
4. hover/pressed/disabled state fidelity
5. live content framing and polish

### 6. Review through the gate

Every pass should clear `C:\UE\T66\Docs\UI\SCREEN_REVIEW_GATE.md` before it is treated as a meaningful improvement.

## Current Priority

The first active screen under this process is the main menu. See:

- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference`

## Useful Carryover From Old Frontend Docs

The older `FrontendDesign/MASTER.md` was still useful in three ways and that guidance is intentionally preserved here:

- preserve structure first, skin second
- judge passes by mechanical craft before taste
- keep a blunt review language instead of calling unfinished work done
