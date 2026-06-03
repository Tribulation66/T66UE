You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_BossProjectileManager\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Pass 2 Plan Packet - Boss Projectiles Into Projectile Manager

Date: 2026-05-28

## Working Goal

Migrate boss projectile firing from `AT66BossProjectile` actor spawning into `UT66ProjectileManagerSubsystem` with HISM rendering, while keeping `AT66BossBase` as a rich actor and preserving current boss firing patterns, visual language, collision behavior, and damage attribution.

## User Constraints And Locked Decisions

- Boss actors stay rich `AT66BossBase`; only their projectiles move to the manager.
- Preserve boss firing patterns. Pattern logic remains in `AT66BossBase`; the manager owns only projectile state, collision, damage, and HISM rendering.
- Preserve any special boss projectile behavior discovered in the live audit.
- Stage 17 Four Horsemen is a MultiBoss encounter, so manager capacity must account for multiple bosses firing simultaneously.
- Unique Debuff, Goblin Thief, Gambler-specific systems, hero projectiles, enemywaveperf acceptance finalization, B.11+, and deleting old projectile files are out of scope.
- The previous placed-miniboss pass is done. Pass 2 smoke should naturally runtime-confirm floors 2, 3, and 4 placed minibosses while reaching the boss.

## Applicable Instructions

