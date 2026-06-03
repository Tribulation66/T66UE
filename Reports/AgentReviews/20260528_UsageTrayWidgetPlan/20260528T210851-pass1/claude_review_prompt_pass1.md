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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_UsageTrayWidgetPlan\implementation_plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Claude/Codex Usage Tray Widget Implementation Plan Packet

## Working Goal

Create a full reviewed implementation plan for a Windows Claude/Codex usage tray widget showing each provider logo with weekly remaining percent, without building or executing it yet.

## User Request

The user wants a small Windows taskbar-area widget that shows the Claude and Codex logos with the weekly remaining percentage next to each logo. They explicitly said not to build or execute it yet, and asked for the full plan to execute it.

## Clarified Goal

Plan a local Windows utility that displays two compact provider rows/icons:

- Claude logo + weekly remaining percent
- Codex logo + weekly remaining percent

The utility should be suitable for the taskbar/system-tray area and backed by structured usage data discovered in the previous research pass.

## Safe Assumptions

- Planning-only for this turn. No production code, app scaffolding, install, autostart registration, or persistent process launch will be performed.
- Default implementation target should be a native Windows utility under `C:\UE\T66\Tools\UsageTray\` unless the user later chooses to keep it outside the game repo.
- The visible surface should be a small always-on-top, borderless, taskbar-adjacent WPF window anchored near the notification area, paired with a normal notification-area tray icon for settings/show/hide/exit.
- Reason: Windows 11 supports notification icons, but does not provide a normal supported API for arbitrary persistent text controls inside the exact clock/battery/system-tray cluster. A tray-adjacent overlay is the lowest-risk way to get "logos + percent next to them" instead of forcing unreadable text into a 16-32px tray icon.
- Weekly display should mean remaining percentage, calculated as `100 - usedPercent` / `100 - utilization`.
- Tooltip or click popup should include both weekly and 5-hour windows plus reset times.
- Mini/minigame systems are out of scope.

## Applicable Repo Instructions

- `AGENTS.md`
  - Goal created before planning.
  - Live repo and machine state checked.
  - Planning-only boundary must be respected.
  - Claude cross-review required before substantive output.
  - Quota denominator inference/routing must not be guessed; use structured values already discovered.
  - Report artifacts under `Reports/`.
  - Avoid broad Git/LFS scans.
- `Tools/README.md`
  - `Tools` is for durable operator tools that are not Unreal runtime code or editor Python scripts.
  - Master tools should be small, reusable, and documented.
  - Generated logs/temp folders should stay outside durable tree or be deleted after review.
- `Reports/AGENTS.md`
  - Use `Reports/AgentReviews` for review packets and reviewer outputs.

No `Tools/pending_issues_*.md` exists at the time of this plan.

## Current Machine State Checked

- `.NET SDK 9.0.311` installed.
- Windows Desktop runtimes installed: `Microsoft.WindowsDesktop.App 8.0.x` and `9.0.13`.
- Node `v22.18.0` and npm `11.5.2` available, but .NET should be preferred for native tray/overlay UI.
- PowerShell 5.1 and PowerShell 7.5.5 available.
- Codex executable exists at `C:\Users\DoPra\AppData\Local\OpenAI\Codex\bin\codex.exe`.
- Claude CLI executable exists under `C:\Users\DoPra\AppData\Roaming\Claude\claude-code\...` and `C:\Users\DoPra\.local\bin\claude.exe`.
- Claude credentials exist at `C:\Users\DoPra\.claude\.credentials.json`; implementation must not print or persist token values.

## Data Sources Already Proven

### Codex

Use local Codex app-server:

```text
codex app-server --listen stdio://
initialize
initialized
account/rateLimits/read
```

Known returned shape:

- `rateLimits.primary.usedPercent`
- `rateLimits.primary.windowDurationMins` = 300 for 5-hour window
- `rateLimits.primary.resetsAt`
- `rateLimits.secondary.usedPercent`
- `rateLimits.secondary.windowDurationMins` = 10080 for weekly window
- `rateLimits.secondary.resetsAt`
- `rateLimitsByLimitId` for additional model-specific limits such as Codex Spark

Current prior probe returned weekly `usedPercent = 82`, so remaining was about `18%`.

### Claude

Preferred implementation path:

- Use documented Claude Code statusline JSON if a clean non-interactive way is available. It includes `rate_limits.five_hour.used_percentage`, `rate_limits.seven_day.used_percentage`, and reset timestamps.

Fallback/currently proven path:

- Read `C:\Users\DoPra\.claude\.credentials.json` in memory.
- Use `claudeAiOauth.accessToken` only in memory.
- GET `https://api.anthropic.com/api/oauth/usage`.
- Normalize returned:
  - `five_hour.utilization`
  - `five_hour.resets_at`
  - `seven_day.utilization`
  - `seven_day.resets_at`
  - optional model-specific weekly fields such as `seven_day_sonnet`

