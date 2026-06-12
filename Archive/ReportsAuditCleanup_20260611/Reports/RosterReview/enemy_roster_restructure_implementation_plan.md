# Enemy Roster Restructure — Consolidated Implementation Plan

Status: PLAN + INVESTIGATION ONLY. No implementation performed. For Pablo review.
Date: 2026-05-29
Author: T66 Operator (Claude), under Codex approval `20260529_EnemyRosterRestructurePlan`.
Authoritative inputs: live T66 repo code/data; prior report `Reports\RosterReview\enemy_roster_review.md`.

---

## 1. Executive Summary

This plan covers Pablo's 11 locked enemy-roster restructure decisions. Investigation confirms most required foundations already exist in live code; the work is more refactor/retarget/data-edit than greenfield build.

Key findings that change the picture from the prior report (`enemy_roster_review.md`):

- **Loan Shark IS implemented and wired** (prior report did not surface it). Complete debt-driven chaser with data row and spawn/despawn logic. Item 1 is mostly verification.
- **Backrooms Stalker room encounter IS built and reachable** (prior report said "unimplemented/disabled"; that note is stale). 8% spawn per tower stage, maze + doors + unkillable chaser already live. Item 2 is largely verification, not build.
- **Tower floor structure already matches Pablo's target** (5 floors: 1 start / 2-4 mob / 5 boss; 3 descent gates per stage). Item 10 is terminology rename + confirmation.
- **Mega-mob mechanism already exists** (`ConfigureAsMob(MobID)` + `ApplyMiniBossMultipliers`). Item 6 needs a per-(stage,gate) MobID assignment table replacing the hardcoded `"Slime"`.
- **Gambler boss is a complete entity** suitable for repurposing into Vendor boss (item 5). The steal-trigger plumbing exists; the work is retargeting the spawn path and stripping Gambler identity.

Net: this is a data-heavy + targeted-code-refactor program, splittable into a Removal pass, a Repurpose/Retarget pass, and an Additive (10 mobs + 12 mega-mob gate assignments) pass.

---

## 2. Current Foundations Found

| Foundation | Status | Primary anchors |
|---|---|---|
| Loan Shark special | FOUND — complete & wired | `Source\T66\Gameplay\T66LoanShark.cpp/.h`; `Content\Data\LoanShark.csv`; `T66GameMode.cpp:1384` `TrySpawnLoanSharkIfNeeded()`, spawn `:1468`, despawn `:1214-1217` |
| Backrooms Stalker room encounter | FOUND — built & reachable | `T66GameMode_Backrooms.cpp` (`CVarT66BackroomsSpawnChance=0.08` :30; `SpawnBackroomsPocketIfNeeded()` :447; doors :588-621; `HandleBackroomsDoorInteracted` :929→`EnterBackrooms` :948); `T66BackroomsDoorInteractable.cpp:106`; `T66GameMode_Bootstrap.cpp:484,:736`; `UniqueEnemies.csv` BackroomsChaser (Slime visual, unkillable) |
| Goblin Thief (to remove) | FOUND | `T66EnemyDirector.cpp:1107-1160` (Luck-biased wave spawn); `T66RngTuningConfig.h:94-97` (GoblinWaveChanceBase, GoblinCountPerWave); `T66GameMode_Lab.cpp:159` (AT66GoblinThiefEnemy); class `AT66GoblinThiefEnemy` |
| Debuff enemy (to remove) | FOUND | `T66GameMode_Lab.cpp:163` (AT66UniqueDebuffEnemy, Lab-only); class + status projectile |
| Gambler boss (repurpose→Vendor) | FOUND — complete entity | `T66GamblerBoss.cpp/.h` (AT66BossBase, BossID="GamblerBoss", MaxHP=1000, 6 hit-zones, Vendor token drop) |
| Casino-anger boss spawn (to remove) | FOUND | `T66PlayerController_Overlays.cpp:5983` `TriggerCasinoBossIfAngry()`, spawn `:6052` |
| Shop steal trigger (repoint→Vendor boss) | FOUND | `T66CasinoVendorTabWidget.cpp:2453` `OnStealStop()`, `:2481` calls TriggerCasinoBossIfAngry; `T66RunStateSubsystem_EconomyInventory.cpp` steal resolution |
| Mega-mob / gate guardian mechanism | FOUND | `T66GameMode_Tower.cpp:146` `T66SpawnTowerGateGuardian` (ConfigureAsMob("Slime") + ApplyMiniBossMultipliers(3.0,2.0,1.75)); `:554` IsPlacedTowerMinibossFloor; `:574` EnsurePlacedTowerMinibossForFloor; `:538` guardian-required gating |
| Tower floor structure | FOUND — matches target | `T66TowerMapTerrain.h:153-156` / `.cpp:58-62` (Start=1, FirstGameplay=2, LastGameplay=4, Boss=5) |
| Dormant random-miniboss tuning (to remove) | FOUND | `T66EnemyDirector.h:64-73` (MiniBossChancePerWave=0.10, MiniBossScale, HP/Dmg scalars); `.cpp:1162-1164` already disabled (INDEX_NONE) |
| Enemy data | FOUND | `Content\Data\Enemies.csv` (50 rows; Archetype + Feeling + StageTag columns); `Content\Data\Stages.csv` (EnemyA..EnemyJ slots); `Content\Data\UniqueEnemies.csv` |
| Missing archetype classes (honesty note) | FOUND (pending) | `pending_issues_Enemies.md` [Major]: no AT66Exploder/Stutterer/BurrowerEnemy — 12 archetype mobs already run family-fallback only |