- Root `AGENTS.md`: goal discipline, live-repo-first inspection, Claude review before implementation, staged standalone verification for playable runtime changes, one combined packet after pass, avoid broad Git/LFS scans.
- `Gameplay/GAMEPLAY_AGENTS.md`: gameplay runtime ownership, read `Gameplay/README.md`, prefer data-authored tuning, compile/build verification plus staged standalone validation for runtime-facing gameplay changes.
- `Gameplay/Combat/MASTER_COMBAT.md`: update after material projectile/boss/damage changes; current combat projectile semantics and debug roles must stay coherent.
- `Gameplay/Combat/pending_issues_Combat.md`: no directly blocking issue, but keep combat pending notes separate from gameplay pending notes.
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`: relevant because the audit/gap docs live under `PerformanceSystem`; do not add unrelated optimizer fixes.
- `Reports/AGENTS.md`: review packets and completion packet go under `Reports/AgentReviews`.

PPF: skipped. This pass is gameplay infrastructure migration, not a visual/media/VFX authoring task. The boss projectile visuals are runtime behavior to preserve from existing code, not a new solved-category VFX artifact.

## Live Audit Findings

### Current Boss Projectile Actor Behavior

Source: `Source/T66/Gameplay/T66BossProjectile.h/.cpp`

- Actor class: `AT66BossProjectile`, `AActor`.
- Components:
  - `USphereComponent CollisionSphere`
  - `UStaticMeshComponent VisualMesh`
  - `UProjectileMovementComponent ProjectileMovement`
  - transient Niagara trail component and cached trail/impact systems.
- Movement:
  - `ProjectileMovement->InitialSpeed/MaxSpeed` set by `SetTargetLocation`.
  - Gravity scale is `0`.
  - `bRotationFollowsVelocity=true`.
  - `InitialLifeSpan=6.0f`.
- Collision:
  - Collision sphere radius defaults to `24.f`.
  - Query-only overlap, ignores all channels except `ECC_Pawn`, `ECC_WorldStatic`, and `ECC_WorldDynamic`.
  - Ignores owner.
  - Hero damage only applies when overlap actor is `AT66HeroBase` and overlapped component passes `T66CombatShared::IsHeroHurtboxComponent`.
  - Non-hero overlaps spawn impact VFX and destroy the projectile.
  - Non-hero hurtbox rejection on the hero is logged and projectile continues.
- Damage:
  - `DamageHP = max(1, DamageHearts) * 20`.
  - Applies through `RunState->ApplyDamage(DamageHP, GetOwner(), "BossProjectile", this)`.
  - Source attribution resolves to `BossID` because `UT66RunStateSubsystem_Combat.cpp` resolves `AT66BossBase` attackers to `Boss->BossID`.
- Presentation:
  - Mesh shape by `ET66BossAttackProfile`:
    - Sharpshooter and Duelist: cone
    - Juggernaut: cylinder
    - Balanced and Gambler: sphere
  - Scale by profile:
    - Sharpshooter: `(0.22, 0.22, 0.60)`
    - Juggernaut: `(0.28, 0.28, 0.45)`
    - Duelist: `(0.18, 0.18, 0.56)`
    - Gambler: `(0.26)`
    - Balanced: `(0.22)`
  - `VisualScaleMultiplier` clamps to `[0.35, 5.0]` and also scales collision radius as `24 * max(1, VisualScaleMultiplier)`.
  - Tint comes from boss `AttackPrimaryColor` / `AttackSecondaryColor`.
  - Trail and impact Niagara systems vary by attack profile and use a per-frame VFX budget controlled by `T66.VFX.BossProjectileMaxPerFrame` and `T66.VFX.BossProjectileUseEffectsScalability`.

### Current Boss Firing Patterns

Source: `Source/T66/Gameplay/T66BossBase.cpp`

- `AT66BossBase::Awaken()` starts repeating `FireAtPlayer()` at `FireIntervalSeconds`.
- Data inputs from `Bosses.csv` include `FireIntervalSeconds`, `ProjectileSpeed`, `ProjectileDamageHearts`, and `BossPartProfile`.
- Common helper flow:
  - `QueueProjectileShotTowards`
  - `QueueProjectileShotDirection`
  - `QueueProjectileFanBurst`
  - `QueueRadialBurst`
  - `SpawnProjectileInDirection`
  - `SpawnScaledProjectileInDirection`
- Generic attack profiles:
  - `Sharpshooter`: 3/5-shot fan plus aimed shot; late phase adds two offset shots.
  - `Juggernaut`: 5/7-shot fan plus 6/8/10-shot radial burst.
  - `Duelist`: paired side shots; later phases add fan and radial burst.
  - `Gambler`: radial burst plus fan; late phase adds aimed shot.
  - `Balanced`: 3/5/6-shot fan; later phases add aimed shots and fan.
- `Dungeon_SewerSlimeKing` special path:
  - Picks live boss part (`LeftLobe`, `RightLobe`, `LeftBase`, `RightBase`, `MouthCore`).
  - Lobe volley emits five delayed scaled projectiles.
  - Mouth projectile emits one larger scaled projectile from `MouthCore`.
  - Base parts spawn lane blocker hazards, not projectile manager projectiles.
  - Telegraph and lane blocker actors are not in scope for migration.
- Patterns are already timer/schedule based and should remain in the boss actor. The manager call should sit where `SpawnActor<AT66BossProjectile>` sits today.

### Current Projectile Manager Behavior And Gaps

Source: `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h/.cpp`

- Current capacity: `MaxProjectiles = 256`.
- Current type support is effectively single-type:
  - `EnemySpitProjectileTypeIndex = 0`.
  - `GetProjectileComponent()` ignores the requested `ProjectileTypeIndex` and always resolves to type `0`.
  - `FireProjectile()` stores `Projectile.ProjectileTypeIndex = EnemySpitProjectileTypeIndex` regardless of input.
- Current visual support:
  - one HISM sphere type with flat hostile projectile color.
  - transform scale comes from `FT66TemporaryProjectileSystem::ProfileEnemySpit()` and does not account for boss profile, secondary tint, or visual scale multiplier.
- Current damage support:
  - hits apply `RunState->ApplyDamage(Projectile.Damage, SourceMob, "EnemyProjectile", SourceMob)`.
  - this is correct for basic mob projectiles but not for boss delivery naming or boss damage-causer semantics.
- Current collision support:
  - hero capsule segment sweep by manager.
  - non-hero sphere sweep against pawn/world static/world dynamic.
  - peer `AT66EnemyBase` / `AT66MobBase` bodies are ignored.
  - boss actors are not currently ignored as projectile peers.

### Autocapture And Boss-Flow Seams

Source: `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`, `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp`, `Source/T66/Gameplay/T66BossGate.cpp`

- `T66GameplayAutoCapture=<mode>` is parsed in `AT66PlayerController::StartGameplayAutomationCapture()` and dispatched through `ApplyGameplayAutomationCaptureMode()`. There is no external registry; adding `bossprojectilemanager` means adding one more explicit mode branch in that function.
- Production boss spawning is owned by `AT66GameMode::SpawnBossForCurrentStage()`.
  - It reads `RunState->GetCurrentStage()`.
  - It loads `FStageData` and `BossEncounterID`.
  - It resolves `BossEncounterMembers`.
  - It spawns encounter bosses through the same code path used by Stage 17 Four Horsemen.
- Production boss awakening is owned by `AT66BossGate::TryTriggerForActor()`, which calls `Boss->ForceAwaken()` for registered bosses and pauses enemy director spawning.
- Smoke automation must drive these production seams. If the implementation cannot drive the existing stage/boss-gate path without fake setup, it must stop and revise rather than validating a non-production boss setup.

### Capacity Audit

- Current max in the manager is 256 active projectiles.
- Worst current simultaneous boss scheduling from the inspected patterns:
  - Single generic late-phase `Juggernaut`: fan 7 + radial 10 = 17 queued shots in one fire cycle.
  - Four Horsemen worst case if all four are Juggernaut-equivalent simultaneously: `17 * 4 = 68` queued shots over short stagger windows.
  - With 6-second projectile lifetime and Stage 17 fire intervals around `1.28-1.34s`, a pessimistic concurrent count can exceed 256 if all bosses retain peak burst cadence and projectiles do not hit/expire quickly: roughly `68 * ceil(6 / 1.28) = 340`.
- Plan: raise manager capacity to `512` for this pass. This gives roughly 50% headroom above the 340 pessimistic sustained estimate while keeping the flat array small enough for linear scan in this pass.
- Capacity overflow behavior is deterministic: drop the newly requested projectile, increment `DroppedFires`, and do not evict existing projectiles. Boss smoke fails if `DroppedFires > 0`, because a dropped boss fan/radial shot is a visible pattern gap.

### Boss Projectile Visual Type Decision

- Bosses do not all share one visual. The current actor changes mesh shape, scale, tint, trail, and impact by `ET66BossAttackProfile`, with optional secondary tint and visual scale multiplier.
- Decision: use exact visual-key HISM component buckets, not per-instance custom data and not a new material.
  - Type `0`: existing enemy spit bucket.
  - Boss buckets are keyed by:
    - mesh/profile (`Balanced`, `Sharpshooter`, `Juggernaut`, `Duelist`, `Gambler`)
    - effective tint color (primary or secondary, quantized to a stable byte color key)
  - Each bucket owns one HISM component and one dynamic flat-color material, so two same-profile bosses with different colors do not collapse to one stale color.
  - `VisualScaleMultiplier` stays per projectile via instance transform scale and per-projectile collision radius, not as a component bucket dimension.
- Bucket cardinality is bounded:
  - max exact boss visual buckets: `32`.
  - plus one pre-created overflow bucket per boss profile mesh (`5`) and the existing enemy-spit bucket.
  - no eviction during play; buckets live for the world subsystem lifetime and are released during subsystem deinitialization.
  - if exact bucket creation would exceed the cap or component creation fails, use the matching profile overflow bucket with a documented fallback tint, emit one warning per overflow reason, and continue firing. Overflow use is a visual fallback, not a projectile drop.
- This avoids an art/material artifact. `FT66VisualUtil::GetFlatColorMaterial()` stays the base material; no per-instance custom data material is introduced this pass.
- Stage 17 validation must confirm all four Horsemen have manager-fired projectiles in flight and at least two distinct colors are visible simultaneously. If the exact buckets overflow during this smoke, that is a failure to investigate, not an accepted final state.

### Trail And Impact VFX Decision

- Decision: preserve boss projectile fire audio and boss projectile trail/impact VFX through manager-owned Niagara components/effects.
- The existing budget CVars move with the behavior:
  - `T66.VFX.BossProjectileMaxPerFrame`
  - `T66.VFX.BossProjectileUseEffectsScalability`
- Manager behavior:
  - On boss projectile activation, if the VFX budget allows, spawn the same profile-specific trail Niagara as a manager-owned component, store it on the projectile record, and update its transform with the projectile.
  - On boss projectile deactivation by hero/world impact, spawn the same profile-specific impact Niagara if budget allows.
  - On every deactivation path (hero hit, world hit, lifetime expiry, capacity/drop cleanup, source invalidation, reset, and subsystem deinitialization), detach and destroy any trail component before the slot can be reused.
  - Slot activation starts from a fully reset record; a reused projectile slot must never inherit a previous owner's trail component, profile, tint, source pointer, or collision radius.
- Boss fire audio stays on `AT66BossBase` in the existing firing helpers. The manager does not own boss fire audio; it owns projectile body, collision, trail, impact, and lifetime.
- HISM remains the required visible projectile body. Niagara trails/impacts preserve the old visual language; they are not the primary collision/rendering owner.
- If implementation discovers a hard engine limitation with manager-owned trail components, stop and revise the packet rather than silently shipping trail-less parity.

### Source Ownership And Invalid-Source Rule

- Managed projectiles must not hold raw boss pointers for later damage.
- Store sources as `TWeakObjectPtr<AActor>` plus cached `SourceMobID` / `BossID`.
- On hit:
  - if the source actor is still valid, apply damage through the existing path using that actor as both attacker and damage causer for boss projectiles: `RunState->ApplyDamage(DamageHP, BossActor, "BossProjectile", BossActor)`.
  - if a boss source is invalid before impact, drop/deactivate the projectile, increment a diagnostic counter such as `DroppedInvalidSource`, and do not apply damage. This avoids stale-pointer crashes and avoids fabricating damage attribution through a manager pseudo-source.
- The smoke must include a boss-death-during-flight check proving in-flight managed boss projectiles do not crash and do not misattribute damage after their boss owner is invalid.

### Movement And Collision Parity Checklist

- Movement parity from `AT66BossProjectile`:
  - velocity is straight-line `Direction * ProjectileSpeed`.
  - gravity remains zero.
  - lifetime remains `6.0s`.
  - HISM instance rotation is updated from velocity each tick to match old `bRotationFollowsVelocity=true`.
  - visual scale follows the existing profile scale multiplied by `VisualScaleMultiplier`.
  - collision radius is `24.f * max(1.f, VisualScaleMultiplier)`.
- Collision parity and intentional change:
  - hero hurtbox damage remains gated to the hero/hurtbox path.
  - world static/dynamic impacts still deactivate the projectile and play impact VFX.
  - peer enemies, lightweight mobs, and boss actors are ignored so boss projectiles do not disappear on other enemy bodies. This is an intentional alignment with basic enemy projectile manager behavior, and smoke must verify there is no obvious unintended hero punch-through in a crowded boss/mob scene.

## Implementation Plan

### Task 1 - Extend Managed Projectile Data

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h`
- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp`

Add enough per-projectile state for boss semantics:

- delivery method (`EnemyProjectile` vs `BossProjectile`)
- whether the source is a boss
- weak source actor pointer (`TWeakObjectPtr<AActor>`) and cached source ID; no raw boss pointer may be retained across frames
- attack profile / visual profile
- primary/secondary color and secondary-tint flag, if needed for component/material selection
- visual scale multiplier
- collision radius derived from old boss projectile behavior
- lifetime as per-projectile state (`6.0` for boss projectiles in this pass), not a manager-wide constant
- optional manager-owned trail component pointer for boss projectiles
- impact VFX payload for boss projectiles
- invalid-source drop accounting for boss projectiles whose owner dies or is torn down mid-flight

Confirmed existing manager support:

- `FT66ManagedProjectile` already has `Radius`.
- `FindHeroHitFraction(...)` and `FindNonHeroImpact(...)` consume `Projectile.Radius`.
- The boss work must set that radius correctly (`24 * max(1, VisualScaleMultiplier)`), not add a second unused radius field.

Do not move boss pattern scheduling into the manager.

### Task 2 - Implement Real Projectile Type Resolution

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h/.cpp`

