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

### Capacity Audit

- Current max in the manager is 256 active projectiles.
- Worst current simultaneous boss scheduling from the inspected patterns:
  - Single generic late-phase `Juggernaut`: fan 7 + radial 10 = 17 queued shots in one fire cycle.
  - Four Horsemen worst case if all four are Juggernaut-equivalent simultaneously: `17 * 4 = 68` queued shots over short stagger windows.
  - With 6-second projectile lifetime and Stage 17 fire intervals around `1.28-1.34s`, a pessimistic concurrent count can exceed 256 if all bosses retain peak burst cadence and projectiles do not hit/expire quickly: roughly `68 * ceil(6 / 1.28) = 340`.
- Plan: raise manager capacity to `512` for this pass. This is still a small fixed flat array, keeps linear scanning acceptable, and provides headroom for Four Horsemen plus ordinary boss patterns. Document the measured active peak in smoke.

### Boss Projectile Visual Type Decision

- Bosses do not all share one visual. The current actor changes mesh shape, scale, tint, trail, and impact by `ET66BossAttackProfile`, with optional secondary tint and visual scale multiplier.
- Decision: use exact visual-key HISM component buckets, not per-instance custom data and not a new material.
  - Type `0`: existing enemy spit bucket.
  - Boss buckets are keyed by:
    - mesh/profile (`Balanced`, `Sharpshooter`, `Juggernaut`, `Duelist`, `Gambler`)
    - effective tint color (primary or secondary, quantized to a stable byte color key)
  - Each bucket owns one HISM component and one dynamic flat-color material, so two same-profile bosses with different colors do not collapse to one stale color.
  - `VisualScaleMultiplier` stays per projectile via instance transform scale and per-projectile collision radius, not as a component bucket dimension.
- This avoids an art/material artifact. `FT66VisualUtil::GetFlatColorMaterial()` stays the base material; no per-instance custom data material is introduced this pass.
- Stage 17 validation must confirm coexisting Horsemen shots remain visually distinguishable in flight.

### Trail And Impact VFX Decision

- Decision: preserve boss projectile fire audio and boss projectile trail/impact VFX through manager-owned Niagara components/effects.
- The existing budget CVars move with the behavior:
  - `T66.VFX.BossProjectileMaxPerFrame`
  - `T66.VFX.BossProjectileUseEffectsScalability`
- Manager behavior:
  - On boss projectile activation, if the VFX budget allows, spawn the same profile-specific trail Niagara as a manager-owned component, store it on the projectile record, and update its transform with the projectile.
  - On boss projectile deactivation by hero/world impact, spawn the same profile-specific impact Niagara if budget allows.
  - On expiry/drop/reset, clean up any active trail component.
- HISM remains the required visible projectile body. Niagara trails/impacts preserve the old visual language; they are not the primary collision/rendering owner.
- If implementation discovers a hard engine limitation with manager-owned trail components, stop and revise the packet rather than silently shipping trail-less parity.

## Implementation Plan

### Task 1 - Extend Managed Projectile Data

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h`
- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp`

Add enough per-projectile state for boss semantics:

- delivery method (`EnemyProjectile` vs `BossProjectile`)
- whether the source is a boss
- attack profile / visual profile
- primary/secondary color and secondary-tint flag, if needed for component/material selection
- visual scale multiplier
- collision radius derived from old boss projectile behavior
- lifetime as per-projectile state (`6.0` for boss projectiles in this pass), not a manager-wide constant
- optional manager-owned trail component pointer for boss projectiles
- impact VFX payload for boss projectiles

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
- boss visual-key buckets: HISM mesh/material match current profile shape and effective tint color.
- manager bounds summary works across all projectile components, not only type `0`.
- `ProjectileManagerSummary` reports active peak and dropped fires across all types.

### Task 3 - Add Boss Fire API

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h/.cpp`
- `Source/T66/Gameplay/T66BossBase.cpp`

Options:

- Add a boss-specific wrapper such as `FireBossProjectile(...)` that accepts boss profile/color/scale and internally calls shared slot allocation.
- Or extend `FireProjectile(...)` with a structured spec. Prefer a small `FT66ManagedProjectileFireParams` struct if the call surface becomes too wide.

The API must pass:

- source actor = boss
- source ID = `BossID`
- origin/direction/speed
- damage HP = `ProjectileDamageHearts * 20`
- radius = `24 * max(1, VisualScaleMultiplier)`
- lifetime = `6.0`
- delivery = `BossProjectile`
- visual profile and tint data.
- live boss actor pointer as the `Attacker` passed to `RunState->ApplyDamage`. Do not substitute a manager pseudo-source for the damage source.

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

### Task 5 - Preserve Collision And Damage Semantics

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp`