---

## 3. Numbered Item Plan

### Item 1 — Loan Shark special (confirm foundation)
- **Current state:** Fully implemented. `AT66LoanShark` is a debt-driven chaser: speed `BaseMoveSpeed=650 + MoveSpeedPer100Debt=50` per 100 debt; damage `BaseDamageHearts=1 + 1` per `DebtPerExtraHeart=200`; touch damage via `RunState->ApplyDamage(CurrentDamageHearts*20,...,"LoanSharkTouch")`; avoids NPC safe zones; despawns when debt paid. Data row `LoanShark,LoanShark,650.0,50.0,1,200` in `LoanShark.csv`. Spawned by `TrySpawnLoanSharkIfNeeded()` when `GetLoanSharkPending() && debt>0`.
- **Proposed approach:** No build required. Verify it is registered as one of exactly the two specials and is reachable in current builds. Confirm DataTable binding for `FLoanSharkData` resolves.
- **Files/data touched:** None (verification only). If roster registry enumerates specials, confirm Loan Shark + Backrooms Stalker are the only two.
- **Verification needed:** In-editor/runtime confirm Loan Shark spawns on debt; confirm no third special remains after Goblin Thief/Debuff removal.
- **Risks/open questions:** None to foundation. Open question: is there a "specials registry" list that needs Goblin Thief/Debuff de-registered (see items 3-4)?