Change `GetProjectileComponent()` and `FireProjectile()` so `ProjectileTypeIndex` is honored.

Expected minimum:

- type `0`: existing enemy spit behavior unchanged.
- boss visual-key buckets: HISM mesh/material match current profile shape and effective tint color, bounded by the 32 exact bucket + 5 overflow bucket rule.
- manager bounds summary works across all projectile components, not only type `0`.
- `ProjectileManagerSummary` reports active peak and dropped fires across all types.
- HISM update uses velocity-derived rotation each tick for boss projectiles, matching old projectile-movement orientation.

### Task 3 - Add Boss Fire API

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h/.cpp`
- `Source/T66/Gameplay/T66BossBase.cpp`

Decision:

- Add a small `FT66ManagedProjectileFireParams` struct and route both existing enemy-spit calls and new boss calls through one allocation path.
- Add a boss-specific wrapper such as `FireBossProjectile(const FT66ManagedProjectileFireParams& Params)` only if it keeps the call sites clearer; the underlying source of truth is the struct.

The API must pass:

- source actor = boss
- source ID = `BossID`
- origin/direction/speed
- damage HP = `ProjectileDamageHearts * 20`
- radius = `24 * max(1, VisualScaleMultiplier)`
- lifetime = `6.0`
- delivery = `BossProjectile`
- visual profile and tint data.
- weak boss actor pointer and cached `BossID`. On hit, use the live boss actor as the `Attacker`/damage causer only if the weak pointer is still valid. Do not substitute a manager pseudo-source for the damage source.

### Task 4 - Replace Boss SpawnActor Calls

Files:

- `Source/T66/Gameplay/T66BossBase.cpp`
- `Source/T66/Gameplay/T66BossBase.h` if helper signatures need adjustment.

Replace the two `SpawnActor<AT66BossProjectile>` call sites in:

- `SpawnProjectileInDirection`
- `SpawnScaledProjectileInDirection`

The existing helpers still compute:

- awaken/status gates
- shot direction
- spawn location
- speed scaling
- visual scale multiplier
- secondary tint
- audio fire event.
- per-shot delayed scheduling for generic fan/radial bursts and Sewer Slime King lobe/mouth projectiles.

The only behavior that changes is projectile storage/rendering/damage moving to the manager.

Call-site mapping:

- `SpawnProjectileInDirection`: one manager boss projectile fire with profile scale and primary tint.
- `SpawnScaledProjectileInDirection`: one manager boss projectile fire with caller-provided `VisualScaleMultiplier` and primary/secondary tint flag.
- `QueueProjectileFanBurst`, `QueueRadialBurst`, `QueueProjectileShotTowards`, and `QueueProjectileShotDirection`: unchanged scheduling; they continue to reach the two spawn helper replacements.
- `QueueSewerSlimeKingLobeVolley` and `SpawnSewerSlimeKingMouthProjectile`: lobe/mouth projectiles route through the manager via the scaled helper.
- Sewer Slime King base lane blockers/telegraphs: untouched and explicitly out of scope because they are not `AT66BossProjectile` spawns.

### Task 5 - Preserve Collision And Damage Semantics

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp`

