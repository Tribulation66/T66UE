# Completion Review Packet - AI Usage Tray Widget Build

## Working Goal

Build and launch the Windows AI usage tray widget showing Operator plus Claude/Codex usage, using the reviewed tray plan and keeping it as personal tooling outside the T66 game repo.

## Operator / Validator

- Operator: Codex
- Validator: Claude

## Review Artifacts

Build plan:

`C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayWidgetBuild\build_review_packet.md`

Build plan review:

`C:\UE\T66\Reports\AgentReviews\20260529_UsageTrayWidgetBuild\20260528T215907-pass1\claude_review_pass1.md`

Verdict: `APPROVE`

## Built Paths

Source:

`C:\Users\DoPra\Tools\AIUsageTray`

Published app:

`C:\Users\DoPra\AppData\Local\T66UsageTray\app\T66.AIUsageTray.exe`

Runtime data:

- `C:\Users\DoPra\AppData\Local\T66UsageTray\operator-state.json`
- `C:\Users\DoPra\AppData\Local\T66UsageTray\usage-cache.json`
- `C:\Users\DoPra\AppData\Local\T66UsageTray\logs\usage-tray.log`

## Implemented Features

- .NET 9 WPF tray-adjacent overlay.
- Notification-area tray icon with menu.
- Visible `Operator:` line.
- Claude and Codex weekly remaining percent display.
- Tooltip/detail text with weekly and 5-hour data.
- Manual refresh.
- Manual Operator menu choices:
  - Claude
  - Codex
  - Unknown
- Operator state file with role metadata only.
- Claude usage collector:
  - Reads `.claude\.credentials.json` in memory.
  - Uses OAuth access token only in memory.
  - Calls `https://api.anthropic.com/api/oauth/usage`.
  - Does not log tokens or raw credentials.
- Codex usage collector:
  - Calls short-lived `codex app-server --listen stdio://`.
  - Parses structured `account/rateLimits/read`.
  - Cleans up the stdio app-server after refresh.
- Redacted log helper.
- Parser/service tests.

## Verification Performed

Build:

- `dotnet build C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release`
  - Passed, 0 warnings, 0 errors.

Tests:

- `dotnet test C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release --no-build`
  - Passed: 5/5.

Publish:

- `dotnet publish C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\T66.AIUsageTray.csproj -c Release -r win-x64 --self-contained false -o %LOCALAPPDATA%\T66UsageTray\app`
  - Passed.

Launch:

- Process is running:
  - Process name: `T66.AIUsageTray`
  - Path: `C:\Users\DoPra\AppData\Local\T66UsageTray\app\T66.AIUsageTray.exe`

Visible window verification:

- Enumerated windows for the process.
- Visible window:
  - Title: `AI Usage Tray`
  - Rect: left `1622`, top `928`, right `1908`, bottom `1020`
  - Size: `286x92`
- Post-review screenshot proof:
  - `C:\Users\DoPra\AppData\Local\T66UsageTray\proof_overlay.png`
  - Confirms rendered `Operator: Codex`, `Claude 98%`, and `Codex 16%`.

Live cache:

- Claude:
  - Weekly used: `2`
  - Weekly remaining: `98`
  - Five-hour used: `11`
  - Five-hour remaining: `89`
- Codex:
  - Weekly used: `84`
  - Weekly remaining: `16`
  - Five-hour used: `13`
  - Five-hour remaining: `87`

Operator state:

```json
{
  "operator": "Codex",
  "validator": "Claude",
  "scope": "Thread",
  "source": "CurrentBuildLaunch"
}
```

Token/log safety:

- Searched runtime folder for:
  - `accessToken`
  - `refreshToken`
  - `Bearer`
  - `Authorization`
  - `claudeAiOauth`
- No matches found.

Codex app-server cleanup:

- Initial verification found orphaned `codex app-server --listen stdio://` processes.
- Stopped the tray app, cleaned only those stdio app-server processes, patched collector cleanup to close stdin and wait before kill/dispose.
- Rebuilt, retested, republished, relaunched.
- Final app-server check shows only the Codex desktop app's normal `app-server --analytics-default-enabled`; no leftover `--listen stdio://` app-server children.

## Known Caveats

- No autostart shortcut was created.
- The first build uses simple letter badges rather than official provider logo assets to avoid a brand/licensing detour.
- The overlay is tray-adjacent, not embedded inside the Windows notification cluster, because the exact cluster is not a supported public UI extension point.
- The Claude usage endpoint is isolated but private/unstable; the UI degrades if it changes.

## Proposed Final Answer

Built and launched. You should see the overlay by the bottom-right tray area now.

It is running from:

`C:\Users\DoPra\AppData\Local\T66UsageTray\app\T66.AIUsageTray.exe`

Source is at:

`C:\Users\DoPra\Tools\AIUsageTray`

It currently shows:

- Operator: Codex
- Claude weekly remaining: 98%
- Codex weekly remaining: 16%

Verification:

- Build passed.
- Tests passed: 5/5.
- Publish passed.
- Process is running.
- Visible window exists at the bottom-right work area.
- No token strings were found in runtime cache/logs.
- No leftover `codex app-server --listen stdio://` processes remain after the cleanup fix.

Notes:

- I did not create autostart yet.
- The provider marks are simple letter badges for now, not official logos.
- The tray menu lets you manually switch Operator between Claude, Codex, and Unknown.

## Review Request

Review whether this completion report is accurate, whether any safety or verification gap remains before telling Pablo the tray is built and visible, and whether the caveats are clear enough.