Current prior probe returned weekly `utilization = 0.0`, matching the screenshot.

Security boundary:

- Never log, cache, print, or write access/refresh tokens.
- Cache only normalized usage snapshots and timestamps.

## PPF Check

```text
PPF CHECK
Objective: Build a small Windows taskbar-area usage widget showing Claude and Codex logos with weekly remaining percentages.
Proven process: Existing local AI usage monitors and statusline widgets use structured provider data first: Codex app-server account/rateLimits/read, Claude Code statusline JSON or local OAuth usage endpoint; Windows tray utilities use NotifyIcon plus a popup/overlay for richer UI.
My planned implementation: A native .NET Windows Desktop/WPF app with a tray icon, taskbar-adjacent always-on-top compact window, structured collectors for Claude and Codex, no OCR by default, and safe credential handling.
Same method class: YES
If NO, why: N/A
User approval required before proceeding: YES, because the user explicitly requested planning only and has not approved implementation yet.
Verification evidence: Collector unit tests with fixture JSON, live read-only probes for Claude and Codex, visible UI screenshot, startup shortcut validation if autostart is enabled, and a manual restart/login-expiry smoke test.
```

## Intended Files/Folders If Implemented

Primary repo-local location:

- `Tools/UsageTray/README.md`
- `Tools/UsageTray/T66.AIUsageTray.sln`
- `Tools/UsageTray/src/T66.AIUsageTray/T66.AIUsageTray.csproj`
- `Tools/UsageTray/src/T66.AIUsageTray/App.xaml`
- `Tools/UsageTray/src/T66.AIUsageTray/App.xaml.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/MainWindow.xaml`
- `Tools/UsageTray/src/T66.AIUsageTray/MainWindow.xaml.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Tray/TrayController.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Collectors/CodexUsageCollector.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Collectors/ClaudeUsageCollector.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Models/UsageSnapshot.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Services/PollingUsageService.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Services/SettingsService.cs`
- `Tools/UsageTray/src/T66.AIUsageTray/Assets/`
- `Tools/UsageTray/src/T66.AIUsageTray.Tests/`

Generated/user-local runtime files:

- `%LOCALAPPDATA%\T66UsageTray\settings.json`
- `%LOCALAPPDATA%\T66UsageTray\usage-cache.json`
- `%LOCALAPPDATA%\T66UsageTray\logs\usage-tray.log`

Optional autostart:

- `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\T66 AI Usage Tray.lnk`

## Implementation Design

### Framework

Use .NET 9 WPF because:

- .NET 9 SDK and Windows Desktop runtime are already installed.
- WPF handles compact transparent always-on-top windows well.
- `System.Windows.Forms.NotifyIcon` can provide the tray icon without external packages.
- No Electron runtime is needed, keeping the app small.

### UI

Primary visible state:

```text
[Claude logo] 100%   [Codex logo] 18%
```

Where the value is weekly remaining percentage.

Window behavior:

