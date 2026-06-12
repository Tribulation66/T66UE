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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayTaskbarHost\implementation_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# AI Usage Tray Taskbar Host - Implementation Review Packet

## Working Goal

Research Windows taskbar embedding examples and update the AI usage widget toward a horizontal taskbar-hosted layout showing `Operator`, `Codex`, then `Claude`, if a practical supported path exists.

## Tier / Roles

- Tier: Tier 1 tooling/UI edit.
- Operator: Codex.
- Validator: Claude.
- Validator helper: `Scripts\Invoke-ClaudePlanReview.ps1`.

## User Constraints

- The widget should be in the Windows taskbar, not floating on the screen.
- The layout should be horizontal.
- Display order must be `Operator`, then `Codex`, then `Claude`.
- The app should continue showing the live current operator and usage percentages.
- User decision after first Validator pass: use the unsupported taskbar-docking technique and treat floating fallback as failure.

## Live Context Checked

- Root instructions: `C:\UE\T66\AGENTS.md`.
- Report routing instructions: `C:\UE\T66\Reports\AGENTS.md`.
- Current tray app source:
  - `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml`
  - `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`
  - `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\T66.AIUsageTray.csproj`
- No app-local `AGENTS.md` file exists under `C:\Users\DoPra\Tools\AIUsageTray`.

## Online Research Summary

- Microsoft has an official DeskBand sample for taskbar-hosted bands/toolbars, which proves there is a legitimate Windows shell-extension route for taskbar embedding: `https://learn.microsoft.com/en-us/samples/microsoft/windows-classic-samples/deskbands/`.
- Real apps have implemented taskbar information displays; `TrafficMonitor` is a public example that shows network speed, CPU, memory, and similar telemetry in the Windows taskbar: `https://github.com/zhongyang219/TrafficMonitor`.
- `EverythingToolbar` is another public taskbar-toolbar example built around the Windows taskbar/deskband pattern: `https://github.com/srwi/EverythingToolbar`.

## Implementation Options Considered

1. COM DeskBand shell extension.
   - Pros: closest official historical taskbar extension model.
   - Cons: heavier engineering surface, registration/unregistration risk, Explorer restart/integration risk, Windows 11 taskbar limitations, overkill for this one local utility.

2. Inject/dock the WPF window as a child of the taskbar window.
   - Pros: small change to existing app, no COM registration, can be validated quickly, matches the class of taskbar telemetry utilities.
   - Cons: unsupported shell-window technique; Windows updates can change taskbar internals, so a fallback is needed.

3. Keep the current floating overlay.
   - Pros: stable and already working.
   - Cons: violates the user's corrected requirement.

## Proposed Plan

Use option 2 as the implementation. Do not preserve option 3 as a success fallback. If taskbar docking fails, keep the tray icon available, log the failure, expose a `Re-dock in taskbar` tray command, and do not claim implementation success.

1. Add a `TaskbarHostService` in the tray app that:
   - Finds the taskbar window (`Shell_TrayWnd`).
   - Finds the notification area (`TrayNotifyWnd`) when available.
   - Changes the WPF window to a visible child window.
   - Parents it to the taskbar and positions it horizontally just left of the notification area.
   - Reports failure if the taskbar host cannot be found or docking fails.

2. Update `MainWindow.xaml`:
   - Resize to a short horizontal bar.
   - Render three compact segments in this order:
     - `Operator: <current operator>`
     - `Codex <weekly remaining percent>`
     - `Claude <weekly remaining percent>`
   - Keep details in the tooltip and tray menu.

3. Update `MainWindow.xaml.cs`:
   - Use taskbar docking on load and on show/re-dock.
   - Keep the existing tray icon/menu, refresh timer, operator menu, usage cache, and log behavior.
   - Add a tray menu item to re-dock if Explorer/taskbar state changes.
   - If docking fails, keep the visible widget hidden and use logs/tray notification rather than floating.

4. Build, publish, launch, and verify:
   - `dotnet build`.
   - `dotnet test`.
   - `dotnet publish` to `%LOCALAPPDATA%\T66UsageTray\app`.
   - Launch the published exe.
   - Confirm the app window's parent is the taskbar window when possible.
   - Capture visual proof showing horizontal `Operator | Codex | Claude`.
   - Check runtime logs.
   - Check for no leaked `codex app-server --listen stdio://` collectors.
   - Scan the runtime folder for obvious token/credential leakage strings.

## Intended Edit Scope

- Edit only the local tray utility under `C:\Users\DoPra\Tools\AIUsageTray`.
- Write review/proof artifacts under `C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayTaskbarHost`.
- Do not edit Unreal gameplay code, minigame code, staged standalone output, or repo workflow rules.

## Risks And Mitigations

- Risk: Windows 11 taskbar internals may reject or hide a child WPF window.
  - Mitigation: explicit failure state with tray re-dock command; floating fallback is not success.
- Risk: Window placement could overlap system tray icons.
  - Mitigation: use `TrayNotifyWnd` bounds when available and place the widget to its left.
- Risk: A transparent child WPF window may render inconsistently under Explorer.
  - Mitigation: keep a compact solid/semi-opaque surface and verify by screenshot.
- Risk: Explorer restart could detach the widget.
  - Mitigation: provide a tray menu `Re-dock in taskbar`.

## Rollback Considerations

- Revert `MainWindow.xaml`, `MainWindow.xaml.cs`, and the new taskbar host service file.
- Republish the previous build if needed.
- No production Unreal assets or T66 gameplay files are touched.

## Verification Evidence Required

- Build success.
- Test success.
- Publish success.
- Running published process.
- Parent-window evidence proving the widget HWND is a child of `Shell_TrayWnd`; floating or only-overlapping results are failure.
- Visual screenshot proof of horizontal order.
- Runtime token scan clean for obvious auth strings.
- No orphan `codex app-server --listen stdio://` collector processes.

## Relevant Current Source Shape

- `MainWindow.xaml` currently defines a `286x92` transparent, topmost, non-taskbar floating overlay. It uses a vertical stack with `Operator` on top and two usage cards below. Current order is Claude then Codex, which must be changed.
- `MainWindow.xaml.cs` currently calls `PositionNearTray()` on load/show. That method positions the floating overlay at `SystemParameters.WorkArea.Right - Width - 12` and `SystemParameters.WorkArea.Bottom - Height - 12`. This positioning path must be removed from the normal success path.
- The current app already has:
  - Refresh timer and collectors for Claude/Codex usage.
  - Tray icon and context menu.
  - Manual operator selection.
  - Usage cache writes.
  - Tooltip with detailed usage.

## Reviewer Request

Review this plan as the Validator. Check for flawed assumptions, unsafe scope, missing files, inadequate verification, contradictions with the repo instructions, or an overcomplicated implementation route. First non-empty line must be exactly one of:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: NEEDS_HUMAN_DECISION`
- `Verdict: BLOCK`

Then provide concise findings.

</review_packet>
