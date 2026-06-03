# Phase 2 Completion — Enemy Roster Restructure (Import / Build / Runtime Verification)

Operator: Claude (claude-opus-4-8, FullOperator)
Date: 2026-05-29
Approval: `codex_operator_approval_phase2.md` (Codex APPROVE)
Phase 1 gate: `phase1_validator_check.md` (APPROVE), `phase1_completion.md`

This artifact is an Operator deliverable, not a greenlight. Codex remains final proof owner.

---

## Result Summary

| Gate | Target | Result |
|---|---|---|
| DataTable rebuild | DT_Enemies / DT_Stages / DT_Items / DT_PlayerExperience from source | PASS (4/4 reloaded + saved) |
| F3 Build | T66Editor Win64 Development compiles | PASS (Succeeded) |
| F3 Static/grep | No Goblin enemy, Debuff enemy/projectile, dormant random-miniboss tuning, gambler-as-enemy, tower gameplay-floor identifiers | PASS |
| F4 Gate guardians (runtime) | Each gate spawns assigned scaled mega-mob, blocks descent until killed | PASS (runtime proof, exit 0) |
| F1 Backrooms Stalker (runtime) | Backrooms special works (Stalker pursues, exit + reward) | PASS (runtime proof, staged build, exit 0) |
| F4 Roster data | 12 mobs/theme, 10 new placeholder mobs, waves resolve without Goblin/Debuff | PASS (data-level) |
| F2 Vendor boss / token / casino | Vendor boss on failed steal; casino anger spawns nothing; gambling functions; vendor token | PASS (source/data-level) |
| F1 Loan Shark | Loan Shark works | PASS (source/system-level; no dedicated runtime route) |
| Staged standalone | Refresh + shortcut | PASS (BuildCookRun ExitCode=0, shortcut updated) |

No blockers outside approved scope were encountered. Two pre-existing, out-of-scope items are documented below (not fixed).

---

## 1. DataTable Rebuild (scoped: roster tables only)

Driver: `phase2_logs/rebuild_roster_datatables.py` (temporary, task-scoped — rebuilds ONLY the four approved roster tables; deliberately does not touch DT_Bosses/DT_StatusEffects/DT_BossEncounters/DT_BossEncounterMembers).

Command:
```
UnrealEditor-Cmd.exe T66.uproject -run=pythonscript ^
  -script="C:/UE/T66/Reports/AgentReviews/20260529_EnemyRosterRestructureImplementation/phase2_logs/rebuild_roster_datatables.py"
```

Log: `phase2_logs/rebuild_datatables.log`

Result markers:
```
=== PHASE2-REBUILD START ===
Imported DataTable 'DT_Stages' - 0 Problems   -> /Game/Data/DT_Stages OK
Imported DataTable 'DT_Enemies' - 0 Problems  -> /Game/Data/DT_Enemies OK
Imported DataTable 'DT_Items' - 0 Problems    -> /Game/Data/DT_Items OK
Imported DataTable 'DT_PlayerExperience' - 20 Problems -> /Game/Data/DT_PlayerExperience OK (see note)
=== PHASE2-REBUILD SUMMARY DT_Stages=OK DT_Enemies=OK DT_Items=OK DT_PlayerExperience=OK ===
=== PHASE2-REBUILD ALL DONE ===
```
All four `.uasset` files were re-saved to `Content/Data/`.

DT_PlayerExperience 20 "Problems" note: all 20 are `LootWheelsPerStage`, `LootWheelRarityWeights`, `LootWheelRewardWeightsByRarity`, `LootWheelGoldRangeByRarity` (4 fields x 5 difficulty rows) missing from `PlayerExperience.json`. These are pre-existing LootWheel struct fields never authored in the JSON source; they are unrelated to the roster restructure (the renamed mob-floor fields imported with 0 problems). Rows imported and asset saved with engine default-fill. Out of scope (LootWheel/Mini-adjacent), not a Phase 2 blocker. See "Out-of-scope items" below.

---

## 2. Build (F3)

Command:
```
Build.bat T66Editor Win64 Development -project=C:\UE\T66\T66.uproject -waitmutex
```
Log: `phase2_logs/build_t66editor.log`
Result: `Target is up to date` / `Result: Succeeded`.

