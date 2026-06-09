# Original User Request

User wants Codex and Claude to figure out the full implementation pipeline for turning the approved FriendslopStyle Round06 Main Menu visual reference into real Unreal UI.

The user specifically asked:

- Explain next steps and the full process pipeline before implementation.
- Include how corrections and iterations are done.
- Use the available tools, primarily account-backed ImageGen plus Codex/Claude/Unreal work.
- Look into FlatStyle specifically for the part that made screens match reference images by location, size, and positioning.
- Reuse only the FlatStyle reference-fidelity/layout breakdown process, not the FlatStyle visual/chrome approach.
- Figure out the new component: making real UI elements and new 9-slice/N-slice assets.
- Author the central instruction file under FriendslopStyle/FrenchFlop for every screen about this approach.
- Send the implementation plan/instruction file for user approval before any implementation, asset generation, or UI runtime edits.
- If approved later, then generate assets and implement/iterate until it genuinely looks like the reference.
- Prevent lazy implementation where criteria are individually checked but the holistic screen still does not look like the reference.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Produce a repo-grounded implementation/process plan for turning the Round06 FriendslopStyle reference into real Unreal UI, specifically reusing only the FlatStyle reference-fidelity/layout breakdown process where applicable, then author the central FriendslopStyle instruction file. No UI implementation, asset generation, or runtime edits beyond docs in this pass.
Stop condition: plan and instruction doc are authored, Claude has independently reviewed and cross-reviewed, and the result is ready for user approval before implementation begins.

# Relevant Repo Rules

- Root AGENTS.md requires T66 task contracts, live repo checks, Claude validator when available, and no native goal tools.
- UI/UI_AGENTS.md routes UI work through `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.
- `UI/UI_AGENTS.md` currently bans generated raster chrome for the existing FlatStyle path. The user explicitly wants a separate FriendslopStyle alternative, so the new doc must define a new method class rather than silently treating FlatStyle rules as the target visual approach.
- Process-governed visual/UI work requires PPF, artifact parity, mechanism/discriminator gates before implementation.
- This pass stops at user approval. It must not generate production assets, edit Slate code, import textures, or run implementation.

# Repo Findings To Ground The Answer

Useful FlatStyle/fidelity process:

- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` Step 0.5 requires reference geometry extraction before implementation.
- Geometry tables record the reference image path, native resolution, normalized 1920x1080 bounding boxes, tolerances, and named UI elements.
- `Scripts/GenerateUIGeometryOverlay.py` renders the geometry table over the reference image for sanity checking.
- `T66.UI.DumpScreen` / `T66.UI.DumpWidget` emit tagged widget JSON used by `Scripts/VerifyUIFidelity.py`.
- `Scripts/VerifyUIFidelity.py` consumes a checklist, reference screenshot, capture, and dump, then writes a report/contact sheet.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` says 1920x1080 is the composition source, not a fixed runtime viewport, and requires multi-resolution responsive checks.
- `UI/Checklists/pending_issues_Checklists.md` says the current Main Menu checklist is stale and must be refreshed to the current controls (`TimeWeeklyButton`, `TimeAllTimeButton`, `PartySizeDropdown`, `DifficultyDropdown`, metric check buttons, friend rows, top-bar Home).

Round06 reference:

- `UI/FriendslopStyle/Reference/MainMenu/Round06/manifest.md`
- Final mockup: `UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- The manifest says the mockup is not runtime UI chrome, labels must remain live/localizable, and it was based on a fresh current capture/dump.
- The accepted direction is inflated/rubber/bouncy surfaces using the current dark/red/green/yellow palette and current Main Menu structure.

Existing Slate slice/brush mechanics:

- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp` creates a brush with `DrawAs = ESlateBrushDrawType::Box` when margins are nonzero and stores the brush margin.
- `Source/T66/UI/Style/T66Style.cpp` has an explicit comment for generated panel textures: `9-slice: corners preserved, center stretches`.
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp` has an older horizontal-sliced button widget that draws left cap, center stretch, right cap and overlays live content.
- `Source/T66/UI/Style/T66FlatStyle.cpp` wraps those reference sliced helpers for FlatStyle, but FriendslopStyle should not reuse the flat visual language as the style target.

# Validator Ask

Please provide an independent, repo-grounded answer:

1. Is the proposed method class viable with this repo's current tools?
2. What exact pipeline should the FriendslopStyle central instruction file require?
3. What should be reused from FlatStyle and what should be forbidden?
4. How should ImageGen-generated mockups become real Unreal UI elements without baking text or causing distortion?
5. What correction/iteration gates prevent a superficially checked but holistically wrong result?
6. Any missing risks or process gaps before Codex authors the central instruction file?

Return concrete corrections and must-have requirements. Do not implement or edit files.
