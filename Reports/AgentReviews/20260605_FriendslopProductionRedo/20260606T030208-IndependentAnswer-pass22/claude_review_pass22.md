Result: OK

## Independent Answer

The user picked decision-block option 1: derive runtime plates from exact Round06 reference crops, remove only the masked live-content zones locally, run the strict reference component gate, and ship a plate only on a true PASS. No API/CLI/`OPENAI_API_KEY`. This is approved policy — proceed.

**Yes, record the exception explicitly.** The prior "do not ship crop/masked runtime plates" boundary lives in three places that currently still forbid it, so they must be reconciled before any plate is wired, or the next agent will correctly self-block:

1. `decision_block.md` lines 20-24 and the pass14 contract addendum lines 16-18, 24 ("not permission to ship direct cropped reference pixels as runtime plates").
2. `fresh_agent_main_menu_pass14_prompt_final.md` line 115 ("Do not repeat... using crop/inpaint/masked plates as runtime assets").
3. The pass log's Pass12 entries and the Pass09 posthoc invalidation, which is the institutional reason the no-crop rule exists.

Record the exception as a dated, scoped amendment (user-approved 2026-06-06) in:
- `pass_log.md` — a new Pass14 entry noting the user-approved boundary change and citing the decision-block + user-decision text.
- `main_menu_pass14_component_contract_addendum.md` — amend the "Corrected Pass14 Extraction Rule" so the crop-derived plate path is permitted *under the gate*, superseding the current "not permission to ship" wording.

Keep the amendment narrow: crop-*derived* plates with live-content zones removed, gated, are now allowed; raw unmodified crops and API/CLI remain forbidden.

**Critical risks / stop conditions before runtime wiring:**

- **Gate currently fails everything.** `pass14_candidate_component_gate_report.md` shows all six families auto-FAIL (IoU 0.14–0.66, every Manual Status = REVIEW, `Result: FAIL`). Policy approval does not lower the bar — no plate is wired until its row flips to PASS with no remnant/smudge. The approval changes the *source method*, not the gate threshold.
- **Content-removal completeness.** The whole failure history (Pass09/Pass12 false accepts) came from partially-erased reference children. Every text/glyph/skull/score/avatar/checkbox zone in the mask must be fully blanked, or it is the same "looks masked/painted over" failure the user rejected.
- **Acceptance precedence unchanged.** `VerifyUIFidelity` `FAIL=0` is still not acceptance; `pass14_visual_scorecard.md Result: PASS` is the holistic gate (addendum lines 87-94). Do not call pass14 done on structural counts.
- **Title is out of scope for this exception.** Per addendum/parity gate, the title becomes live layered Slate text, not a crop plate — keep it that way.
- **No silent fallback.** If crop-derived + gate still can't pass a family, stop for the user rather than shipping a lookalike or reaching for API/CLI.

## Evidence Checked

- `pass14_direct_reference_plate_prompt.md` (the prompt under review)
- `decision_block.md` — option 1 text and the three blocking reasons
- `fresh_agent_main_menu_pass14_prompt_final.md` — pass14 contract, no-crop mistake list, acceptance bar
- `pass14_reference_component_gate/pass14_candidate_component_gate_report.md` — all 6 families FAIL/REVIEW, `Result: FAIL`
- `main_menu_pass14_component_contract_addendum.md` — current extraction rule still says crops are not shippable
- `pass_log.md` — Pass09 posthoc invalidation and Pass12 crop-plate history (origin of the no-crop rule)

## Questions Or Blockers

None requiring the user — the decision is already made. The work is Codex's to execute. Flagging only that the documentation reconciliation above is a prerequisite, not optional.

## Caveats

- I did not visually inspect the crop/mask/contact images themselves; the gate report's numeric FAILs are my basis for the "everything currently fails" claim.
- "Strict gate" is only meaningful if the gate thresholds aren't quietly relaxed to make the approved method pass; verify the gate script wasn't loosened alongside the policy change.
- The exception is scoped to the Main Menu pass14 families. It should not be read as a global FriendslopStyle license to ship crop-derived plates on future screens without the same gate.
