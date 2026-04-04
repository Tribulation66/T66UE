# T66 Performance Audit — March 16, 2026

## Overview

This document catalogs every identified runtime performance inefficiency in the T66 codebase, ordered by impact. Each entry includes the exact location, root cause, and a recommended solution that accounts for the game's existing infrastructure (EnemyPoolSubsystem, ActorRegistrySubsystem, UITexturePoolSubsystem, etc.) and pixel-art VFX style.

---

## P0 — Critical (frame drops during normal gameplay)

### 1. Pixel VFX spawning at unsustainable rates

**Files:**
- `Source/T66/Gameplay/T66BossProjectile.cpp` — `Tick`, lines 46–72
- `Source/T66/Gameplay/T66UniqueDebuffProjectile.cpp` — `Tick`, lines 64–90
- `Source/T66/Gameplay/T66HeroPlagueCloud.cpp` — `Tick`, lines 47–81
- `Source/T66/Gameplay/T66BossGroundAOE.cpp` — `Tick`, lines 52–133
- `Source/T66/Gameplay/T66CombatComponent.cpp` — `T66SpawnBloodSpray`, lines 955–1031

**Cause:** The game's pixel-art VFX style places individual tinted Niagara instances at specific positions to create trails, clouds, rings, and blood sprays. Each actor calls `UNiagaraFunctionLibrary::SpawnSystemAtLocation` in its Tick at high frequency:

| Actor | Particles per batch | Interval | Spawns/sec/actor |
|-------|---------------------|----------|-------------------|
| BossProjectile | 2 | 0.04s | ~50 |
| UniqueDebuffProjectile | 2 | 0.04s | ~50 |
| HeroPlagueCloud | 6 | 0.08s | ~75 |
| BossGroundAOE (warning) | 8 | 0.06s | ~133 |
| BossGroundAOE (pillar) | 10 | 0.05s | ~200 |
| Blood spray (per death) | 24–40+ | one-shot | burst |

During a boss fight with multiple projectiles, an AOE, and a plague cloud active simultaneously, this can exceed 500 Niagara component spawns per second. Each spawn creates a UObject, registers it with the world, and adds draw calls.

Additionally, `FName(TEXT("User.Tint"))` and `FName(TEXT("User.SpriteSize"))` are constructed from string literals inside every spawn loop iteration, adding unnecessary hash-table lookups.

**Solution:**

Create a `UT66PixelVFXSubsystem` (WorldSubsystem) that centralizes all pixel VFX spawning:

- **Component pool:** Pre-allocate a ring buffer of Niagara components (e.g. 80–120). "Spawn" takes a component from the pool, sets its world location, activates it with tint/size parameters, and marks it as "in use." When the Niagara system finishes (or after a fixed lifetime), the component returns to the pool. No UObject creation or destruction in the hot path.
- **Global budget:** A per-frame spawn cap (e.g. 60). When the budget is exhausted, lower-priority spawns (trails) are silently dropped. Callers provide a priority (trail = low, AOE ring = medium, death burst = high). This guarantees a frame-time ceiling regardless of how many VFX-producing actors exist.
- **Cached FNames:** The subsystem holds `static const FName` members for `User.Tint`, `User.SpriteSize`, and `User.Color`, used by all callers.
- **API:** `SubSys->SpawnPixel(Location, Tint, SpriteSize, Priority)` replaces all direct `SpawnSystemAtLocation` calls.

All four Tick-based VFX spawners and `T66SpawnBloodSpray` are refactored to call through this subsystem.

Longer-term: redesign the `NS_PixelParticle` Niagara asset to support batch emission (accept a position+tint array via data interface so one system activation can emit N positioned pixels). This would further reduce the component count from N-per-batch to 1-per-batch.

---

### 2. HeroBase Tick: per-frame OverlapMulti for enemy touch

**File:** `Source/T66/Gameplay/T66HeroBase.cpp` — `Tick`, lines 507–548

