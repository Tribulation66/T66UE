Result: OK

## Independent Answer

Don't re-run the full Main Menu yet. The amended pipeline is currently only *theoretically* stronger — the new containment + visual-scorecard gates have never actually been driven to a real PASS on anything. Prove the new process end-to-end on one small, load-bearing slice first, then scale.

Recommended sequence:

1. **Pick one vertical slice, not the whole menu.** Choose the element family that exposed the prior false positive most clearly — the right-side leaderboard panel + one row family (that's where the "row too wide for panel" overflow lived per the FailureSolutions draft). It exercises plate authoring, containment, *and* the visual gate in one shot.

2. **Author it the new way (production plates first).** Produce the transparent PNG plates for that slice (panel plate, row plate, header) per Point 1 of the agreed solution — not generic stretched chrome. This is the actual behavior change; if you skip it you'll just reproduce the prior result with stricter gates failing it.

3. **Author the checklist contract for that slice.** Add the containment assertions and the `visual_gate=PASS` item, plus the table/list contract (panel outer bounds, content inset, body clip, first-row bounds, `max row width <= body width`, row geometry relative to body). Confirm `VerifyUIFidelity.py` reads them — note it requires `--visual-scorecard` pointing at a markdown file with a `Result:`/`Overall:` line, or the gate auto-FAILs.

4. **Run the full new DONE definition on the slice:** structured dump clean → containment clean → visual scorecard authored and PASS → both Codex and Claude independently open the image artifacts before reading the PASS count. Any one gate failing = NEEDS_WORK.

5. **Only after the slice clears all gates, scale back to the full Main Menu**, reusing the proven plate-authoring + checklist pattern per family.

What must be proven before scaling: that the new gates can reach a *legitimate* PASS (not just convert the old capture's 96/6 into a fail), and that a containment/overflow regression is actually caught by the checklist rather than the eye. Until one slice demonstrates both, scaling just multiplies unproven work.

## Evidence Checked

- `Scripts/VerifyUIFidelity.py` — confirmed `compare_containment` (L275), `parse_containment_expected` (L325), `load_visual_scorecard_result` (L344), and `visual_gate` item handling (L448); scorecard requires `--visual-scorecard` file with a `Result/Overall/Visual Fidelity/Visual Gate:` line or it FAILs.
- `Reports/AgentReviews/20260605_FriendslopStyleFailureSolutions/codex_solution_draft.md` — the agreed three-point fix (production plates first; layout-lock/containment; redefined DONE + mandatory visual scorecard) and the one-line policy that the verifier is necessary-but-not-sufficient.
- Original prompt + task contract for this step.

## Questions Or Blockers

None requiring the user — this is a recommendation, and the prompt explicitly asks for sequencing only, no implementation.

## Caveats

- I inferred the leaderboard panel + row as the best first slice from the FailureSolutions draft; if Codex/user knows a different family was the worst offender, swap the slice — the sequence holds regardless.
- The new gates have not been exercised to a real PASS in any artifact I found; my "prove the gate can pass legitimately" point is the key risk and should not be skipped.
- Scope respected: read-only, no implementation proposed beyond sequencing.
