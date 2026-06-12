# FullOperator Prompt: Demo Gating Visibility Phase 1

You are Claude Operator for `C:\UE\T66`; Codex is Validator/Finisher.

Use FullOperator mode only inside the approved Phase 1 scope from:

`Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase1.md`

Do not use native goal tools. Do not commit, push, tag, reset, clean, or run broad
Git/LFS status/diff across Unreal binary folders. Do not inspect or change
Mini/minigame runtime code. The user resolved the deprecated inventory scope as
documentation-only for a later phase; do not implement docs in this phase.

Working task:
Operator: Claude
Validator: Codex
Scope: Phase 1 only. Move drugs, diploma upgrades, and Steam/secret
achievements out of demo/coming-soon locking and into available demo content.
Stop condition: approved files are changed, focused verification is run or
explicitly deferred with reason, and a completion packet is written.

Approved files:
- `Config/DefaultDemoMode.ini`
- `Source/T66/Core/T66BuffSubsystem.cpp`
- `Source/T66/UI/Screens/T66AchievementsScreen.cpp`

Expected implementation:
1. Set `bAllowDrugPurchases=true` in `Config/DefaultDemoMode.ini`.
2. Set `MaxDiplomaUpgradesPerStat` to the full supported count from
   `UT66BuffSubsystem::MaxFillStepsPerStat` (currently expected to be 4; verify
   from source before editing).
3. Replace `UT66BuffSubsystem::AreSingleUseBuffPurchasesAllowed()` hard `false`
   with the central `UT66ReleaseVariantSubsystem::AreDrugPurchasesAllowed()`
   result. If the subsystem include is already available nearby, use existing
   local patterns; otherwise include the smallest required header in the same
   file.
4. Neutralize the demo achievement row lock so Steam and Secret achievement row
   overlays no longer activate in demo mode.

Verification:
- Run the smallest focused compile command available for the T66 C++ target.
- If UI capture/dump is practical inside this phase, capture/dump PowerUp or
  Achievements proof; otherwise explicitly defer capture to the later combined
  UI visibility proof phase and explain why.
- Write a completion packet to:
  `Reports/AgentReviews/DemoGatingVisibility/phase1_completion_packet.md`

Completion packet must include:
- Outcome
- Files changed
- Exact verification commands and pass/fail markers
- Any skipped verification and why
- Token ledger with Claude token count if exposed by the helper manifest or
  otherwise `Unavailable`
- Caveats
