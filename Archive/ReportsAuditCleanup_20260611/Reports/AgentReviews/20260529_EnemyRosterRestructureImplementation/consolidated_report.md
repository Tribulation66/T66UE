# Enemy Roster Restructure — Consolidated Report for Pablo

- Date: 2026-05-29
- Implementer (Operator): Claude (`claude-opus-4-8`, FullOperator), two phases
- Validator / final proof owner: Codex
- Status: **Implemented and validated within available automation.** Codex final verdict: APPROVE.
- This report consolidates the two operator phases and Codex's two validation passes. Source files were not touched while writing this — it is report-only.

---

## 1. Pending Issues / Pablo Decisions

This is the part that needs your attention. Everything else (Sections 2–6) is informational.

### 1a. Decisions you need to make now

1. **Real models + behavior tuning for the 10 new mobs.** They were added as data rows with `ModelStatus=Placeholder` and route through family/behavior logic only — they have no real art yet. You need to decide when these get real models and combat tuning. Until then they will spawn using placeholder visuals. The 10 are: CursedCrow, FamishedGhoul (Dungeon); WillOWisp, GoreStag (Forest); GullDiver, Hammerjaw (Ocean); ReconOrb, CarapaceBrute (Martian); CinderWraith, BrimstoneBrute (Hell).

