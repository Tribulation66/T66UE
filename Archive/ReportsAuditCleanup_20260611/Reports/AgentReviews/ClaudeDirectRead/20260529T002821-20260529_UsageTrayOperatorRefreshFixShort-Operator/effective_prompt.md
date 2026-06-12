You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to integrate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.
# Claude Operator Prompt: Usage Tray Refresh Fix Short

Goal: review a focused fix for the T66 usage widget.

Issue:
- `.t66\operator-state.json` and AppData `operator-state.json` both say Operator=Claude, Validator=Codex.
- Running widget still shows Operator=Codex because it loads operator state only on startup/manual tray menu selection.
- Codex usage briefly became `--%` because a transient Codex collector failure wrote null percentages to `usage-cache.json`; later refresh recovered.

Files to inspect:
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\AppPaths.cs`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\Services\OperatorStateService.cs`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\Models\ProviderUsage.cs`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray.Tests\UsageParserTests.cs`

Proposed fix:
1. Add canonical repo state path `C:\UE\T66\.t66\operator-state.json` to `AppPaths`.
2. Make `OperatorStateService.Load()` prefer valid repo state, fallback to AppData; make `Save()` write both.
3. Have the placement timer refresh operator display so externally-run `Set-T66Operator.ps1` updates the visible label without restart.
4. If a provider refresh returns null percentages but the previous/cache value has percentages, keep the previous value and mark provider `Stale` instead of showing `--%`.
5. Add tests.

Answer only:
- Approve/revise the proposed fix.
- Note any risks.
- Name verification gates.

Do not edit files or run shell commands.