**Cause:** The hero needs to detect when enemies are physically close enough to trigger a "touch bounce" (knockback). Because enemies use blocking collision (not overlap), overlap events don't fire between them and the hero. The code works around this by running `World->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(EnemyTouchRadius), Params)` every single frame. The cost of this query scales with the number of pawns in the world.

**Solution:**

Throttle the query to ~5 Hz (every 0.2s) using an accumulator. The bounce itself already has a cooldown (`EnemyBounceCooldown`), so checking 5× per second is more than sufficient to detect contact before the cooldown expires. Additionally, skip the query entirely when the hero's velocity is near zero (no new contacts possible if nobody is moving toward the hero — enemies have their own movement which still triggers, so use a generous threshold).

```
EnemyTouchCheckAccum += DeltaSeconds;
if (EnemyTouchCheckAccum < 0.2f) skip;
EnemyTouchCheckAccum = 0.f;
// ... existing overlap query ...
```

This reduces the overlap from ~60 queries/sec to ~5, an 85–90% reduction.

---

### 3. HeroBase Tick: per-frame ground LineTraces

**File:** `Source/T66/Gameplay/T66HeroBase.cpp` — `Tick`, lines 353–415

**Cause:** The hero continuously tracks `LastSafeGroundLocation` via two `LineTraceSingleByChannel` calls (ECC_WorldStatic then ECC_Visibility) every frame, both when grounded and when falling. This provides teleport-back-on-fall safety. Two traces per frame is excessive because the hero's position changes slowly relative to frame rate.

**Solution:**

Throttle to 10–15 Hz (every 0.066–0.1s) via an accumulator. The safe ground position doesn't need sub-frame precision — the hero covers a few units per frame at most, and the system's tolerance (`TerrainRecoveryFallSeconds`, `TerrainRecoveryCooldown`) operates on a much longer timescale.

```
GroundTraceAccum += DeltaSeconds;
if (GroundTraceAccum < 0.075f) skip ground trace section;
GroundTraceAccum = 0.f;
// ... existing TraceGround logic ...
```

When falling, the recovery check can remain at a slightly higher rate (e.g. 15 Hz) since timing matters more for the teleport decision.

---

### 4. CombatComponent TryFire: idol attacks duplicate primary overlap queries

**File:** `Source/T66/Gameplay/T66CombatComponent.cpp` — `TryFire`, lines 683–812

**Cause:** After the primary attack runs its overlap query (Pierce capsule at line 414, Slash sphere at line 459, etc.), the idol loop iterates each equipped idol and runs an independent overlap query of the same shape for the same attack type. With 3 idols equipped, a Pierce hero runs 4 capsule overlaps per attack; a Slash hero runs 4 sphere overlaps. These are redundant because the primary and idol attacks share the same shape, center, and range.

Bounce and DOT idol attacks use `FindClosestEnemyInRange` (which walks the small `EnemiesInRange` list) or `ApplyDOT` (single target), so they don't add overlap queries. The duplication only affects Pierce and AOE/Slash idol types.

**Solution:**

Cache the primary attack's overlap results and reuse them for idols of the same attack type:

- After `PerformPierce` runs, store `TArray<AActor*> CachedPierceTargets` (the sorted `InLine` list) in a local variable.
- After `PerformSlash` runs, store `TArray<FOverlapResult> CachedSlashOverlaps`.
- In the idol loop: if the idol's attack type matches the primary's, iterate the cached results instead of running a new OverlapMulti. Apply idol-specific damage, tint, and falloff to the cached target list.
- If the idol's attack type differs from the primary's (e.g. hero is Pierce but idol is AOE), run the query — this is rare and unavoidable.

This reduces overlap queries per attack from (1 + N_idols) to typically 1 or 2 total.

---

## P1 — High (noticeable lag under specific conditions)

### 5. CombatComponent: DataTable lookups every attack

**File:** `Source/T66/Gameplay/T66CombatComponent.cpp` — `TryFire`, lines 384–391 and 688–689

