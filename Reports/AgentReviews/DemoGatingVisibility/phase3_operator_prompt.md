# FullOperator Prompt: Demo Gating Visibility Phase 3

You are Claude Operator for `C:\UE\T66`; Codex is Validator/Finisher.

Use FullOperator mode only inside the approved Phase 3 scope from:

`Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase3.md`

Do not use native goal tools. Do not commit, push, tag, reset, clean, run broad
Git/LFS scans, or inspect Unreal binary asset folders. Do not edit
Mini/minigame/deprecated runtime code.

Working task:
Operator: Claude
Validator: Codex
Scope: Phase 3 only. Hide non-deprecated demo-gated UI entries instead of
showing `COMING SOON`, then update demo docs/rules to match.
Stop condition: approved code/docs are changed, focused compile passes or the
failure is reported with exact blockers, and a completion packet is written.

Behavior targets:
- Hero selection carousel and hero grid should only show demo-playable heroes.
- Companion selection/grid should only show demo-playable companions where the
  demo gate is the reason for lock/overlay.
- Difficulty lists/dropdowns in approved non-Mini UI should only expose playable
  difficulties in demo, so demo shows Easy only.
- Lab button should be omitted when `IsRunCategoryPlayable(Lab)` is false.
- Daily Descent main-menu CTA should be omitted when unavailable in demo, while
  click/navigation guards remain.
- Extra casino game entries may be hidden if they are demo-gated and in the
  approved casino UI files. Do not touch deprecated arcade/minigame runtime code.
- The shared overlay helper stays available for any remaining intentional users.

Docs:
- Update `Demo/DEMO_RELEASE_INSTRUCTIONS.md` so it no longer says unavailable
  visible UI should use `COMING SOON`; it should say demo-gated content should be
  hidden from visible UI while backend/navigation guards remain.
- Update `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` so entries no longer describe
  the current target state as "listed but overlaid" after Phase 3.

Verification:
- Run the smallest focused C++ compile command available for T66.
- Do not run staged standalone; Phase 4 owns staging and shortcut verification.
- Write a completion packet to:
  `Reports/AgentReviews/DemoGatingVisibility/phase3_completion_packet.md`

Completion packet must include:
- Outcome
- Files changed
- Exact verification commands and pass/fail markers
- Code-level proof for heroes, companions, difficulties, Lab, Daily Descent
- Remaining overlay usage classification
- Token ledger with Claude token count if exposed by helper manifest or
  otherwise `Unavailable`
- Caveats
