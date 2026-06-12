Verdict: NEEDS_HUMAN_DECISION

## Blockers

- None that pure Codex revision can clear. The deciding issue is risk acceptance the user must own (see Major Issues #1).

## Major Issues

1. **No supported taskbar-embedding path likely exists on this OS, and the chosen path is an unsupported hack whose fallback violates the user's hard requirement.** The host is Windows 11 Home build 26200. The COM **DeskBand** API (Option 1) is effectively removed/non-functional on the Windows 11 taskbar, so the "official" route does not actually deliver the requirement here. Option 2 (re-parenting a WPF window to `Shell_TrayWnd`/positioning beside `TrayNotifyWnd`) is explicitly unsupported, breaks across Explorer/taskbar updates, and is exactly the technique modern Windows 11 has been hardening against. The only fallback offered is the **floating overlay** — which the user explicitly rejected. So the realistic outcomes are: (a) fragile unsupported docking that may silently detach, or (b) fall back to the very behavior the user banned. Choosing among "accept the unsupported hack," "accept floating fallback," or "relax the requirement" is a risk/product decision only the user can make. The working goal even hedges this ("if a practical supported path exists") without resolving it. Save a `decision_block.md` and ask once.

2. **Verification can pass while the user's hard constraint fails.** Step 4 says "Confirm the app window's parent is the taskbar window **when possible**" and treats fallback as success. If docking silently fails and the app floats, the run can be reported "done" while violating the not-floating requirement. Acceptance must treat fallback-to-floating as a FAIL/escalation, not a pass.

## Minor Issues

- `TrafficMonitor` and `EverythingToolbar` are cited as precedent, but both rely on different mechanisms (EverythingToolbar uses a deskband/registration approach; TrafficMonitor uses its own always-on-top window with known Win11 taskbar caveats). They do not validate the specific "re-parent WPF child into `Shell_TrayWnd`" approach proposed. State the actual mechanism each uses before leaning on them as proof.
- Edit scope targets `C:\Users\DoPra\Tools\AIUsageTray`, which is outside the `C:\UE\T66` repo and outside Claude's read scope; this validation could not inspect the actual `MainWindow.xaml.cs`/csproj. Note that the operator must verify current behavior (refresh timer, usage cache, tray menu) against live source before claiming it is preserved.

## Clarifying Questions

- Given there is no reliable supported Win11 taskbar-embedding route, do you accept the unsupported `Shell_TrayWnd` child-window technique with its detach risk, or do you want a different target (e.g., a thin always-on-top bar pinned to the taskbar strip, or a tray flyout)?
- If taskbar docking fails at runtime, what is the desired behavior — keep retrying/redock only, hide entirely, or fall back to floating? (Floating contradicts your stated constraint.)

## Required Verification

- `dotnet build`, `dotnet test`, `dotnet publish` to `%LOCALAPPDATA%\T66UsageTray\app`, launch published exe.
- Parent-window/window-rect evidence proving the window is actually hosted in the taskbar (not merely overlapping it); a fallback-to-floating result must be reported as constraint-failure, not success.
- Screenshot proving horizontal order `Operator | Codex | Claude` with live operator and live usage percentages.
- Runtime log check, no orphan `codex app-server --listen stdio://` collectors, clean token/credential string scan of the runtime folder.

## Rationale

The plan is well-structured, scoped to the local utility, and has sane rollback. But its central premise — a "practical supported path" for taskbar embedding — does not hold on Windows 11 26200: the supported API is gone and the chosen approach is an unsupported hack whose only fallback is the user-rejected floating mode. That tradeoff is a risk-acceptance/product decision, not something Codex can resolve by revising the plan, so the correct move is to save a decision block, ask the user once, and stop.

