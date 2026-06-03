# Decision Block - AI Usage Tray Taskbar Host

## Working Goal

Research Windows taskbar embedding examples and update the AI usage widget toward a horizontal taskbar-hosted layout showing `Operator`, `Codex`, then `Claude`, if a practical supported path exists.

## Review Artifact

`C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayTaskbarHost\20260528T224118-pass1\claude_review_pass1.md`

## Validator Verdict

`Verdict: NEEDS_HUMAN_DECISION`

## Decision Needed

The Validator found that the supported Windows DeskBand route is not a reliable Windows 11 taskbar path for this machine, while the practical app-level route is an unsupported `Shell_TrayWnd` child-window docking technique. This can still be attempted, but the user must accept the Windows-update/Explorer-detach risk.

## User Options

1. Accept the unsupported taskbar-docking technique and treat floating fallback as failure. SELECTED by user.
2. Accept the unsupported taskbar-docking technique and allow floating fallback if docking fails.
3. Do not use unsupported docking; switch to a thin pinned overlay, tray flyout, or another target.

## Safe Default

Option 1 best matches the user's corrected requirement: taskbar-hosted only, horizontal order `Operator | Codex | Claude`, and no success claim if the app floats.

## Required Follow-Up

After the user chooses an option, update or replace the implementation packet if needed, rerun Validator review if the scope changed, then implement and verify.

## User Decision

The user chose option 1: use the unsupported taskbar-docking technique and treat floating fallback as failure.
