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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayWidgetBuild\build_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# AI Usage Tray Widget Build Packet

## Working Goal

Build and launch the Windows AI usage tray widget showing Operator plus Claude/Codex usage, using the reviewed tray plan and keeping it as personal tooling outside the T66 game repo.

## User Request

Pablo said: "go ahead and build the trey I should see it here when youre done" and provided a screenshot of the Windows notification area.

## Request Tier

Tier 1. This creates and launches a local Windows utility.

## Operator / Validator

- Operator: Codex
- Validator: Claude via `Scripts\Invoke-ClaudePlanReview.ps1`

## Applicable Reviewed Plan

Primary plan:

`C:\UE\T66\Reports\AgentReviews\20260528_UsageTrayWidgetPlan\implementation_plan_packet.md`

Last approval:

`C:\UE\T66\Reports\AgentReviews\20260528_UsageTrayWidgetPlan\20260528T211339-pass3\claude_review_pass3.md`

Current build uses the default personal-tool location from that plan:

`C:\Users\DoPra\Tools\AIUsageTray`

Rationale: the plan already defines the personal-tool path as default and repo-local `Tools/UsageTray` only if the user explicitly wants source tracked with T66. The user's go-ahead did not request repo-local source.

## Current Machine Checks

- `dotnet --version`: `9.0.311`
- Windows Desktop runtime available: `Microsoft.WindowsDesktop.App 9.0.13`
- Codex executable exists: `%LOCALAPPDATA%\OpenAI\Codex\bin\codex.exe`
- Claude CLI exists: `%USERPROFILE%\.local\bin\claude.exe`
- Claude credentials file exists: `%USERPROFILE%\.claude\.credentials.json`
- `C:\Users\DoPra\Tools\AIUsageTray` does not exist yet.
- `%LOCALAPPDATA%\T66UsageTray` does not exist yet.

## Implementation Scope

Create a .NET 9 WPF app at:

`C:\Users\DoPra\Tools\AIUsageTray`

Publish output:

`%LOCALAPPDATA%\T66UsageTray\app`

Runtime data:

- `%LOCALAPPDATA%\T66UsageTray\settings.json`
- `%LOCALAPPDATA%\T66UsageTray\operator-state.json`
- `%LOCALAPPDATA%\T66UsageTray\usage-cache.json`
- `%LOCALAPPDATA%\T66UsageTray\logs\usage-tray.log`

Launch the app after build so the compact overlay appears near the Windows notification area.

## Features To Implement

- Borderless, topmost WPF overlay anchored near the bottom-right work area.
- Notification-area tray icon with context menu.
- Visible compact text:
  - `Operator: <Claude|Codex|Unknown>`
  - Claude weekly remaining percent.
  - Codex weekly remaining percent.
- Tooltip/detail text with weekly and five-hour usage/reset status.
- Manual refresh.
- Manual operator choices in tray menu:
  - Claude
  - Codex
  - Unknown
- Operator state stored only as role metadata in `%LOCALAPPDATA%\T66UsageTray\operator-state.json`.
- Claude usage collector:
  - Read `%USERPROFILE%\.claude\.credentials.json` in memory.
  - Use `claudeAiOauth.accessToken` only in memory.
  - Call `https://api.anthropic.com/api/oauth/usage`.
  - Do not log tokens or raw credential JSON.
- Codex usage collector:
  - Use short-lived `codex app-server --listen stdio://`.
  - Parse structured `account/rateLimits/read` output.
  - Clean up app-server process.
- Cache only normalized usage snapshots.
- Log only redacted status/errors.

## Out Of Scope

- Autostart/startup shortcut.
- Installer packaging.
- Repo-local source.
- Unreal/game source or assets.
- Automatic operator-state updates from chat commands.
- Refreshing Claude OAuth tokens automatically.
- Logo licensing work beyond simple local text/letter marks for this first build.

## Verification Plan

- Build:
  - `dotnet build ... -c Release`
  - `dotnet publish ... -c Release -r win-x64 --self-contained false`
- Tests:
  - Include parser/service tests if feasible within the first implementation.
  - At minimum, run live collectors through the app and verify no crash.
- Live verification:
  - Launch published exe.
  - Confirm process is running.
  - Confirm no orphan `codex app-server` process remains after refresh.
  - Confirm app writes no credential tokens to logs.
  - Confirm operator-state file is role metadata only.
  - Confirm the overlay should be visible near the tray.

## Risks

- Claude usage endpoint is private/unstable. Mitigation: isolate collector and show degraded status.
- Codex app-server protocol can change. Mitigation: parse defensively and show degraded status.
- Exact Windows tray cluster embedding is unsupported. Mitigation: tray-adjacent overlay plus tray icon.
- User may want a different default Operator. Mitigation: initial missing state shows `Unknown`; tray menu lets the user set Claude/Codex.

## Review Request

Review whether it is safe to proceed with this implementation under the already reviewed plan and the user's go-ahead, using the personal-tool path without asking another location question.

</review_packet>
