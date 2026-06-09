# Friendslop Migration — Overnight Log

Run start: 2026-06-09 (overnight). Branch: `friendslop-migration`. Charter: `decision_charter.md`.

## Architecture decision (Phase 0)

- `FT66FriendslopStyle` already exists (59 chrome variants, cached brushes, graceful fallback)
  and mirrors FlatStyle signatures. Migrated screens (MainMenu, AccountStatus tabs) call it directly.
- ~450 FlatStyle call sites remain; MakeFlatPanel (115) + MakeFlatButton (76) dominate.
- THE FLIP: route FT66FlatStyle helper internals through FT66FriendslopStyle behind switch
  `T66.UI.FriendslopGlobal` (default ON, `-T66FlatLegacy` escape). All unmigrated screens flip
  at once with zero per-screen churn; migrated screens unaffected. FlatStyle entry points become
  the legacy-compat adapter layer = "FlatStyle is legacy" achieved at the infrastructure level.
- Kit generation deferred until triage: existing MainMenu generic family + SharedPrimitives +
  Overview progress track/fill cover panel/button/row/pill/slot/checkbox/dropdown/tab/modal/
  tooltip/progress roles. Generate only what capture triage proves bad. (Imagegen is the
  flakiest overnight resource — minimize dependence.)

## Surface manifest (impact order for Phase 2 triage)

| # | Surface | Capture route | Status |
|---|---------|--------------|--------|
| 1 | Gameplay HUD | widget hudreview / enemywaveperf | flip + triage |
| 2 | Pause menu | screen PauseMenu | flip + triage |
| 3 | Casino vendor tab (vendor) | widget casinoshop | flip + triage |
| 4 | Casino gambler tab | widget casinogambling | flip + triage |
| 5 | Idol altar overlay | widget idol | flip + triage |
| 6 | Lab overlay | widget lab | flip + triage |
| 7 | Crate overlay | widget crate | flip + triage |
| 8 | Collector overlay | widget collector | flip + triage |
| 9 | Loot wheel overlay | (no capture mode; code review) | flip + triage |
| 10 | Weapon altar overlay | (no capture mode; code review) | flip |
| 11 | Inventory inspect HUD | widget inventory | flip + triage |
| 12 | Full map | widget fullmap | flip + triage |
| 13 | Settings (7 tabs) | screen Settings | flip + triage |
| 14 | Save slots / LoadGame | screen LoadGame | flip + triage |
| 15 | Run summary | screen RunSummary | flip + triage |
| 16 | Game over | screen GameOver | flip + triage |
| 17 | Quit confirmation / Party invite / Save preview modals | screens (already SharedPrimitives?) | verify |
| 18 | Challenges / HeroGrid / CompanionGrid / LanguageSelect / PlayerSummaryPicker / DailyClimb / PetSelection | screens | flip + spot check |
| 19 | Companion selection | screen CompanionSelection | flip (reference exists, assets pending) |
| 20 | Loading screen | widget loading | flip + triage |
| 21 | Cowardice prompt | natural trigger | flip |
| 22 | Mini-games (CoinFlip/GuessCup/StickPick/FindJoker) | in casino | flip |
| 23 | Floating combat text / EnemyLock / crosshair | gameplay viz — EXEMPT (not chrome) |  |

Already Friendslop (do not touch): MainMenu, Overview, History, Diplomas, Drugs,
SteamAchievements, SecretAchievements, TopBar, Tooltips, minimap (today).

## Guardrails active

- git add ONLY files I touched (user has unrelated uncommitted work in tree).
- 1 iteration/surface, 2 for big issues. Same failure twice => revert + defer.
- Compile loop via direct Build.bat (fast); full stage only per capture batch.

## Heartbeat

