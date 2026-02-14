# T66 Performance & Vulnerability Audit

**Date:** 2025-02-13  
**Scope:** Frame drops during gameplay (60→30 FPS), and other performance/risk hotspots.  
**No code changes made; audit only.**

---

## Executive summary

The most likely **root causes of frame drops** during a run are:

1. **T66LoanShark::Tick** — Full `TActorIterator<AT66HouseNPCBase>` **every frame** while the Loan Shark is alive (critical).
2. **UT66CombatComponent::TryFire** — Three `TActorIterator` scans (GamblerBoss, BossBase, EnemyBase) **every fire interval** (~0.2–0.5 s) to find the closest target.
3. **UT66GameplayHUDWidget::RefreshMapData** — Four `TActorIterator` scans every **0.25 s** for minimap/full map (HouseNPC, StageGate, EnemyBase, MiasmaBoundary).
4. **AT66EnemyBase safe-zone check** — Every **0.25 s** per enemy: shared cache refresh every 2 s (one iterator), then **N×M** distance checks (N enemies × M NPCs) each 0.25 s.
5. **AT66MiasmaManager::UpdateFromRunState** — Invoked every **stage timer second**; can spawn many miasma tiles in one frame when `EnsureSpawnedCount(Desired)` catches up.
6. **Heavy delegate fan-out** — RunState broadcasts (e.g. `HeroProgressChanged`, `StageTimerChanged`) trigger multiple HUD/GameMode/Combat refreshes; when they fire often, cost adds up.
7. **Per-frame Tick load** — Many actors tick every frame (Hero, Companion, all Enemies, Bosses, Gates, LoanShark, projectiles, slash effects); with high enemy count this is significant.

Secondary contributors: **T66WheelOverlayWidget::TickSpin** at ~60 Hz while wheel is open; **CompanionBase::Tick** line trace every frame for ground; **HouseNPCBase::Tick** line trace every frame until gravity-settled; **Sync LoadObject** in a few gameplay paths (e.g. UniqueDebuffEnemy mesh, CombatComponent preload).

---

## 1. Critical: TActorIterator in hot paths

### 1.1 T66LoanShark::Tick — **every frame** (CRITICAL)

**File:** `Source/T66/Gameplay/T66LoanShark.cpp` (lines 84–116)

- **What:** While the Loan Shark actor exists, `Tick()` runs every frame and performs a full `TActorIterator<AT66HouseNPCBase>(GetWorld())` to implement “stay out of NPC safe zones.”
- **Impact:** One full world scan per frame. With many actors in the level, this alone can push frame time over the 60 FPS budget and trigger engine frame-rate smoothing (e.g. drop to 30 FPS).
- **Recommendation:** Use a shared, cached list of safe-zone NPCs (e.g. refreshed every 1–2 s or on level load), same pattern as in `AT66EnemyBase` and `AT66EnemyDirector`. Never iterate all House NPCs in Tick.

---

### 1.2 UT66CombatComponent::TryFire — every fire interval

**File:** `Source/T66/Gameplay/T66CombatComponent.cpp` (lines 298–334)

- **What:** Each time the hero tries to fire (timer-driven, `EffectiveFireIntervalSeconds`, typically ~0.2–0.5 s), the code runs three separate actor iterators to find the closest target: `AT66GamblerBoss`, `AT66BossBase`, `AT66EnemyBase`.
- **Impact:** Three full world scans per fire. At 2–5 attacks per second this becomes 6–15 full scans per second. With 50+ enemies, each iterator is expensive.
- **Recommendation:** Maintain a single “current target” or a small candidate set updated at lower frequency (e.g. 2–5 Hz), or use a spatial structure (e.g. grid, octree) for “nearest enemy in range” instead of scanning all actors every fire.

---

### 1.3 UT66GameplayHUDWidget::RefreshMapData — every 0.25 s

**File:** `Source/T66/UI/T66GameplayHUDWidget.cpp` (lines 1048–1109, timer at 627)

- **What:** A timer calls `RefreshMapData` every 0.25 s. That function runs four `TActorIterator` loops: `AT66HouseNPCBase`, `AT66StageGate`, `AT66EnemyBase`, `AT66MiasmaBoundary`, building a marker array for minimap/full map.
- **Impact:** Four full world scans four times per second. With many enemies, the `AT66EnemyBase` iterator is costly and can spike frame time when the timer fires.
- **Recommendation:** Reduce refresh rate (e.g. 0.5 s), or maintain cached lists of “map-relevant” actors and only update positions; avoid full iterator over all enemies 4×/s.

