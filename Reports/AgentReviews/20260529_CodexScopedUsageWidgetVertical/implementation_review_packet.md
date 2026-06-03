# Codex-Scoped Usage Widget Vertical Layout - Implementation Review Packet

## Working Goal

Update the existing Codex-scoped AI usage widget to use a larger vertical layout: `Operator` at the top, `Claude` below it, and `Codex` below Claude, with no small `X` or `C` badge logos.

## Tier / Roles

- Tier: Tier 1 local tooling/UI edit.
- Operator: Codex.
- Validator: Claude.
- Validator helper: `Scripts\Invoke-ClaudePlanReview.ps1`.

## User Constraints

- Keep the Codex-scoped behavior from the previous pass: visible only while Codex is open/active, not ever-present on the desktop.
- Change away from the horizontal layout.
- Layout order must be:
  1. `Operator`
  2. `Claude`
  3. `Codex`
- Make the font bigger.
- Remove the small `X` and `C` logos/badges.

## Live Context Checked

- Root instructions: `C:\UE\T66\AGENTS.md` in the thread context.
- Report routing instructions: `C:\UE\T66\Reports\AGENTS.md`.
- App-local instruction search: no `*AGENTS.md` under `C:\Users\DoPra\Tools\AIUsageTray`.
- Current source:
  - `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml`
  - `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`
  - `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\Services\CodexWindowTrackerService.cs`
- Current behavior:
  - Width `430`, height `44`, horizontal `Operator | Codex | Claude`.
  - Uses `CodexWindowTrackerService` with a 500ms placement timer.
  - The positioning formula uses window `Width`/`Height`, so increasing `Height` should automatically keep the widget anchored above the Codex lower-right area.

## Proposed Implementation

1. Patch `MainWindow.xaml` only unless build errors require code-behind adjustment.
   - Set a taller compact overlay size, approximately `260x118`.
   - Replace the three-column grid with a three-row stack/grid.
   - Row 1: `Operator: <value>` with larger text.
   - Row 2: `Claude <percent>` with larger text.
   - Row 3: `Codex <percent>` with larger text.
   - Remove all badge `Border`/`TextBlock` elements for `X` and `C`.
   - Preserve `x:Name` bindings: `OperatorText`, `ClaudeText`, `CodexText`, `StatusText`.

2. Preserve `MainWindow.xaml.cs` behavior:
   - Keep refresh/timer/collector/cache logic unchanged.
   - Keep Codex foreground-only show/hide behavior unchanged.
   - Keep no-activate/tool-window styles unchanged.
   - Keep tray menu unchanged.

3. Build, test, publish, and launch.

## Intended Edit Scope

- Edit only the local utility under `C:\Users\DoPra\Tools\AIUsageTray`.
- Write proof/review artifacts under `C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidgetVertical`.
- No Unreal gameplay/content/minigame/staged build files.
- No Codex app package edits.

## Risks / Rollback

- Risk: taller widget may cover more Codex content than the horizontal bar.
  - Mitigation: keep width narrower, verify screenshot in the user-requested lower-right area.
- Risk: text could crowd in a compact vertical widget.
  - Mitigation: use larger but bounded font sizes and ellipsis for operator.
- Rollback: revert `MainWindow.xaml` to the prior horizontal layout and republish.

## Verification Required

- `dotnet build C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release`
- `dotnet test C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release --no-build`
- Publish to `%LOCALAPPDATA%\T66UsageTray\app`.
- Launch published exe.
- Screenshot proof while Codex is foreground showing vertical `Operator`, `Claude`, `Codex`, larger text, and no X/C badges.
- Confirm the widget hides when Codex is not foreground.
- Runtime log check.
- No orphan `codex app-server --listen stdio://` collector processes.
- Runtime folder scan for obvious auth strings: `accessToken|refreshToken|Bearer|Authorization|claudeAiOauth`.

## Reviewer Request

Review this plan as the Validator. Check for flawed assumptions, unsafe scope, missing files, inadequate verification, contradictions with repo instructions, or any reason this should not proceed as a small Codex-scoped layout change. First non-empty line must be exactly one of:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: NEEDS_HUMAN_DECISION`
- `Verdict: BLOCK`

Then provide concise findings.