**Cause:** Every time the hero fires, `TryFire` calls `GI->GetHeroData(Hero->HeroID, HeroDataOut)` which does `DataTable->FindRow`. Then for each equipped idol, `GI->GetIdolData(IdolID, IdolData)` does another FindRow. Hero data and idol data are constant during a run (they only change when inventory changes). FindRow is a hash-map lookup — individually cheap, but unnecessary on a timer that fires multiple times per second.

The component already caches `CachedRunState` in `BeginPlay` (line 53) and already has a `RecomputeFromRunState()` method called on inventory change (line 193). The infrastructure for caching exists but isn't used for hero/idol data.

**Solution:**

Add `FHeroData CachedHeroData` and `TArray<TPair<FIdolData, ET66ItemRarity>> CachedIdolSlots` as members. Populate them in `RecomputeFromRunState()` (which already fires on InventoryChanged, HeroProgressChanged, SurvivalChanged, DevCheatsChanged). TryFire reads from these cached values. No data table access in the hot path.

---

### 6. CombatComponent: subsystem lookup per damage application

**File:** `Source/T66/Gameplay/T66CombatComponent.cpp` — `ApplyDamageToActor`, lines 835–836

**Cause:** Every call to `ApplyDamageToActor` resolves `GetWorld()->GetGameInstance()` and `GI->GetSubsystem<UT66FloatingCombatTextSubsystem>()`. In a single TryFire call, ApplyDamageToActor can be called many times (primary target + splash targets + idol targets + bounce chain targets). Each call re-resolves the subsystem.

**Solution:**

Cache `UT66FloatingCombatTextSubsystem*` as a member, resolved once in `BeginPlay` alongside `CachedRunState` (line 53). Use the cached pointer in `ApplyDamageToActor`. The subsystem lives on the GameInstance and outlives the component.

---

### 7. HUD NativeTick: unthrottled RefreshDPS and Slate invalidation

**File:** `Source/T66/UI/T66GameplayHUDWidget.cpp` — `NativeTick` (line 672), `RefreshDPS` (lines 741–752), `RefreshCooldownBar` (lines 680–718), `RefreshSpeedRunTimers`

**Cause:** `NativeTick` calls `RefreshDPS`, `RefreshCooldownBar`, and `RefreshSpeedRunTimers` every frame.

- `RefreshDPS` calls `DamageLog->GetRollingDPS()` (which trims the sample buffer), rounds the result, then calls `DPSText->SetText(FText::Format(...))` and `DPSText->SetColorAndOpacity(...)` every frame. Slate's `SetText` always invalidates layout even when the text is identical — Slate does not internally diff.
- `RefreshCooldownBar` calls `CooldownBarFillBox->SetWidthOverride(...)` every frame, which always invalidates layout.
- Both `RefreshCooldownBar` and `RefreshSpeedRunTimers` use `static int32` for their "last displayed" cache, which is shared across all widget instances (breaks in PIE with multiple players).

**Solution:**

- **RefreshDPS:** Add `int32 LastDisplayedDPS = -1` and `FLinearColor LastDisplayedDPSColor` as member variables. Compute DPS, round to int, compare to `LastDisplayedDPS`. Only call `SetText`/`SetColorAndOpacity` when the value or color changes. Optionally, move RefreshDPS to a 4–5 Hz timer instead of NativeTick — DPS updates every 0.2s are imperceptible.
- **RefreshCooldownBar:** Add `int32 LastDisplayedBarWidthPx = -1` as a member. Compute the bar width, round to nearest 2 pixels, compare. Only call `SetWidthOverride` when it changes. Replace the `static int32 LastRemainingCs` with a member variable.
- **RefreshSpeedRunTimers:** Replace `static int32 LastSpeedRunTotalCs` with a member variable.

---

### 8. HUD RefreshHUD: called by 16+ delegates, does full portrait resolve

**File:** `Source/T66/UI/T66GameplayHUDWidget.cpp` — `NativeConstruct` (lines 754–780), `RefreshHUD` (lines 1782–1850)

**Cause:** `NativeConstruct` binds `RefreshHUD` to InventoryChanged, PanelVisibilityChanged, StageChanged, DifficultyChanged, CowardiceGatesTakenChanged, IdolsChanged, HeroProgressChanged, UltimateChanged, SurvivalChanged, DevCheatsChanged, and OnSettingsChanged. Many of these fire together on a single game event (e.g. item pickup fires InventoryChanged + HeroProgressChanged).

