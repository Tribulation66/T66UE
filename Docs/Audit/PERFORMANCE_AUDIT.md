# T66 Performance & Vulnerability Audit

**Date:** 2025-02-13 (audit); **Last updated:** 2025-02-15 (fixes applied).  
**Scope:** Frame drops during gameplay (60→30 FPS), and other performance/risk hotspots.

---

## Status summary (post-fixes)

| # | Item | Status | What was done |
|---|------|--------|----------------|
| 1.1 | T66LoanShark::Tick | **FIXED** | Cached NPC list refreshed every 2 s (was every frame). |
| 1.2 | UT66CombatComponent::TryFire | **FIXED** | Target search throttled to ~4 Hz (MinTargetSearchIntervalSeconds); cache revalidate every 12 fires. |
| 1.3 | RefreshMapData | **FIXED** | Map cache: full 4× TActorIterator only every 1.5 s; other ticks update positions from cache. |
| 1.4 | AT66EnemyBase safe-zone | **FIXED** | SafeZoneCheckIntervalSeconds increased to 1.0 s (was 0.25 then 0.5). |
| 2.1 | MiasmaManager EnsureSpawnedCount | **FIXED** | Max 8 tiles per frame (was unbounded). |
| 2.2 | EnemyDirector SpawnWave | **Not changed** | Could cap spawns per frame with a pending queue; left as-is for now. |
| 3 | HouseNPCBase gravity trace | **FIXED** | Trace runs every 3rd tick only until settled. |
| 3 | CompanionBase ground trace | **FIXED** | Ground LineTrace every 3rd tick. |
| 5 | Delegate fan-out / RefreshHUD | **FIXED** | RefreshHUD throttle increased to 0.5 s (was 0.25 s). |

**Lag tracking:** A `[LAG]` logger (UT66LagTrackerSubsystem + FLagScopedScope) was added so slow operations log under `[LAG]` with cause. Use `T66.LagTracker.Enabled` and `T66.LagTracker.ThresholdMs` to tune.

---

## Executive summary (original)

The most likely **root causes of frame drops** during a run were:

1. **T66LoanShark::Tick** — Full `TActorIterator<AT66HouseNPCBase>` **every frame** while the Loan Shark is alive (critical). → **FIXED.**
2. **UT66CombatComponent::TryFire** — Three `TActorIterator` scans every fire interval. → **FIXED** (throttled + cache).
3. **UT66GameplayHUDWidget::RefreshMapData** — Four `TActorIterator` scans every 0.5 s. → **FIXED** (cache, full refresh every 1.5 s).
4. **AT66EnemyBase safe-zone check** — Every 0.25 s per enemy, N×M checks. → **FIXED** (1.0 s interval).
5. **AT66MiasmaManager::UpdateFromRunState** — Many tiles in one frame. → **FIXED** (8 per frame cap).
6. **Heavy delegate fan-out** — RefreshHUD on many events. → **FIXED** (0.5 s throttle).
7. **Per-frame Tick load** — HouseNPCBase/CompanionBase line traces. → **FIXED** (every 3rd tick).

Secondary: **T66WheelOverlayWidget::TickSpin** at ~60 Hz while wheel open; **EnemyDirector::SpawnWave** can still spawn a full wave in one frame (not changed).

---

## 1. Critical: TActorIterator in hot paths

### 1.1 T66LoanShark::Tick — **every frame** (CRITICAL) — **FIXED**

**File:** `Source/T66/Gameplay/T66LoanShark.cpp` (lines 84–116)

- **What:** While the Loan Shark actor exists, `Tick()` runs every frame and performs a full `TActorIterator<AT66HouseNPCBase>(GetWorld())` to implement “stay out of NPC safe zones.”
- **Impact:** One full world scan per frame. With many actors in the level, this alone can push frame time over the 60 FPS budget and trigger engine frame-rate smoothing (e.g. drop to 30 FPS).
- **Fix applied:** Uses a static `FSafeNPCache` refreshed every 2 s; per-frame only walks the cached NPC list.

---

