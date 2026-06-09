# Pending Issues - UI Screens

## Game Over Ranking Pyramids Use Pre-Submit Rank Cache [Major]

What's wrong: The Game Over ranking pyramid reads the leaderboard subsystem's local/backend-accepted rank cache, but the current live-run submit path is still owned by the Run Summary activation flow. That means the Game Over pyramid can show the player's already accepted rank state and selected board family, while the current run's authoritative backend rank arrives after the existing Run Summary submission step.

Why it is out of scope now: Moving final-run submission earlier requires a shared idempotent end-run submission service so Game Over and Run Summary do not double-save recent-run snapshots, double-submit backend payloads, or split final accounting/coupon side effects.

What fixing it would entail: Move `UT66RunSummaryScreen::ProcessRunSummaryLeaderboardSubmission` ownership into a core run-finish submission service, guard it per run using lifecycle reset on `NewRun`, subscribe Game Over to backend rank completion for a rebuild, and make Run Summary consume the same cached submission result.

## Resolved 2026-06-08 - Hero Selection Player Settings Compile Errors Blocked Staged Standalone

- Former severity tag: [Blocker]
- What was wrong: During the 2026-06-07 floor landing-bounce pass, `Scripts\StageStandaloneBuild.ps1` reached the build phase and failed in `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` because `UT66PlayerSettingsSubsystem` was undeclared at the call sites that read or write `ShowPartySuspendedLeaderboardPopup`. The same staged build also exposed separate Core session compile errors recorded in `Source/T66/Core/pending_issues_Core.md`.
- Resolution: The 2026-06-08 floor landing-bounce verification rerun explicitly cleaned the `T66Editor Win64 Development` and `T66 Win64 Development` targets, then completed `Scripts\StageStandaloneBuild.ps1` successfully with a fresh 100-action compile/link, cook, stage, and package. It refreshed `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, verified both standalone shortcuts target that executable, and verified a packaged quit smoke with exit code `0`. The final staged smoke log is `Saved/AgentReviews/FloorLandingBounce_20260608/staged_quit_smoke_after_clean_rebuild.log`.
