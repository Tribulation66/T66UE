# UI Agents

## Owns

Frontend UI, Slate screens, screen/modal reference fidelity, generated UI chrome, captures, compare reports, layout sizing, top-bar screens, and UI runtime asset routing.

## Trigger Words

UI, screen, modal, Slate, frontend, reference image, capture, screenshot comparison, layout, top bar, hero selection, settings, Stage 2, fidelity, generated chrome, button plate, sprite sheet.

## Read First

- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` for reference-image UI migration.
- `UI/Instructions/UI_GENERATION_INSTRUCTIONS.md` for generated UI chrome.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` for responsive layout rules.
- `UI/Processes/LootUIAnimationAuthoringProcedure.md` for loot crate/chest/bag/wheel post-interaction 2D/UI animation work.
- `UI/Reference/UI_STAGE2_CAPTURE_READINESS_REFERENCE.md` for Stage 2 screen names and capture routing.
- `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md` only when creating a fresh target prompt.

## Hard Rules

- Do not declare reference UI work complete without the fidelity loop.
- Do not run full UAT/package/stage for one screen unless the coordinating pass requires it.
- Do not bake live labels, player data, scores, or localized text into UI art.
- Do not use Pillow/PIL or local pixel repair for generated UI art.
- Do not report selection cards or shared reward cards as target-owned live loot animations.

## Verification

Use build, capture, compare, fidelity report, and resolution checks as the task requires.
