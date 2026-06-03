# Completion Packet: Demo Gating Visibility — Phase 4

Operator: Claude (FullOperator)
Validator/Finisher: Codex
Approval artifact: `Reports/AgentReviews/DemoGatingVisibility/codex_operator_approval_phase4.md`
Proof root: `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4`
Date: 2026-05-30

## Outcome

**FULL** — with one expected, documented caveat (hero carousel wraparound; see Caveats).

Staged standalone refreshed, both shortcuts verified to the staged exe, and
Unreal-owned screenshot + widget-dump proof produced for every requested screen.
No demo-gated `COMING SOON` / `DemoOverlay` overlays appear on any moved-to-available
surface, the MainMenu Daily Descent CTA is hidden, the difficulty surface shows
`Easy`, and the hero carousel contains no `COMING SOON` placeholder entries.

## Staging command and pass/fail marker

- Command: `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`
- Log: `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/stage_standalone.log`
- Marker: `BUILD SUCCESSFUL` / `AutomationTool exiting with ExitCode=0 (Success)` → **PASS**
- Build path: full rebuild (a source file was added → makefile invalidated) + cook + pak + package.
- Staged exe verified present and fresh:
  `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  (rebuilt 2026-05-30 08:50, 320,423,424 bytes).
- Loose runtime content roots re-copied by the staging script (Fonts, Arcade, UI,
  Video, Movies, Mini/TD/Deck/Idle data + SourceAssets).

## Shortcut target verification

Both shortcuts refreshed by the staging script and verified to point at the staged exe:

| Shortcut | Target | Match | Arguments | WorkingDirectory |
|---|---|---|---|---|
| `C:\UE\T66\T66 Standalone.lnk` | `...\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` | MATCH | `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush` | `...\Binaries\Win64` |
| `...\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk` | `...\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` | MATCH | `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush` | `...\Binaries\Win64` |

Pinned taskbar shortcut was present and was refreshed (not skipped).

## Capture route and demo enforcement

- Capture tool: `Scripts/CaptureT66UIWidget.ps1` (Unreal-owned), one launch per screen
  producing a 1920x1080 screenshot (`-T66AutoScreenshot`) and a widget-tree JSON dump
  (`-T66AutoDumpWidget`) against the staged exe. No desktop screenshots used.
- Dump target: `Class=<ScreenWidget>` (resolves the active screen's Slate root).
  Note: the initial MainMenu dump used `ViewportIndex=0`, which resolved to
  `T66FrontendTopBarWidget` (top bar only), so a second `Class=T66MainMenuScreen`
  dump (`MainMenu_screen.json`) was taken for the structural MainMenu proof.
- Demo enforcement: demo mode is forced by the cooked config
  `Config/DefaultDemoMode.ini` → `[/Script/T66.T66DemoModeSettings] bForceDemoMode=true`
  (`UnavailableContentText=COMING SOON`, `AllowedDifficultyIDs=Easy`,
  `AllowedHeroIDs=Hero_1..Hero_5`). Captures also passed `-T66Demo` as belt-and-suspenders.
  No release-variant `UE_LOG` marker exists, so demo-active state is evidenced by the
  observed gated behavior (Easy-only difficulty, hidden Daily Descent, no Minigames tab),
  not a log string.

## Screenshot / dump / log artifact paths

All under `Reports/Proof/DemoGatingVisibility/2026-05-30-phase4/`:

| Screen | Screenshot | Dump |
|---|---|---|
| MainMenu | `MainMenu.png` | `MainMenu.json` (ViewportIndex=0 → top bar) + `MainMenu_screen.json` (Class=T66MainMenuScreen) |
| HeroSelection | `HeroSelection.png` | `HeroSelection.json` |
| Diplomas (PowerUp) | `Diplomas.png` | `Diplomas.json` |
| Drugs (PowerUp) | `Drugs.png` | `Drugs.json` |
| SteamAchievements | `SteamAchievements.png` | `SteamAchievements.json` |
| Achievements — Secret tab | `Achievements_Secret.png` | `Achievements_Secret.json` |

- Staging log: `stage_standalone.log`
- Retention marker: `.report-run.json` (`expiresAfterDays: 15`)
- Per-launch game logs are written to the staged `Saved\Logs\T66.log` and are
  overwritten by each launch (last launch = Secret re-capture, clean
  `-T66AchievementsTab=Secret` command line confirmed).

## Findings per requested screen

Consolidated dump scan (all 0 = absent):

| Dump | COMING SOON | DemoOverlay | DailyDescentButton |
|---|---|---|---|
| MainMenu.json | 0 | 0 | 0 |
| MainMenu_screen.json | 0 | 0 | 0 |
| HeroSelection.json | 0 | 0 | 0 |
| Diplomas.json | 0 | 0 | 0 |
| Drugs.json | 0 | 0 | 0 |
| SteamAchievements.json | 0 | 0 | 0 |
| Achievements_Secret.json | 0 | 0 | 0 |

- **MainMenu** — PASS. Center CTA stack contains only `MainMenu.Center.EnterTribulationButton`
  and `MainMenu.Center.LoadGameButton`; `MainMenu.Center.DailyDescentButton` is absent
  (dump) and not visible (screenshot). Confirms `IsDailyDescentAvailable()` returns false
  in demo (`T66MainMenuScreen.cpp:573-578`). Top bar shows Account/Home/Power Up/Achievements
  with no Minigames tab.
- **HeroSelection** — PASS (with carousel caveat). Difficulty surface shows `Easy`. No
  `COMING SOON` placeholder entries in the carousel. Carousel exposes 7 physical portrait
  slots (`Portrait01`–`Portrait07`); see Caveats.
- **Diplomas (PowerUp)** — PASS. Both sub-tabs (`DIPLOMAS (PERMANENT)`, `DRUGS (ONE TIME USE)`)
  visible; diploma cards render with `GRADUATE` actions and no coming-soon overlay.
- **Drugs (PowerUp)** — PASS. Drugs tab active; Damage/Attack-Speed drug items render with
  `BUY` actions and no coming-soon overlay.
- **SteamAchievements** — PASS. `STEAM` and `SECRET` sub-tabs both present; Steam achievement
  rows are claimable (`CLAIM`) with no coming-soon overlay (confirms achievement rows are no
  longer demo-locked).
- **Achievements — Secret tab** — PASS. `-T66AchievementsTab=Secret` activated the Secret tab
  (`SECRET ACHIEVEMENTS 0/???`); obfuscated rows are claimable with no coming-soon overlay.
  Parser uses exact `Equals("Secret")` (`T66AchievementsScreen.cpp:2313`), so the visible
  Secret tab proves a clean value was parsed.

## Skipped proof and reasons

- None of the six requested screens were skipped.
- The first MainMenu dump (`MainMenu.json`, `ViewportIndex=0`) is retained but is NOT the
  load-bearing MainMenu structural proof because it resolved to the top-bar widget only;
  `MainMenu_screen.json` (`Class=T66MainMenuScreen`) is the authoritative MainMenu dump.

## Token Ledger

AuthoringTokens: Claude:Unavailable
ReviewTokens: Codex:Unavailable
FinishTokens: Codex:Unavailable
OperatorIsAuthoring: YES
PerModel: Claude=Unavailable, Codex=Unavailable
TargetMet: Unavailable
Notes: This was an interactive Claude FullOperator session, not a
  `Scripts\Invoke-ClaudeDirectRead.ps1` helper run, so no helper manifest
  (`ClaudeTokensSpent`) is exposed for this run — reported `Unavailable` per protocol
  rather than `0`. Codex token usage is to be filled by the Validator via
  `Scripts\Get-CodexTokenUsage.ps1`.

## Caveats

- **Hero carousel wraparound (expected):** The hero carousel renders 7 physical portrait
  slots (`HeroSelection.TopRow.HeroCarousel.Portrait01`–`Portrait07`) while the demo allow-list
  has only 5 playable heroes (`Hero_1`–`Hero_5`). The carousel therefore repeats playable
  heroes across physical slots via existing wraparound. This is a visual caveat, NOT a
  coming-soon placeholder: there are no `COMING SOON` entries and no `DemoOverlay` in the
  carousel dump. Do NOT claim the carousel reduced to exactly five physical boxes.
- **No demo-mode log string:** `UT66ReleaseVariantSubsystem` emits no `UE_LOG` marker, so
  demo-active state is inferred from gated behavior + cooked config rather than a log line.
- **Captures run at staged desktop resolution internally:** widget dumps record a 2560x1600
  viewport (host desktop), while screenshots are forced to 1920x1080 by the capture script;
  normalized basis in dumps is 1920x1080.
- Claude-produced proof is evidence, not final acceptance. Codex remains the final proof
  owner and user-facing reporter.
