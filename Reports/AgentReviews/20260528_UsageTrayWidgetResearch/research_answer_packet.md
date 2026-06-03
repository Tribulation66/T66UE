# Usage Tray Widget Research Answer Packet

## Working Goal

Research practical ways to extract the visible Claude and Codex weekly usage remaining values from local apps and assess whether existing tools/projects already do this.

## User Request

The user clarified that the desired percentages are visibly displayed in both Codex and Claude app UIs, so they believe the computer should be able to grab them. They asked for online research into whether anyone has already done this, at least for extracting the information if not for a widget.

## Applicable Instructions

- Root `AGENTS.md` from `C:\UE\T66` is active.
- A native goal was created.
- Mini/minigame scope is excluded.
- Answer-only research; no implementation was requested yet.
- Claude review is required before substantive output.
- `ANTHROPIC_API_KEY` was checked in Process/User/Machine and is unset.
- OpenAI/Codex claims were checked against official OpenAI docs or official `openai/codex` repository docs first.

## Research Sources

Official / primary:

- OpenAI Codex config docs: `sqlite_home`, log dirs, statusline/status config, and memories min rate-limit config exist, but no simple documented public API key endpoint for subscription weekly remaining.
- OpenAI Codex slash commands docs: `/status` and `/statusline` exist; `/statusline` can include footer items `model/context/limits/git/tokens/session`.
- OpenAI `openai/codex` app-server README: `codex app-server` is the interface used by rich Codex UIs; it supports JSON-RPC over stdio; `account/rateLimits/read` fetches ChatGPT rate limits; fields include `usedPercent`, `windowDurationMins`, and `resetsAt`.
- Anthropic Claude Code statusline docs: statusline stdin JSON includes `rate_limits.five_hour.used_percentage`, `rate_limits.seven_day.used_percentage`, and reset timestamps for Claude.ai subscribers after the first API response.
- Anthropic support docs: Claude usage limits count across Claude surfaces; usage is affected by model, context, and features.

Community / product evidence:

- `aimo`: unified dashboard/browser extension for Claude, Codex, ZAI, and Ollama; exposes local server `/api/usage` and lists Claude 5-hour/weekly and Codex 5-hour/weekly metrics.
- Tokus: cross-platform desktop system tray app for Claude Code, Codex, Cursor, etc.; advertises Windows support, session/weekly limit tracking, reset time, local-only behavior.
- Wburn: Windows 11 widgets for Claude Code, Codex, and Gemini CLI; reads existing CLI tokens and tracks Claude daily/weekly and OpenAI Codex 5-hour/weekly windows.
- VS Code AI Usage Monitor extension: shows Claude + Codex usage in status bar; says Claude comes from local Claude Code credentials and Anthropic OAuth API; Codex comes from `codex app-server` method `account/rateLimits/read`.
- Codex Pulse extension: starts local Codex executable with `codex app-server --listen stdio://`, calls `account/rateLimits/read`, and falls back to session quota events.
- SessionWatcher / AIHelper macOS: commercial/menu-bar examples that monitor Claude and Codex subscription windows.
- Reddit/GitHub examples: Windows Claude tray apps exist, including implementations that use Anthropic OAuth usage endpoint and local Claude transcript files.

## Local Probes Performed

Claude:

- Verified `C:\Users\DoPra\.claude\.credentials.json` exists and contains `claudeAiOauth` fields by key name only; token values were not printed.
- Queried `https://api.anthropic.com/api/oauth/usage` with the local Claude OAuth access token.
- Response included:
  - `five_hour.utilization = 3.0`, reset `2026-05-29T00:50:00.758134-03:00`
  - `seven_day.utilization = 0.0`, reset `2026-06-03T15:00:00.758164-03:00`
  - `seven_day_sonnet.utilization = 0.0`
  - extra usage disabled fields
- This matches the user's Claude screenshot class: weekly limits visible as used percentage and reset time.

Codex:

- Verified local Codex executable at `C:\Users\DoPra\AppData\Local\OpenAI\Codex\bin\codex.exe`.
- Started `codex app-server --listen stdio://`, sent `initialize`, `initialized`, then `account/rateLimits/read`.
- Response included:
  - main `codex.primary.usedPercent = 4`, `windowDurationMins = 300`
  - main `codex.secondary.usedPercent = 82`, `windowDurationMins = 10080`
  - `planType = pro`
  - `rateLimitsByLimitId.codex_bengalfox` for `GPT-5.3-Codex-Spark`