Required behavior:

- Hero collision uses the existing manager capsule segment hit test.
- Boss projectile hit applies `RunState->ApplyDamage(DamageHP, BossActor, "BossProjectile", BossActor)` only while the weak source actor is valid.
- Boss projectile with invalid source deactivates without applying damage and increments invalid-source drop accounting.
- Boss source resolves to `BossID` in `[CombatDamage]` logs.
- Peer enemies, lightweight mobs, and boss actors should not destroy boss projectiles.
- World static/dynamic impacts should deactivate boss projectiles, matching the old non-hero impact/destroy semantics.
- Rejected non-hero hero hurtbox should not apply damage.
- This is an intentional collision behavior change relative to the old actor path: boss shots will no longer self-destruct on basic mob or boss peer bodies. It matches the basic enemy projectile peer-filter rule and must be documented in the completion packet.
- Add a crowded-scene smoke check: projectiles should visibly pass through enemy/boss peers without prematurely disappearing, but should still only damage the hero when the hero capsule/hurtbox path is hit.

### Task 6 - Capacity Increase

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h`

Raise `MaxProjectiles` from `256` to `512`.

Document rationale:

- current single-boss bursts are at most ~17 queued shots per firing cycle.
- Four Horsemen can plausibly queue ~68 shots per combined cycle.
- six-second lifetime plus ~1.3-second fire intervals needs >256 headroom in worst case.

Smoke must quote `ProjectileManagerSummary ActivePeak` and `DroppedFires` for single-boss and production-path Stage 17 Four Horsemen validation.

Overflow behavior:

- drop the newly requested fire;
- increment `DroppedFires`;
- never evict an already active projectile;
- fail smoke if any boss projectile is dropped.

### Task 7 - Deprecate `AT66BossProjectile`

Files:

- `Source/T66/Gameplay/T66BossProjectile.h`

Add a top-of-class deprecation comment:

```cpp
// DEPRECATED: Boss projectiles are now owned by UT66ProjectileManagerSubsystem.
// Kept temporarily for asset/reference compatibility until cleanup.
```

Do not delete `.h/.cpp` files this pass.

Also add compile-visible deprecation where it is Unreal-safe:

- Prefer `UCLASS(..., Deprecated)` on `AT66BossProjectile`.
- Add `UE_DEPRECATED(5.7, "Boss projectiles are managed by UT66ProjectileManagerSubsystem.")` to C++ constructor or helper declarations if compatible with UHT/compile.
- If class-level or constructor-level deprecation causes UHT/compiler incompatibility, keep the comment and document the attempted stronger deprecation in the completion packet rather than breaking the build.

### Task 8 - Add Non-Shipping Boss Projectile Smoke Automation

Files:

- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`