### 1.2 UT66CombatComponent::TryFire — every fire interval — **FIXED**

**File:** `Source/T66/Gameplay/T66CombatComponent.cpp` (lines 298–334)

- **What:** Each time the hero tries to fire (timer-driven, `EffectiveFireIntervalSeconds`, typically ~0.2–0.5 s), the code runs three separate actor iterators to find the closest target: `AT66GamblerBoss`, `AT66BossBase`, `AT66EnemyBase`.
- **Impact:** Three full world scans per fire. At 2–5 attacks per second this becomes 6–15 full scans per second. With 50+ enemies, each iterator is expensive.
- **Recommendation:** Maintain a single “current target” or a small candidate set updated at lower frequency (e.g. 2–5 Hz), or use a spatial structure (e.g. grid, octree) for “nearest enemy in range” instead of scanning all actors every fire.

---

### 1.3 UT66GameplayHUDWidget::RefreshMapData — **FIXED**

**File:** `Source/T66/UI/T66GameplayHUDWidget.cpp` (RefreshMapData; timer 0.5 s)

- **What:** A timer calls `RefreshMapData` every 0.5 s. That function ran four `TActorIterator` loops for minimap/full map.
- **Impact:** Four full world scans twice per second could spike frame time.
- **Fix applied:** Map cache (MapCache, MapCacheRefreshIntervalSeconds = 1.5). Full 4× TActorIterator only every 1.5 s; on other ticks we build markers from cached actor refs and update positions only.

---

### 1.4 AT66EnemyBase safe-zone check — **FIXED**

**File:** `Source/T66/Gameplay/T66EnemyBase.cpp` (lines 299–353), `T66EnemyBase.h` (SafeZoneCheckIntervalSeconds = 0.25f)

- **What:** Every 0.25 s each enemy runs safe-zone logic. A **static** cache of `AT66HouseNPCBase` is refreshed at most every 2 s (one `TActorIterator` when the first enemy needs a refresh). After that, every enemy still runs a loop over all cached NPCs doing `GetActorLocation()` and distance checks.
- **Impact:** Every 0.25 s: up to one full world iterator (when cache expires), plus **N_enemies × N_NPCs** distance checks. With 30 enemies and 10 NPCs that’s 300 distance/location ops every 0.25 s, plus a full iterator every 2 s. Can cause periodic spikes.
- **Fix applied:** SafeZoneCheckIntervalSeconds set to 1.0 s (was 0.25, then 0.5). Same cache; N×M loop runs at 1 Hz per enemy.

---

### 1.5 Other TActorIterator usages (not in tight hot paths)

- **GameMode:** `HandleDifficultyChanged` iterates all `AT66EnemyBase` and `AT66BossBase` (on difficulty change only). `TrySpawnLoanSharkIfNeeded` can do up to 10× `TActorIterator<AT66HouseNPCBase>` when finding spawn. Level setup / one-shot flows use many iterators (StaticMeshActor, lights, etc.) — acceptable if not per-frame.
- **T66BossGate::TryTriggerForActor:** One `TActorIterator<AT66BossBase>` when gate triggers (one-shot).
- **T66EnemyDirector::SpawnWave:** Uses cached House NPC list (refresh every 2 s), which is good; spawn wave itself is on a timer, not every frame.
- **T66PlayerController:** Ultimate uses one iterator over enemies (one-shot). TeleportToNPC uses one iterator over House NPCs (one-shot). These are fine.

---

## 2. Miasma and spawning hitches

### 2.1 AT66MiasmaManager::UpdateFromRunState / EnsureSpawnedCount — **FIXED**

**File:** `Source/T66/Gameplay/T66MiasmaManager.cpp` (lines 52–86, 88–110)