- The user's Codex screenshot showed weekly remaining around `19%`; local probe showing `82% used` implies about `18% remaining`, so this is the right data source.

## Findings

1. The user is correct: this is grabbable. For our purpose, we do not need OCR or pixel scraping as the primary path.
2. Codex has an official local structured API path: `codex app-server` + `account/rateLimits/read`.
3. Claude has two practical structured paths:
   - Official Claude Code statusline JSON contains rate-limit fields.
   - The local OAuth credentials plus Anthropic OAuth usage endpoint returns the same style of quota data. This endpoint is used by community tools but should be treated as less formally stable than the statusline contract unless Anthropic documents it as public API.
4. Existing tools absolutely exist:
   - Windows: Tokus, Wburn, AgentWatch, VS Code AI Usage Monitor, Codex Pulse, multiple Claude tray apps.
   - macOS: SessionWatcher, AIHelper, AI Usage Monitor.
   - Browser/local dashboard: aimo.
5. A custom Windows tray widget remains feasible and now has a clear implementation path:
   - Poll Claude endpoint/statusline-derived cache every 60-300 seconds.
   - Poll Codex app-server every 60-300 seconds.
   - Convert `usedPercent` or `utilization` to remaining percentage with `100 - used`.
   - Display weekly first, optionally include 5-hour windows in tooltip.

## Caveats

- We should not store or print tokens. The collector should read local credentials only in memory and store only normalized usage snapshots.
- Anthropic OAuth endpoint use may be unofficial/private even though it works and community tools use it; the fallback should be Claude Code statusline JSON if possible.
- Polling should be modest, e.g. every 5 minutes, matching several community tools and avoiding unnecessary API calls.
- UI Automation/OCR remains a fallback if structured paths break, not the first choice.
- Existing commercial/free tools may already satisfy the need; choosing build-vs-use is a product decision if the user wants the fastest result.

## Proposed Final Answer

Yes, you were right. The visible values are not just theoretically grabbable; on this machine I was able to get both values through structured paths without OCR.

For Codex, the clean path is `codex app-server --listen stdio://` and the JSON-RPC method `account/rateLimits/read`. I ran a local probe and it returned the same class of data as your screenshot:

- `primary`: 5-hour window, `usedPercent: 4`
- `secondary`: weekly window, `usedPercent: 82`, duration `10080` minutes
- `82% used` means about `18% remaining`, which lines up with the `19%` weekly remaining in your screenshot.

For Claude, there are two paths:

- Official Claude Code statusline JSON includes `rate_limits.five_hour.used_percentage`, `rate_limits.seven_day.used_percentage`, and reset timestamps.
- Your local Claude OAuth credentials also worked against `https://api.anthropic.com/api/oauth/usage`; the response returned `five_hour.utilization`, `seven_day.utilization`, and reset times. Your current weekly utilization came back as `0.0`, matching the screenshot showing `0% used`.

I also found that people have already built this category of tool:

- Tokus: Windows/macOS/Linux tray app for Claude Code, Codex, Cursor, etc.
- Wburn: Windows 11 widgets for Claude Code, Codex, and Gemini CLI.
- AI Usage Monitor VS Code extension: Claude + Codex status-bar usage; says it uses Claude local credentials and Codex `account/rateLimits/read`.
- Codex Pulse: VS Code extension specifically using `codex app-server` and `account/rateLimits/read`.
- aimo: browser extension/local dashboard aggregating Claude, Codex, ZAI, and Ollama.
- Several Windows Claude tray apps exist on Reddit/GitHub.

So the plan is much better than OCR:

1. Build a tiny local collector that reads Claude and Codex structured quota data.
2. Normalize both into `{ provider, weeklyUsed, weeklyRemaining, weeklyReset, fiveHourUsed, fiveHourRemaining, fiveHourReset }`.
3. Put a Windows tray icon or tiny taskbar-adjacent window on top of that collector.

I would only use UI Automation/OCR as a fallback if either vendor changes the structured surfaces.

Sources to cite:

- OpenAI Codex app-server README
- OpenAI Codex slash commands docs
- Claude Code statusline docs
- aimo article
- Tokus
- Wburn
- VS Code AI Usage Monitor
- Codex Pulse

