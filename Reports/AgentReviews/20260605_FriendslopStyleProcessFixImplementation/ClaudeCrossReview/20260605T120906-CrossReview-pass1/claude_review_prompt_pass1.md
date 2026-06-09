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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleProcessFixImplementation\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleProcessFixImplementation\codex_final_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleProcessFixImplementation\ClaudeIndependent\20260605T115310-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok go for it

Context:

The user approved implementing the process/tooling solutions proposed after the FriendslopStyle Main Menu pilot was judged not good enough. The prior agreed solution set:

1. Amend FriendslopStyle so runtime chrome quality comes from externally authored/cleaned transparent PNG plates where needed, not generic Unreal-reconstructed rubber atoms.
2. Add containment/fitting checks so rows and child controls cannot overflow parent panels while the verifier still reports PASS.
3. Add a blocking visual fidelity scorecard/holistic gate so `VerifyUIFidelity.py` PASS counts cannot stand in for actual visual fidelity.

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: implement the approved process/tooling fixes for FriendslopStyle fidelity assessment: authored-plate instruction amendment, containment/fitting checks, and a blocking visual scorecard gate. Treat this as process/tooling only, not redoing the Main Menu screen in this pass.
Stop condition: docs and verifier/checklist support are updated, verified with a focused smoke test, and Claude cross-review is clean.
```

Relevant repo evidence:

- `UI/UI_AGENTS.md` routes FriendslopStyle through `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` currently says the reference is a visual-direction target and runtime uses sliced raster chrome atoms rendered by Slate.
- `Scripts/VerifyUIFidelity.py` currently evaluates checklist items, draws a contact sheet, and exits on FAIL count only; it does not enforce holistic visual gates or containment-specific assertions.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` already says parent containment and no clipping are required, but current verifier/checklist support is weak.
- `UI/FriendslopStyle/Checklists/main_menu_checklist.md` has a plain-text Holistic Visual Gate section that `VerifyUIFidelity.py` does not evaluate.

Ask:

Give a repo-grounded independent implementation plan for this process/tooling pass. Focus on what files to edit and what verification should prove the pass. Do not implement; Codex is operator.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

This is a process/tooling pass only — do not re-implement the Main Menu screen. The three approved fixes map cleanly onto specific files. Here is the repo-grounded plan.

**1. Authored-plate instruction amendment** — `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
- Sections 8 (ImageGen Rules), 9 (Slice Rules), and 11 Step E (Runtime Chrome Asset Authoring) currently allow "Unreal-reconstructed" sliced chrome and only *prefer* transparent backgrounds ("If the generator cannot produce reliable alpha, use a flat removable matte"). Tighten this: where a region needs real chrome quality, the runtime brush must come from an externally authored/cleaned transparent PNG plate, not a generic Slate-reconstructed atom. Make alpha-clean PNG the required input, not the fallback.
- Update the Section 2 Method Class line ("Unreal renders those assets as Slate Box/9-slice") and Section 10 to state the plate is the source of quality.
- No change needed to `UI/UI_AGENTS.md` lines 25–26 — the raster-chrome routing already scopes correctly to FriendslopStyle. Only the instructions file body needs the plate-quality amendment.

**2. Containment/fitting checks** — `Scripts/VerifyUIFidelity.py` + checklist support
- The verifier today has `absolute_x/y`, `absolute_width/height`, and `geometry.normalized.*` aliases (lines 46–49) but **no parent-child relationship and no overflow assertion**. A row can overflow its panel and every individual checklist item still PASSes. Add an explicit containment check: a new checklist property (e.g. `contained_in=<ParentTag>` with optional padding tolerance) that fails when the child's absolute rect escapes the parent's absolute rect on any edge.
- **Evidence gap Codex must verify before coding:** confirm the dump JSON actually exposes resolvable parent tags or absolute rects for both child and parent. The verifier indexes widgets flat `by_tag` (lines 319–323) and has no parent linkage. If the dump lacks parent references, the containment property must look up the named parent tag's absolute rect — that works with the existing flat index, so authoring `contained_in=<ParentTag>` is feasible without a dump schema change. Codex should confirm absolute rects are present in a real dump, not assume.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` already states the rule ("no child control escapes its panel", "Do not accept clipped controls"); this pass gives it teeth in the verifier rather than changing the doc.

**3. Blocking visual scorecard gate**
- Today `main()` returns nonzero only on FAIL count (line 463–466); the checklist's "Holistic Visual Gate" section (lines 115–124) is explicitly *not evaluated*. Two viable designs — Codex should pick one:
  - (a) A separate scorecard step that requires a human/structured holistic verdict file to be present and marked PASS before the overall exit code can be 0; or
  - (b) Extend the verifier to treat an unsatisfied holistic-gate manifest as a blocking nonzero exit, so PASS counts alone can't green-light.