- Borderless, small, dark translucent background, white text.
- Always-on-top but click-through disabled by default so it can be dragged.
- Anchored to bottom-right monitor work area, just above or beside system tray.
- Remembers user-adjusted position.
- Context menu:
  - Refresh now
  - Show/hide compact widget
  - Toggle launch on startup
  - Open logs folder
  - Exit

Tooltip/click popup:

- Claude weekly remaining and reset time
- Claude 5-hour remaining and reset time
- Codex weekly remaining and reset time
- Codex 5-hour remaining and reset time
- Last refresh time
- Source status: OK / stale / auth expired / app-server unavailable

System tray icon:

- One small combined app icon for the utility.
- It can show color state but should not be expected to display all percentages legibly.

Logo handling:

- First try to extract icons from installed local app resources or current app assets already present on the machine.
- If local extraction is not sufficient, use simple bundled provider mark assets added under `Assets/` only after checking licensing/brand guidance.
- Do not hotlink web images at runtime.

### Data Model

```text
ProviderUsage
  Provider: Claude | Codex
  WeeklyUsedPercent: double?
  WeeklyRemainingPercent: double?
  WeeklyResetAtLocal: DateTimeOffset?
  FiveHourUsedPercent: double?
  FiveHourRemainingPercent: double?
  FiveHourResetAtLocal: DateTimeOffset?
  Source: ClaudeStatusline | ClaudeOAuthUsage | CodexAppServer
  Status: Ok | Stale | AuthMissing | AuthExpired | ToolUnavailable | ParseError | NetworkError
  LastUpdatedAtLocal: DateTimeOffset
```

### Codex Collector

Implementation steps:

1. Resolve Codex executable:
   - Prefer `Get-Command codex`.
   - Fallback to `%LOCALAPPDATA%\OpenAI\Codex\bin\codex.exe`.
2. Start a short-lived `codex app-server --listen stdio://` process on each poll, or keep one background app-server process if short-lived startup proves too slow.
3. Send JSON-RPC initialize/initialized/account rate limit request.
4. Parse `rateLimits.secondary.usedPercent` as weekly used.
5. Parse `rateLimits.primary.usedPercent` as 5-hour used.
6. Convert Unix reset timestamps to local `DateTimeOffset`.
7. Kill/cleanup the app-server process after successful poll if using short-lived mode.

Initial plan should use short-lived polling every 5 minutes for simplicity. If startup overhead is too high, switch to a persistent background app-server wrapper after verifying lifecycle behavior.

### Claude Collector

Implementation steps:

1. Preferred attempt: identify a stable way to invoke or read Claude Code statusline JSON without relying on a running interactive session. If viable, use that because it is documented.
2. Fallback: read `%USERPROFILE%\.claude\.credentials.json` in memory.
3. Extract `claudeAiOauth.accessToken` without logging it.
4. GET `https://api.anthropic.com/api/oauth/usage`.
5. Parse `seven_day.utilization` and `five_hour.utilization`.
6. Convert ISO reset timestamps to local time.
7. If 401/403 or token expiry occurs:
   - Do not attempt to write credentials in the first implementation.
   - Show `Claude auth expired` and tell the user to open Claude/Claude Code once.
   - Optionally add refresh-token support only in a later reviewed pass.

### Polling

- Default interval: 5 minutes.
- Manual refresh button: immediate one-shot poll.
- Network timeout: 10 seconds per provider.
- Failure behavior:
  - Keep last known value for up to 30 minutes, marked stale.
  - After stale threshold, show `--%` or warning color.
- Avoid high-frequency polling.

### Security

- Do not write provider tokens to app logs or cache.
- Do not include raw provider JSON in logs unless redacted.
- `usage-cache.json` stores normalized percentages/resets only.
- If crash logs are added, sanitize headers and credentials.
- Use read-only file access for credentials.

### Build/Install

Implementation build commands:

```powershell
dotnet build C:\UE\T66\Tools\UsageTray\T66.AIUsageTray.sln -c Release
dotnet test C:\UE\T66\Tools\UsageTray\T66.AIUsageTray.sln -c Release
dotnet publish C:\UE\T66\Tools\UsageTray\src\T66.AIUsageTray\T66.AIUsageTray.csproj -c Release -r win-x64 --self-contained false -o C:\UE\T66\Saved\Tools\UsageTray\publish
```