Confirmed again as part of the staged BuildCookRun below (`dotnet UnrealBuildTool ... ExitCode=0`, `BUILD COMMAND COMPLETED`).

---

## 3. Static / Grep Checks (F3)

Source = `Source/T66`, Data = `Content/Data`.

- Removed/renamed production identifiers — 0 live hits: `UniqueDebuff`, `DebuffEnemy`, `DebuffProjectile`, `ProfileUniqueDebuff`, `MiniBossChancePerWave`, `ActiveMiniBoss`, `CasinoAnger`, `AngerLevel`, `ShopAnger`, `GamblerBoss`, `AT66Gambler`, tower `GameplayFloor`.
- Enemy roster archetypes (live `Enemies.csv`): only `Melee`(20), `Rush`(13), `Flying`(11), `Ranged`(16) — NO `Exploder`/`Stutterer`/`Burrower`.
- Enemy roster banned-token scan (Goblin/Debuff/Exploder/Stutterer/Burrower/MiniBossFeel across all fields): 0 hits.

Surviving-by-design (validated in Phase 1, retained intentionally):
- `Goblin`: deprecated `UMETA(Hidden)` enum + legacy save/backend/loc/`Item_Goblin` compat + hero weapon asset `SM_GoblinoChad_Cleaver` (hero gear, not an enemy).
- `MiniBoss`: retained gate-guardian / tutorial / debug system (this is the proven F4 mega-mob gate system).
- `Gambler`: retained gambling surface + achievements + deprecated token enum.

---

## 4. Runtime Smoke (F1, F2, F4)

### 4a. Gate guardians — F4 core (PASS)
Route: `-T66GameplayAutoCapture=minibosstraversalproof` on `/Game/Maps/GameplayLevel` (editor `-game`).
Command (PowerShell — see path-mangling note):
```
UnrealEditor.exe C:\UE\T66\T66.uproject /Game/Maps/GameplayLevel -game -windowed -ResX=1280 -ResY=720 ^
  -T66GameplayAutoCapture=minibosstraversalproof -unattended -nop4 -nosplash -abslog=<phase2_logs>\minibosstraversalproof.log
```
Log: `phase2_logs/minibosstraversalproof.log`
Marker:
```
[MinibossTraversalProofSummary] Terminal=1 Floors=2->3->4
  Floor2GuardianSpawned=1 Floor2BlockedWhileAlive=1 Floor2UnblockedAfterDeath=1 Floor2InteractAfterDeath=1
  Floor3GuardianSpawned=1 Floor3BlockedWhileAlive=1 Floor3UnblockedAfterDeath=1 Floor3InteractAfterDeath=1
  Floor4GuardianSpawned=1 Floor4BlockedWhileAlive=1 Floor4UnblockedAfterDeath=1 Floor4InteractAfterDeath=1
  Pass=1
FPlatformMisc::RequestExitWithStatus(0, 0, MinibossTraversalProofComplete)
```
Proves: each gate (floors 2/3/4) spawns its assigned guardian, blocks descent while alive, unblocks + allows interact after death. Retained scalars (HP 3.0x / Damage 2.0x / Scale 1.75) per `T66GameMode_Tower.cpp`.

### 4b. Backrooms Stalker — F1 (PASS)
Route: canonical README staged-build route (`-T66BackroomsAutoQA=exit` + `T66.Backrooms.ForceSpawn 1`) run against the freshly cooked standalone.
Command:
```
T66.exe -T66Entry=Run:Tower -ExecCmds="T66.Backrooms.ForceSpawn 1" -T66BackroomsAutoQA=exit ^
  -unattended -nop4 -nosplash -forcelogflush -abslog=<phase2_logs>\backrooms_qa_exit_staged.log
```
Log: `phase2_logs/backrooms_qa_exit_staged.log`
Markers:
```
T66.Backrooms.ForceSpawn = "1"
[BackroomsQA] Phase=Scheduled Mode=exit StartDelay=1.00
[BackroomsQA] Phase=Entered Mode=exit Entered=1 InventoryHidden=1 WeaponHidden=1
  StageTimerPaused=1 SpeedRunPaused=1 PauseFlag=1 ChaserSpawned=1 ChaserDistance=7440.0 MoveTargetResolved=1
[BackroomsQA] Phase=Exit Mode=exit ChallengeEnded=1 RewardGranted=1 InventoryRestored=1
  WeaponRestored=1 StageTimerRestored=1 SpeedRunRestored=1 InventoryCount=2
FPlatformMisc::RequestExitWithStatus(0, 0, T66BackroomsQAExitPass)
```
Proves: Backrooms special activates, Stalker/chaser spawns and resolves a pursuit move target toward the hero, and exit + reward restore work. Exit status 0 (pass).

