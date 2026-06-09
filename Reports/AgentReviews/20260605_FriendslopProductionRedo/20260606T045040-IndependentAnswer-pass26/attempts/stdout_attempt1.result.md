Result: OK

## Independent Answer

This is a self-contained implementation-planning request. The user already approved implementation, scope is fixed, all governing docs exist in-repo, and the only generative tool needed (account-backed imagegen for the background) is explicitly permitted. Codex can execute internally. Below is the independent plan Codex should follow, grounded in `UI_FIDELITY_LOOP_INSTRUCTIONS.md` and the current pass14 state.

**Important context the plan must respect:** pass14 already reports `PASS=264 FAIL=0 UNSURE=0` with a `Result: PASS` visual scorecard. The five process fixes are deliberately gate-*hardening* fixes targeting failure modes (inpaint halos, blank-but-wrong plates, title fidelity, topbar text geometry) that the current gates rated PASS. **Expect pass15 to regress to FAIL when the new gates land — that is the correct, honest outcome, not a defect.** Do not retune thresholds to preserve the 264/0/0 banner. The stop condition explicitly forbids accepting structural PASS counts over visual failures.

Recommended sequence (process fixes first, background last):

1. **Kill inpaint/text-erasure plates (fix 1).** `pass12_generate_reference_inpaint_plates.py` is the offending lineage. Replace plate generation with a parametric reconstruction script (`pass15_generate_reconstructed_plates.py`) that builds CTA / topbar-pill / invite / search / panel chrome from reference-derived parameters (corner radius, bevel profile, gloss gradient, outline weight, fill color) — not from masked crops of the reference. Output transparent PNGs only; no baked labels/icons/names/scores.
2. **Residual-content gate (fix 2).** New checker that crops each live-content zone from the runtime capture and fails on halos, smears, pillow-shading, patch seams, or texture discontinuity. Wire it as a blocking checklist item, not advisory.
3. **Blank-but-different gate (fix 3).** Compare reconstructed runtime plate crops vs reference crops on shape, bevel, gloss direction, outline thickness, highlight placement, material read. A plate that is "blank" but the wrong shape/material must FAIL.
4. **Title asset decision (fix 4).** Current title is live Slate `CHADPOCALYPSE` passing `is_label=true`. Per the prompt, generic live font is unacceptable if it does not reproduce the reference. Codex should attempt a matching Slate text style or a title-logo pipeline and gate it against the reference title crop. If neither convincingly reproduces the reference, escalate the title-as-asset question to Pablo via the pass log rather than silently shipping the generic font.
5. **Per-button topbar text geometry gates (fix 5).** Add label-bounds, icon-bounds, baseline, min-padding, vertical-centering, and max-fill-ratio assertions per topbar button (Settings/Globe/Account/Profile/PowerUp/Achievements/Ticket/Power). These extend the existing Containment section in `main_menu_checklist.md`.
6. **Background swap (fix 6) — only after 1–5 pass.** Generate a rubbery Friendslop/Fall Guys/PEAK-material variant of the golden statue/skylight via account-backed imagegen, preserving theme colors and overall statue shape/composition. Use the existing center background image as visual context per the Section 9 reference-crop workflow. No API imagegen, no `OPENAI_API_KEY`.

After each change, run the full loop (capture → dump → VerifyUIFidelity.py with `--visual-scorecard` → triage). Honor the iteration cap / ESCALATE rules in Section 8.

## Evidence Checked
- Prompt file `pass15_process_and_background_prompt.md` — read in full; matches the task brief.
- Reference image and pass14 capture — both exist at the cited paths.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` — loop steps, visual-gate semantics, "structural PASS ≠ visual match" (§2.3), content-stub/imagegen policy (§9), escalation (§8).
- `pass14_verify_report.md` (264/0/0, visual_gate=PASS) and `pass14_visual_scorecard.md` (Result: PASS) — establishes current baseline.
- MainMenu Codex artifact dir — confirms inpaint-plate lineage (`pass11_generate_fixed_plates.py`, `pass12_generate_reference_inpaint_plates.py`) and existing scorecard/checklist/responsive/manual infrastructure.

## Questions Or Blockers
None requiring the user right now. Two items Codex must self-manage and surface only if they actually block:
- **Account-backed imagegen availability (fix 6).** If the built-in account imagegen is not actually invocable in Codex's environment, the background swap cannot complete and that specific deliverable escalates to Pablo. API imagegen / `OPENAI_API_KEY` is not an allowed fallback.
- **Title fidelity (fix 4).** If neither a matching Slate style nor a title-logo asset convincingly reproduces the reference, the "is generic font acceptable?" call is Pablo's, not Codex's — route it to the pass log/escalation rather than accepting silently.

## Caveats
- Proof artifacts Codex should produce for pass15: `pass15_generate_reconstructed_plates.py` + regenerated transparent plates; updated `main_menu_checklist.md` with new residual-content, blank-but-different, title, and topbar-geometry gates; `friendslop_pass15_fixture_capture.png` + dump (+utf8); `friendslop_pass15_fixture_dump`/verify report; `pass15_visual_scorecard.md` with an honest verdict; `pass15_component_crop_sheet.png` and reference-component gate output; responsive captures (1280×720, 1600×900) + responsive report; updated `manual_interaction_checklist.md`; the new background asset + its imagegen prompt/source-crop recorded in the registry; and a `pass_log.md` pass15 entry.
- The new gates are markdown-scorecard-backed, not automated image similarity (per §6.1) — both Codex and this Validator must visually inspect reference/capture/contact-sheet before any `Result: PASS`.
- Do not let the background swap (fix 6) start before the five process fixes verify; bundling them risks masking a plate regression behind a new background.
- Runtime must stay real Slate with live text/data/icons/state; plates remain reusable transparent chrome with nothing baked in.

Result: OK
