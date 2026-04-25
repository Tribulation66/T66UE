---
name: ui-sprite-families
description: Generate clean runtime-ready UI scene plates and foreground art families from an approved reference and measured layout. Use when Codex must produce a UI-free background plate plus button, panel, top-bar, leaderboard chrome, toggle, or icon family boards and slices with real normal, hover, pressed, or disabled states and no baked localizable text.
---

# UI Sprite Families

Generate the UI-free scene/background plate and reusable foreground runtime art families from the approved reference. This skill is for clean asset production, not layout measurement or Unreal implementation.

## Open These First

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\UI_IMAGE_BACKEND_POLICY.md`
- `C:\UE\T66\Docs\UI\UI_SKILL_ARCHITECTURE.md`
- `C:\UE\T66\Docs\UI\UI_Reconstruction_Sprite_Sheet_Workflow.md`
- `C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_HANDOFF.md` when working on the main menu
- the relevant `reference_layout.json`
- the relevant `content_ownership.json`

## Core Job

Produce family boards and slices for recurring chrome:
- UI-free scene/background plates
- top bar plates
- CTA plates
- panel shells
- leaderboard chrome
- toggles
- icons

Prerequisite: the target screen must already have an approved screen-specific reference at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`. That reference must have been generated from the canonical main-menu anchor, the current target screenshot, and the target layout list. If it is missing or came from a general style prompt, stop and route back to `$ui-style-reference`.

Expected outputs:
- family boards
- sliced runtime assets
- saved prompt packs
- state list per family
- asset validation notes for dimensions, alpha, nine-slice margins, and state anchors
- scene plate validation notes proving no foreground UI or live content is baked into the background

Native Codex `image_gen` is the only allowed image-generation path for this workflow. Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling. If native generation cannot produce a required family, report a blocker instead of routing to a external-tool fallback.

Sprite-family generation is not the end of the screen task. After accepted assets are produced and sliced/staged, continue to `$ui-runtime-reconstruction` and `$ui-packaged-review` unless blocked.

## Simplified Contract

Sprite-family work is the **asset phase**. Do not solve runtime placement here.

For each family, produce the clean asset at the intended runtime proportion and record how it should be placed. The handoff should answer:
- what image owns the visible shell
- what runtime widget or live content owns the interior
- what dimensions and states are valid
- whether the asset is safe to slice or helper-only

Do not create a whole-screen composite as a shortcut for placement. Do not bake localizable labels, live avatars, leaderboard rows, names, scores, or values into runtime plates.

Hard rule: generated assets must come correct from image generation. Do not manually pixel-edit, clean up, mask, erase/fill, cover-patch, clone, repaint, or screenshot-repair generated boards, plates, or slices. If pixels are wrong, regenerate the relevant board or family. Deterministic slicing/cropping from an accepted board is allowed.

## Workflow

### 1. Choose one layer or family at a time

Before generating any board, verify:
- the screen pack exists under `C:\UE\T66\UI\screens\<screen_slug>`
- the screen-specific reference exists
- the layout list and element/state checklist exist
- runtime-owned interiors are recorded

Do not ask one generation pass to solve unrelated widget classes. Work family by family:
- `scene-plate`
- `topbar-family`
- `cta-family`
- `panel-shell-family`
- `leaderboard-chrome-family`
- `toggle-family`
- `icon-family`

Image-generation rule:
- use native Codex `image_gen` for family boards
- if native output drifts, revise the prompt and regenerate
- if the required artifact cannot be produced, mark the screen blocked

### 2. Use measured proportions

- Pull the actual target sizes from the layout manifest.
- Match aspect ratios before generating anything.
- Generate native-proportion art instead of stretching a fixed plate later.
- For the active main menu pack, generate assets from a native `1920x1080` canonical frame.
- Archive raw generated sources and normalize acceptable landscape outputs into the `1920x1080` authoring baseline when needed; regenerate badly framed or structurally wrong outputs instead of resizing or repairing them.
- Helper-prepped images may guide prompting or inspection, but they remain `helper-only` unless explicitly reviewed as `runtime-safe`.
- Full reference screenshots, buttonless masters, and textless masters are not runtime backgrounds.
- Generate the scene/background plate with all topbar, side panel, leaderboard, CTA, text, button, avatar, icon, media, preview, and live-content pixels removed.
- If runtime owns the interior, generate the shell around the live well instead of faking the inserted content.
- For the main menu, title display art may be baked; tagline, CTA labels, social/leaderboard text, values, and states must not be baked into runtime plates.

### 3. Keep runtime art clean

- Do not bake localizable labels into final control art.
- Generate empty plates and shells when runtime text must stay live.
- Generate socket frames, empty backplates, or open apertures when runtime owns portraits, icons, avatars, media, or previews.
- Record the required states:
  - `normal`
  - `hover`
  - `pressed`
  - `disabled`
  - `selected` when needed

### 4. Slice and name deterministically

- Preserve family-specific naming.
- Keep sliced assets grouped by family.
- Save the prompt that produced a successful family so later screens can reuse it.

### 5. Validate assets before handoff

Check every promoted slice or atlas:
- scene plate contains no baked foreground UI chrome, controls, labels, leaderboard rows, avatars, icons, media, previews, or live content
- dimensions match the manifest slot or declare a uniform scale strategy
- transparent padding and alpha outside the silhouette are clean
- nine-slice margins are recorded for stretchable plates
- normal, hover, pressed, disabled, and selected states share stable anchors
- no unintended baked localizable text, runtime values, avatars, icons, or live imagery

### 6. Reject weak methods

Do not rely on:
- generic cleanup passes over contaminated screenshots
- manual pixel cleanup, masks, erase/fill, cover patches, clone/repaint fixes, or screenshot repair
- buttonless or textless full-screen masters as production backgrounds
- a single board trying to cover every UI element
- non-uniform stretching of fixed plate art

## Guardrails

- Match the approved reference, not older UI families.
- Match the screen-specific reference, not only the general main-menu style.
- Preserve shell logic and border structure.
- Keep the scene plate UI-free; foreground components own UI chrome.
- Keep interaction-state ownership in mind when generating plates.
- If a button or panel cannot stretch cleanly, regenerate it at the correct proportion instead of forcing it.
- Do not bake runtime-owned portraits, icons, media, or preview content into a family board.
- Do not promote an asset that fails dimensions, alpha, nine-slice, or state-anchor validation.
- Do not use removed external generation tooling or request manifests as an alternate image path.
- Do not report the screen complete from this asset stage alone.

## Handoff

Hand off:
- final family boards
- final slices
- scene/background plate path and validation notes
- prompt pack used
- `content_ownership.json`
- state matrix
- asset validation notes

Then move to `$ui-runtime-reconstruction`.