- **What:** `UpdateFromRunState()` is invoked from `AT66GameMode::HandleStageTimerChanged`, which is broadcast **every time the stage timer’s integer second changes** (i.e. about once per second). It computes a `DesiredCount` of miasma tiles from elapsed time and calls `EnsureSpawnedCount(DesiredCount)`. That function runs a **while** loop spawning tiles one by one until `SpawnedTiles.Num() >= DesiredCount`.
- **Impact:** If the game lags or the timer jumps, `DesiredCount` can be much larger than current spawn count. One frame then spawns many actors (SpawnActorDeferred + FinishSpawning + Add to array), causing a large hitch.
- **Fix applied:** MaxTilesToSpawnPerFrame = 8; loop capped so at most 8 tiles per call; rest spread over subsequent seconds.

---

### 2.2 AT66EnemyDirector::SpawnWave — **Not changed**

**File:** `Source/T66/Gameplay/T66EnemyDirector.cpp` (lines 107–250+)

- **What:** Spawn wave runs on a timer. It can spawn up to `EffectivePerWave` enemies (max 200) in one go.
- **Impact:** Spawning many actors in one frame can hitch when a large wave spawns.
- **Note:** A per-frame spawn cap would require a pending queue and extra state; left as-is. Consider batching in a future pass.

---

## 3. Per-frame Tick load

Actors that **tick every frame** during gameplay:

| Actor / Component        | File                    | What Tick does |
|--------------------------|-------------------------|----------------|
| **AT66GameMode**         | T66GameMode.cpp         | RunState timer ticks (stage, speedrun, hero timers), skill rating, loan shark spawn/despawn. Light. |
| **AT66HeroBase**          | T66HeroBase.cpp         | Stage slide timer; HeroSpeedSubsystem update; MaxWalkSpeed; animation state (Alert/Run). Subsystem + anim. |
| **AT66CompanionBase**     | T66CompanionBase.cpp    | Anim state; **LineTrace ground every 3rd tick** (fixed); SetActorLocation; heal timer. |
| **AT66EnemyBase**         | T66EnemyBase.cpp        | Safe-zone check every 1.0 s (fixed); armor/confusion timers; chase/flee AI. |
| **AT66LoanShark**         | T66LoanShark.cpp        | Cached NPC list every 2 s (fixed; was iterator every frame). |
| **AT66BossBase**          | T66BossBase.cpp         | Awaken distance check; chase player. Light. |
| **AT66GamblerBoss**       | T66GamblerBoss.cpp      | (Inherits / custom Tick — check for heavy work.) |
| **AT66VendorBoss**        | T66VendorBoss.cpp       | Same. |
| **AT66StartGate**         | T66StartGate.cpp        | Distance check to player; trigger. Light. |
| **AT66BossGate**          | T66BossGate.cpp         | Distance check to player; trigger. Light. |
| **AT66HeroProjectile**    | T66HeroProjectile.cpp   | Target check; steering; Destroy when hit. Light. |
| **AT66SlashEffect**       | T66SlashEffect.cpp      | Age/lifetime; scale and opacity updates; Destroy when done. Light. |
| **AT66HouseNPCBase**      | T66HouseNPCBase.cpp     | **Gravity LineTrace every 3rd tick** until settled (fixed); then optional face-player. |

- **CompanionBase:** Ground trace runs every 3rd tick (fixed).
- **HouseNPCBase:** Gravity-settle trace runs every 3rd tick until `bGravitySettled` (fixed).
- **EnemyBase:** Safe-zone at 1.0 s; with many enemies, Tick count and safe-zone N×M still add up but at lower frequency.

Cumulative effect: with 40 enemies + 1 companion + 1 hero + bosses + gates + Loan Shark + projectiles + slash effects, Tick and the iterators/traces above can push frame time over budget and trigger 60→30 FPS smoothing.

---

## 4. Timers and high-frequency callbacks