### 4c. Roster waves + new mobs + theme counts — F4 (PASS, data-level)
Verified against live `Content/Data/Enemies.csv` and `Stages.csv`:
- 60 enemies, unique IDs, **exactly 12 per theme** (Dungeon/Forest/Ocean/Martian/Hell = 12 each).
- 10 new mobs with placeholder visuals (`ModelStatus=Placeholder`): CursedCrow, FamishedGhoul, WillOWisp, GoreStag, GullDiver, Hammerjaw, ReconOrb, CarapaceBrute, CinderWraith, BrimstoneBrute. (Other 50 = `MeshReady`.)
- Stage slots expanded to `EnemyA..EnemyL` (12 slots); all 20 stages' mob references resolve to enemy rows; **no Goblin/Debuff in any wave slot**.

The gate-guardian runtime proof (4a) additionally exercises the per-gate roster (`RosterData.EnemyA..EnemyL`) live.

### 4d. Vendor boss / token / casino — F2 (PASS, source/data-level)
- Vendor boss on failed steal: `T66CasinoVendorTabWidget.cpp:2419-2423` — `// Any failed steal attempt summons the Vendor hidden boss. if (bStealFailed) { SpawnVendorBoss(); }`.
- Casino anger spawns nothing: `CasinoAnger`/`AngerLevel`/`ShopAnger`/`SpawnAnger` = 0 hits in source.
- Casino gambling still functions: gambling surface present across RunState/UI/backend (`Gamble`/`Casino` live in source; not removed).
- Vendor token: `Items.csv` has `Item_VendorToken` with `SecondaryStatType=VendorToken`; DT_Items rebuilt 0 problems.

### 4e. Loan Shark — F1 (PASS, source/system-level)
`AT66LoanShark` class, `TrySpawnLoanSharkIfNeeded()` (called from `T66GameMode.cpp:1210,1264`), `bLoanSharkPending` state machine in `T66RunStateSubsystem`. No dedicated `-T66...AutoQA=loanshark` runtime route exists; verified at system level. See skipped-verification note.

---

## 5. Staged Standalone Refresh

