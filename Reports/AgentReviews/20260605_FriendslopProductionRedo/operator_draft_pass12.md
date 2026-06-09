# Operator Draft - Pass12 Reference-Crop Fixed Plates

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: implement a new FriendslopStyle Main Menu pass using the approved fixed-size/per-element plate strategy, then verify with current capture/dump/fidelity/visual evidence.
Stop condition: new screen version produced, current evidence reviewed, and any remaining blockers reported rather than papered over.

## User-Identified Problems

1. Previous sliced/buttons appeared cut in half by a visible horizontal/vertical seam.
2. Friend/leaderboard content could overflow its panel or row bounds.
3. Prior UI elements were categorically different from the Round06 reference, especially side panels and central CTA buttons.

## Process Used

- Followed `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- Followed `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- Used the fixed-size plate strategy approved by the user after the structural analysis.
- Kept `CHADPOCALYPSE` as the documented static branding crop; live mutable text/data remains Slate-owned.

## Pass11 Status

Pass11 proved the fixed-size strategy and fixed the split/overflow classes, but it used deterministic generic blank plates. Claude cross-review rejected pass11 as final material-fidelity proof because the plates were not derived from Round06 reference regions. Pass11 is now documented as structural-only and superseded.

Pass11 rejected evidence:

- `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260605T235058-CrossReview-pass7\claude_review_pass7.md`
- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass11_fixture_capture.png`
- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass11_fixture_fidelity.md`

## Pass12 Implementation

Generated 26 reference-crop fixed plates with:

- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_generate_reference_inpaint_plates.py`

The generator writes all three required roots:

- `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\`
- `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`
- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`

Plate method:

- Crop each target plate from the actual Round06 reference region at fixed runtime size.
- Mask baked live-content areas such as labels, avatars, level text, progress bars, row names, rank/score data, and button text.
- Fill masked control interiors from safe Round06 edge/body material when possible.
- Preserve fixed-size image drawing so there is no 9-slice reconstruction seam.
- For left/right panels, preserve the Round06 shell while rebuilding the interior as a blank dark surface so live child rows are not duplicated under Slate content.

## Current Evidence

Reference:

- `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`

Plate evidence:

- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_reference_inpaint_plate_contact_sheet.png`
- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_reference_inpaint_plate_comparison.png`
- `C:\UE\T66\UI\FriendslopStyle\SliceSpecs\main_menu_round06_pass12_reference_inpaint_plate_specs.md`

Fixture evidence:

- Capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_capture.png`
- Dump: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_dump_utf8.json`
- Contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_contact_sheet.png`
- Material crop sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_crop_sheet.png`
- Material verdict: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_verdict.md`
- Visual scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_visual_scorecard.md`
- Verifier report: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_fidelity.md`
- Result: `PASS=251 FAIL=0 UNSURE=0`

Live-data robustness evidence:

- Capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_live_capture.png`
- Dump: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_live_dump_utf8.json`
- Note: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_live_robustness.md`
- Result: `PASS`; live friend rows/action buttons/party slots/leaderboard content remain inside their owning panels.

Documentation updated:

- `C:\UE\T66\UI\FriendslopStyle\friendslop_asset_registry.md`
- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass_log.md`

## Operator Assessment

Pass12 is materially better than pass11 and addresses the three user-raised structural problems:

- Split/cut controls: fixed-size images and current capture show no visible split center or manual text-cover bar artifacts.
- Fitting/overflow: fixture verifier and live capture show rows/actions inside panel bounds.
- Reference mismatch: plates are now derived from Round06 crop regions instead of generic clean-sheet atoms.

Known accepted delta: panel interiors are intentionally cleaned blank shells to avoid baking reference-only child content under live Slate rows. The full-screen reference crop includes child rows inside panels, but runtime must keep those children live.

## Material-Fidelity Verdict

The aggregate `PASS=251 FAIL=0 UNSURE=0` covers topology, containment, live
text, interaction/state checks, and the visual scorecard hook. It is not used as
the sole material proof. Focused equal-pixel crops are documented at
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_crop_sheet.png`
with verdicts in
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_verdict.md`.

Per-element material verdicts:

- Top bar: PASS.
- Left social panel: PASS with accepted delta for cleaned blank live-content interior.
- Right leaderboard/filter panel: PASS with accepted delta for cleaned blank live-content body.
- CTA/button family: PASS.
- Whole-screen glance: PASS with accepted delta for lower panel density caused by live-content blanking.

CTA state note: fixed PNG plates are the base chrome. Existing Slate
click/hover/toggle ownership remains intact through the dump-verified click
handlers and selected/default state routing; the pass did not replace the
buttons with static full-screen art.

## PPF Close

```text
PPF CLOSE
Process used: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md plus UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md and the user-approved fixed-size/per-element plate strategy.
Matches declared process: YES
Evidence: pass12 reference-crop plate contact sheet, fixture capture/dump/contact sheet, visual scorecard, VerifyUIFidelity PASS=251 FAIL=0 UNSURE=0, live-data robustness capture, and pass log/spec/registry updates.
```

## Mechanism Close

```text
MECHANISM CLOSE
Mechanism: Fixed-size plate continuity
Status: PRESENT
Evidence: pass12 uses fixed-size PNG image plates for the 26 target controls/panels and no runtime 9-slice reconstruction.
Discriminator test: no visible split-center bands or manual text-cover bars appear in the fixture capture.
Reported status: FULL

MECHANISM CLOSE
Mechanism: Live content containment
Status: PRESENT
Evidence: fixture verifier containment rows pass, and live-data robustness capture shows current friend rows/action buttons/party slots inside the left panel.
Discriminator test: rows fit inside their owner panels in both deterministic fixture and live runtime data paths.
Reported status: FULL

MECHANISM CLOSE
Mechanism: Reference-derived material
Status: PRESENT
Evidence: pass12 generator crops each runtime plate from the corresponding Round06 region and masks live-content areas instead of using generic blank clean-sheet atoms.
Discriminator test: current plate contact sheet shows Round06 source crops beside cleaned runtime plates, with baked labels/player data removed.
Reported status: FULL
```
