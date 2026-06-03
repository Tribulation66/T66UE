You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidgetVertical\implementation_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
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

</review_packet>