Command:
```
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1
  (BuildCookRun -platform=Win64 -clientconfig=Development -stage -pak -package)
```
Log: `phase2_logs/stage_standalone_build.log`
Result: `COOK COMMAND COMPLETED` (Success - 0 error(s), 3 warning(s)), `STAGE`/`PACKAGE COMMAND COMPLETED`, `BUILD SUCCESSFUL`, `AutomationTool exiting with ExitCode=0`.
- Refreshed exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` (2026-05-29).
- Shortcuts updated: `T66 Standalone.lnk` (project root) and pinned taskbar shortcut -> refreshed exe.
- Staged SaveGames preserved to `Saved\StageBackups\StandaloneSaveGames_20260529_091323` (24 files).

The 3 cook warnings are all unrelated to the roster restructure:
1. `T66Mini.Build.cs`: referenced dir `Public\UI\Components` does not exist (Mini module — excluded scope).
2. `T66Hero1AxeAOEVFXLabActor.cpp(353)`: Niagara `IsReadyToRun` C4996 deprecation (VFX, unrelated).
3. `r.Upscale.Quality` CVar priority + `ToonShadingCommon.ush` material include path (engine/material, unrelated).

---

## 6. Scoped Fixups Applied

None to source/data. The only Phase 2 authoring was the temporary scoped rebuild driver (`phase2_logs/rebuild_roster_datatables.py`) and re-saving the four roster `.uasset` files from their Phase 1 source. No compile/import/runtime blocker caused by the roster restructure was found.

Process note (path mangling): the first gate-guardian launch via Git Bash mangled `/Game/Maps/GameplayLevel` into `C:/Program Files/Git/Game/Maps/GameplayLevel` (MSYS Unix-path conversion), so the map failed to load and the unattended engine exited. Re-running via PowerShell `Start-Process` resolved it. Backrooms `-ExecCmds="T66.Backrooms.ForceSpawn 1"` similarly required the inner quotes to survive to Unreal's command line (single verbatim ArgumentList string); without it the CVar stayed `0` and the door never force-spawned. Both are launcher-quoting issues, not project defects.

---

## 7. Out-of-Scope Items (documented, NOT fixed)

1. **`Scripts/ValidateEnemyBossRosterData.py` is stale vs. the new roster schema.** It still asserts `len(enemies)==50`, `STAGE_SLOTS=EnemyA..EnemyJ`, `ModelStatus=="MeshReady"`, includes removed `Exploder/Stutterer/Burrower` in `ALLOWED_ARCHETYPES`, `MiniBossFeel` in `ALLOWED_FEELINGS`, and `len(per theme)==10`, plus a hardcoded 5-Core/2-Rare/3-Late 10-slot stage-distribution model. It will fail against the current 60-enemy / 12-slot / Placeholder data. A correct rewrite requires the authoritative new distribution rules (Core/Rare/Late ratios across 12 slots and A..L fill order), which are design intent not derivable with confidence from the data alone. Left unmodified to avoid encoding wrong invariants; flagged for a follow-up that pairs the validator with the new design spec. (Phase 1 data was already validated directly via `phase1_validator_check.md` anchor spot checks.)

2. **`PlayerExperience.json` is missing LootWheel fields** (`LootWheelsPerStage`, `LootWheelRarityWeights`, `LootWheelRewardWeightsByRarity`, `LootWheelGoldRangeByRarity`) for all 5 difficulty rows, producing the 20 benign import "Problems" above. Pre-existing, LootWheel/Mini-adjacent, unrelated to the roster restructure. Rows import with engine default-fill. Not authored/fixed here (out of scope; would need the intended LootWheel tuning values).

---

## 8. Verification NOT Run (with reason)

- **Loan Shark / Vendor boss / casino gambling end-to-end runtime capture**: no Unreal-owned `-T66...AutoQA` route exists for these surfaces. Verified at source/system + data level (Section 4d/4e). Building new automation hooks for them was judged beyond the approved "run available routes" + minimal-hook scope for this task; flagged as a candidate follow-up if Codex wants live captures.
- **Editor `-game` Backrooms route**: attempted, does not reach `PrepareMainMapStage` headless (landscape-readiness gate doesn't fire without progression), so the entry door/pocket never spawn. This is why the canonical README route uses the cooked standalone; F1 Backrooms was proven there instead (Section 4b). Documented rather than worked around.

---

## 9. Token / Process Ledger

- Roles: Claude = Operator (authoring/proof-gathering). Codex = Validator/Finisher and final proof owner. This artifact is not a self-greenlight.
- Excluded actions respected: no Git commit/stage/push/tag/reset/clean/checkout; no B.13 sandbox deletion; no unrelated cleanup; no real model/art generation; no broad casino redesign; no Mini/minigame work beyond the explicitly-required casino gambling verification surface; no broad Git/LFS scans over Unreal binary asset folders; no prior-log substitution (all gates re-run current).
- User-owned dirty work preserved: did not modify `Content/Data/pending_issues_Data.md` or other user-dirty files; out-of-scope findings recorded here instead.
- Logs/evidence under `phase2_logs/`: `rebuild_roster_datatables.py`, `rebuild_datatables.log`, `build_t66editor.log`, `minibosstraversalproof.log`, `backrooms_qa_exit_staged.log`, `stage_standalone_build.log` (plus earlier editor-route `backrooms_qa_exit.log` retained showing the route limitation).
