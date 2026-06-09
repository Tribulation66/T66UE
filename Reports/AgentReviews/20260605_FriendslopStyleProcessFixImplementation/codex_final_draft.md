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
