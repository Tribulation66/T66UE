Codex Approval: APPROVE

## Approved Task

Phase 4 of Demo Gating Visibility: refresh the staged standalone build, verify the standalone shortcut target, produce Unreal-owned UI proof for the demo-gating changes, and write the final completion packet.

## Approved Scope

Approved write locations:

- `Saved/StagedBuilds/Windows/T66/**` and staged build side effects produced by `Scripts/StageStandaloneBuild.ps1`.
- `T66 Standalone.lnk` and the pinned taskbar shortcut only through `Scripts/StageStandaloneBuild.ps1`.
- `Reports/Proof/DemoGatingVisibility/**` for screenshots, widget dumps, logs, and raw proof metadata.
- `Reports/AgentReviews/DemoGatingVisibility/phase4_completion_packet.md`.

Approved commands/tooling:

- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`
- `Scripts/CaptureT66UIScreen.ps1`
- `Scripts/CaptureT66UIWidget.ps1`
- Focused PowerShell checks of shortcut target/arguments and proof files.
- Focused text searches over logs/dumps/proof files.

## Approved Tool Surface

Claude FullOperator may run staging, launch the staged executable through the capture scripts, write proof outputs under `Reports/Proof/DemoGatingVisibility`, and inspect resulting screenshots/dumps/logs as needed. It may not edit source/config/docs in this phase.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Demo/DEMO_AGENTS.md`, `UI/UI_AGENTS.md`, and `Reports/AGENTS.md`.
- Use Unreal-owned capture/dump routes, not desktop screenshots.
- Verify the shortcut target is `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Do not use native goal tools.
- Do not run broad Git/LFS scans.

## Explicitly Excluded Actions

- No source/config/docs edits.
- No Mini/minigame runtime work.
- No Git commit, push, tag, reset, checkout, clean, or broad LFS scan.
- No destructive cleanup of proof/build folders beyond what the staging script normally owns.

## Verification Required After Operator Run

- StageStandaloneBuild pass/fail marker and staged exe path.
- Shortcut target/arguments/working directory proof for the repo shortcut and pinned taskbar shortcut when present.
- Unreal-owned screenshots and/or widget dumps for:
  - MainMenu: Daily Descent CTA hidden in demo.
  - HeroSelection: demo hero carousel has no `COMING SOON` placeholder entries and difficulty surface shows `Easy`.
  - PowerUp/Diplomas and PowerUp/Drugs: moved-to-available surfaces no longer show coming-soon overlays.
  - Achievements Steam and Secret: moved-to-available surfaces no longer show coming-soon overlays.
- If a requested capture/dump cannot be produced, record the exact command failure and preserve any logs.
- Completion packet must classify final status FULL/PARTIAL and list any visual caveats, especially the hero carousel wraparound behavior if present.

## Approval Rationale

The code/docs phases have compiled and now need the repo-required staged standalone and Unreal-owned UI proof before the user-facing completion report.
