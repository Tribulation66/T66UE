# FriendslopStyle Main Menu Pass11 Fixed-Plate Implementation Request

Read-only validator pass first. Do not edit files. Give implementation advice and risks for Codex to apply.

## User approval

The user approved proceeding after the structural solution gate. The approved direction is to prefer size-specific fixed-image plates for CTAs and large panels when slicing would damage Round06 fidelity.

## Task contract

Working task:
Operator: Codex
Validator: Claude
Scope: implement a new FriendslopStyle Main Menu pass using the approved fixed-size/per-element plate strategy, then verify with current capture/dump/fidelity/visual evidence.
Stop condition: new screen version produced, current evidence reviewed, and any remaining blockers reported rather than papered over.

## Process rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `UI/UI_AGENTS.md`, `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`, `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`, and `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- Do not use a blind full-screen mockup as runtime UI.
- Runtime chrome must be reusable transparent plates or plate families. Live Slate must keep labels, player data, scores, counts, handlers, and states.
- Generic blank rubber atoms are not sufficient; per-element/per-size plates must match the Round06 reference category.
- Fixed-size plates are approved when they preserve fidelity better than slicing.
- The previous clean alpha sheet/source is inadequate. Pass10 failed the visual scorecard because top bar material, left panel, right panel, CTA/button family, and whole-screen glance did not match Round06.

## Current evidence

- Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- Pass10 capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_capture.png`
- Pass10 visual scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_visual_scorecard.md`, `Result: FAIL`
- Clean-sheet slice spec says plates were generated from `friendslop_mainmenu_runtime_chrome_sheet_alpha.png` by 9-slice compositing clean caps/centers.
- `T66FriendslopStyle.cpp` currently loads Round06 assets as fixed `Image` brushes with zero margin, so visible seams in Round06 assets are baked into the generated PNG plates, not caused by live Slate slicing.

## Ask

Provide independent repo-grounded implementation guidance for the pass:

1. What files/assets should Codex inspect and likely change?
2. What implementation sequence best preserves the process rules?
3. What traps would cause another false pass?
4. What should the verification checklist/scorecard include beyond the current structured report?
5. Any concerns about using fixed-image plates for panels/CTAs in this specific screen?