---

### 1.4 AT66EnemyBase safe-zone check — every 0.25 s per enemy

**File:** `Source/T66/Gameplay/T66EnemyBase.cpp` (lines 299–353), `T66EnemyBase.h` (SafeZoneCheckIntervalSeconds = 0.25f)

- **What:** Every 0.25 s each enemy runs safe-zone logic. A **static** cache of `AT66HouseNPCBase` is refreshed at most every 2 s (one `TActorIterator` when the first enemy needs a refresh). After that, every enemy still runs a loop over all cached NPCs doing `GetActorLocation()` and distance checks.
- **Impact:** Every 0.25 s: up to one full world iterator (when cache expires), plus **N_enemies × N_NPCs** distance checks. With 30 enemies and 10 NPCs that’s 300 distance/location ops every 0.25 s, plus a full iterator every 2 s. Can cause periodic spikes.
- **Recommendation:** Increase `SafeZoneCheckIntervalSeconds` (e.g. 0.5–1.0 s). Consider a single shared “safe zone query” (e.g. on a timer) that writes a compact representation (e.g. circles) and have enemies sample that instead of each doing an NPC loop.

---

### 1.5 Other TActorIterator usages (not in tight hot paths)

- **GameMode:** `HandleDifficultyChanged` iterates all `AT66EnemyBase` and `AT66BossBase` (on difficulty change only). `TrySpawnLoanSharkIfNeeded` can do up to 10× `TActorIterator<AT66HouseNPCBase>` when finding spawn. Level setup / one-shot flows use many iterators (StaticMeshActor, lights, etc.) — acceptable if not per-frame.
- **T66BossGate::TryTriggerForActor:** One `TActorIterator<AT66BossBase>` when gate triggers (one-shot).
- **T66EnemyDirector::SpawnWave:** Uses cached House NPC list (refresh every 2 s), which is good; spawn wave itself is on a timer, not every frame.
- **T66PlayerController:** Ultimate uses one iterator over enemies (one-shot). TeleportToNPC uses one iterator over House NPCs (one-shot). These are fine.

---

## 2. Miasma and spawning hitches

### 2.1 AT66MiasmaManager::UpdateFromRunState / EnsureSpawnedCount

**File:** `Source/T66/Gameplay/T66MiasmaManager.cpp` (lines 52–86, 88–110)

- **What:** `UpdateFromRunState()` is invoked from `AT66GameMode::HandleStageTimerChanged`, which is broadcast **every time the stage timer’s integer second changes** (i.e. about once per second). It computes a `DesiredCount` of miasma tiles from elapsed time and calls `EnsureSpawnedCount(DesiredCount)`. That function runs a **while** loop spawning tiles one by one until `SpawnedTiles.Num() >= DesiredCount`.
- **Impact:** If the game lags or the timer jumps, `DesiredCount` can be much larger than current spawn count. One frame then spawns many actors (SpawnActorDeferred + FinishSpawning + Add to array), causing a large hitch.
- **Recommendation:** Cap the number of tiles spawned per call (e.g. spawn at most 5–10 per frame) and spread the rest over the next seconds, or throttle `UpdateFromRunState` so it doesn’t run every second with a large delta.

---

### 2.2 AT66EnemyDirector::SpawnWave

**File:** `Source/T66/Gameplay/T66EnemyDirector.cpp` (lines 107–250+)

- **What:** Spawn wave runs on a timer (e.g. every few seconds). It can spawn up to `EffectivePerWave` enemies (capped by difficulty, max 200) in one go, with placement checks and safe-zone tests.
- **Impact:** Spawning many actors in one frame can hitch. Less frequent than miasma but still a possible spike when a large wave spawns.
- **Recommendation:** Consider spawning in small batches (e.g. 3–5 per frame) with a short delay until the wave count is met.

---

## 3. Per-frame Tick load

Actors that **tick every frame** during gameplay:

