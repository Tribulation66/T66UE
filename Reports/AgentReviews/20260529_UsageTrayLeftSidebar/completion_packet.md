# Completion Packet: Usage Tray Left Sidebar Placement

## Outcome

Moved the AI usage widget from the bottom-right Codex content area to the left
sidebar column. The widget now uses a 263 DIP width to match the observed sidebar
width, a 154 DIP height, and smaller typography suitable for the sidebar.

The placement now anchors to the Codex window's left edge and bottom sidebar
area above Settings, which places it below the Chats section in the user's
screenshot.

## Files Changed

- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`
- `Reports/AgentReviews/20260529_UsageTrayLeftSidebar/claude_operator_prompt.md`

## Verification

- `dotnet test C:\Users\DoPra\Tools\AIUsageTray\T66.AIUsageTray.sln -c Release`
- `Scripts\Invoke-ClaudeDirectRead.ps1` syntax check
- Published to `%LOCALAPPDATA%\T66UsageTray\app`
- Restarted `T66.AIUsageTray.exe`
- Confirmed the restarted process exists and the WPF window reports `263x154`.

The live placement is Codex-foreground scoped, so the app applies it when the
Codex window is foreground.

## Review / Validation

Claude Operator artifact:

- `Reports/AgentReviews/ClaudeDirectRead/20260529T025602-UsageTrayLeftSidebar-Operator/claude_direct_read_operator.md`
- Manifest token count: 81,849

Codex integrated the selected bottom-sidebar interpretation and verified the
build/test/publish path.

## Token Ledger

OperatorTokens: 81,849
ValidatorTokens: 73,570 at pre-final goal check
OperatorShare: 52.7% at pre-final goal check
TargetMet: NO for this small visual implementation turn

## Caveats

The exact sidebar width is currently a tuned Codex layout constant (`263` DIP).
If Codex changes its sidebar width in a future app update, this may need one
constant adjustment.