2. **Roster validator design rules (now in place, but worth confirming).** The repo validator `Scripts/ValidateEnemyBossRosterData.py` was rewritten by Codex to match the new schema (60 enemies / 12 per theme / `EnemyA..EnemyL` / placeholder allowed only for the 10 new mobs, and each theme's 4 stages must reference all 12 theme mobs). If your intended Core/Rare/Late distribution across the 12 slots differs from "each theme's set references all 12 mobs," tell us and we'll tighten the validator. Right now it encodes the conservative rule, not a specific Core/Rare/Late ratio.

3. **When to retire the compatibility redirects.** `Config/DefaultEngine.ini [CoreRedirects]` currently carries 3 `PropertyRedirects` (mob-floor field renames) and 2 `FunctionRedirects` (VendorToken UFUNCTIONs). These bridge old binary assets/Blueprints to the new names. They can be removed once every uasset/Blueprint is rebuilt and re-saved against the new names — and dropping the deprecated `GamblerToken` enum value + legacy save fields should only happen behind an explicit save-migration pass. Your call on timing.

### 1b. Deferred Pass E concept / asset review items

- The 10 new placeholder mobs need a concept/art pass (models, animation, VFX bindings) before they read as finished enemies. This was explicitly deferred — family/behavior routing works today, visuals do not.

### 1c. Accepted caveats / proof gaps

- **Vendor (failed-steal hidden boss) and Loan Shark are verified at source/system + data level only**, because no dedicated runtime AutoQA route exists for either surface. The trigger wiring is confirmed in source (failed steal → `SpawnVendorBoss`; debt-pending → `TrySpawnLoanSharkIfNeeded`, `AT66LoanShark` exists), but neither was driven end-to-end in a live capture this pass. Building new automation hooks for them was judged out of scope. Flagged as a candidate follow-up if you want live captures.
- **`DT_PlayerExperience` still reports 20 LootWheel field import "Problems"** (4 LootWheel fields × 5 difficulty rows missing from `PlayerExperience.json`). Accepted as pre-existing, LootWheel/Mini-adjacent, and unrelated to the roster restructure. Rows still import and the asset saves with engine default-fill. Not fixed here.

### 1d. Issues already resolved by this pass (no action)

- The earlier `AT66UniqueDebuffProjectile` retention deviation was closed — the projectile and all its call sites were fully removed.
- The 3 deferred mob-floor field renames were applied, bridged by redirects so nothing breaks at runtime.
- VendorToken is now the canonical runtime name; legacy names retained only as marked compat hooks.
- **Stale roster validator was corrected by Codex after Claude's implementation, and now passes:** `Enemy/boss roster validation passed: 20 stages, 60 enemies, 20 encounters, 23 boss rows.` (Claude had left it stale and out of scope; Codex applied the bounded fix.)

---

## 2. No Action Needed (looks alarming, is intentional)

- **Legacy `GamblersToken` / `GamblerToken` names still exist in source.** Retained only for old save / data / backend compatibility (serialized save fields `ActiveGamblersTokenLevel`, `GamblersTokenUnlockedLevel`; the `Item_GamblersToken` item-ID alias; the deprecated `ET66SecondaryStatType::GamblerToken` enum value). The canonical runtime name is `VendorToken`. Renaming these away would break existing saves — leave them.
- **The casino gambling interactable is still present and functional.** Only the casino *anger* boss-spawning system was removed (`CasinoAnger`/`AngerLevel`/`ShopAnger` → 0 source hits). Gambling itself was deliberately not redesigned.
- **`MiniBoss` references still appear in source.** These are the retained, proven gate-guardian / mega-mob system (Section 3-E), not the removed dormant random-promotion tuning. Minibosses remain rich actors; basic mobs route through lightweight family logic.
- **Backrooms uses its own reward assets.** The QuickRevive vending/icon assets deleted elsewhere in the working tree did **not** break the Backrooms QA route — the reward still granted (verified, Section 4).

---

## 3. What Changed

Organized by the original task sections.

### A — Removals
- **Goblin Thief** fully removed (class/spawn/RNG/Lab/gold-steal). Surviving `Goblin` hits are a deprecated `UMETA(Hidden)` enum + legacy compat hooks + an unrelated hero weapon asset (`SM_GoblinoChad_Cleaver`). No `Goblin` row in `Enemies.csv`.
- **Debuff enemy AND its projectile** fully removed: `AT66UniqueDebuffEnemy` + `AT66UniqueDebuffProjectile` deleted; `ProfileUniqueDebuff`, perf/lag projectile counters, Backrooms cleanup filter, and the debug-preview spawn all removed. 0 hits.
- **Dormant random-miniboss promotion tuning** removed (`MiniBossChancePerWave`, `ActiveMiniBoss` → 0). Tower guardian `ApplyMiniBossMultipliers` constants kept as source of truth.
- **Speculative archetypes** (`Exploder`/`Stutterer`/`Burrower`) and **`MiniBossFeel`** cleared from `Enemies.csv`; stale pending docs closed.

### B — Vendor Hidden Boss
- `AT66GamblerBoss` repurposed into `AT66VendorBoss` (old class deleted); mechanics/hit-zones/AOE/token drop preserved.
- Trigger is **any failed steal**, no threshold (your locked decision).
- Casino anger system fully removed; gambling interactable kept functional.
- Token renamed to canonical `VendorToken` (item `Item_VendorToken`, runtime API, helpers). Exactly one hidden boss remains: Vendor. (One behavior fix noted: the recompute-skip was broadened to also skip canonical `VendorToken`, fixing a latent double-count — Codex signed off.)

### C — Mob-Floor Rename
- "gameplay floor" → "mob floor" across identifiers/strings/logs/docs, **plus** 3 field renames: `GameplayFloorsPerStage`→`MobFloorsPerStage`, `InitialEnemiesPerGameplayFloor`→`InitialEnemiesPerMobFloor`, `InitialTowerEnemiesPerGameplayFloor`→`InitialTowerEnemiesPerMobFloor`.
- Applied in source, `PlayerExperience.json` (all 5 stage blocks), and the validator. `PropertyRedirects` bridge old binary assets until rebuild.
- The distinct `GameplayLevelNumber` / theme concepts were deliberately kept (separate, non-floor concept).
- Gate guardians confirmed on descent transitions 2→3, 3→4, 4→5; no behavior change.

### D — 12 Mobs Per Theme
- Stage schema expanded `EnemyA..EnemyJ` → `EnemyA..EnemyL` (+2 slots; `Stages.csv` now 22 columns).
- 10 placeholder mob rows added (2 per theme). Every roster MobID in `Stages.csv` resolves to a row in `Enemies.csv`. Result: 60 enemies, exactly 12 per theme.

### E — Mega-Mob Gate Assignment
- Hardcoded `ConfigureAsMob("Slime")` replaced with per-gate assignment via `T66ResolveTowerGateGuardianMobID(...)`; `"Slime"` kept only as fallback.
- Data-driven from existing `DT_Stages`: `SlotIndex = (LocalStage-1)*3 + GateIndex` spreads 12 theme mobs across 12 gate encounters. The 10 new IDs were added to the family resolver (routing only; real models deferred).

### F — Verification
- See Section 4.

---

## 4. Verification Status (with evidence paths)

All evidence is under `Reports/AgentReviews/20260529_EnemyRosterRestructureImplementation/`. Each marker below was confirmed present in the cited log.

| Gate | Evidence path | State |
|---|---|---|
| DataTable rebuild (4/4) | `phase2_logs/rebuild_datatables.log` | **PASS** — `SUMMARY DT_Stages=OK DT_Enemies=OK DT_Items=OK DT_PlayerExperience=OK`; Stages/Enemies/Items = 0 Problems; PlayerExperience = 20 LootWheel Problems (accepted, §1c) |
| T66Editor build | `phase2_logs/build_t66editor.log` | **PASS** — `Result: Succeeded` |
| Staged standalone build | `phase2_logs/stage_standalone_build.log` | **PASS** — COOK/STAGE/PACKAGE COMPLETED, `BUILD SUCCESSFUL`, `ExitCode=0 (Success)` |
| Mega-mob gate guardians (runtime) | `phase2_logs/minibosstraversalproof.log` | **PASS** — Floors 2/3/4 each: GuardianSpawned/BlockedWhileAlive/UnblockedAfterDeath/InteractAfterDeath all =1, `Pass=1`, exit status 0 |
| Backrooms Stalker (runtime, staged build) | `phase2_logs/backrooms_qa_exit_staged.log` | **PASS** — ChaserSpawned=1, MoveTargetResolved=1, `RewardGranted=1`, InventoryRestored, `InventoryCount=2`, exit status 0 |
| Static / grep removals | (Phase 1 & 2 packets) | **PASS** — 0 live hits for removed/renamed production identifiers |
| Roster schema validator | `Scripts/ValidateEnemyBossRosterData.py` (Codex-fixed) | **PASS** — `20 stages, 60 enemies, 20 encounters, 23 boss rows` |
| Vendor trigger / token | source/data grep | **PASS (source/data-level only — §1c)** |
| Loan Shark | source/data grep | **PASS (source/system-level only — §1c)** |

Note on the Backrooms log: the QA trace shows the QuickRevive HUD probe reading `Item=0/IconVisible=0` during the run and then the reward arriving (`Item=1`, `RewardGranted=1`, final `InventoryCount=2`) at exit — confirming the reward path completed and did **not** break on the deleted QuickRevive vending/icon assets.

---

## 5. Repo State / Git

- The working tree remains **dirty with many pre-existing, unrelated changes** (chrome migration, combat VFX, demo skin data, deleted QuickRevive/Cliff assets, etc.).
- This pass did **not** stage, commit, revert, clean, tag, or reset anything. User-owned dirty files (e.g. `Content/Data/pending_issues_Data.md`) were left untouched.
- Whenever you decide to commit, you'll be separating the roster restructure changes from the surrounding unrelated work yourself — nothing has been bundled or staged on your behalf.

---

## 6. Recommended Next Passes

1. **Art/concept pass for the 10 placeholder mobs** (models, anims, VFX bindings) to clear `ModelStatus=Placeholder`.
2. **Live runtime captures for Vendor failed-steal and Loan Shark** if you want them proven beyond source/system level — requires new AutoQA routes.
3. **Author the missing LootWheel fields in `PlayerExperience.json`** to clear the 20 benign import problems (needs intended LootWheel tuning values).
4. **Retire the compat redirects** (`PropertyRedirects` + `FunctionRedirects`) once all uassets/Blueprints are rebuilt against the new names; drop the deprecated `GamblerToken` enum/save fields only behind an explicit save-migration pass.
5. **Confirm/tighten the validator's distribution rule** if your intended Core/Rare/Late ratio across the 12 slots differs from the current "all 12 mobs referenced per theme" check.
