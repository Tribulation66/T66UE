Codex Approval: APPROVE

## Approved Task

Phase 1 of Demo Gating Visibility: move drugs, diploma upgrades, and Steam/secret achievements from demo-gated/coming-soon behavior into available demo content.

## Approved Scope

Approved edits are limited to:

- `Config/DefaultDemoMode.ini`
  - Set demo drug purchases available through the existing release-variant config.
  - Raise the demo diploma cap to the full supported fill-step count.
- `Source/T66/Core/T66BuffSubsystem.cpp`
  - Replace the hard-disabled single-use drug purchase availability with the central release-variant gate.
  - Keep the existing purchase/save path intact unless a compile/runtime issue requires a small local fix in this same subsystem.
- `Source/T66/UI/Screens/T66AchievementsScreen.cpp`
  - Remove or neutralize the demo-mode lock for Steam and secret achievement rows so existing row overlays do not render in demo mode.

## Approved Tool Surface

Claude FullOperator may read files, edit the approved files, run focused searches over the approved source/config areas, run focused compile commands, and produce verification artifacts/logs under `Reports/AgentReviews/DemoGatingVisibility` or existing build/log locations.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Demo/DEMO_AGENTS.md`, `UI/UI_AGENTS.md`, and `Reports/AGENTS.md`.
- Do not use native goal tools.
- Do not run broad Git/LFS status or diffs over Unreal binary asset folders.
- Preserve user changes outside this approved scope.
- This approval covers Phase 1 only. It does not approve Phase 2 docs, Phase 3 UI hiding, or Phase 4 staged standalone refresh.

## Explicitly Excluded Actions

- No Mini/minigame runtime code, asset, capture, or implementation changes.
- No deprecated-feature code changes.
- No hero carousel, difficulty dropdown, Daily Descent, Lab, companion, arcade, or minigame visibility changes in this phase.
- No Git commit, push, tag, reset, checkout, clean, or broad LFS scan.
- No staged standalone refresh in this phase unless compile/capture verification cannot otherwise prove Phase 1 behavior and Codex approval is updated.

## Verification Required After Operator Run

- Report exact files changed.
- Run a focused C++ compile if available and report the command plus pass/fail marker.
- Provide code-level proof that:
  - `AreSingleUseBuffPurchasesAllowed()` now delegates to the central release-variant gate.
  - demo config enables drug purchases.
  - demo config sets the diploma cap to the full `UT66BuffSubsystem::MaxFillStepsPerStat` value.
  - achievement row overlays no longer activate from demo mode.
- If UI capture/dump is practical in this phase, produce it; otherwise explain why it is deferred to the later combined UI proof phase.

## Approval Rationale

The read-only Operator packet identified concrete, localized seams and the user resolved the deprecated-inventory Mini/minigame scope decision as documentation-only. Phase 1 matches the user's requested ordering and does not require Mini/minigame runtime work.