| Actor / Component        | File                    | What Tick does |
|--------------------------|-------------------------|----------------|
| **AT66GameMode**         | T66GameMode.cpp         | RunState timer ticks (stage, speedrun, hero timers), skill rating, loan shark spawn/despawn. Light. |
| **AT66HeroBase**          | T66HeroBase.cpp         | Stage slide timer; HeroSpeedSubsystem update; MaxWalkSpeed; animation state (Alert/Run). Subsystem + anim. |
| **AT66CompanionBase**     | T66CompanionBase.cpp    | Anim state from SpeedSubsystem; **LineTraceSingleByChannel** every frame for ground; SetActorLocation; heal timer. |
| **AT66EnemyBase**         | T66EnemyBase.cpp        | Safe-zone check every 0.25 s (see above); armor/confusion timers; chase/flee AI. |
| **AT66LoanShark**         | T66LoanShark.cpp        | **TActorIterator<AT66HouseNPCBase> every frame** (critical). |
| **AT66BossBase**          | T66BossBase.cpp         | Awaken distance check; chase player. Light. |
| **AT66GamblerBoss**       | T66GamblerBoss.cpp      | (Inherits / custom Tick — check for heavy work.) |
| **AT66VendorBoss**        | T66VendorBoss.cpp       | Same. |
| **AT66StartGate**         | T66StartGate.cpp        | Distance check to player; trigger. Light. |
| **AT66BossGate**          | T66BossGate.cpp         | Distance check to player; trigger. Light. |
| **AT66HeroProjectile**    | T66HeroProjectile.cpp   | Target check; steering; Destroy when hit. Light. |
| **AT66SlashEffect**       | T66SlashEffect.cpp      | Age/lifetime; scale and opacity updates; Destroy when done. Light. |
| **AT66HouseNPCBase**      | T66HouseNPCBase.cpp     | **LineTraceSingleByChannel** every frame until gravity settled; then optional face-player. |

- **CompanionBase:** One line trace per companion per frame for grounding.
- **HouseNPCBase:** One line trace per NPC per frame until `bGravitySettled` (within `GravitySettleDuration`). Many NPCs = many traces per frame early in the level.
- **EnemyBase:** With many enemies, the sheer number of Ticks (and the 0.25 s safe-zone block with cache + N×M checks) adds up.

Cumulative effect: with 40 enemies + 1 companion + 1 hero + bosses + gates + Loan Shark + projectiles + slash effects, Tick and the iterators/traces above can push frame time over budget and trigger 60→30 FPS smoothing.

---

## 4. Timers and high-frequency callbacks

- **RefreshMapData:** 0.25 s (four iterators) — see above.
- **RefreshFPS:** 0.25 s — likely light.
- **UpdateWorldDialoguePosition:** 0.05 s (20 Hz) while world dialogue is open — verify implementation is cheap.
- **T66WheelOverlayWidget::TickSpin:** **0.016f (~62.5 Hz)** while wheel is spinning — near every-frame.
- **T66GameplayHUDWidget::TickWheelSpin:** 0.033f (~30 Hz) — acceptable.
- **T66GamblerOverlayWidget::TickCoinSpin:** 0.033f — acceptable.
- **T66VendorOverlayWidget::TickStealBar:** 0.033f — acceptable.
- **Combat TryFire:** EffectiveFireIntervalSeconds (e.g. 0.2–0.5 s) — three iterators each time.
- **Miasma tiles / boundary:** Damage timers (e.g. every few seconds) — apply damage; not iterator-heavy.
- **EnemyDirector SpawnWave:** SpawnIntervalSeconds (e.g. 2 s) — can spawn many at once.

---

## 5. Delegate broadcasts and HUD refresh

**RunStateSubsystem** has many multicast delegates (HeartsChanged, GoldChanged, StageTimerChanged, HeroProgressChanged, etc.). Key listeners:

- **T66GameplayHUDWidget** subscribes to many of them and calls **RefreshHUD** for several (InventoryChanged, HeroProgressChanged, DifficultyChanged, IdolsChanged, etc.). **RefreshHUD** calls RefreshEconomy, RefreshTutorialHint, RefreshStageAndTimer, RefreshSpeedRunTimers, RefreshBossBar, **RefreshLootPrompt**, RefreshHearts, RefreshStatusEffects, plus portrait/panel updates.
- **AT66GameMode:** StageTimerChanged → HandleStageTimerChanged → **MiasmaManager->UpdateFromRunState()** and TrySpawnLoanSharkIfNeeded. DifficultyChanged → HandleDifficultyChanged → iterate **all** enemies and bosses to apply scalar.
- **UT66CombatComponent:** InventoryChanged / HeroProgressChanged / SurvivalChanged / DevCheatsChanged → HandleInventoryChanged → RecomputeFromRunState + possibly reset fire timer + RefreshAttackRangeRing.

So when RunState broadcasts frequently (e.g. StageTimerChanged every second, or HeroProgressChanged when stats change), multiple systems run: HUD full refresh, miasma update, difficulty scalar over all enemies, combat recompute. That can create a periodic spike (e.g. once per second) and contribute to the engine deciding 60 FPS is not sustainable.

