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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidget\completion_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Codex-Scoped AI Usage Widget - Completion Review Packet

## Working Goal

Investigate whether the AI usage widget can be placed natively inside the Codex desktop app. If there is no supported native Codex app embedding path, update the local widget so it appears only while the user is working in Codex, positioned in the lower-right empty area shown by the user, with horizontal order `Operator | Codex | Claude`.

## Roles

- Operator: Codex.
- Validator: Claude.
- Implementation greenlight: `C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidget\20260528T231659-pass1\claude_review_pass1.md`.

## Native Codex Embedding Finding

Official Codex docs and local app state did not show a supported custom in-app desktop panel/widget surface. Codex supports config, plugins, skills, hooks, apps/connectors, MCP servers, and notifications, but not a documented way to add custom UI inside the Codex desktop app chrome. The implementation therefore uses the user-approved fallback: a companion overlay that only appears while Codex is foreground/active.

## Implemented Changes

Local utility source only under `C:\Users\DoPra\Tools\AIUsageTray`:

- Added `Services\CodexWindowTrackerService.cs`.
  - Finds visible main Codex desktop HWND by process name `Codex`, class `Chrome_WidgetWin_1`, and title `Codex`.
  - Reads `GetForegroundWindow`, root HWND, process ownership, `GetWindowRect`, and minimized/visible state.
  - Returns hidden state unless Codex is the active foreground work surface.
- Updated `MainWindow.xaml`.
  - Compact horizontal layout.
  - Order: `Operator`, `Codex`, `Claude`.
  - No taskbar button, no activation, transparent overlay background.
- Updated `MainWindow.xaml.cs`.
  - Removed taskbar docking startup behavior.
  - Removed `Re-dock in taskbar` tray command and replaced it with `Reposition for Codex`.
  - Uses a 500ms `DispatcherTimer` to reposition/hide/show based on Codex foreground/window geometry.
  - Applies `WS_EX_TOOLWINDOW` and `WS_EX_NOACTIVATE` so the widget does not steal Codex focus.
- Deleted interrupted taskbar implementation files:
  - `Services\TaskbarHostService.cs`
  - `Views\TaskbarWidgetForm.cs`

## Verification Performed

- Build:
  - `dotnet build C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release`
  - Result: succeeded, 0 warnings, 0 errors.
- Tests:
  - `dotnet test C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release --no-build`
  - Result: passed 5/5 tests.
- Publish:
  - `dotnet publish C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\T66.AIUsageTray.csproj -c Release -r win-x64 --self-contained false -o %LOCALAPPDATA%\T66UsageTray\app`
  - Result: succeeded.
- Launched published exe:
  - Running process: `T66.AIUsageTray.exe`, PID `23888`.
- Visible proof while Codex is foreground:
  - Screenshot: `C:\UE\T66\Reports\AgentReviews\20260529_CodexScopedUsageWidget\codex_scoped_widget_visible.png`
  - Shows horizontal `Operator: Codex | Codex 14% | Claude 97%` in the lower-right Codex window area.
- Hidden proof when another app is foreground:
  - Focused Notepad.
  - Probe result: `WidgetVisibleAfterNotepadFocus=false`.
- Move/resize-follow proof:
  - Temporarily restored/moved Codex to `80,80,1600,900`.
  - Widget rect before: `1450,936,1880,980`.
  - Widget rect after move: `1210,884,1640,928`.
  - `Moved=true`.
  - Codex restored to maximized rect `-8,-8,1928,1040`.
- Source cleanup proof:
  - Search for taskbar docking terms under app source only finds `ShowInTaskbar="False"`, no taskbar docking implementation path.
- Runtime cache:
  - Latest cache showed Claude weekly remaining `97%`, Codex weekly remaining `14%`.
- Orphan collector check:
  - No `codex.exe` processes matching `app-server --listen stdio://`.
- Token/credential scan:
  - `rg -n "accessToken|refreshToken|Bearer|Authorization|claudeAiOauth" %LOCALAPPDATA%\T66UsageTray`
  - Result: no matches.
- Runtime log:
  - Latest log has usage refresh success after the new build.
  - Older taskbar log lines remain historical from the interrupted attempt only.

## Caveats

- This is not a native Codex in-app panel because no supported native embedding surface was found.
- The overlay is top-level/no-activate and tracks Codex foreground state; it is intentionally hidden when another app has focus.
- Detection depends on current Codex desktop window class/title (`Chrome_WidgetWin_1`, `Codex`) and may need adjustment if Codex changes its Electron shell window metadata.

## Reviewer Request

Review the completion claims. Check whether the implementation matches the reviewed scope, whether verification proves the user-visible behavior, whether any blocked taskbar behavior remains active, and whether the done report should be revised. First non-empty line must be exactly one of:

- `Verdict: APPROVE`
- `Verdict: REVISE`
- `Verdict: NEEDS_HUMAN_DECISION`
- `Verdict: BLOCK`

Then provide concise findings.

</review_packet>
