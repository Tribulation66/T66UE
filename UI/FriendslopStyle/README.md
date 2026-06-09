# FriendslopStyle UI Router

This folder owns the FriendslopStyle raster-chrome UI lane.

## Current Authority

Read `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` first. It is the
single source of truth for FriendslopStyle process, asset provenance, runtime
ownership, and acceptance gates.

`UI/UI_AGENTS.md`, `AGENTS.md`, the fidelity loop, and the layout instructions
still apply. This README only routes FriendslopStyle-specific files.

## Fresh Agent Quick Start

When starting a FriendslopStyle screen with no prior context:

1. Read this file, then `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
2. Read `UI/FriendslopStyle/Screens/README.md` for the per-screen folder
   contract.
3. Read the target screen folder, for example
   `UI/FriendslopStyle/Screens/MainMenu/`.
4. Confirm the user-provided visual family breakdown for that screen. Family
   count is screen-specific; Main Menu used five families, but future screens
   may use fewer or more.
5. One local Codex CLI imagegen worker is launched for each visual `FAIL`
   family that requires generation. Do not launch only one representative
   worker when several families fail.

If `UI/FriendslopStyle/Screens/<Screen>/` does not exist, create it before the
screen pass starts and copy the Main Menu folder shape as the practical example.
Keep all screen-owned docs and static screen artifacts in that folder.

## Active Screen Docs

Screen docs live under:

`UI/FriendslopStyle/Screens/<Screen>/`

Use the target screen folder's `README.md` for the active checklist, geometry,
slice specs, component contract, and scorecard template. Main Menu is the pilot
example:

`UI/FriendslopStyle/Screens/MainMenu/`

Every active screen folder must maintain an element manifest ledger. The ledger
is the full per-screen inventory. For generated-raster Friendslop screens,
every implementation pass first classifies visual families as visual `PASS` or
visual `FAIL`:

- visual `PASS`: no image regeneration is needed for that family;
- visual `FAIL`: image regeneration is required for that family.

For every visual `FAIL` family, the pass then classifies all elements inside
that family as visual `PASS` or visual `FAIL`, launches one local Codex CLI
imagegen worker for that family, implements every generated element, then runs
a sizing/fitting pass and a wiring/functionality `PASS`/`FAIL` gate.

For each approved FriendslopStyle reference, first generate and cache one
textless reference breakdown through a local Codex CLI worker. Then crop one
textless context for each user-declared visual family. Family workers receive
those textless family contexts and must output both a contact sheet and
individualized backgroundless runtime PNGs. Manual cropping from family sheets
is not the runtime asset path.

Family worker prompts are extraction instructions, not style descriptions. See
`FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` Section 2.2.1 for the full
Allowed/Forbidden definition. Do not use descriptive/adjectival material, shape,
color, vibe, polish, or game-comparison language in those prompts. The textless
family crop is the only visual style authority; the prompt may list elements,
output paths, alpha/canvas requirements, and no-baked-content rules only.

FriendslopStyle reports process coverage, not Codex-owned visual acceptance:
declared-family assessment coverage, failed-family worker coverage,
regenerated asset implementation coverage, sizing/fitting work performed, and
the wiring/functionality gate. Do not use `FULL`, `PARTIAL`, or a layout
`PASS`/`FAIL` result for FriendslopStyle visual acceptance; the user reviews
the current capture/contact sheet and decides the next visual direction.

The current Main Menu visual reference example lives under:

`UI/FriendslopStyle/Reference/MainMenu/Current/`

## Visual Asset Authorship

Production visual pixels for runtime chrome, title art, and Friendslop
background art are authored only by account-backed built-in imagegen run in a
separate local Codex CLI worker, or by a specific user-approved exception
documented in the active screen contract.

Do not generate Friendslop iteration assets directly in the main Codex app chat.
Do not use `OPENAI_API_KEY`, OpenAI API scripts, browser screenshots, web image
URLs, or old generated-image folders as a substitute for a fresh CLI worker
generation.

Reference crops are measurement and comparison targets only. They are not
runtime asset sources. Alpha extraction, cropping, resizing, slicing, and contact
sheets are mechanical processing and QA operations only; they may not repair,
paint, inpaint, blur, smooth, recolor, clone, or synthesize production pixels.

If a component visually fails because the asset pixels are wrong, regenerate or
replace the asset. Do not salvage it with manual masking or Pillow-style patching.

## Archive

`UI/FriendslopStyle/Archive/` contains historical prompts, worker requests,
deprecated slice specs, and pass artifacts. Files under `Archive/` are not
current process rules and must not be used as permission for future screens.

`UI/FriendslopStyle/SourcePrompts/` stores source-art prompts. These are input
records, not process authorities.
