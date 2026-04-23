# UI Image Backend Policy

## Purpose

This file records the current backend policy for UI image generation after real A/B testing on Hero Selection.

Do not treat backend choice as a taste preference. Use the backend that best matches the reconstruction stage.

## Current Tool Reality

- Native Codex `image_gen` is available directly in the Codex app.
- In this local tool surface, `image_gen` does not expose explicit `quality`, `size`, or repo-local attachment parameters.
- The local ChatGPT bridge remains available for attachment-heavy or manifest-driven requests, but it is UI automation and therefore more brittle.

## Tested A/B

Date:
- `2026-04-23`

Screen:
- `HeroSelection`

### Style-reference A/B

Bridge canonical master:
- `C:\UE\T66\SourceAssets\UI\HeroSelectionReference\screen_master.png`

Native Codex full-screen result:
- `C:\UE\T66\SourceAssets\UI\HeroSelectionReference\ABTests\hero_selection_native_fullscreen_shell_board.png`

Verdict:
- use the bridge-style attachment-backed path for `ui-style-reference` canonical full-screen masters

Why:
- the bridge result preserved the full screen contract and read like a real screen master
- the native result respected the ownership audit, but it overcorrected into a shell-only screen board instead of a stage-ready canonical reference
- the native full-screen output is still useful as a helper shell provenance artifact, but not as the primary screen authority

### Sprite-family A/B

Bridge center-family board:
- `C:\UE\T66\SourceAssets\UI\HeroSelectionReference\SpriteSheets\20260423-121115-use-case-ui-mockup-asset-type-game-ui-runtime-co.png`

Native Codex center-family board:
- `C:\UE\T66\SourceAssets\UI\HeroSelectionReference\ABTests\hero_selection_native_center_family_board.png`

Verdict:
- prefer native Codex `image_gen` first for `ui-sprite-families`

Why:
- the native board covered more usable runtime families in one pass
- the native board separated states more clearly
- the native board was more slice-ready and read like runtime chrome inventory instead of a curated presentation board
- the bridge board remained useful as a style cue and fallback, but not the best primary family source for this test

### Reliability Note

The full-screen bridge rerun using the updated ownership-aware prompt timed out:

- `Timed out after 239 seconds waiting for the rendered image to settle.`

That does not invalidate the bridge result already promoted into `screen_master.png`, but it does confirm that the bridge path is operationally brittle.

## Current Operational Rule

### `ui-style-reference`

Prefer:
- bridge or another attachment-backed path when the task is to build a canonical full-screen screen master from multiple reference anchors

Use native Codex `image_gen` for:
- helper shell boards
- ownership-aware supplements
- quick spot checks or alternatives

Do not treat the native full-screen output as the canonical master unless it clearly preserves the screen contract.

### `ui-sprite-families`

Prefer:
- native Codex `image_gen` first

Use the bridge when:
- native generation materially drifts
- explicit attachment-heavy prompting is necessary
- a repo-side request manifest is required for reproducibility

### `ui-reference-prep`

This stage remains separate:
- deterministic resample first for layout helpers
- AI upscale for helper-only inspection

Do not use either backend choice here as a substitute for ownership-aware family generation.

## Review Rule

Re-run this A/B policy only when:

- the native Codex image tool surface changes materially
- the bridge becomes much more reliable
- a new screen type exposes a different backend strength

Until then, use this split as the default.
