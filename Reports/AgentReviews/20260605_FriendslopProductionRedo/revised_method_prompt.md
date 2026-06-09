# FriendslopStyle Main Menu Revised Method Review

User rejected the current crop-derived plate approach because it manually covered baked button/text regions with visible bars/smears. We must stop that method.

Task contract:

```text
Working task: Produce a new FriendslopStyle Main Menu screen version that visually matches the approved Round06 reference.
Operator: Codex
Validator: Claude
Scope: Replace the bad manual text-erasure crop strategy with a process-valid runtime chrome strategy, then implement/capture/verify the new screen.
Stop condition: Current 1920x1080 runtime capture plus dump/report/contact sheet/visual gate, or a hard blocker in the method.
```

Relevant hard constraints:

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`, and `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- Do not paste the full reference image into runtime UI.
- Do not bake labels, names, scores, ticket counts, friend state, or localization into runtime plates.
- Do not erase text from full-screen crops with manual bars/blurs/smears.
- Runtime chrome must be transparent PNG plates or plate families with live Slate text/data/icons over them.
- The clean alpha sheet exists at `SourceAssets/UI/FriendslopStyle/MainMenu/friendslop_mainmenu_runtime_chrome_sheet_alpha.png`.
- The approved reference exists at `UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png`.
- Current likely better direction: derive blank per-size plates from the clean alpha chrome sheet by slice/composite of clean caps/centers, use the reference only for geometry and visual comparison, then wire those plates into `FT66FriendslopStyle`, `T66MainMenuScreen`, `T66FrontendTopBarWidget`, and `T66FlatLeaderboardPanel`.

Please independently review this revised direction read-only. Return:

1. Whether this direction is process-valid under FriendslopStyle rules.
2. Any fatal issue that requires a user decision before Codex implements.
3. Specific implementation warnings for avoiding another visual-fidelity failure.
4. A short recommended acceptance gate.

End with `Result: OK` if Codex can proceed internally, or `Result: NEEDS_USER` only if a user-only decision is required.
