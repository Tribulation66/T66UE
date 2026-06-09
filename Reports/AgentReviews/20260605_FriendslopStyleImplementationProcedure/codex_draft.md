# Codex Draft For Cross-Review

Codex authored the central FriendslopStyle process file:

`C:\UE\T66\UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`

This is intentionally a draft approval artifact. It does not implement UI, generate runtime assets, import textures, or modify Slate code.

## Main Decisions In The Draft

- FriendslopStyle is a separate style lane from FlatStyle.
- Approval of the file is required before implementation.
- Generated raster chrome is allowed only for FriendslopStyle after user approval and must not be treated as a FlatStyle workflow.
- The full-screen Round06 reference is visual direction only, not runtime chrome.
- Runtime UI must be built from live Slate widgets, tagged layout, live text/data, and reusable sliced raster chrome assets.
- The reusable FlatStyle part is only the fidelity/process spine: geometry extraction, overlay, tagged dumps, checklist, `VerifyUIFidelity.py`, pass logs, contact sheets, responsive checks.
- The forbidden FlatStyle carryover is its flat visual target and global no-raster-chrome implementation method.
- The implementation pass should create a Friendslop-specific runtime style layer such as `FT66FriendslopStyle`, not expose old `MakeReference*` APIs as the screen-facing contract.
- The Main Menu pilot must use Round06 and refresh the stale Main Menu verification checklist into a FriendslopStyle checklist.
- The draft adds PPF, artifact parity, mechanism manifest, slice/spec, ImageGen runtime chrome rules, correction loop, holistic gestalt gate, responsive gate, and manual interaction gate.

## Evidence Used

- `UI/UI_AGENTS.md`
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`
- `UI/Checklists/pending_issues_Checklists.md`
- `UI/FriendslopStyle/Reference/MainMenu/Round06/manifest.md`
- `Scripts/GenerateUIGeometryOverlay.py --help`
- `Scripts/VerifyUIFidelity.py --help`
- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp`
- `Source/T66/UI/Style/T66FlatStyle.cpp`

## Cross-Review Request

Please review the authored file directly and compare it to your independent answer. Flag:

1. Any contradiction with root/UI process rules that needs to be fixed before user approval.
2. Any missing must-have artifact or gate for preventing a lazy/holistically wrong implementation.
3. Any misleading technical claim about 9-slice/3-slice, Unreal Slate brush usage, ImageGen, or the fidelity loop.
4. Any paths or script commands that are wrong in the current repo.
5. Any wording that could let a later implementation skip user approval, bake text/data, use the full-screen mockup as runtime UI, or call a partial result complete.

Do not edit files. Return concrete corrections only.
