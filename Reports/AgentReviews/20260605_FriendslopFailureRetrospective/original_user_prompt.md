# Original User Prompt

Ok so it still looks absolutely dog shit and nothing like the reference, get together with claude and seriously we cannot let stuff like this pass. Not only rows dont fit inside their panel the UI elements LOOK NOTHING like the reference, i mean seriously other than buttons sharing the color red, they are NOTHING alike, both you and claude need to really rack your brains on how our whole approach needs to change because it wont be small adjustments to cover this much of a quality gap

## Task Contract

```text
Working task: diagnose why the FriendslopStyle Main Menu pass was falsely accepted, compare the produced capture against the reference, and define a replacement authoring/validation approach before any more implementation.
Operator: Codex
Validator: Claude
Scope: process failure review and replacement plan for FriendslopStyle Main Menu fidelity; no implementation edits in this pass unless the plan explicitly needs a small report artifact.
Stop condition: repo-grounded process-change plan reviewed by Claude, with explicit quality gates and no claim that the current pass is acceptable.
```

## Key Artifact Paths

- Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- Produced capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass09_capture.png`
- Produced contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass09_contact_sheet.png`
- Produced visual scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass09_visual_scorecard.md`
- Prior Claude pass4 that incorrectly greenlit the result: `C:\UE\T66\Saved\AgentReviews\20260605_FriendslopFullScreenRedo\20260605T141002-CrossReview-pass4\claude_review_pass4.md`

## Rules To Apply

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `UI\UI_AGENTS.md`, `UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`, and `UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- This is a planning/failure-analysis pass, not a visual tweak pass.
- Do not defend the prior PASS. Identify why it should have failed and what must change in the authoring and validation pipeline.
