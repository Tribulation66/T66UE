User approved implementation.

Working task:
Operator: Codex
Validator: Claude
Scope: Implement the five recommended FriendslopStyle Main Menu process fixes, then replace the center background image with a version of the existing golden statue/skylight image rendered in the same theme/colors but with rubbery Friendslop/Fall Guys/PEAK-like material instead of stone.
Stop condition: Fresh process docs/gates/assets/code, no API/OPENAI_API_KEY use, current capture/dump/verifier/contact sheets/visual scorecard/responsive/manual status, and an honest result. Do not accept structural PASS counts over visual failures.

Required fixes:
1. Stop using inpainted/masked runtime plates for button centers. Reconstruct CTA/topbar/invite-style chrome from reference-derived parameters, not patched text-erasure crops.
2. Add a residual-content gate that fails halos, smears, pillows, patches, or texture discontinuity inside live-content zones.
3. Add a blank-but-different gate that compares rendered runtime crops to reference crops for shape, bevel, gloss direction, outline thickness, highlight placement, and material read.
4. Treat title as its own asset decision. Generic live font is not acceptable if it does not reproduce the reference; create or validate a title-logo pipeline or matching Slate text style.
5. Add per-button topbar text placement geometry gates: label bounds, icon bounds, baseline, minimum padding, vertical centering, and max fill ratio.
6. After the five process fixes, replace the background with a rubbery golden-statue/skylight variant that preserves theme colors and overall statue shape/composition.

Important constraints:
- Follow AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, UI/UI_AGENTS.md, UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md, UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md, and UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md.
- Do not use native goal tools.
- Do not use API imagegen or OPENAI_API_KEY.
- Use built-in account-backed imagegen only for the background asset if needed.
- Runtime UI must remain real Slate UI with live text/data/icons/state and reusable transparent PNG chrome plates where needed. Do not paste the full reference image into UI; do not bake labels/icons/player names/scores/localized text into runtime plates.

Reference:
`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`

Current pass14 capture:
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_capture.png`

Please provide an independent repo-grounded implementation plan, call out risks or blockers, and name the proof artifacts Codex should produce. End with `RESULT: OK` or `RESULT: NEEDS_USER`.