Autostart setup should be opt-in:

- Create a startup shortcut only after user approval.
- Target published executable.
- Do not modify registry in the first pass.

### Verification Plan

Plan verification before any claim of completion:

1. Unit tests:
   - Codex JSON fixture parses weekly and 5-hour windows.
   - Claude JSON fixture parses weekly and 5-hour windows.
   - Reset timestamp conversion.
   - Remaining percent calculation clamps to 0-100.
   - Token redaction helper prevents credential logging.
2. Live read-only provider probes:
   - Codex app-server returns `rateLimits.secondary`.
   - Claude collector returns `seven_day` or reports a clear auth state.
3. UI smoke:
   - Launch app manually from publish folder.
   - Confirm compact window shows both logos and percentages.
   - Confirm tray icon context menu works.
   - Confirm manual refresh updates `LastUpdatedAt`.
   - Confirm app exits cleanly and leaves no orphan Codex app-server process.
4. Autostart, only if enabled:
   - Create startup shortcut.
   - Verify shortcut target and working directory.
   - Restart app manually via shortcut.
5. Negative tests:
   - Temporarily simulate missing Claude credentials with test path/config override, not by moving the real file.
   - Temporarily simulate missing Codex executable with test config override.
   - Confirm UI shows `--%` or warning state rather than crashing.

### Rollback Plan

- No implementation this turn.
- If implemented later:
  - Delete `Tools/UsageTray/` for source rollback.
  - Delete `Saved\Tools\UsageTray\publish` for build output rollback.
  - Delete `%LOCALAPPDATA%\T66UsageTray` for local settings/cache rollback.
  - Delete startup shortcut if created.
  - No Unreal runtime assets or game source should be touched.

## Out Of Scope

- Unreal Engine code, assets, staged standalone builds, and taskbar `T66 Standalone.lnk`.
- Mini/minigame systems.
- Building a Windows Widgets board plugin first. That can be a later path if the user prefers Windows Widgets over taskbar adjacency.
- Using OCR/pixel scraping as primary data collection.
- Refreshing Claude OAuth tokens automatically in the first implementation.
- Publishing or packaging an installer.
- Adding cloud sync or account-level server storage.

## Risks

- Exact placement inside the Windows system tray cluster is not supported by normal public APIs. Mitigation: use taskbar-adjacent overlay plus tray icon.
- Claude OAuth usage endpoint works but may be private/less stable. Mitigation: prefer statusline contract when viable and keep clear fallback/error state.
- Codex app-server protocol can change. Mitigation: isolate parser/transport, tests with fixtures, clear degraded UI.
- Logo licensing/brand usage. Mitigation: prefer locally installed app icons or user-approved assets.
- Credential exposure. Mitigation: in-memory token use, no raw JSON/token logs, redacted errors.
- Polling too often. Mitigation: 5-minute default and manual refresh.

## Proposed Execution Order After User Go-Ahead

1. Create `Tools/UsageTray` project skeleton and README.
2. Add data models and parser tests with fixture JSON from redacted prior probes.
3. Implement Codex collector.
4. Implement Claude collector with statusline attempt first; OAuth fallback if needed.
5. Implement WPF compact widget UI and tray context menu.
6. Add settings/cache/logging with redaction.
7. Add publish script or README commands.
8. Run tests and live read-only probes.
9. Launch UI manually and screenshot/inspect the result.
10. Optional: add startup shortcut after user approval.

## Review Request

Review this planning-only packet for:

- Whether it respects the user's instruction not to build yet.
- Whether the `Tools/UsageTray` placement is appropriate.
- Whether the tray-adjacent overlay assumption is a safe default.
- Whether Claude/Codex data source choices are accurate and safe.
- Whether verification is sufficient before implementation completion.


</review_packet>
