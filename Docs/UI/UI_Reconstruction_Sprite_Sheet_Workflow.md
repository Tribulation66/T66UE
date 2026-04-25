# UI Reconstruction Sprite Sheet Workflow

## Goal

Turn a locked reference screen into a reusable UI kit without drifting away from the source image.

## Future Screens

For new screens, use the generalized pipeline in:

- `Docs/UI/UI_Screen_Reference_Pipeline.md`
- `Docs/UI/UI_Screen_Intake_Template.md`
- `Scripts/ScaffoldUIScreenReference.py`

Use the scaffold script before starting a new screen pack so the offline target, helper references, scene plate prompt, prompt pack, and runtime family folders all start from the same structure.

## Current Main Menu Pipeline

1. Use the main menu reference screenshot as the golden calibration screen and offline visual target.
2. Generate a canonical reference-layout manifest from that screenshot before runtime tuning:
   - `SourceAssets/UI/MainMenuReference/reference_layout.json`
   - `Source/T66/UI/Style/T66MainMenuReferenceLayout.generated.h`
3. Treat the reference canvas as measured pixel space, not a loose mockup:
   - all widget placements should come from the manifest/header
   - do not invent new offsets in runtime code if a box already exists in the layout artifact
   - validate against a canonical `1920x1080` baseline packaged capture and supported aspect buckets before final approval
4. Keep the runtime and scripts on the same measured boxes:
   - scripts use the JSON for crops, overlays, and future slice automation
   - Unreal runtime uses the generated header for actual widget placement
5. Generate one UI-free scene/background plate plus isolated foreground component boards by family instead of one giant all-in-one sheet.
   - generated assets must come correct from image generation
   - deterministic slicing/cropping from an accepted board is allowed
   - manual pixel cleanup, masks, erase/fill, cover patches, clone/repaint fixes, and screenshot repair are not allowed
   - if the generated board is wrong, regenerate the board or family
6. Keep each generation session short and narrow in scope:
   - top bar family
   - center CTA family
   - panel family
   - leaderboard chrome family
   - decorative/icon family if needed
7. For runtime components, request empty reusable plates:
   - no baked words
   - no baked numerals
   - no baked player names
   - title wordmark may be baked display art
   - tagline and all menu/social/leaderboard labels, values, and states stay live/localizable
8. Request explicit state coverage where applicable:
   - normal
   - hover
   - pressed
   - disabled
   - selected when the control type needs it
9. Reconstruct the in-game screen from the generated kit, not from the full screenshot.
10. Do not use buttonless or textless full-screen masters as production backgrounds. If the reference contains visible buttons, labels, panels, or leaderboard rows, generate a true UI-free scene/background plate and separate foreground component families before runtime integration.

## Main Menu Outputs

- Generated sheet copies are stored at:
  - `SourceAssets/UI/MainMenuReference/SpriteSheets/mainmenu_topbar_sheet_gen_v1.png`
  - `SourceAssets/UI/MainMenuReference/SpriteSheets/mainmenu_cta_sheet_gen_v1.png`
  - `SourceAssets/UI/MainMenuReference/SpriteSheets/mainmenu_cta_sheet_gen_v2.png`
  - `SourceAssets/UI/MainMenuReference/SpriteSheets/mainmenu_panels_sheet_gen_v1.png`
  - `SourceAssets/UI/MainMenuReference/SpriteSheets/mainmenu_decor_sheet_gen_v1.png`
- Prompt packs for those runs are stored at:
  - `Docs/UI/PromptPacks/MainMenuSpriteSheets/`
- Desktop-generated source files are left intact under:
  - `%USERPROFILE%/.codex/generated_images/<thread-id>/`
- Copy generated images into repo-owned source-asset folders; do not move or delete the desktop originals.
- Reference-layout artifacts are stored at:
  - `SourceAssets/UI/MainMenuReference/reference_layout.json`
  - `Source/T66/UI/Style/T66MainMenuReferenceLayout.generated.h`
  - `SourceAssets/UI/MainMenuReference/debug/reference_crop_overlay.png`

## Prompting Rules