Each `RefreshHUD` call does: `GetHeroData` (DataTable FindRow) → `ResolveHeroPortrait` (soft path resolution) → `BindSharedBrushAsync` (texture pool request) → 7× `FText::Format` + `SetText` for hero stats → portrait border color update. The portrait variant depends on hearts remaining (lines 1803–1812), but hearts change rarely.

**Solution:**

- **Coalesce:** Instead of binding `RefreshHUD` directly to all delegates, bind a lightweight `MarkHUDDirty()` that sets `bool bHUDDirty = true`. At the start of `NativeTick`, check `bHUDDirty`; if true, call `RefreshHUD` once and clear the flag. This collapses delegate storms into at most one RefreshHUD per frame.
- **Cache portrait state:** Add `FName LastPortraitHeroID`, `ET66HeroBodyType LastPortraitBodyType`, `ET66HeroPortraitVariant LastPortraitVariant` as members. In RefreshHUD, compute the current variant from hearts. Only call `GetHeroData` + `ResolveHeroPortrait` + `BindSharedBrushAsync` if any of the three changed. Portraits change extremely rarely during a run.

---

### 9. BossGroundAOE: pillar VFX continues during linger phase

**File:** `Source/T66/Gameplay/T66BossGroundAOE.cpp` — `Tick`, lines 100–133; `ActivateDamage`, lines 136–218

**Cause:** After `ActivateDamage` fires (damage is applied, a 30-particle burst is spawned at line 192), the actor continues ticking to spawn pillar VFX: 10 Niagara components every 0.05s (~200 spawns/sec). A `DestroyTimerHandle` is set to destroy the actor after `PillarLingerSeconds`, but until then, Tick keeps spawning. During a boss fight with multiple AOE pillars lingering simultaneously, this is hundreds of cosmetic-only spawns per second.

**Solution:**

After `ActivateDamage` spawns the impact burst, either:
- **Disable tick entirely** (`SetActorTickEnabled(false)`) and let the linger phase be visual-only via the impact burst particles fading out, or
- **Drastically reduce the pillar rate** during linger (e.g. 2 particles every 0.5s instead of 10 every 0.05s — a 50× reduction) since the pillar is just ambient glow at that point.

The first option is simplest and most robust.

---

### 10. FloatingCombatText: actor spawned per damage number

**File:** `Source/T66/Core/T66FloatingCombatTextSubsystem.cpp` — `ShowDamageNumber` (line 36), `ShowDamageTaken` (line 54)

**Cause:** Every damage event calls `World->SpawnActor<AT66FloatingCombatTextActor>()` and sets `SetLifeSpan` for auto-destroy. In high-damage scenarios (fast attack speed, multiple targets, AOE), this creates and destroys many actors per second. Each spawn involves UObject allocation, component registration, and world registration.

**Solution:**

Create a `UT66FloatingTextPoolSubsystem` following the exact pattern of the existing `UT66EnemyPoolSubsystem`:

- Pre-spawn a ring buffer of `AT66FloatingCombatTextActor` instances (e.g. 24–30), hidden and moved far away.
- `ShowDamageNumber` takes the next actor from the ring buffer, re-parents it, sets text/position/lifetime, and makes it visible.
- When the lifetime expires, the actor is hidden and returned to the pool (or the ring index simply advances, recycling the oldest).
- If all slots are in use, the oldest visible number is recycled. This naturally caps the on-screen count.

No SpawnActor/Destroy in the hot path. The pattern is already proven in the codebase via EnemyPoolSubsystem.

---

### 11. Ultimate abilities: TActorIterator over all enemies

**File:** `Source/T66/Gameplay/T66PlayerController.cpp` — `HandleUltimatePressed`, lines 718–825

