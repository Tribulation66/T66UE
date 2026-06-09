You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleNextStep\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleNextStep\codex_final_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleNextStep\ClaudeIndependent\20260605T124335-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok what should be done next?

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: answer what should be done next after the FriendslopStyle process/tooling fix, without making implementation changes.
Stop condition: give a concrete next-step sequence that respects the new process and avoids repeating the prior false-positive fidelity pass.
```

Current state:

- The previous FriendslopStyle Main Menu implementation was judged not good enough.
- The pipeline was amended so structured verifier PASS is not enough.
- `VerifyUIFidelity.py` now supports containment checks and visual scorecard gates.
- The amended Main Menu checklist makes the prior capture fail (`PASS=96 FAIL=6 UNSURE=0`) instead of falsely passing.
- The user is asking for the next move, not asking for implementation yet.

Ask:

Give a concise repo-grounded recommendation for the next work step. Focus on practical sequencing and what should be proven before scaling to the full Main Menu again.

</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
# Codex Final Draft For Claude Cross-Review

## Intended Answer

The next step should be a narrow vertical-slice proof, not a full Main Menu redo.

Start with the right leaderboard panel and one row family, because that is where
the previous process visibly failed. This slice exercises all three corrected
areas at once: authored PNG plates, containment/fitting, and the visual
scorecard.

Recommended order:

1. Freeze the slice target from Round06: right panel shell, header, filter area,
   table body, and one leaderboard row.
2. Author transparent PNG plates for that slice using the new method. Do not use
   one generic stretched rubber atom unless it passes against the exact cropped
   reference region at runtime size.
3. Write the slice checklist contract: exact panel/body/row boxes,
   `contained_in` assertions with content insets, row height/width constraints,
   and `visual_gate=PASS`.
4. Implement only that slice in Unreal.
5. Capture, dump, verify, and fill the visual scorecard. Both Codex and Claude
   should inspect the reference/capture/contact sheet before citing PASS counts.
6. If the slice reaches a legitimate PASS, scale the same pattern across the
   rest of Main Menu. If it cannot pass, the blocker is in the asset-authoring
   method, not the broader screen.

The key proof before scaling is that the new gates can produce a real PASS on
one production-quality slice, not just fail the old capture.

## Cross-Review Request

Check if this is the right next-step recommendation and whether it misses any
important caveat from the process/tooling fix.

</codex_draft>