Required behavior:

- Hero collision uses the existing manager capsule segment hit test.
- Boss projectile hit applies `RunState->ApplyDamage(DamageHP, BossActor, "BossProjectile", BossActor)`.
- Boss source resolves to `BossID` in `[CombatDamage]` logs.
- Peer enemies, lightweight mobs, and boss actors should not destroy boss projectiles.
- World static/dynamic impacts should deactivate boss projectiles, matching the old non-hero impact/destroy semantics.
- Rejected non-hero hero hurtbox should not apply damage.
- This is an intentional collision behavior change relative to the old actor path: boss shots will no longer self-destruct on basic mob or boss peer bodies. It matches the basic enemy projectile peer-filter rule and must be documented in the completion packet.

### Task 6 - Capacity Increase

Files:

- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h`

Raise `MaxProjectiles` from `256` to `512`.

Document rationale:

- current single-boss bursts are at most ~17 queued shots per firing cycle.
- Four Horsemen can plausibly queue ~68 shots per combined cycle.
- six-second lifetime plus ~1.3-second fire intervals needs >256 headroom in worst case.

Smoke must quote `ProjectileManagerSummary ActivePeak` and `DroppedFires` for single-boss and simulated Four Horsemen validation.

### Task 7 - Deprecate `AT66BossProjectile`

Files:

- `Source/T66/Gameplay/T66BossProjectile.h`

Add a top-of-class deprecation comment:

```cpp
// DEPRECATED: Boss projectiles are now owned by UT66ProjectileManagerSubsystem.
// Kept temporarily for asset/reference compatibility until cleanup.
```

Do not delete `.h/.cpp` files this pass.

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
  - force-awakens the stage boss long enough to fire managed projectiles.
- MultiBoss path:
  - command option such as `T66BossProjectileSmoke=FourHorsemen`,
  - directly spawns or initializes the four Stage 17 boss IDs in the current gameplay world for capacity/color validation,
  - force-awakens them,
  - waits long enough to collect `ProjectileManagerSummary`,
  - exits cleanly.

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
- Save:
  - screenshots to `C:\UE\T66\Saved\Codex\Gameplay\BossProjectileManager\`
  - log to `C:\UE\T66\Saved\StandaloneLogs\T66_BossProjectileManager_Smoke.log`

MultiBoss capacity/color smoke:

- Use the same automation with `T66BossProjectileSmoke=FourHorsemen`.
- Confirm:
  - four boss IDs are spawned/awakened,
  - managed boss projectiles fire,
  - no `DroppedFires`,
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

## Risks And Mitigations

- Risk: manager currently ignores projectile type, so a simple call-site swap would silently render boss shots as enemy spit and deliver `EnemyProjectile`.
  - Mitigation: implement typed visuals and delivery before changing boss call sites.
- Risk: boss projectile actor currently owns Niagara trail/impact VFX.
  - Mitigation: move the existing trail/impact system selection and per-frame VFX budget into manager-owned boss projectile behavior. If this cannot be preserved, stop before implementation completion and revise scope.
- Risk: Stage 17 MultiBoss may exceed the old 256 slots.
  - Mitigation: raise to 512 and document active peak in smoke where feasible.
- Risk: HISM components with shared material cannot represent arbitrary per-boss colors if one component is reused across bosses.
  - Mitigation: use exact visual-key component buckets keyed by mesh/profile and effective tint color; verify Stage 17 colors in the Four Horsemen smoke.
- Risk: old actor collision destroyed on any non-hero overlap while current manager ignores enemy peers.
  - Mitigation: peer-filter enemies/mobs/bosses as intended, but preserve world static/dynamic deactivation. Document this as intentional peer-filter alignment with basic enemy projectiles.
- Risk: adding boss logic to manager could couple boss-specific code into the basic-mob manager.
  - Mitigation: use a small typed fire spec and keep pattern logic in `AT66BossBase`.

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