### Item 2 — Backrooms Stalker room encounter (confirm + keep Slime placeholder)
- **Current state:** Built and reachable. `SpawnBackroomsPocketIfNeeded()` runs at tower stage setup (`Bootstrap.cpp:484,:736`), gated on tower layout, not-lab, not-already-spawned, no Quick Revive owned, then 8% RNG (`CVarT66BackroomsSpawnChance=0.08`). Builds maze, spawns entry/closed/exit doors; door interact → `EnterBackrooms`. Chaser = `BackroomsChaser` (UniqueEnemies.csv): CharacterVisual=Slime, MoveSpeed=760, unkillable, bypasses lethal saves. Reward = Quick Revive.
- **Proposed approach:** No build required. Treat as the second special ("Backrooms Stalker"). Keep Slime placeholder visual (matches Pablo's decision). Update the **stale** doc note in `pending_issues_Gameplay.md:49` ("Backrooms gameplay remains unimplemented/disabled") — but note doc edits are out of current write scope; flag for a follow-up pass.
- **Files/data touched:** None now. Future: terminology/naming if "Backrooms Stalker" should be the canonical special name.
- **Verification needed:** Force-spawn via `CVarT66BackroomsForceSpawn`, enter room, confirm chaser pursues, flee/exit works, Quick Revive grants.
- **Risks/open questions:** **Git working tree shows deletions of `QuickReviveIcon.uasset` and the QuickReviveVending assets.** The Backrooms reward is Quick Revive — these deletions may break the reward or its UI. Flag to Pablo before any restructure; this is pre-existing working-tree state, out of this plan's scope to fix.

### Item 3 — Remove Goblin Thief entirely
- **Current state:** Enemy class `AT66GoblinThiefEnemy`; Luck-biased wave spawn in `T66EnemyDirector.cpp:1107-1160` (`GoblinWaveChanceBase`, `GoblinCountPerWave`); supporting RNG tuning `T66RngTuningConfig.h:94-97`; Lab spawn path `T66GameMode_Lab.cpp:159`; gold-steal mechanic in the class.
- **Proposed approach (removal):** Delete the wave-spawn block in EnemyDirector; remove `GoblinWaveChanceBase`/`GoblinCountPerWave` from RngTuningConfig (+ any tuning CSV/asset rows); remove Goblin branch in Lab spawn; delete `AT66GoblinThiefEnemy` class; remove any Enemies.csv row / specials-registry entry; remove gold-steal hooks. Grep for all references before deletion.
- **Files/data touched:** `T66EnemyDirector.cpp/.h`, `T66RngTuningConfig.h` (+ tuning data), `T66GameMode_Lab.cpp`, `T66GoblinThiefEnemy.*`, `Enemies.csv`/registry, any Lab UI exposing the Goblin test spawn.
- **Verification needed:** Build compiles; no dangling references; Lab spawn menu no longer lists Goblin; waves spawn without Goblin.
- **Risks/open questions:** Lab spawn menu enum/index may shift; check UI that references GoblinThief by index. Confirm no save/achievement depends on it.

### Item 4 — Remove Debuff enemy entirely
- **Current state:** `AT66UniqueDebuffEnemy` (floating debuff enemy) + status projectile; **Lab-only** spawn (`T66GameMode_Lab.cpp:163`, "UniqueEnemy" branch). Not in normal waves.
- **Proposed approach (removal):** Remove Lab spawn branch; delete `AT66UniqueDebuffEnemy` + its status projectile class; remove supporting code and any data/registry rows. Grep references.
- **Files/data touched:** `T66GameMode_Lab.cpp`, `T66UniqueDebuffEnemy.*`, debuff projectile class, any Lab UI entry.
- **Verification needed:** Build compiles; Lab menu no longer lists it; no dangling references.
- **Risks/open questions:** Confirm the status-projectile class is not shared with any other enemy before deleting.

### Item 5 — Vendor hidden boss (repurpose Gambler; retarget trigger; remove casino-anger spawn)
- **Current state:** `AT66GamblerBoss` is a complete `AT66BossBase` (BossID="GamblerBoss", MaxHP=1000, 6 hit-zones, Gambler attack profile, pink/cyan, ground AOE, CharacterVisual "GamblerBoss", drops Item_VendorToken + GamblersToken upgrade, ResetGamblerAnger on death). Spawned by `TriggerCasinoBossIfAngry()` (`T66PlayerController_Overlays.cpp:5983`, spawn `:6052`) when `CasinoAnger01>=1.0 && !BossActive`. Shop steal flow exists: `T66CasinoVendorTabWidget.cpp:2453 OnStealStop()` resolves outcome and at `:2481` calls TriggerCasinoBossIfAngry.
- **Proposed approach:**
  1. Repurpose the Gambler entity into "Vendor" boss: rename class/BossID/visual identity ("GamblerBoss"→"VendorBoss"), strip Gambler-specific identity (naming, achievement label if Pablo wants), keep mechanics/hit-zones/AOE.
  2. **Retarget trigger:** drive the Vendor boss from the existing "attempt to steal from shop" mechanic. On a failed/triggering steal in `OnStealStop()`, spawn the Vendor boss directly instead of (or as the replacement for) the casino-anger path.
  3. **Remove casino-anger-to-boss spawn:** delete/neutralize `TriggerCasinoBossIfAngry()`'s spawn-on-anger path so anger no longer summons the boss. **Do NOT touch casino as interactable/gambling feature** — only remove the boss spawn path.
  4. Remove all "Gambler-as-enemy" references.
- **Files/data touched:** `T66GamblerBoss.cpp/.h` (→ Vendor), `T66PlayerController_Overlays.cpp:5983-6062` (remove anger spawn), `T66CasinoVendorTabWidget.cpp` (steal→boss spawn), boss visual/data registry, achievement/upgrade naming if rename desired, any CharacterVisual map entry.
- **Verification needed:** Steal-from-shop attempt spawns Vendor boss; casino anger no longer spawns a boss; gambling/casino interactable still functions; boss death still drops Vendor token.
- **Risks/open questions:** Decide whether the GamblersToken achievement/upgrade is renamed or kept. Confirm `CasinoAnger01` is still used elsewhere (anger may still gate other behavior even if it no longer spawns the boss). Decide exact steal condition that triggers the boss (any failed steal? threshold?). Confirm only ONE hidden boss remains after this.

### Item 6 — 12 mega-mob minibosses per difficulty (replace placeholder Slime guardian)
- **Current state:** `T66SpawnTowerGateGuardian` (`T66GameMode_Tower.cpp:146`) spawns ONE guardian via `ConfigureAsMob("Slime")` + `ApplyMiniBossMultipliers(3.0,2.0,1.75)`, `bDropsLoot=false`, tag `T66_Tower_DescentGuardian`. `IsPlacedTowerMinibossFloor` (:554) and `EnsurePlacedTowerMinibossForFloor` (:574) drive placement; `:538` sets HoleActor `bRequiresGuardianDefeated`. Structure already = 3 gates/stage × 4 stages = 12 gate encounters/difficulty. Mega-mob mechanism (model/behavior reuse + scaled HP/Dmg/Size) already exists.
- **Proposed approach:** Replace the hardcoded `"Slime"` MobID with a per-(stage, gate) assignment so each of the 12 gate slots gets a distinct basic mob's mega version. Add an assignment table (data-driven preferred: a CSV/DataTable keyed by difficulty+stage+gateIndex→MobID; or a code constant map if data wiring is heavier). Keep `ApplyMiniBossMultipliers` scaling (reuse/refactor). Keep must-kill-to-proceed gating. Minibosses stay rich actors.
- **Proposed default assignment (12 gate slots/difficulty — sensible defaults for Pablo to adjust):** map each difficulty's 4 stages × 3 gates to that theme's mob families, biasing toward "MiniBossFeel" mobs first, then heavier mobs. Concrete defaults to be tabulated in Section 4 once Pablo confirms the basic-mob→theme mapping (final 12/theme after item 7).
- **Files/data touched:** `T66GameMode_Tower.cpp:146` (parameterize MobID), `:15-18` constants, new gate-assignment data (CSV/DataTable) or constant map, possibly `T66TowerMapTerrain` if gate index is needed.
- **Verification needed:** Each gate spawns the intended mega-mob with scaled HP/Dmg/Size; gate cannot be passed until killed; 12 distinct encounters across a difficulty.
- **Risks/open questions:** Does `ConfigureAsMob` accept any basic MobID and resolve its model/behavior, or only some? Need per-mob validation. Gate index source (how to know which of the 3 gates on a stage) must be confirmed. Whether assignment should be per-difficulty-distinct or shared across difficulties.

### Item 7 — Add 10 basic mobs (placeholder model/behavior → 12 per theme end state)
- **Current state:** `Enemies.csv` (50 rows) backs `DT_Enemies`. Families resolve via `FT66EnemyFamilyResolver::ResolveEnemyClass` to Melee/Rush/Flying/Ranged classes. `Stages.csv` has EnemyA..EnemyJ (10 slots) per stage; many "None". Basic mobs route lightweight (`AT66MobBase`).
- **Proposed approach:** Add 10 rows to `Enemies.csv` (and rebuild `DT_Enemies`), each using a placeholder model/behavior of an existing mob, per Pablo's spec:
  - Dungeon: Cursed Crow (Flying, placeholder Cave Bat); Famished Ghoul (Rush, placeholder Rat Pack)
  - Forest: Will-o-Wisp (Flying, Hive Wasp); Gore Stag (Rush, Tusker Boar)
  - Ocean: Gull Diver (Flying, Ghost Ray); Hammerjaw (Melee, Crab Guard)
  - Martian: Recon Orb (Flying, Saucer Drone); Carapace Brute (Melee, Crystal Crawler)
  - Hell: Cinder Wraith (Flying, Fire Skull); Brimstone Brute (Melee, Bone Knight)
  - Set FamilyID/RoleID to match the placeholder source; ModelStatus=Placeholder; StatusEffectOnHit=None; assign StageTag/PrimaryColor/etc. Then add each to the relevant `Stages.csv` EnemyA..J slots to reach 12/theme.
- **Files/data touched:** `Content\Data\Enemies.csv` (+ `DT_Enemies.uasset` rebuild), `Content\Data\Stages.csv` (+ `DT_*` if applicable). XP/Rarity values per existing siblings.
- **Verification needed:** New mobs resolve to correct family class, spawn in waves on the right stages, use placeholder visual without errors; theme counts reach 12.
- **Risks/open questions:** Confirm exact existing-mob source IDs (Cave Bat, Rat Pack, etc.) match real EnemyIDs in Enemies.csv. Stage slot capacity (only 10 EnemyA..J slots/stage) vs 12 mobs/theme — confirm how 12/theme maps onto stage slots (themes span multiple stages). uasset rebuild requires editor; out of current write scope (data-only edit deferred to implementation).

### Item 8 — Remove Exploder/Stutterer/Burrower archetype tags (12 mobs → plain family behavior)
- **Current state:** `Enemies.csv` Archetype column: Exploder×5 (MimicLure, SporeBomb, SeaMine, CrystalBomber, SinEater), Stutterer×4 (CryptWraith, AnglerfishStalker, MindSlug, DemonSentinel), Burrower×3 (VineStrangler, SandTunneler, HellWyrm). `pending_issues_Enemies.md` [Major]: no AT66Exploder/Stutterer/BurrowerEnemy classes exist — these mobs **already** run family-fallback behavior (exploders don't explode, etc.).
- **Proposed approach:** Clear the Archetype value (→ plain/None) for these 12 rows in `Enemies.csv`; rebuild `DT_Enemies`. Because no archetype classes exist, no behavior code changes are needed — this is honest cleanup that matches current runtime reality. Update `pending_issues_Enemies.md` to close the [Major] (doc edit deferred — out of current write scope).
- **Files/data touched:** `Content\Data\Enemies.csv` (Archetype column, 12 rows) + `DT_Enemies.uasset`.
- **Verification needed:** Mobs spawn with family behavior unchanged; no resolver path keys off Archetype.
- **Risks/open questions:** Confirm nothing reads Archetype for spawn weighting / VFX / UI before clearing.

### Item 9 — Remove MiniBoss-feel tag (6 mobs → ordinary basic mobs)
- **Current state:** `Enemies.csv` Feeling=MiniBossFeel for 6: TombSpider, TreantAncient, AnglerfishStalker, PlasmaSentinel, BoneKnight, DemonSentinel.
- **Proposed approach:** Clear Feeling=MiniBossFeel for these 6 rows; rebuild `DT_Enemies`. They remain ordinary basic mobs.
- **Files/data touched:** `Content\Data\Enemies.csv` (Feeling column, 6 rows) + `DT_Enemies.uasset`.
- **Verification needed:** No spawn/selection logic keys off Feeling==MiniBossFeel (e.g., the old random-promotion path). Since random promotion is dormant (item 11), low risk.
- **Risks/open questions:** Confirm Feeling is not used by item 6's mega-mob default assignment heuristic AFTER this change (if defaults bias toward MiniBossFeel, capture the original list before clearing).

### Item 10 — Wording/naming: "gameplay floors" → "mob floors"; confirm gate placement
- **Current state:** `T66TowerMapTerrain` uses `FirstGameplayFloorNumber=2`, `LastGameplayFloorNumber=4`, `StartFloorNumber=1`, `BossFloorNumber=5`. Structure already correct: floor 1 start/no enemies, 2-4 mob floors, 5 boss/no mobs; 3 descent gates per stage (2→3, 3→4, 4→5).
- **Proposed approach:** Rename "gameplay floor" identifiers/strings to "mob floor" (or chosen term) across `T66TowerMapTerrain.*`, `T66GameMode_Tower.cpp` (`IsPlacedTowerMinibossFloor`, etc.), and any UI/log strings. Pure terminology refactor — no behavior change. Confirm (and document) gate guardians sit only on the 2→3, 3→4, 4→5 transitions.
- **Files/data touched:** `T66TowerMapTerrain.h/.cpp`, `T66GameMode_Tower.cpp`, any references to "GameplayFloor", UI strings.
- **Verification needed:** Build compiles after rename; grep shows no stale "gameplay floor" identifiers; gate placement confirmed on the 3 transitions only.
- **Risks/open questions:** Confirm Pablo's preferred term ("mob floor" vs other). Rename touches many call sites — do as a contained pass.

### Item 11 — Remove dormant random-miniboss-promotion tuning
- **Current state:** `T66EnemyDirector.h:64-73`: `MiniBossChancePerWave=0.10`, `MiniBossScale=1.75`, `MiniBossHPScalar=3.0`, `MiniBossDamageScalar=2.0`; `ActiveMiniBoss` at :179. Promotion already disabled in `.cpp:1162-1164` (MiniBossIndex=INDEX_NONE). Minibosses are now exclusively placed mega-mobs (item 6).
- **Proposed approach:** Remove the dormant tuning fields and `ActiveMiniBoss` state and the disabled promotion block. Ensure scaling needed by item 6 lives in the Tower guardian path (`ApplyMiniBossMultipliers`), not here.
- **Files/data touched:** `T66EnemyDirector.h:64-73,:179`, `.cpp:1162-1164` (+ any references to these fields / any tuning CSV rows).
- **Verification needed:** Build compiles; no references to removed fields; placed mega-mobs (item 6) still scale correctly via the Tower path.
- **Risks/open questions:** Confirm `ApplyMiniBossMultipliers` constants in Tower are the source of truth (they are: `:15-18`), so removing EnemyDirector copies is safe.

---

## 4. Affected Files/Data Map

### Code
| File | Items | Change type |
|---|---|---|
| `T66LoanShark.cpp/.h` | 1 | Verify only |
| `T66GameMode_Backrooms.cpp`, `T66BackroomsDoorInteractable.cpp` | 2 | Verify only |
| `T66EnemyDirector.cpp:1107-1160` / `.h` | 3, 11 | Remove Goblin wave spawn; remove dormant miniboss tuning |
| `T66RngTuningConfig.h:94-97` | 3 | Remove Goblin RNG tuning |
| `T66GameMode_Lab.cpp:159,:163` | 3, 4 | Remove Goblin + Debuff Lab spawn branches |
| `T66GoblinThiefEnemy.*` | 3 | Delete class |
| `T66UniqueDebuffEnemy.*` + debuff projectile | 4 | Delete classes |
| `T66GamblerBoss.cpp/.h` | 5 | Repurpose → Vendor; strip Gambler identity |
| `T66PlayerController_Overlays.cpp:5983-6062` | 5 | Remove casino-anger boss spawn |
| `T66CasinoVendorTabWidget.cpp:2453,:2481` | 5 | Retarget steal → Vendor boss spawn |
| `T66GameMode_Tower.cpp:146,:15-18,:538,:554,:574` | 6 | Parameterize gate MobID; per-gate assignment |
| `T66TowerMapTerrain.h/.cpp` | 10 | Rename gameplay→mob floor |

### Data / Content
| File | Items | Change type |
|---|---|---|
| `Content\Data\Enemies.csv` (+ `DT_Enemies.uasset`) | 3, 7, 8, 9 | Remove Goblin row; add 10 mob rows; clear Archetype (12); clear Feeling (6) |
| `Content\Data\Stages.csv` (+ DT) | 7 | Add new mobs to EnemyA..J slots → 12/theme |
| New gate-assignment data (CSV/DataTable) | 6 | 12 (stage,gate)→MobID mappings/difficulty |
| `Content\Data\LoanShark.csv` | 1 | Verify only |
| `Content\Data\UniqueEnemies.csv` | 2 | Verify only (BackroomsChaser) |
| Tuning CSV/data for Goblin + dormant miniboss (if any) | 3, 11 | Remove rows |

### Docs (deferred — outside current write scope, flag for follow-up)
- `pending_issues_Gameplay.md:49` stale "Backrooms unimplemented/disabled" note → update.
- `pending_issues_Enemies.md` [Major] archetype-class note → close after item 8.

---

## 5. Sequencing / Pass Split

Recommended as separate approved passes (each its own Codex packet):

1. **Pass A — Verification (no edits):** Confirm Loan Shark (item 1) and Backrooms Stalker (item 2) in editor/runtime. Capture the Quick Revive asset-deletion risk. Output: verification report.
2. **Pass B — Removal:** Goblin Thief (3), Debuff enemy (4), dormant miniboss tuning (11), tag clears (8, 9). All low-coupling deletions/data clears. Build + grep clean.
3. **Pass C — Repurpose/Retarget:** Vendor boss from Gambler + steal trigger + remove casino-anger spawn (5). Single coherent boss-path change.
4. **Pass D — Terminology:** gameplay→mob floor rename (10). Contained refactor, easiest after removals reduce call sites.
5. **Pass E — Additive:** 10 new mobs (7), then 12 mega-mob gate assignments (6). Data-heavy; needs editor for uasset rebuilds. Item 6 default-assignment depends on item 7's final 12/theme set.

Rationale: removals first shrink surface area; repurpose and rename next; additive last (depends on the cleaned roster and needs the most design confirmation + editor work).

---

## 6. Unclear / Missing Foundation

- **No archetype classes exist** (AT66Exploder/Stutterer/Burrower). Item 8's tag removal is therefore honest cleanup, but confirm nothing reads `Archetype` for non-behavior purposes (VFX/UI/weighting) before clearing.
- **Mega-mob MobID coverage (item 6):** unverified whether `ConfigureAsMob` resolves every basic MobID's model/behavior. Needs per-mob validation before the 12-slot assignment is trusted.
- **Gate index source (item 6):** how the spawn code knows which of the 3 gates on a stage it is (for per-gate MobID) is not yet traced — must be confirmed.
- **Stage-slot vs 12-per-theme (item 7):** `Stages.csv` exposes only EnemyA..EnemyJ (10 slots) per stage; 12/theme must map across multiple stages. Confirm the theme→stages mapping.
- **Existing-mob source IDs (item 7):** the named placeholders (Cave Bat, Rat Pack, Hive Wasp, Tusker Boar, Ghost Ray, Crab Guard, Saucer Drone, Crystal Crawler, Fire Skull, Bone Knight) must be matched to real EnemyIDs in Enemies.csv before authoring rows.
- **CasinoAnger usage post-item-5:** confirm `CasinoAnger01` is not relied on elsewhere once it no longer spawns the boss.
- **GamblersToken achievement/upgrade (item 5):** keep or rename to Vendor — Pablo decision.
- **Specials/boss registry:** is there a single list enumerating specials/hidden bosses that must be updated so exactly two specials + one hidden boss remain? Not yet located.
- **Quick Revive asset deletions (working tree):** `QuickReviveIcon.uasset` + QuickReviveVending assets are deleted in the current working tree — may break the Backrooms reward. Pre-existing, out of this plan's scope, but flagged.
- **uasset/DataTable rebuilds** require the editor and are out of the current read/report-only scope; data edits (items 6-9) are deferred to implementation passes.

---

## 7. Out-of-Scope / Non-Actions

Per Codex approval, NO implementation was performed. Explicitly NOT touched:
- No source/config/content/save/CSV/uasset/DataTable/runtime/build/stage/Git changes.
- No cleanup or deletion.
- No casino interactable / gambling-feature changes (item 5 removes only the boss spawn path).
- No status-effect mob work, no real model work.
- No B.13 sandbox deletion; no deprecated rich-mob/CVar/projectile-class deletion.
- No broad Git/LFS scans.
- No Mini/minigame inspection or edits.
- Doc updates to `pending_issues_*.md` are noted as needed but NOT made (deferred to implementation passes).

---

## 8. Technical Traceability

- Loan Shark: `T66LoanShark.cpp/.h`; `LoanShark.csv`; `T66GameMode.cpp:1384,:1468,:1214-1217`.
- Backrooms: `T66GameMode_Backrooms.cpp:30,:41,:447,:588-621,:929,:948`; `T66BackroomsDoorInteractable.cpp:106`; `T66GameMode_Bootstrap.cpp:484,:736`; `UniqueEnemies.csv` BackroomsChaser.
- Goblin Thief: `T66EnemyDirector.cpp:1107-1160`; `T66RngTuningConfig.h:94-97`; `T66GameMode_Lab.cpp:159`.
- Debuff enemy: `T66GameMode_Lab.cpp:163`.
- Gambler/Vendor boss: `T66GamblerBoss.cpp/.h`; `T66PlayerController_Overlays.cpp:5983,:6052`.
- Shop steal: `T66CasinoVendorTabWidget.cpp:2453,:2481`; `T66RunStateSubsystem_EconomyInventory.cpp` (steal resolution, ~:300-380).
- Mega-mob/guardian: `T66GameMode_Tower.cpp:15-18,:146,:538,:554,:574`.
- Floor structure: `T66TowerMapTerrain.h:153-156`/`.cpp:58-62`.
- Dormant miniboss tuning: `T66EnemyDirector.h:64-73,:179`; `.cpp:1162-1164`.
- Enemy data: `Enemies.csv` (50 rows; Archetype Exploder×5/Stutterer×4/Burrower×3; Feeling MiniBossFeel×6); `Stages.csv` (EnemyA..J); `UniqueEnemies.csv`.
- Missing archetype classes: `pending_issues_Enemies.md` [Major].
- Stale Backrooms note: `pending_issues_Gameplay.md:49`.

---

*End of plan. Investigation and report only; no implementation performed. Awaiting Pablo review.*