Add a narrow non-shipping automation mode rather than relying on a brittle manual playthrough:

- `T66GameplayAutoCapture=bossprojectilemanager`
- Normal path:
  - enters tower gameplay,
  - steps the existing floor-transition flow through floors 2, 3, and 4,
  - confirms each placed Slime guardian blocks/unblocks its `AT66TowerDescentHole`,
  - kills the placed guardian through the existing damage path,
  - enters the boss floor,
  - awakens the stage boss through the existing boss-gate path, or through a `#if !UE_BUILD_SHIPPING` helper that delegates to `AT66BossGate::TryTriggerForActor()` rather than bypassing production `Awaken()` semantics,
  - waits long enough to observe managed projectiles.
- MultiBoss path:
  - command option such as `T66BossProjectileSmoke=FourHorsemen`,
  - sets up a validation-only run state for Stage 17 and calls the existing production `AT66GameMode::SpawnBossForCurrentStage()` path so `BossEncounterID` and `BossEncounterMembers` drive the Four Horsemen spawn,
  - creates or uses the existing boss gate and triggers production awakening through the same gate/delegate path used in normal play,
  - waits long enough to collect `ProjectileManagerSummary`,
  - exits cleanly.

Do not manually spawn the four boss IDs outside `SpawnBossForCurrentStage()` unless review is rerun. The point of this smoke is production-path MultiBoss validation, not a synthetic projectile stress test.