- Treat the job as `reorganize the attached reference into a production sprite sheet`, not `invent a UI kit`.
- Say `exact reconstruction of the reference art style, not a redesign`.
- Name the exact asset family to avoid drift.
- Require a flat neutral presentation board so the assets stay isolated.
- Require front-on orthographic presentation so the assets are easy to slice and import.
- Explicitly ban baked text on localizable controls.
- Ask for exact proportions, border thickness, corner cuts, and material language from the reference.
- Ask for shell-only, socket-frame, empty-backplate, or open-aperture handling when runtime owns the interior.
- Use separate runs for separate families instead of stretching one conversation too far.
- For the anchor screen, do not force bad generated or manually repaired artifacts into runtime just because they exist. If a family is visibly wrong in-engine, generate a tighter family-specific replacement board and promote that board to the source pack.
- When extracting a family from the anchor screen, explicitly say whether the request is for:
  - a presentation board
  - a runtime sprite sheet
  - empty runtime plates with no labels
  - UI-free scene/background plate

## Session Rules

- Do not keep one long image-generation session alive across too many board types; results begin to drift.
- Prefer one focused generation pass per family.
- If the model starts simplifying shapes or inventing extra ornaments, reset with a fresh run using the original screen reference.
- If a first-pass family board comes back too chunky or off-proportion, do a second pass that narrows the ask to that one family and explicitly call out proportion drift. This was necessary for the center CTA family on the main menu.
- Do not repair generated boards with manual pixel edits. Regenerate instead.
- Save the prompts used for successful boards so later screens can reuse the structure.
- When prompting from inside Codex Desktop, the built-in generator can be used even when `OPENAI_API_KEY` is not exported to the shell; treat the saved image under `.codex/generated_images/` as the source artifact and copy it into the repo.

## Runtime Rules

- Final runtime UI uses real widgets, not hotspot overlays.
- Final runtime composition uses a UI-free scene/background plate plus separate foreground components for topbar, side panels, leaderboard chrome, and center CTA stack.
- Full reference screenshots, buttonless masters, and textless masters are offline targets or helper prompts only.
- Final runtime labels are real localizable text.
- Generated art is for chrome, frames, fills, icons, and state plates.
- Runtime placement should use the reference-layout header rather than freehand numeric tuning.
- Preserve separate shell rects, visible control rects, and live-content rects.
- Use scalable runtime assets where possible:
  - individual textures
  - nine-slice brushes
  - small icon atlases if useful
- The visible button plate should own hover, pressed, disabled, and selected feedback. Do not keep those states in a hidden overlay layer.

## Coordinate Rules

- Each reconstructed screen gets one canonical canvas size taken from the source reference.
- Normal 16:9 references use `1920x1080` as the authoring and baseline review canvas.
- Boxes should be recorded in reference pixels as:
  - `x`
  - `y`
  - `width`
  - `height`
- Runtime code should transform `reference pixels -> widget pixels` through one explicit transform helper.
- Runtime code must preserve anchors, safe zones, DPI scaling, and aspect behavior when transforming into `16:9`, `16:10`, ultrawide, and smaller/windowed captures.
- Debugging should compare runtime captures against:
  - the original reference
  - the generated layout overlay
- If a plate or shell does not align to the recorded box, fix the asset or the box. Do not add ad hoc nudges around the mismatch.

## Validation Rules

- Compare the rebuilt layered composition against the original reference screenshot at the same canonical baseline resolution.
- Then validate supported runtime aspect buckets for layout distortion, text fit, and background/safe-zone behavior.
- Reject the pass if the runtime background layer contains baked foreground UI chrome, controls, labels, leaderboard rows, or live content.
- Freeze live regions when possible, or mask only the truly dynamic content.
- Strict-diff shell regions; mask runtime-owned text, values, avatars, icons, images, media, and previews.
- Before runtime handoff, validate dimensions, alpha, nine-slice margins, and state anchors for promoted assets.
- Validate both visuals and interaction:
  - hover feedback
  - pressed feedback
  - one-click routing behavior
  - hitbox matching the visible plate
- Judge in this order:
  1. silhouette and spacing
  2. proportions and anchors
  3. text fit
  4. material and contrast match
  5. hover/pressed/selected behavior