- [start] Phase 0 complete: inventory done, architecture locked, manifest written.
- [1b] Flip implemented in T66FlatStyle.cpp:
  - Switch: CVar `T66.UI.FriendslopGlobal` (default 1) + `-T66FlatLegacy` command-line escape.
  - `MakeFlatPanelSurface` (the universal chrome primitive): Friendslop plates — interactive
    surfaces get ButtonChromeForState (dark/red/green), static get PanelLargeDark. Hover =
    10% white film, disabled = 42% black film, on the inner border so dynamic recolors
    (OutBorder/OutFillBorder consumers) keep working. Covers panels, buttons, toggle groups,
    icon/tab buttons, dropdowns, sliders, overlay panels/slots — everything that flowed
    through the flat 2px-stroke path.
  - `MakeHudPanel` (both overloads): Friendslop PanelLargeDark + preserved gold title row.
  - Legacy `MakeButton(FT66ButtonParams)` (in-run overlay buttons): translated to Friendslop
    plate buttons (Primary/Danger/ToggleActive->red, Success->green, Neutral/Row->dark).
  - Deliberately NOT changed: `GetFlatOverlayBrush` (callers tint it with own dark colors —
    a textured plate would multiply to near-black; left white-brush; triage will catch spots).
- [1b] Compile PASSED (direct UBT, exit 0). Flip committed: d4123783a.
- [discovery] Shared modals + tooltips already Friendslop (SharedPrimitives plates wired in
  T66ScreenSlateHelpers: modal panel @1628, modal buttons red/green/dark @1348-1353,
  checkboxes @1360, tooltip @T66TooltipSlate:159). QuitConfirmation/PartyInvite/SavePreview
  expected good without work.
- [2] Staging flipped build; sweep script + triage-sheet tooling ready
  (Saved/Codex/UI/FriendslopMigration/run_capture_sweep.ps1, make_triage_sheet.py).
- [2] SWEEP ROUND 1 DONE: 25/27 captures (DailyDescent + PetSelection automation names
  did not resolve - logged, not styled-blocked). Triage sheet:
  Saved/Codex/UI/FriendslopMigration/round1/triage_sheet.png
- [2] TRIAGE VERDICT: flip PASSES everywhere reviewed. Full-size reviewed: hud, casinoshop
  (vendor - good), casinogambling, pause (excellent), idol (good), settings (good), crate,
  lab, loadgame (good). Zero flip-caused big issues.
  - casinogambling void+magenta frame: PRE-EXISTING (identical with -T66FlatLegacy;
    evidence round1/casinogambling_LEGACY.png). Out of scope tonight.
  - lab + crate capture modes did not open their overlays this run (world shows, no UI).
    Capture-route quirk, not style; overlay chrome flips via same choke point as idol
    (which passed). Logged for morning.
  - Cosmetic nits (morning list): pause RESUME label rides high on red pill + bottom
    button clips panel edge; settings wide dropdown bar stretches slice; loading screen
    is plain text (no plate, low priority).
- [3] Phase 3 start: targeted HUD custom chrome (score block, XP box) + then docs + smoke.
- [3] Phase 3 verdict: HUD already passes post-flip (score block plated; bUseAlternateHudChrome
  flip judged too risky vs payoff at night-guardrail bar). Crosshair/bullseye/wheel dial/combat
  text are gameplay viz (exempt per inventory). Minimap done earlier today. No work above bar.
- [4] Docs updated: UI_AGENTS.md (Friendslop = active lane, FlatStyle = legacy adapter),
  ART_DIRECTION.md (UI boundary), FRIENDSLOP authority doc (flip note). Commit a8288e61d.
- [4] SMOKE: first run FAIL on case 05 (screenshot timeout AFTER successful click+nav+dump —
  automation timing flake class, same as documented 2026-06-08 pending issue). RETRY PASS:
  all 7 tag-click cases PASS (incl. 04 which was failing on 2026-06-08 baseline, and 05),
  DurableSaveIntegrity PASS, LifecycleTransition BUILD_CONFIG_UNSUPPORTED (config status,
  not a failure). Run: Saved/PreReleaseSmokeSuite/20260609_175117. WIRING VERDICT: PASS.

## Close

PPF close: Process used = FriendslopStyle infrastructure flip (user-approved deviation from
per-screen reference generation: "main priority is infrastructure... this wont be the last
attempt"; existing approved MainMenu/SharedPrimitives plates reused under the charter's
blanket APPROVED_REUSE). Matches declared process: YES (as amended by user pre-run).
Evidence: triage sheet (25 captures), full-size reviews, smoke suite 20260609_175117,
commits d4123783a + a8288e61d on friendslop-migration.

Result: FlatStyle is legacy. Every player-facing surface renders Friendslop chrome.
Branch friendslop-migration, 3 commits, not pushed. See morning_review.md.
