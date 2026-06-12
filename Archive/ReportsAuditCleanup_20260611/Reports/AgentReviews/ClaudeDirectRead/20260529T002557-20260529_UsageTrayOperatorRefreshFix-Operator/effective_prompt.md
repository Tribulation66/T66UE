You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to integrate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.
# Claude Operator Prompt: Usage Tray Operator Refresh Fix

Working goal: fix the visible T66 usage widget so it reflects Claude-as-Operator, restore Codex usage display resilience, and clarify how ongoing chats adopt operator state.

Tier: Tier 1. This changes tooling/runtime behavior and process guidance.

Active roles:
- Operator: Claude Code, `claude-opus-4-8`, read-only direct-read helper.
- Validator/Integrator: Codex in the active workspace.

Observed issue from user screenshot:
- Widget still displays `Operator: Codex`.
- Widget shows `Codex --%`.

Live state Codex already checked:
- `C:\UE\T66\.t66\operator-state.json` contains Operator=Claude, Validator=Codex, Scope=Global.
- `C:\Users\DoPra\AppData\Local\T66UsageTray\operator-state.json` contains the same Claude/Codex state.
- Running tray process started at `2026-05-28 23:44:37`, before the state switch at `23:58:53`.
- `MainWindow.xaml.cs` calls `UpdateOperatorDisplay()` only on load or tray-menu manual selection; it does not reload external state while running.
- `OperatorStateService.Load()` reads only AppData `operator-state.json`, not the repo-canonical `.t66\operator-state.json`.
- `usage-cache.json` briefly held Codex `Status=NetworkError` with null percentages after repeated `Codex usage failed: InvalidOperationException`; a later refresh recovered Codex to 13% weekly remaining.

Proposed fix for you to review:
1. Patch `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\AppPaths.cs` to add canonical repo operator-state path `C:\UE\T66\.t66\operator-state.json`.
2. Patch `OperatorStateService` so `Load()` prefers the repo-canonical state when valid, then falls back to AppData state. Patch `Save()` so tray menu writes both repo and AppData state.
3. Patch `MainWindow.xaml.cs` so the placement timer also calls `UpdateOperatorDisplay()`, and make `UpdateOperatorDisplay()` reapply the current snapshot when the operator label changes.
4. Patch refresh behavior so if a provider refresh fails with null percentages but a previous/cache provider value exists, the UI keeps the last usable value and marks the provider `Stale` instead of showing `--%`.
5. Add focused unit tests for repo-state preference and stale fallback if practical.
6. Build/test/publish/restart tray.

Questions to answer:
- Is this the smallest reliable fix?
- Any risk with reading the repo state every 500 ms?
- Should we prefer file watcher instead?
- Any concern with keeping stale Codex values on transient failures?
- Anything else Codex should patch before implementing?

Do not edit files or run shell commands. Produce a concrete recommendation and verification gates for Codex.

