# Claude Operator Packet Request

Working goal: Move the AI usage widget into the Codex left sidebar below Chats with sidebar-matching width and adjusted font size.

Role state: Operator=Claude, Validator/Integrator=Codex.

You are the read-only Operator. Keep this narrow and do not edit files.

Inspect only:

- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml`
- `C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray\MainWindow.xaml.cs`

User request:

- Move widget to the left side below Chats in the Codex app.
- Make the widget the same width as the left sidebar exactly.
- Reduce the font a bit if needed.

Return a concise Operator Packet with:

- Exact proposed XAML size/font row changes.
- Exact proposed `PositionWithinCodexWindow` placement changes.
- Verification commands.
- Risks/caveats.
- Token routing can say this run's manifest will supply Claude token count.

Do not over-analyze unrelated repo files.