---

## 6. Synchronous loads and allocations

- **LoadObject / LoadClass in gameplay:**  
  - `T66PlayerController`: LoadClass for screen mapping.  
  - `T66UniqueDebuffEnemy`: LoadObject for static mesh (when that enemy type is used).  
  - `T66CombatComponent`: ShotSfx.LoadSynchronous(), SlashVFXNiagara.LoadSynchronous() in BeginPlay (once).  
  - `T66GameInstance`, `T66GameMode`: mostly async (StreamableManager) or one-time (e.g. HDRI in level setup).  
- **AddStructuredEvent / EventLog:** RunState calls AddStructuredEvent on many events (gold, items, etc.), which does EventLog.Add, TrimLogsIfNeeded, and LogAdded.Broadcast. No direct listener for LogAdded in the grep; cost is mainly allocation and broadcast. TrimLogsIfNeeded only does work when over MaxEventLogEntries / MaxStructuredEventLogEntries.

These are smaller contributors than the iterator and Tick hotspots but can add up or cause occasional hitches if they happen during combat.

---

## 7. Other vulnerabilities and risks

- **Static caches in EnemyBase and EnemyDirector:** Both use a static `FSafeNPCache` (or similar) for House NPCs. Written from multiple actors on the game thread. No locking; acceptable if only the game thread touches it, but if any async code ever touched it, it would be unsafe.
- **Loan Shark Tick:** Besides performance, the iterator-every-frame is a maintainability and correctness risk (e.g. if world has many NPCs or iterator behavior changes).
- **Difficulty change:** HandleDifficultyChanged iterates every enemy and every boss in the world. With very high counts this could be a noticeable one-time spike when difficulty changes.
- **MiasmaManager:** EnsureSpawnedCount spawning many tiles in one frame can cause a single large frame time spike and trigger smoothing to 30 FPS for a short period.

---

## 8. Recommended priority

1. **Fix T66LoanShark::Tick** — Remove per-frame TActorIterator; use a cached NPC list refreshed every 1–2 s (same pattern as EnemyBase/EnemyDirector).
2. **Optimize UT66CombatComponent::TryFire** — Avoid three full-world iterators per fire; use cached target or spatial query.
3. **Throttle or cache RefreshMapData** — Reduce rate and/or cache actor lists; avoid four full iterators every 0.25 s.
4. **Relax enemy safe-zone check** — Increase SafeZoneCheckIntervalSeconds and/or centralize safe-zone query.
5. **Cap miasma spawns per frame** — In EnsureSpawnedCount, spawn at most N tiles per call; spread the rest over subsequent frames.
6. **Review StageTimerChanged / HeroProgressChanged fan-out** — Ensure HUD and GameMode handlers stay cheap; consider coalescing or throttling RefreshHUD.
7. **Profile** — Use Unreal Insights or `stat game` / `stat FPS` to confirm which of these shows up as the largest cost during 60→30 drops.

---

## 9. Files referenced

| File | Relevant areas |
|------|----------------|
| T66LoanShark.cpp | Tick (84–116): TActorIterator every frame |
| T66CombatComponent.cpp | TryFire (298–334): 3× TActorIterator; timer 65, 95 |
| T66GameplayHUDWidget.cpp | RefreshMapData (1048–1109); timer 627; RefreshHUD 1402 |
| T66EnemyBase.cpp | Tick (287–362); SafeZoneCheckIntervalSeconds 0.25; static cache + iterator every 2 s |
| T66EnemyBase.h | SafeZoneCheckIntervalSeconds = 0.25f |
| T66MiasmaManager.cpp | UpdateFromRunState 52–86; EnsureSpawnedCount 88–110 |
| T66GameMode.cpp | HandleStageTimerChanged (585–594); HandleDifficultyChanged (596–620); Tick 541 |
| T66RunStateSubsystem.cpp | Many Broadcasts; TickHeroTimers, TickStageTimer, TickSpeedRunTimer |
| T66CompanionBase.cpp | Tick 170: LineTraceSingleByChannel every frame |
| T66HouseNPCBase.cpp | Tick 222: LineTraceSingleByChannel until settled |
| T66WheelOverlayWidget.cpp | TickSpin timer 0.016f (374) |
| T66PlayerController.cpp | UpdateWorldDialoguePosition 0.05f (1196, 1224, 1251) |

---

*End of audit.*