- **RefreshMapData:** 0.5 s timer; full four iterators only every 1.5 s (cached); see above.
- **RefreshFPS:** 0.25 s — likely light.
- **UpdateWorldDialoguePosition:** 0.05 s (20 Hz) while world dialogue is open — verify implementation is cheap.
- **T66WheelOverlayWidget::TickSpin:** **0.016f (~62.5 Hz)** while wheel is spinning — near every-frame.
- **T66GameplayHUDWidget::TickWheelSpin:** 0.033f (~30 Hz) — acceptable.
- **T66GamblerOverlayWidget::TickCoinSpin:** 0.033f — acceptable.
- **T66VendorOverlayWidget::TickStealBar:** 0.033f — acceptable.
- **Combat TryFire:** Target search throttled to ~4 Hz; three iterators only when cache miss and throttle allows.
- **Miasma tiles / boundary:** Damage timers (e.g. every few seconds) — apply damage; not iterator-heavy.
- **EnemyDirector SpawnWave:** SpawnIntervalSeconds (e.g. 2 s) — can spawn many at once.

---

## 5. Delegate broadcasts and HUD refresh

**RunStateSubsystem** has many multicast delegates (HeartsChanged, GoldChanged, StageTimerChanged, HeroProgressChanged, etc.). Key listeners:

- **T66GameplayHUDWidget** subscribes to many and calls **RefreshHUD** for several. **RefreshHUD** is throttled to at most once per 0.5 s (RefreshHUDThrottleSeconds). It calls RefreshEconomy, RefreshTutorialHint, RefreshStageAndTimer, RefreshSpeedRunTimers, RefreshBossBar, RefreshLootPrompt, RefreshHearts, RefreshStatusEffects, plus portrait/panel updates.
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

## 8. Recommended priority (all addressed except EnemyDirector)

1. ~~**Fix T66LoanShark::Tick**~~ — **Done:** cached NPC list every 2 s.
2. ~~**Optimize UT66CombatComponent::TryFire**~~ — **Done:** throttle + cache revalidate every 12 fires.
3. ~~**Throttle or cache RefreshMapData**~~ — **Done:** map cache; full iterators every 1.5 s.
4. ~~**Relax enemy safe-zone check**~~ — **Done:** SafeZoneCheckIntervalSeconds = 1.0 s.
5. ~~**Cap miasma spawns per frame**~~ — **Done:** MaxTilesToSpawnPerFrame = 8.
6. ~~**Review delegate fan-out / RefreshHUD**~~ — **Done:** RefreshHUD throttle 0.5 s; HouseNPCBase/CompanionBase trace every 3rd tick.
7. **EnemyDirector::SpawnWave** — Not changed; could add per-frame spawn cap with pending queue in future.
8. **Profile** — Use Unreal Insights or `stat game` / `stat FPS`; use `[LAG]` logs (T66LagTrackerSubsystem) to attribute spikes.

---

## 9. Files referenced

| File | Relevant areas |
|------|----------------|
| T66LoanShark.cpp | Tick: cached NPC list every 2 s |
| T66CombatComponent.cpp | TryFire: throttle + cache; TargetRevalidateEveryNFires 12, MinTargetSearchIntervalSeconds 0.25 |
| T66GameplayHUDWidget.cpp | RefreshMapData: map cache 1.5 s; RefreshHUD throttle 0.5 s |
| T66EnemyBase.cpp | Tick: SafeZoneCheck every 1.0 s; static cache every 2 s |
| T66EnemyBase.h | SafeZoneCheckIntervalSeconds = 1.0f |
| T66MiasmaManager.cpp | UpdateFromRunState; EnsureSpawnedCount capped at 8/frame |
| T66GameMode.cpp | HandleStageTimerChanged; HandleDifficultyChanged; Tick |
| T66RunStateSubsystem.cpp | Many Broadcasts; TickHeroTimers, TickStageTimer, TickSpeedRunTimer |
| T66CompanionBase.cpp | Tick: ground LineTrace every 3rd tick |
| T66HouseNPCBase.cpp | Tick: gravity LineTrace every 3rd tick until settled |
| T66LagTrackerSubsystem | [LAG] logging; T66.LagTracker.Enabled, T66.LagTracker.ThresholdMs |
| T66WheelOverlayWidget.cpp | TickSpin timer 0.016f (374) |
| T66PlayerController.cpp | UpdateWorldDialoguePosition 0.05f (1196, 1224, 1251) |

---

*End of audit.*