**Cause:** All ultimate types (SpearStorm, ChainLightning, default) use `TActorIterator<AT66EnemyBase>` and `TActorIterator<AT66BossBase>` to find every enemy and boss in the world. `TActorIterator` scans the entire UWorld actor list, which is O(total actors) regardless of how many are enemies. The `UT66ActorRegistrySubsystem` already maintains `GetEnemies()` but does not yet track bosses.

**Solution:**

- Extend `UT66ActorRegistrySubsystem` with `RegisterBoss(AT66BossBase*)`, `UnregisterBoss(AT66BossBase*)`, and `const TArray<TWeakObjectPtr<AT66BossBase>>& GetBosses() const`, following the exact same pattern as the existing enemy and NPC registration.
- Have `AT66BossBase` register in `BeginPlay` and unregister in `EndPlay`.
- Replace all `TActorIterator<AT66EnemyBase>` and `TActorIterator<AT66BossBase>` in `HandleUltimatePressed` with `Registry->GetEnemies()` and `Registry->GetBosses()`.
- Apply the same replacement in `AT66GameMode::ApplyDifficultyToAllEnemies` (lines 786–797) which also uses TActorIterator for enemies and bosses.

---

## P2 — Medium (minor lag or scaling concern)

### 12. CompanionBase Tick: repeated subsystem and pawn lookups

**File:** `Source/T66/Gameplay/T66CompanionBase.cpp` — `Tick`, lines 162–244

**Cause:** Every frame, per companion, the Tick resolves `GetWorld()->GetGameInstance()`, `GetSubsystem<UT66HeroSpeedSubsystem>()`, `GetSubsystem<UT66AchievementsSubsystem>()`, `GetSubsystem<UT66RunStateSubsystem>()`, `World->GetFirstPlayerController()`, and `PC->GetPawn()`. Additionally, `GetCompanionUnionStagesCleared(CompanionID)` is called every frame even though union tier changes only when achievements are unlocked.

**Solution:**

- Cache all subsystem pointers and the hero pawn in `BeginPlay` as members (same pattern as `CachedRunState` in CombatComponent).
- Cache the union stages cleared value in `BeginPlay` and subscribe to `UT66AchievementsSubsystem::AchievementsUnlocked` to refresh it on change.
- For the hero pawn: use a `TWeakObjectPtr<APawn>` cache with null-check re-resolve (same pattern as `CachedPlayerPawn` in T66EnemyBase at line 461).

---

### 13. HouseNPCBase: face-player Tick at full frame rate after gravity settled

**File:** `Source/T66/Gameplay/T66HouseNPCBase.cpp` — `Tick`, lines 211–260

**Cause:** After `bGravitySettled = true` (gravity settle logic completes), the Tick continues running every frame if `bFacePlayerAlways` is true. It calls `GetFirstPlayerController()->GetPawn()` and does rotation math every frame per NPC. With many NPCs, this is many unnecessary Tick calls for a cosmetic behavior.

**Solution:**

After gravity is settled, throttle the face-player update to 5–10 Hz (every 0.1–0.2s). The player can't perceive NPC rotation updates faster than ~10 Hz. Use an accumulator:

```
FacePlayerAccum += DeltaSeconds;
if (FacePlayerAccum < 0.15f) return;
FacePlayerAccum = 0.f;
// ... existing face-player logic ...
```

Alternatively, disable Tick entirely after gravity settles and start a low-frequency timer for face-player.

---

### 14. BossGate and StartGate: Tick never disabled after trigger

**Files:**
- `Source/T66/Gameplay/T66BossGate.cpp` — `Tick`, lines 51–64
- `Source/T66/Gameplay/T66StartGate.cpp` — `Tick`, lines 51–67

**Cause:** Both gates set `bTriggered = true` in `TryTriggerForActor` but never call `SetActorTickEnabled(false)`. After triggering, Tick still runs every frame, calling `GetPlayerPawn` and then returning at the `if (bTriggered) return;` check.

**Solution:**

In `TryTriggerForActor`, after setting `bTriggered = true` and completing the trigger logic, call `SetActorTickEnabled(false)`. This is a one-line fix per gate.

---

### 15. EnemyDirector: O(n²) PendingSpawns.RemoveAt(0)