This automation is validation-only, `#if !UE_BUILD_SHIPPING`, and must not alter production gameplay rules.

### Task 9 - Documentation Updates

Files:

- `Gameplay/Combat/MASTER_COMBAT.md`
- `PerformanceSystem/Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md` if the previous route/acceptance issue needs an update.
- `Gameplay/Combat/pending_issues_Combat.md` for any newly discovered combat-specific out-of-scope issue.
- Final combined packet under `Reports/AgentReviews/20260528_BossProjectileManager/`.

Required doc changes:

- state that basic enemy projectiles and boss projectiles now flow through `UT66ProjectileManagerSubsystem`.
- keep Unique Debuff projectiles explicitly out of scope and still actor-based/lab-only.
- document boss pattern ownership: boss actor schedules patterns, manager owns projectile instances.
- document capacity `512` and Four Horsemen rationale.
- mark `AT66BossProjectile` deprecated, not deleted.
- `Gameplay/Combat/MASTER_COMBAT.md`: combat-facing projectile semantics, delivery method, boss pattern ownership, damage attribution, and deprecated boss projectile actor note.
- `PerformanceSystem/Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`: close the boss half of the "Boss and Unique Debuff Projectiles Bypass the Projectile Manager" gap while leaving Unique Debuff open/out of scope.
- `Source/T66/Gameplay/pending_issues_Gameplay.md`: only update gameplay/migration issues tied to the placed-miniboss and boss projectile manager pass status.
- `Gameplay/Combat/pending_issues_Combat.md`: only add or update combat-specific deferred issues discovered during implementation, such as remaining Unique Debuff projectile actor path, if not already tracked.