- Either way the deliverable is: PASS on the structured items can no longer stand in for visual fidelity — the gate must be able to block even with zero structured FAILs (mirrors Step J's "can fail even when individual checklist items pass").

**Verification that proves the pass (focused smoke test, not a Main Menu redo):**
- A tiny fixture dump + checklist where a child deliberately overflows its parent → verifier must report FAIL / nonzero exit on the new containment property.
- A fixture where all structured items PASS but the holistic gate is unsatisfied → verifier must still block (nonzero).
- A clean fixture → exit 0.
- Run the existing real Main Menu dump/checklist through the amended script to confirm no regression in current parsing.

## Evidence Checked
- `Scripts/VerifyUIFidelity.py` (full) — exit logic on FAIL count only (463–466); aliases incl. absolute rects (46–49); flat `by_tag` index, no parent linkage (319–323); `tag_prefix_count` is the only non-per-widget property.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (full) — Sections 8/9/10/11E on chrome authoring; matte-as-fallback language; Steps J/K/L holistic/responsive/manual gates already exist as prose.
- `UI/FriendslopStyle/Checklists/main_menu_checklist.md` — Holistic Visual Gate section (115–124) explicitly unevaluated.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` — Parent Containment + Visual Gate rules already authored.
- `UI/UI_AGENTS.md` — raster-chrome routing already correctly scoped (25–26).

## Questions Or Blockers
None requiring the user. The path is decided and in scope; Codex can execute. One technical prerequisite Codex must confirm (not the user): that a real dump exposes absolute rects for both child and parent so the containment property is computable.

## Caveats
- Scope discipline: this is tooling/docs only. Resist adding new chrome assets or touching the Main Menu screen this pass.
- The holistic gate cannot be fully automated — design (a) vs (b) determines whether it blocks on a present-but-unsatisfied manifest or requires a human verdict artifact; either is acceptable per the stop condition, but the gate must be *capable of blocking with zero structured FAILs* or it doesn't satisfy fix #3.
- Don't regress existing checklist parsing — the smoke test must include a current real Main Menu run, not just synthetic fixtures.

</validator_independent_answer>

<codex_draft>
# Codex Final Draft For Claude Cross-Review

## Task Contract

```text
Working task:
Operator: Codex
Validator: Claude
Scope: implement the approved process/tooling fixes for FriendslopStyle fidelity assessment: authored-plate instruction amendment, containment/fitting checks, and a blocking visual scorecard gate. Treat this as process/tooling only, not redoing the Main Menu screen in this pass.
Stop condition: docs and verifier/checklist support are updated, verified with a focused smoke test, and Claude cross-review is clean.
```

## Changed Files

- `Scripts/VerifyUIFidelity.py`
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`
- `UI/FriendslopStyle/Checklists/main_menu_checklist.md`
- `UI/FriendslopStyle/Checklists/visual_scorecard_template.md`
- `UI/FriendslopStyle/Elements/main_menu_element_manifest.md`
- `UI/FriendslopStyle/friendslop_asset_registry.md`
- `UI/FriendslopStyle/SliceSpecs/main_menu_slice_specs.md`
- Fixture/proof files under `Saved/Codex/UI/FriendslopStyle/ProcessFix/`

## Summary

Implemented the process/tooling fixes, not a Main Menu visual redo.

Verifier changes:

- Added `--visual-scorecard`.
- Added checklist property `visual_gate=PASS`, which fails when the scorecard is missing, lacks a verdict, or says `FAIL`.
- Added checklist property `contained_in=<ParentTag>` using absolute dump geometry.
- Added optional content-area insets with `contained_in=<ParentTag> inset=<left>,<top>,<right>,<bottom>`.

Process docs:

- Amended FriendslopStyle from generic blank chrome atoms toward transparent authored plates or plate families where quality requires it.
- Clarified that Unreal owns placement/live data/interactivity, while the PNG plate owns premium material quality.
- Added visual scorecard as a required FriendslopStyle acceptance artifact.
- Updated the general UI fidelity loop so structural PASS counts are necessary but not sufficient.
- Updated layout rules to require containment assertions for reference-driven checklists.
- Marked first-pilot Main Menu asset/manifest/slice records as pilot artifacts, not final accepted method.

Main Menu checklist:

- Added containment assertions for center CTA stack, left panel children, and right leaderboard local row inside the panel content inset.
- Added `MainMenu.VisualScorecard | visual_gate=PASS`.

## Verification

Command group run:

- `python -m py_compile .\Scripts\VerifyUIFidelity.py`
- fixture containment failure:
  - report `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\ProcessFix\containment_fail_report.md`
  - output `PASS=1 FAIL=1 UNSURE=0`, exit `1`
- fixture visual failure with scorecard FAIL:
  - report `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\ProcessFix\visual_fail_report.md`
  - output `PASS=1 FAIL=1 UNSURE=0`, exit `1`
- fixture missing scorecard:
  - report `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\ProcessFix\visual_missing_scorecard_report.md`
  - output `PASS=1 FAIL=1 UNSURE=0`, exit `1`
- clean fixture:
  - report `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\ProcessFix\clean_pass_report.md`
  - output `PASS=3 FAIL=0 UNSURE=0`, exit `0`
- amended real Main Menu prior capture:
  - report `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\ProcessFix\main_menu_amended_report.md`
  - output `PASS=96 FAIL=6 UNSURE=0`, exit `1`
  - expected fail lines include left content inset failures, `MainMenu.Right.RankingRowLocal` escaping `MainMenu.Right.LeaderboardPanel inset=32,0,32,0`, and `MainMenu.VisualScorecard` failing from scorecard FAIL.

## Intended Final Answer

Done for the process/tooling pass. I did not redo the Main Menu visuals.

The pipeline now has hard gates for the three failures:

1. FriendslopStyle instructions now require externally authored/cleaned transparent PNG plates or plate families when the visual quality needs them. Generic rubber atoms are no longer enough unless they pass the reference-specific visual scorecard at runtime size.
2. The verifier now supports `contained_in=<ParentTag>` and `contained_in=<ParentTag> inset=<left>,<top>,<right>,<bottom>`, so child rows/controls can fail when they escape their panel or content area.
3. The verifier now supports `visual_gate=PASS` backed by `--visual-scorecard`. Missing scorecard or `Result: FAIL` blocks even if all structured widget assertions pass.

Smoke tests prove overflow fails, failed/missing scorecards block, a clean fixture passes, and the previous Main Menu capture now correctly fails instead of being accepted.

Claude cross-review requested: verify this satisfies the user's `go for it` request as a process/tooling implementation and that no implementation of Main Menu visuals was accidentally claimed.

</codex_draft>