**File:** `Source/T66/Gameplay/T66EnemyDirector.cpp` — `SpawnNextStaggeredBatch`, line 373

**Cause:** `PendingSpawns.RemoveAt(0)` shifts all remaining elements left on every iteration. Over a full wave of N enemies, this is O(n²) total element moves.

**Solution:**

Use index-based processing to preserve spawn order without shifting:

```
int32 Processed = 0;
while (Processed < PendingSpawns.Num() && Processed < BatchSize)
{
    FPendingEnemySpawn& Slot = PendingSpawns[Processed];
    // ... existing spawn logic ...
    ++Processed;
}
PendingSpawns.RemoveAt(0, Processed, EAllowShrinking::No);
```

Or maintain a `PendingSpawnHeadIndex` member (advancing it each batch, compacting when the batch is fully consumed), similar to the pattern in `T66DamageLogSubsystem::TrimRecentDamageSamples`.

---

### 16. HeroSelectionScreen: TAttribute lambdas with DataTable lookups

**File:** `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` — `BuildSlateUI`, lines 574–606

**Cause:** The hero carousel's 5 slots use `TAttribute` lambdas for `BorderBackgroundColor`, `Image`, and `Visibility`. The `BorderBackgroundColor` lambda calls `UGameplayStatics::GetGameInstance` → `Cast<UT66GameInstance>` → `GetHeroData` (FindRow) per slot. Slate evaluates these attributes during layout and painting — potentially multiple times per frame across 5 slots.

**Solution:**

Cache carousel display state (border colors, brush pointers, visibility) in member variables. Update them when the carousel index or body type changes (in `HandlePrevClicked`, `HandleNextClicked`, `HandleBodyTypeAClicked`, `HandleBodyTypeBClicked`). Bind TAttributes to read from the cached members. No GetHeroData in lambdas.

---

### 17. HeroSelectionScreen: synchronous LoadObject on hero switch

**File:** `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` — `UpdateHeroPreviewVideo`, line 1635

**Cause:** `LoadObject<UFileMediaSource>(nullptr, TEXT("/Game/Characters/Heroes/Knight/KnightClip.KnightClip"))` is a synchronous load that can hitch when switching to the Knight hero if the asset is not already in memory.

**Solution:**

Use `TSoftObjectPtr<UFileMediaSource>` and either preload the asset when the selection screen opens, or load it asynchronously and set the media source in the callback. Show a placeholder or no video until loaded.

---

### 18. VendorOverlay and GamblerOverlay: full stats panel rebuild on inventory change

**Files:**
- `Source/T66/UI/T66VendorOverlayWidget.cpp` — `RefreshStatsPanel`, lines 954–964
- `Source/T66/UI/T66GamblerOverlayWidget.cpp` — `RefreshStatsPanel`, lines 2528–2539

**Cause:** `StatsPanelBox->SetContent(T66StatsPanelSlate::MakeEssentialStatsPanel(...))` tears down all existing Slate widgets in the stats panel and recreates them from scratch (new SVerticalBox, SBorder, STextBlock for each stat line). This runs on every inventory change, which happens on every buy/sell.

**Solution:**

Build the stats panel once (in the overlay's initial construction). Store `TSharedPtr<STextBlock>` references for each stat value. On inventory change, call a lightweight `UpdateStatsValues(RunState)` that only calls `SetText` on the existing text blocks. Only rebuild the panel if the number of stat lines changes (which doesn't happen during normal gameplay).

---

### 19. VendorOverlay: IsBossActive() in TAttribute lambdas

**File:** `Source/T66/UI/T66VendorOverlayWidget.cpp` — lines 490, 501, 624, 645

**Cause:** `SetEnabled(TAttribute<bool>::CreateLambda([this]() { return !IsBossActive(); }))` binds to multiple buttons. `IsBossActive()` resolves `GetWorld()` → `GetGameInstance()` → `GetSubsystem<UT66RunStateSubsystem>()` → `GetBossActive()`. Slate evaluates enabled-state attributes during layout, hit-testing, and painting — multiple times per frame.

