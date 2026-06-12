Verdict: APPROVE

## Blockers
- None.

## Major Issues
- None. Scope is a single local XAML file under `C:\Users\DoPra\Tools\AIUsageTray`, no Unreal/content/staged-build files touched, and the behavioral contract (Codex-scoped show/hide, timers, collector) is explicitly preserved.

## Minor Issues
- The `dotnet test` step assumes a test project exists in the solution. If none does, the command is a no-op rather than real verification — fine, but don't treat a passing/empty `dotnet test` as evidence the layout works. The screenshot is the real proof here.
- Size `260x118` and "larger but bounded" fonts are approximations. Codex should confirm the three rows actually fit without clipping at runtime, not just trust the numbers.
- Positioning claim ("formula uses Width/Height so taller height auto-anchors") is stated but not re-derived. Verify the rendered widget actually lands in the lower-right anchor area after the height change rather than assuming the formula compensates.

## Clarifying Questions
- None blocking. User constraints are explicit and unambiguous (order, bigger font, no X/C badges, Codex-scoped visibility).

## Required Verification
- Build + (any) tests in Release.
- Publish to `%LOCALAPPDATA%\T66UsageTray\app` and launch the published exe.
- Screenshot while Codex is foreground showing vertical `Operator` → `Claude` → `Codex`, larger text, no X/C badges, positioned lower-right without clipping Codex content excessively.
- Confirm widget hides when Codex loses foreground.
- Runtime log check; confirm no orphan `codex app-server --listen stdio://` collector processes.
- Auth-string scan of runtime folder (`accessToken|refreshToken|Bearer|Authorization|claudeAiOauth`).

## Rationale
Tier 1 local tooling edit with a tightly bounded scope, a clear rollback (revert `MainWindow.xaml`, republish), and verification that includes a visual screenshot proving the user-requested layout plus the existing show/hide behavior. No contradictions with `AGENTS.md` routing (artifacts land under the correct Reports path). Remaining items are runtime-confirmation details Codex owns, not decisions requiring the user.

