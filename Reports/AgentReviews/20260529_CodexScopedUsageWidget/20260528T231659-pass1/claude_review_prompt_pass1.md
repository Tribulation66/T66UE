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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidget\implementation_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Codex-Scoped AI Usage Widget - Implementation Review Packet

## Working Goal

Investigate whether the AI usage widget can be placed natively inside the Codex desktop app. If there is no supported native Codex app embedding path, update the local widget so it appears only while the user is working in Codex, positioned in the lower-right empty area shown by the user, with horizontal order `Operator | Codex | Claude`.

## Tier / Roles

- Tier: Tier 1 local tooling/UI edit.
- Operator: Codex.
- Validator: Claude.
- Validator helper: `Scripts\Invoke-ClaudePlanReview.ps1`.

## User Constraints

- Stop pursuing taskbar docking because Windows 11 taskbar composition caused repeated paint/host problems.
- Preferred target: natively placed in the Codex app if supported.
- Fallback target: same general behavior as the original screen overlay, but visible only while Codex is open/active so it is not ever-present on the desktop.
- Requested placement: lower-right area of the Codex app, in the empty space circled by the user.
- Display order remains horizontal: `Operator`, then `Codex`, then `Claude`.

## Live Context Checked

- Root process instructions: `C:\UE\T66\AGENTS.md`.
- Report routing instructions: `C:\UE\T66\Reports\AGENTS.md`.
- Current local tray utility: `C:\Users\DoPra\Tools\AIUsageTray`.
- Existing half-finished taskbar attempt was interrupted and the running `T66.AIUsageTray` process was stopped.
- Live Codex desktop process/window:
  - Main process: `Codex`, path under `C:\Program Files\WindowsApps\OpenAI.Codex_26.519.11010.0_x64__2p2nqsd0c76g0\app\...`
  - Main visible HWND: class `Chrome_WidgetWin_1`, title `Codex`, rect `-8,-8,1928,1040`.
  - Child render HWNDs: `Chrome_RenderWidgetHostHWND` and `Intermediate D3D Window`.
- Official OpenAI Codex docs checked:
  - Codex command overview lists `codex app`, `codex app-server`, plugins, MCP, etc.
  - Codex config reference exposes config keys including apps/connectors, plugins, hooks, MCP servers, notification command hooks, and TUI settings.
  - Codex plugin structure allows skills, hooks, `.app.json`, `.mcp.json`, and assets, but does not define a custom desktop UI panel/widget slot inside the Codex app chrome.

## Feasibility Finding

No supported native Codex desktop embedding surface was found in the official docs or local app shape. Modifying the installed WindowsApps Codex package or injecting a child window into Electron/Chromium internals would be unsupported and update-brittle. The practical implementation should therefore use a companion overlay that tracks the Codex window and only appears when Codex is the active work surface.

## Proposed Implementation

1. Remove the interrupted taskbar-specific implementation path from the active utility:
   - Delete or stop using `TaskbarHostService`.
   - Delete or stop using `TaskbarWidgetForm`.
   - Remove taskbar-specific tray menu wording such as `Re-dock in taskbar`.

2. Add a Codex window tracker service:
   - Locate the main visible Codex desktop HWND by process name `Codex`, class `Chrome_WidgetWin_1`, and title `Codex`.
   - Track whether Codex is foreground/active using `GetForegroundWindow` and process ownership.
   - Read the Codex window rect with `GetWindowRect`.
   - Hide the widget when Codex is closed, minimized, not visible, or not foreground.

3. Restore the app to an overlay window surface rather than a child of taskbar/Codex internals:
   - Borderless, no taskbar button.
   - Topmost only while Codex is foreground.
   - Uses `ShowActivated=false` / no-activate behavior if supported, so it does not steal focus from Codex.
   - Place in lower-right of the Codex window, clamped to screen work area.
   - Keep it compact enough not to cover the composer or progress panel. Initial target: width about `430`, height about `48`, positioned `40px` from the right edge and `52px` above the bottom work-area edge of Codex.

4. Preserve existing data behavior:
   - Claude and Codex usage collectors remain unchanged.
   - Operator state service remains unchanged.
   - Usage cache writes remain unchanged.
   - Tray icon remains available for refresh/operator/exit.

5. Render layout:
   - Horizontal order `Operator | Codex | Claude`.
   - Compact dark bar matching Codex dark UI.
   - Tooltip/detail remains available.

## Intended Edit Scope

- Local utility source only: `C:\Users\DoPra\Tools\AIUsageTray`.
- Report artifacts only: `C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidget`.
- No Unreal gameplay/content/minigame/staged build files.
- No edits to the installed Codex app package under `WindowsApps`.

## Risks And Mitigations

- Risk: Electron window detection changes in a future Codex update.
  - Mitigation: detect by process name plus visible title/class, and fail by hiding rather than showing globally.
- Risk: overlay could cover Codex controls.
  - Mitigation: position in the user-circled lower-right empty zone and verify by screenshot.
- Risk: overlay could remain visible on top of other apps.
  - Mitigation: hide unless foreground window belongs to the Codex process.
- Risk: partial taskbar code could continue to affect behavior.
  - Mitigation: remove taskbar host usage from startup and tray commands.

## Verification Required

- `dotnet build C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release`
- `dotnet test C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release --no-build`
- Publish to `%LOCALAPPDATA%\T66UsageTray\app`.
- Launch published exe.
- Verify widget is visible while Codex is foreground and positioned in the lower-right Codex window region.
- Verify widget hides when Codex is not foreground.
- Screenshot proof of the visible lower-right Codex-scoped widget.
- Runtime log check.
- No orphan `codex app-server --listen stdio://` collector processes.
- Runtime folder scan for obvious auth strings: `accessToken|refreshToken|Bearer|Authorization|claudeAiOauth`.

## Reviewer Request

Review this plan as the Validator. Check for flawed assumptions, unsafe scope, missing files, inadequate verification, contradictions with repo instructions, or a better-supported native Codex embedding route. First non-empty line must be exactly one of:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: NEEDS_HUMAN_DECISION`
- `Verdict: BLOCK`

Then provide concise findings.

</review_packet>
