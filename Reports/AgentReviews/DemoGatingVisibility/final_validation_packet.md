# Final Validation Packet: Demo Gating Visibility

Date: 2026-05-30
Operator: Claude
Validator: Codex
Scope: remove visible demo-coming-soon presentation from non-Mini UI, make drugs/diplomas/achievements available, and document demo-gated invisible content separately from deprecated content.

## Verdict

PASS WITH CAVEAT.

The requested demo-gating model is implemented and proofed in the staged standalone build. Demo-gated non-Mini content is hidden from the visible UI instead of being shown behind `COMING SOON` overlays. Drugs, diploma upgrades, Steam achievements, and Secret achievements are available in demo. The deprecated inventory is documentation-only and did not drive runtime code changes.

## Validated Changes

- `Config/DefaultDemoMode.ini`
  - `MaxDiplomaUpgradesPerStat=4`
  - `bAllowDrugPurchases=true`
- `Source/T66/Core/T66BuffSubsystem.cpp`
  - Single-use buff purchase availability now delegates to the release-variant gate.
- `Source/T66/UI/Screens/T66AchievementsScreen.cpp`
  - Demo achievement row locking is disabled, so Steam and Secret rows no longer use the coming-soon overlay.
- Hero/companion/difficulty surfaces
  - Hero and companion lists use playable-filtered lists.
  - Difficulty menu surfaces use playable difficulties where the user-facing menu is built.
  - Coming-soon overlay wrapping was removed from the non-Mini hero/companion carousel/grid surfaces.
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
  - Daily Descent CTA is omitted when unavailable.
- `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`
  - Non-playable difficulties are skipped instead of rendered with a coming-soon overlay.
- Documentation
  - `Demo/DEMO_GATED_INVISIBLE_CONTENT.md`
  - `Demo/DEPRECATED_CONTENT.md`
  - `Demo/DEMO_RELEASE_INSTRUCTIONS.md`

## Proof

Staged standalone:
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/stage_standalone.log`
- Pass markers: `BUILD SUCCESSFUL`, `AutomationTool exiting with ExitCode=0 (Success)`.
- Staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Repo and pinned taskbar shortcuts were updated to that staged exe by the staging script.

Unreal-owned UI captures and dumps:
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/MainMenu_screen.json`
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/HeroSelection.json`
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/Diplomas.json`
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/Drugs.json`
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/SteamAchievements.json`
- `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/Achievements_Secret.json`

Codex scan result:

| Dump | COMING SOON | DemoOverlay | DailyDescent |
|---|---:|---:|---:|
| MainMenu_screen.json | 0 | 0 | 0 |
| HeroSelection.json | 0 | 0 | 0 |
| Diplomas.json | 0 | 0 | 0 |
| Drugs.json | 0 | 0 | 0 |
| SteamAchievements.json | 0 | 0 | 0 |
| Achievements_Secret.json | 0 | 0 | 0 |

Available-surface evidence:
- `HeroSelection.json`: `Easy` present; `BUY` actions present.
- `Diplomas.json`: `GRADUATE` actions present.
- `Drugs.json`: `BUY` actions present.
- `SteamAchievements.json`: `CLAIM` actions present.
- `Achievements_Secret.json`: `CLAIM` actions present.

## Caveat

The hero carousel still renders seven physical portrait slots and wraps/repeats the playable demo heroes when the playable allow-list has fewer entries. This is not a coming-soon placeholder: the dump and screenshot show no `COMING SOON` text and no `DemoOverlay`. Reducing the carousel to exactly the playable hero count would be a separate layout behavior change.

## Token Ledger

Claude parseable subtotal from helper manifests: 4,433,323 tokens.

Some Claude runs used text output or timed out without a parseable token count, so the complete Claude total is unavailable. Codex final token usage is reported in the user-facing final response from `Scripts\Get-CodexTokenUsage.ps1`.