**Solution:**

Cache `bool bCachedBossActive` as a member. Subscribe to the RunState's `BossChanged` delegate to update it. Bind `SetEnabled` to a lambda that returns the cached bool.

---

### 20. CharacterVisualSubsystem: multiple synchronous loads on first resolve

**File:** `Source/T66/Core/T66CharacterVisualSubsystem.cpp` — `ResolveVisual`, lines 141–251

**Cause:** When resolving a VisualID for the first time, the function calls `LoadSynchronous` for SkeletalMesh, LoopingAnimation, AlertAnimation, and RunAnimation. Each failed animation load triggers up to 2 additional `LoadSynchronous` fallback attempts. The result is cached per VisualID (`ResolvedCache` at line 250), so subsequent resolves are free. But the first resolve of each unique character type can stall the game thread for 10–50ms+.

Additionally, `FindFallbackLoopingAnim` (lines 62–113) does an AssetRegistry scan of all anim assets under `/Game/Characters` on first call per skeleton.

**Solution:**

Preload during the loading screen or run-start transition. At run start (or when the gameplay level begins loading), iterate the hero and companion DataTables, collect the VisualIDs that will be needed, and call `ResolveVisual` for each during the loading phase. Since results are cached, all subsequent runtime calls are instant. The AssetRegistry scan in `FindFallbackLoopingAnim` is also covered because it caches per skeleton.

---

### 21. GameMode Tick: TActorIterator in ApplyDifficultyToAllEnemies

**File:** `Source/T66/Gameplay/T66GameMode.cpp` — `ApplyDifficultyToAllEnemies`, lines 786–797

**Cause:** Uses `TActorIterator<AT66EnemyBase>` and `TActorIterator<AT66BossBase>` to iterate all enemies and bosses when difficulty changes. Fires when the player picks a difficulty totem.

**Solution:**

Same as issue #11: use `ActorRegistrySubsystem->GetEnemies()` and `GetBosses()` (after extending the registry with boss registration).

---

### 22. LoanShark Tick: TActorIterator for NPC safe-zone cache

**File:** `Source/T66/Gameplay/T66LoanShark.cpp` — `Tick`, lines 84–132

**Cause:** Uses `TActorIterator<AT66HouseNPCBase>` every 2 seconds to refresh a static cache of NPCs for safe-zone avoidance. The ActorRegistrySubsystem already maintains an NPC list via `GetNPCs()`.

**Solution:**

Replace the `FSafeNPCache` static struct and its `TActorIterator` refresh with a direct call to `Registry->GetNPCs()`. The registry is always up to date (NPCs register on BeginPlay, unregister on EndPlay). This eliminates the iterator and the 2-second cache refresh entirely.

---

## P3 — Low (minor waste or edge-case concern)

### 23. HeroBase Tick: subsystem lookups every frame

**File:** `Source/T66/Gameplay/T66HeroBase.cpp` — `Tick`, lines 419–440

**Cause:** `GetGameInstance()->GetSubsystem<UT66HeroSpeedSubsystem>()` and `GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()` are resolved every frame. Single hero, so the absolute cost is low, but it's unnecessary.

**Solution:**

Cache both subsystem pointers in `BeginPlay`.

---

### 24. EclipseActor: OrientTowardCamera every frame

**File:** `Source/T66/Gameplay/T66EclipseActor.cpp` — `Tick`, lines 89–106

**Cause:** `OrientTowardCamera()` calls `UGameplayStatics::GetPlayerCameraManager`, gets camera location, computes direction, and calls `SetActorRotation` every frame. This is a billboard effect for the eclipse visual.

**Solution:**

Low cost for a single actor. If desired, throttle to 15–30 Hz or only update when the camera has moved beyond a threshold. Not a priority.

---

### 25. GamblerOverlay: Plinko timer causes frequent layout invalidation

**File:** `Source/T66/UI/T66GamblerOverlayWidget.cpp` — `TickPlinkoDrop` at 0.12s interval; `TickLotteryRevealNext` at 0.6s