## Verification Plan

### Pre-Implementation / Hygiene

- Avoid broad Git/LFS scans.
- Use narrow path diffs only.
- Check for existing `T66.exe`, `RunUAT`, `UnrealEditor-Cmd`, and obvious build/capture leftovers before staged validation.

### Build

Run focused Development build:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex
```

Then refresh staged standalone:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1
```

Verify:

- staged exe exists at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- taskbar/root shortcut target still points to staged exe.

### Smoke

Primary staged smoke:

- Use `T66GameplayAutoCapture=bossprojectilemanager` to automate a normal tower stage through:
  - floor 2 placed miniboss
  - floor 3 placed miniboss
  - floor 4 placed miniboss guarding boss-floor entrance
  - floor 5 boss.
- Confirm boss fires managed projectiles:
  - visible HISM projectiles in flight
  - hero damage log has `SourceID=<BossID>` and `Delivery=BossProjectile`
  - `ProjectileManagerSummary` shows fired/hit counts and acceptable active peak/drop counts.
- Confirm boss projectile trail/impact VFX budget path was exercised, or stop if the trail/impact migration cannot be preserved.
- Confirm HISM projectile bodies rotate along velocity direction for cone/cylinder profiles rather than remaining static/world-aligned.
- Confirm fire audio still comes from the boss actor firing helper.
- Confirm peer filtering in a crowded scene: shots do not vanish on mob/boss bodies, but hero damage still requires the hero hit path.
- Induce boss death mid-pattern and confirm in-flight projectiles deactivate/drop safely without crashing or applying damage with a stale owner.
- Save:
  - screenshots to `C:\UE\T66\Saved\Codex\Gameplay\BossProjectileManager\`
  - log to `C:\UE\T66\Saved\StandaloneLogs\T66_BossProjectileManager_Smoke.log`

MultiBoss capacity/color smoke:

- Use the same automation with `T66BossProjectileSmoke=FourHorsemen`.
- Confirm:
  - four boss IDs are spawned through `SpawnBossForCurrentStage()` from Stage 17 encounter data and awakened through the boss-gate path,
  - managed boss projectiles fire,
  - no `DroppedFires`,
  - no visual-bucket overflow warnings,
  - `ActivePeak` is logged,
  - at least two coexisting projectile colors are visible/distinct in screenshot evidence.

### Static Production Path Check

Use narrow source search:

```powershell
rg -n "SpawnActor<AT66BossProjectile>|AT66BossProjectile::StaticClass\(" Source/T66/Gameplay
```

Expected:

- no production boss firing path still spawns `AT66BossProjectile`.
- class file may still exist.
- `T66GameMode_Backrooms.cpp` may still reference the class for cleanup filtering; document whether that reference remains valid or should be widened to managed projectiles in a later cleanup.

Use a non-Git, path-only asset/text reference scan that does not trigger LFS hashing:

```powershell
rg -n "T66BossProjectile|BossProjectile" Source/T66 Content/Data Config -g "*.cpp" -g "*.h" -g "*.csv" -g "*.ini" -g "*.json" -g "*.uplugin" -g "*.uproject"
rg --files Content | Select-String -Pattern "BossProjectile|T66BossProjectile"
```

Expected:

- no production boss fire site remains on the actor path.
- any remaining asset/class references are compatibility references and are documented rather than deleted.

## Risks And Mitigations

- Risk: manager currently ignores projectile type, so a simple call-site swap would silently render boss shots as enemy spit and deliver `EnemyProjectile`.
  - Mitigation: implement typed visuals and delivery before changing boss call sites.
- Risk: boss projectile actor currently owns Niagara trail/impact VFX.
  - Mitigation: move the existing trail/impact system selection and per-frame VFX budget into manager-owned boss projectile behavior. If this cannot be preserved, stop before implementation completion and revise scope.
- Risk: Stage 17 MultiBoss may exceed the old 256 slots.
  - Mitigation: raise to 512, drop-new on overflow, fail smoke on any boss `DroppedFires`, and document active peak.
- Risk: HISM components with shared material cannot represent arbitrary per-boss colors if one component is reused across bosses.
  - Mitigation: use bounded exact visual-key component buckets keyed by mesh/profile and effective tint color, with deterministic overflow buckets; fail smoke if Stage 17 unexpectedly overflows exact buckets.
- Risk: old actor collision destroyed on any non-hero overlap while current manager ignores enemy peers.
  - Mitigation: peer-filter enemies/mobs/bosses as intended, but preserve world static/dynamic deactivation. Document this as intentional peer-filter alignment with basic enemy projectiles.
- Risk: adding boss logic to manager could couple boss-specific code into the basic-mob manager.
  - Mitigation: use a small typed fire spec and keep pattern logic in `AT66BossBase`.
- Risk: stale boss pointers after boss death or level transition.
  - Mitigation: weak pointers only, invalid-source drop rule, and boss-death-during-flight smoke.
- Risk: trail components leak or carry over between reused projectile slots.
  - Mitigation: reset/deactivate path destroys trail components before reuse and subsystem teardown destroys all remaining manager-owned trail components.

## Out Of Scope

- Boss actor migration to lightweight.
- Unique Debuff projectile migration.
- Goblin Thief, Gambler Boss system changes beyond inherited boss projectile path if applicable.
- Hero projectile manager.
- Deleting `AT66BossProjectile` files.
- B.10 acceptance finalization / enemywaveperf capture pass.
- B.11+ optimization work.
- Boss multipart/BossPartProfile redesign.

## Acceptance Criteria

- Boss projectiles fire through `UT66ProjectileManagerSubsystem`, not `AT66BossProjectile` production actor spawns.
- Boss firing patterns remain actor-owned and visually/factually preserved.
- Boss projectile damage applies to the hero with `SourceID=BossID` and `Delivery=BossProjectile`.
- Boss projectile HISM rendering is visible and has no obvious stuck/flicker artifacts.
- Boss projectile trail and impact VFX remain present through manager-owned VFX behavior.
- Manager capacity is 512 and documented against single-boss plus Four Horsemen load.
- Staged smoke confirms placed miniboss floor traversal, boss projectile firing, and Four Horsemen capacity/color behavior.
- Static search confirms no production `SpawnActor<AT66BossProjectile>` firing path remains.
- Combined completion packet captures implementation, review, verification, caveats, and next steps.

</review_packet>
