# UI Agents

## Owns

Frontend UI, Slate screens, screen/modal reference fidelity, flat Slate chrome, FriendslopStyle raster chrome, captures, fidelity reports, layout sizing, top-bar screens, content stubs, icon manifests, and UI runtime asset routing.

## Trigger Words

UI, screen, modal, Slate, frontend, reference image, capture, screenshot comparison, layout, top bar, hero selection, settings, Stage 2, fidelity, flat style, flat chrome, FT66FlatStyle, FriendslopStyle, Friendslop UI, raster chrome, content stub, icon manifest.

## Read First

- `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md` for the active flat UI design system, screen specs, and stage scope.
- `ART_DIRECTION.md` for the boundary between the 3D FriendSlop/rubber direction and UI FriendslopStyle.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` for the migration and verification loop.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` for responsive layout rules.
- `UI/FriendslopStyle/README.md` then `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` for FriendslopStyle raster-chrome UI work.
- `UI/Reference/UI_STAGE2_CAPTURE_READINESS_REFERENCE.md` for Stage 2 screen names and capture routing.
- `Audit/Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md` for existing UI architecture and legacy chrome context.
- `UI/Processes/LootUIAnimationAuthoringProcedure.md` only for loot crate/chest/bag/wheel post-interaction 2D/UI animation work.
- `UI/Processes/MainMenuVideoBackgroundProcedure.md` only for main-menu background video work.

## Hard Rules

- FriendslopStyle is the active visual lane for ALL player-facing UI chrome (2026-06-09
  global flip). FlatStyle is legacy: its entry points (`FT66FlatStyle::MakeFlat*`,
  `MakeHudPanel`, the legacy `MakeButton`) now render FriendslopStyle plates through the
  `T66.UI.FriendslopGlobal` switch (default on; `-T66FlatLegacy` or setting the CVar to 0
  restores flat rendering for debugging). New UI work should target `FT66FriendslopStyle`
  directly; do not author new screens against the flat-color look.
- Do not declare reference UI work complete without the fidelity loop.
- For legacy FlatStyle-rendering work (only behind the escape switch), do not use generated raster art for chrome. Panels, borders, button plates, tab plates, dropdown shells, frames, dividers, tracks, and other chrome must be Slate-native through `FT66FlatStyle`.
- For FriendslopStyle work, generated raster chrome is allowed only through `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`; it must be decomposed into reusable sliced UI assets and may not be a pasted full-screen mockup.
- The 3D rubber-material target does not apply to 2D UI chrome or content artwork. FriendslopStyle should harmonize with the FriendSlop brand while following its own UI process.
- FriendslopStyle archived prompts, worker requests, inpaint specs, clean-sheet specs, and pass-numbered artifacts are historical evidence only unless the active screen contract explicitly re-accepts them.
- Image generation in the FlatStyle pipeline is allowed only for content artwork stubs and flat icon glyphs when that pipeline explicitly calls for them. Record icon outputs in `UI/icon_manifest.md` and content stubs in `UI/content_stubs_registry.md`.
- Do not run full UAT/package/stage for one screen unless the coordinating pass requires it.
- Do not bake live labels, player data, scores, or localized text into UI art.
- Do not report selection cards or shared reward cards as target-owned live loot animations.

## Verification

Use build, capture, compare, fidelity report, and resolution checks as the task requires.