**Cause:** Plinko timer calls `PlinkoBoardContainer->Invalidate(EInvalidateWidget::Layout)` ~8 times per second during the minigame. Lottery timer invalidates the casino switcher.

**Solution:**

Invalidate only the specific widget that moved (the ball or the revealed number), not the entire container. If that's not feasible in the current layout, this is acceptable cost since these minigames are short-duration overlays with no gameplay simulation running underneath.

---

### 26. CrateOverlay: TickScroll at 60 Hz

**File:** `Source/T66/UI/T66CrateOverlayWidget.cpp` — `TickScroll` timer at 0.016f

**Cause:** The crate scroll animation timer runs at ~60 Hz, calling `StripContainer->SetPadding(Margin)` each tick (layout invalidation).

**Solution:**

Reduce to 30 Hz (0.033f). The visual difference is imperceptible for a scrolling crate strip and halves the Slate invalidation rate. Since this is a short overlay, the impact is minor either way.

---

### 27. UpdateWorldDialoguePosition: 20 Hz timer

**File:** `Source/T66/Gameplay/T66PlayerController.cpp` — lines 1444, 1472, 1499

**Cause:** `SetTimer(..., 0.05f, true)` for `UpdateWorldDialoguePosition` runs at 20 Hz while a world dialogue is open. This does world-to-screen projection to keep a dialogue widget positioned above an NPC.

**Solution:**

Reduce to 10 Hz (0.1f). World dialogues are stationary (NPC doesn't move, camera moves slowly). 10 Hz positional updates are sufficient. Clear the timer when the dialogue closes.

---

### 28. PropSubsystem: TActorIterator for NPC safe-zone check during prop placement

**File:** `Source/T66/Core/T66PropSubsystem.cpp` — line 108

**Cause:** Uses `TActorIterator<AT66HouseNPCBase>` to check safe zones during prop placement.

**Solution:**

Use `ActorRegistrySubsystem->GetNPCs()`.

---

### 29. GameMode: numerous TActorIterator calls for level setup

**File:** `Source/T66/Gameplay/T66GameMode.cpp` — lines 655, 844, 1501, 1610, 1728, 2261, 2268, 2355, 2382, 2421, 2471, 2496, 2512, 2580, 2660, 2687, 2792, 3136, 3811–4497

**Cause:** Many `TActorIterator` calls for level setup, lighting, terrain, and theme application. These run during level transitions or one-time setup, not per-frame.

**Solution:**

Most of these are acceptable because they run during level load or one-time configuration. The ones in `RefreshMapData` and `ApplyDifficultyToAllEnemies` are the exceptions (covered in issues #11 and #21). No action needed for the rest unless level transition hitches become a concern.

---

## Infrastructure summary

The solutions above converge on a small set of reusable patterns:

| Pattern | Existing example in codebase | New application |
|---------|------------------------------|-----------------|
| **Object pool subsystem** | `UT66EnemyPoolSubsystem` | Floating combat text pool, VFX component pool |
| **Actor registry** | `UT66ActorRegistrySubsystem` (enemies, NPCs, gates, boundaries) | Add boss registration; replace remaining TActorIterator in gameplay code |
| **Cached subsystem pointers** | `CachedRunState` in `UT66CombatComponent` | CompanionBase, HeroBase, ApplyDamageToActor, any actor that uses subsystems in Tick |
| **Throttled query timer** | `SafeZoneCheckAccumSeconds` in `AT66EnemyBase` | HeroBase ground trace, HeroBase enemy touch, NPC face-player |
| **Compare-before-set for Slate** | `LastRemainingCs` in `RefreshCooldownBar` (but uses static — change to member) | DPS text, cooldown bar width, speedrun timers, any future HUD stat |
| **Coalesce-to-tick** | (new) | RefreshHUD dirty flag; delegates set flag, one pass per frame applies |
| **Cached run-constant data** | `RecomputeFromRunState` in `UT66CombatComponent` | Hero data, idol data for TryFire |
| **VFX budget subsystem** | (new) | All pixel VFX spawning routed through central subsystem with global cap |
