# Pass14 Reference Gate Result Draft

Operator: Codex

Validator: Claude

Scope: corrected pass14 asset process after user rejected freeform generated
component sheet.

## Work Performed

- Restored the pass13 runtime/source plates that had been overwritten by the
  rejected freeform generated sheet.
- Added a corrected reference-extraction rule to
  `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md`.
- Appended the corrected extraction process to
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass_log.md`.
- Produced reference crops and live-content masks for nine Main Menu component
  families.
- Generated account-backed built-in imagegen candidates for the six primary
  failed families: primary CTA, secondary CTA, search pill, topbar icon plate,
  left panel, and right leaderboard panel.
- Ran the component gate before runtime wiring.

## Key Artifacts

- Reference crop/mask sheet:
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/proof/pass14_reference_crop_mask_sheet.png`
- Candidate gate contact sheet:
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/proof/pass14_candidate_gate_contact_sheet.png`
- Candidate gate report:
  `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/pass14_candidate_component_gate_report.md`
- Decision block:
  `Reports/AgentReviews/20260605_FriendslopProductionRedo/decision_block.md`

## Result

Component gate result: FAIL.

No candidate was wired into runtime. The generated candidates are blank and
content-free, but they drift from the exact reference silhouettes/materials and
remain in `REVIEW` manual status. This prevents another structural-only or
generic-lookalike acceptance.

## Decision Needed

The remaining exact-fidelity paths require user approval:

- allow direct reference-derived runtime plates with strict gates;
- or allow a true local-image/mask inpaint path;
- or explicitly accept approximate built-in-only regeneration.
