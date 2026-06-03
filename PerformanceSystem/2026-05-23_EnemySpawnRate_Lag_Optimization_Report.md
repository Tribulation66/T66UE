# Enemy Spawn Rate Lag Optimization Report

Date: 2026-05-23

Audience: Claude / next optimization reviewer

Project: T66, Unreal Engine 5.7

Scope: diagnostic report only. This pass does not change runtime behavior.

## Executive Summary

The current lag is very likely caused by the enemy-count increase exposing several systems that scale directly with live enemy count. The latest tuning makes each gameplay floor spawn up to 30 runtime enemies per wave with a 90 live-enemy cap, while the wave stagger and interval are both 5 seconds. That means the game can add roughly 6 enemies per second until it hits the live cap, and then it tends to stay near that cap if the player is not clearing enemies quickly.

The highest-confidence immediate issue is that combat debug visualization is enabled by default in non-shipping builds. With 90 live enemies, every frame can draw multiple debug spheres, capsules, and labels per enemy, plus projectile, trap, hero, and damage-volume debug shapes. That debug layer was intentionally added for combat clarity, but it is not cheap enough to leave fully enabled at tripled enemy counts in a Development standalone.

The deeper architecture issue is that a regular enemy is a full `ACharacter` with `CharacterMovement`, collision, hit zone components, visual components, optional widgets, per-frame tick behavior, safe-zone checks, combat timers, and debug draw. That is reasonable at low counts. At 60-120 live enemies, the total frame cost becomes the real budget, even if each individual enemy looks small in isolation.

Runtime evidence confirms that the latest staged Development run had severe frame pacing problems:

- Average FPS: 25.21
- Average frame: 39.67 ms
- 1 percent low FPS: 10.58
- 0.1 percent low FPS: 3.85
- Late-session sustained low FPS dropped into roughly 15-20 FPS windows
- Measured stalls included `GameplayHUD::RefreshMapData` at 49-71 ms

The current PerformanceSystem evidence confirms the lag, but it does not yet directly attribute a hitch to "N live enemies at this exact moment." The code inspection strongly suggests enemy-count scaling, combat debug draw, and HUD/enemy-list refreshes are the first places to instrument and optimize.

## What Changed

The recent spawn-rate change tripled the runtime enemy budget in `Content/Data/PlayerExperience.json` and reloaded `Content/Data/DT_PlayerExperience.uasset`.

Current gameplay-floor runtime values:

- `RuntimeEnemiesPerWave`: 30
- `RuntimeMaxAliveEnemies`: 90
- `RuntimeSpawnIntervalSeconds`: 5.0
- `RuntimeWaveStaggerDurationSeconds`: 5.0
- `RuntimeMaxSpawnsPerStaggeredBatch`: 1

Important implication: because the stagger duration and spawn interval are both 5 seconds, a 30-enemy wave is spread over the entire interval, and the next wave can begin almost immediately after the previous wave has finished materializing, as long as the live cap allows it.

Approximate spawn pressure:

- 30 planned enemies over 5 seconds
- 1 enemy per staggered batch
- About one spawn every 0.17 seconds while the wave is materializing
- Up to 90 live regular enemies before the director stops adding more
- In a 300 second floor, if the player keeps clearing the board, roughly 1,800 regular runtime enemies could be injected over time

This is not just a "faster trickle" change. It changes the normal gameplay state from a modest live enemy population to a sustained high-density enemy board.

## Evidence Reviewed

Source and data reviewed:

- `Content/Data/PlayerExperience.json`
- `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Source/T66/Gameplay/T66EnemyDirector.h`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/Enemies/T66MeleeEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp`
- `Source/T66/Core/T66EnemyPoolSubsystem.h`
- `Source/T66/Core/T66EnemyPoolSubsystem.cpp`
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Spawning.cpp`
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceTypes.h`
- `Source/T66/Core/T66StageProgressionSubsystem.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.cpp`
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp`
- `Source/T66/Gameplay/T66CombatDebugDraw.cpp`
- `Source/T66/Gameplay/T66CombatDebugDraw.h`
- `Source/T66/Core/T66LagTrackerSubsystem.h`
- `Source/T66/Core/T66LagTrackerSubsystem.cpp`
- `Source/T66/Core/T66ActorRegistrySubsystem.h`
- `Source/T66/Core/T66ActorRegistrySubsystem.cpp`
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp`

Runtime evidence reviewed:

- `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260523T231840Z_q_8p5EnDPxNEaKak01gISg/session_summary.md`
- `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260523T231840Z_q_8p5EnDPxNEaKak01gISg/events.jsonl`
- `Saved/StandaloneLogs/T66_Standalone.log`

Evidence caveat:

The PerformanceSystem session proves the game became frame-bound and logged specific stalls, but it does not currently log live enemy count, active projectile count, debug shape count, or per-family enemy counts at the moment of each hitch. The conclusions below combine measured runtime symptoms with source inspection.

## How Enemies Are Built And Organized

### Data Layer

Enemy pressure starts in player experience tuning:

- `Content/Data/PlayerExperience.json`
- `Content/Data/DT_PlayerExperience.uasset`
- `FT66PlayerExperienceRow`
- `UT66PlayerExperienceSubSystem::BuildTowerSpawnBudget`
- `UT66StageProgressionSubsystem::GetCurrentSpawnBudget`

`FT66PlayerExperienceRow` owns the base spawn knobs:

- initial floor enemies
- runtime enemies per wave
- max alive enemies
- runtime spawn interval
- wave stagger duration
- max spawns per staggered batch
- elite and boss modifiers

`BuildTowerSpawnBudget` applies difficulty scaling. The current 30/90 tuning is the data-row baseline, not necessarily the final runtime absolute in every difficulty/scalar condition. The code clamps `RuntimeEnemiesPerWave` up to 128 and `RuntimeMaxAliveEnemies` up to 1024, and the runtime interval can be divided by a difficulty scalar.

Risk: future difficulty changes can accidentally exceed the already-heavy 30/90 board state.

### Spawn Director

Runtime spawning is owned by `AT66EnemyDirector`.

Relevant flow:

1. `StartRuntimeSpawning` enables runtime spawning after the floor is active.
2. `SpawnRuntimeTrickleWave` pulls a budget from stage progression.
3. The director plans pending spawn locations.
4. `SpawnNextStaggeredBatch` materializes the pending spawns over time.
5. `ActiveEnemies` tracks live enemies.
6. `RuntimeMaxAliveEnemies` stops the director from adding more when the live cap is reached.

The director already has an important optimization: it staggers wave materialization instead of spawning all 30 actors in one frame. That protects against a single massive spawn hitch, but it does not solve the steady-state cost once the board reaches 60-90 live enemies.

Spawn planning cost can still be non-trivial. The tower path can attempt multiple candidate locations for each planned enemy, including wall spawn attempts and surface spawn attempts with clearance checks. With 30 planned spawns per wave, that planning work becomes more noticeable, but it is probably not the main cause of sustained 15-25 FPS. The persistent live-enemy cost is more likely.

### Enemy Actor Runtime

Base enemies derive from `AT66EnemyBase`, which derives from `ACharacter`.

Each regular enemy can include:

- `ACharacter` actor overhead
- `CharacterMovementComponent`
- pawn capsule collision
- visual mesh placeholder or imported VAT/mesh visual
- skeletal mesh component support, even when hidden for some variants
- `LockIndicatorWidget` widget component
- body hit zone sphere component
- head hit zone sphere component
- combat health and status state
- movement and behavior tick
- safe-zone checks
- debug hitbox/damage-volume draw
- lag tracker instrumentation scope

The base tick does significant work before family-specific behavior:

- calls `Super::Tick`
- updates mob vertex animation state
- draws debug body/head hit zones when combat debug is enabled
- draws touch damage capsule and labels when enabled
- resolves/caches the player pawn on a timer
- runs spawning-rise or wall-emerge motion if applicable
- updates `CharacterMovement`
- applies movement input in family behavior
- checks safe-zone membership on a timer
- updates status effect timers
- runs family-specific AI behavior

At 10-30 live enemies, this is likely acceptable. At 90 live enemies, all small per-enemy costs become a large fixed frame cost.

### Enemy Families

The standard families are documented in `Gameplay/Combat/MASTER_COMBAT.md` and implemented in subclasses:

- Melee: pursues the hero and deals touch/contact damage.
- Rush: charges or lunges more aggressively.
- Ranged: keeps distance and fires projectiles.
- Flying: uses a flying movement style and visual offset.

There are also special/elite/boss variants outside the regular family loop, but the lag report focuses on the regular enemy population because the recent change multiplied regular spawn pressure.

### Combat And Targeting

Hero combat is generally timer-driven instead of ticking every frame:

- `UT66CombatComponent` has component tick disabled.
- Hero auto-attacks are driven by timers.
- A hero-attached range sphere tracks enemies in range.
- Hero projectiles, idol effects, and enemy/trap projectiles use the temporary projectile visual system for clarity.

This is a good architecture for low and medium counts, but the cost still grows with enemy count:

- the range sphere must maintain more overlaps
- attacks can iterate over `EnemiesInRange`
- bounce/chain attacks can repeatedly search within the same enemy list
- more enemies means more hit zone components and collision candidates
- more enemy deaths means more loot/floating text/death feedback churn

### Ranged Enemy Projectiles

Ranged enemies spawn projectile actors:

- `AT66EnemyProjectileBase`
- sphere collision
- static mesh projectile visual
- accent mesh
- `ProjectileMovementComponent`
- per-projectile tick
- combat debug draw
- fire/impact logging

At high ranged-enemy counts, projectile actors become an additional multiplier. If many of the 90 live enemies are ranged, the game can have many more active collision/tick/logging objects than the live-enemy count alone suggests.

### Enemy Pool

`UT66EnemyPoolSubsystem` exists and can reuse enemy actors:

- keyed by exact enemy class
- acquires pooled enemies when available
- returns enemies to pool on death
- helps avoid actor destruction/creation churn after the first wave

Current risk:

- no clear prewarming was found
- pooling does not reduce the cost of 90 live enemies
- one reviewed standalone log showed `Total acquired=0, reused=0` for that session, so that run did not meaningfully exercise pool reuse

Pooling helps with spawn/death spikes. It does not solve steady-state per-frame cost.

### Combat Debug Visualization

Combat debug visualization was intentionally built for clarity:

- hero hurtbox
- enemy body/head hit zones
- enemy damage/contact volume
- projectile hit boxes
- trap projectile hit boxes
- labels

The current debug system is valuable for debugging invisible damage, but it is expensive at high counts:

- non-shipping default debug view is currently `3`
- non-shipping debug labels default to enabled
- body/head hit zones use `DrawDebugSphere`
- sphere segments are currently high enough to be visually clear
- labels can be drawn every frame
- enemies draw multiple debug primitives per frame
- projectiles/traps/heroes add more primitives on top

This is likely the single largest immediate reason the Development standalone feels much worse after the spawn increase.

Rough shape count at 90 live enemies:

- body hit zone per enemy
- head hit zone per enemy
- touch damage capsule per enemy
- label strings per enemy if labels are enabled
- projectile hitboxes per active projectile
- trap projectile hitboxes per active trap attack
- hero/projectile/idol debug hitboxes

That is at least hundreds of debug draws per frame before normal rendering, movement, collision, HUD, and projectiles.

### HUD And Map Refresh

`T66GameplayHUDWidget_Map.cpp` refreshes map/minimap enemy data from the actor registry.

Measured stalls included:

- `GameplayHUD::RefreshMapData`: 71.07 ms
- `GameplayHUD::RefreshMapData`: 56.80 ms
- `GameplayHUD::RefreshMapData`: 49.77 ms

The minimap caps enemy markers to 48, but the full-map path can reserve and process all live enemies. With the new live cap, map refresh can become a visible hitch source, especially if it runs while the enemy board is saturated.

### PerformanceSystem And Lag Tracker

The PerformanceSystem is useful but currently not specific enough for this issue.

It captured:

- frame variance stutters
- low FPS windows
- single-frame hitches
- memory growth warnings
- project operation stalls
- HUD refresh stalls

It does not yet capture the most important enemy-related state at hitch time:

- live enemy count
- active projectile count
- pending spawn count
- debug visualization mode
- debug shape count estimate
- per-family live counts
- average enemy tick cost as a group
- number of enemies inside hero targeting range
- number of active combat hit zone components

The existing `FLagScopedScope` inside enemy tick can identify single slow enemy ticks above a threshold, but it may miss aggregate cost. Ninety enemies each taking a small amount of time can be the main frame cost without any one enemy crossing the per-operation threshold.

## Most Likely Lag Causes, Ranked

### 1. Combat Debug Draw Default-On In Development

Confidence: very high.

Why it matters:

- The user explicitly asked to make combat hitboxes/projectiles visible.
- The debug view is currently on by default in non-shipping builds.
- Every live enemy draws multiple debug shapes each frame.
- Debug labels are also on by default.
- Tripling live enemy count multiplies debug rendering cost immediately.

Expected symptom:

- Development standalone becomes much slower than expected.
- Visual clarity improves, but high enemy counts become frame-bound.
- Shipping builds may perform better if debug defaults are disabled there.

Recommended first check:

Run the same floor twice:

1. with combat debug view enabled
2. with `T66.Combat.DebugView=0` and `T66.Combat.DebugLabels=0`

If FPS recovers substantially, the immediate fix is to make full combat debug visibility an explicit diagnostic mode, not the default Development experience.

### 2. Full `ACharacter` Enemy Model At 90 Live Enemies

Confidence: high.

Why it matters:

- `ACharacter` plus `CharacterMovement` is a rich actor model.
- It is more expensive than a lightweight actor or batched swarm entity.
- Every enemy ticks every frame.
- Every enemy owns collision, hit zones, visuals, and status state.
- The live cap is now high enough that the architecture itself becomes the bottleneck.

Expected symptom:

- Even with debug off, performance may remain lower than desired at 90 live enemies.
- CPU Game Thread cost rises as the board saturates.
- Collision and movement cost scale with live enemy count.

Recommended first check:

Add grouped timing for all enemy ticks per frame, not only per-enemy slow operations. The question is "how much frame time did all enemies consume together?"

### 3. The Spawn Settings Create A Sustained Full Board

Confidence: high.

Why it matters:

- A 30-enemy wave over 5 seconds is aggressive.
- The next wave can start immediately after the stagger window finishes.
- The live cap is 90.
- If the player does not clear quickly, the board saturates.

Expected symptom:

- Performance degrades over the first 15-30 seconds of a floor.
- Late-session FPS is worse than early-session FPS.
- Lag persists after spawning slows because 90 live actors remain active.

Recommended first check:

Log live enemy count, pending spawn count, and active projectile count every second and attach those numbers to every hitch event.

### 4. Ranged Enemy Projectile Multiplication

Confidence: medium-high.

Why it matters:

- Ranged enemies line-trace for line of sight.
- Ranged enemies spawn projectile actors.
- Projectiles tick, move, collide, draw debug boxes, and log.
- If many live enemies are ranged, active projectile count can spike.

Expected symptom:

- Stutter worsens when ranged enemies are on screen or firing.
- The player may see many red projectile shapes if visibility is working.
- Logs may be noisy from projectile fire/impact events.

Recommended first check:

Log active enemy projectile count and spawned projectile count per second. Also group projectile tick/collision/debug cost per frame.

### 5. Hero Targeting And Chain/Bounce Scaling

Confidence: medium.

Why it matters:

- The hero range sphere has more overlaps at high enemy density.
- Some attacks iterate `EnemiesInRange`.
- Bounce and chain behavior can rescan enemy lists.
- Attack rate, idol effects, and weapon effects can multiply the cost.

Expected symptom:

- Hitches appear when the player attacks into dense groups.
- Higher fire-rate weapons worsen frame time.
- Idol effects that spawn many projectiles or area checks worsen frame time.

Recommended first check:

Instrument hero attack resolution with:

- `EnemiesInRange` count
- candidates scanned
- projectiles spawned
- bounces/chains resolved
- total combat resolution time

### 6. HUD Map Refresh Stalls

Confidence: measured, medium.

Why it matters:

- PerformanceSystem already captured 49-71 ms `GameplayHUD::RefreshMapData` stalls.
- The map refresh reads enemy data from the actor registry.
- Full-map marker work can scale with live enemy count.

Expected symptom:

- Periodic hitches rather than constant low FPS.
- Hitches may correlate with HUD/map refresh cadence.

Recommended first check:

Throttle or dirty-flag map enemy marker refreshes and measure before/after. Also log enemy marker count and refresh reason.

### 7. Death, Loot, Floating Text, And Feedback Bursts

Confidence: medium.

Why it matters:

- Higher enemy density means enemies can die in bursts.
- Death events can spawn loot, text, VFX, hit feedback, score/combat logs, and pool return/reset work.
- Burst cleanup can produce frame spikes even if steady-state tick is acceptable.

Expected symptom:

- FPS drops when the player clears a dense group.
- GC, actor return-to-pool, or UI text updates cluster together.

Recommended first check:

Count deaths per second, loot spawns per second, floating text spawns per second, and pooled returns per second.

### 8. Performance Instrumentation Overhead

Confidence: low-medium.

Why it matters:

- Lag tracker scopes are useful, but many scopes per frame have overhead.
- Event volume can grow when the frame is already bad.
- Per-enemy timing has overhead at high live counts.

Expected symptom:

- Development diagnostics are somewhat slower than a clean Development build.
- Instrumentation overhead is probably secondary to debug draw and live actor cost.

Recommended first check:

Run one test with diagnostics enabled and one with only minimal frame logging, while keeping gameplay conditions the same.

## Risks In The Current Approach

### Debug Clarity And Performance Are Coupled

The current combat clarity work made invisible damage diagnosable, which was necessary. The risk is that debug clarity became a default rendering load instead of a toggleable diagnostic layer. This can make performance appear worse than the actual gameplay systems would be with debug off.

Recommendation: keep the visibility system, but make it explicitly enabled, throttled, or filtered.

### The 90 Cap Is Not A Hard Global Ceiling In All Conditions

The data row says 90, but the player experience subsystem can apply difficulty scaling. That makes 90 the baseline for the row, not necessarily the highest possible runtime state.

Recommendation: add explicit budget reporting to the HUD/dev logs and PerformanceSystem so the actual runtime budget is visible.

### The Current Enemy Model Is Rich For Swarm Counts

Full characters are easier to author and debug, but 90 regular enemies means 90 movement components, 90 pawn capsules, 180 hit zone spheres, 90 visual actors/meshes, optional widgets, and many per-frame behavior updates.

Recommendation: decide whether T66 wants "dozens of rich actors" or "hundreds of lightweight swarm entities." The current code is closer to the first model.

### Pooling Can Hide Spawn Churn But Not Live Cost

Enemy pooling is useful, but it does not make 90 active enemies cheap. If the board is saturated, all pooled actors are still live and ticking.

Recommendation: use pooling for spawn/death spikes, but do not count it as the main fix for sustained low FPS.

### Visual Projectile Clarity Can Increase Actor Count

The temporary projectile system is directionally correct because attacks must be visible. The risk is that every visible projectile becomes another actor/tick/collision/debug object.

Recommendation: visible projectiles should be pooled, have cheap collision, and use grouped timing. Debug hit boxes should be separate from production projectile visuals.

### Map Refresh Is A Measured Hitch Source

The HUD map path is already implicated by measured stalls. If enemy count rises, marker work and registry iteration become more expensive.

Recommendation: fix or throttle map refresh independently of enemy tick work.

## Recommended Plan

### Phase 0: Get Exact Attribution Before Optimizing

Goal: prove how much of the lag is debug draw, enemy tick, projectile tick, map refresh, and spawn planning.

Add or capture the following per-second counters:

- live regular enemies
- live elites
- active enemy projectiles
- active hero projectiles
- pending spawn count
- enemies spawned this second
- enemies killed this second
- loot actors spawned this second
- floating combat text spawned this second
- combat debug view mode
- combat debug labels enabled
- estimated combat debug primitives drawn
- hero `EnemiesInRange` count

Add grouped timings:

- total enemy tick time per frame
- total ranged enemy projectile tick time per frame
- total hero combat resolution time per attack
- total spawn planning time per wave
- total map refresh time per refresh

Attach these to PerformanceSystem hitch events. The desired output for a hitch is:

> Frame was 92.91 ms. Live enemies 84. Enemy tick group 18.4 ms. Combat debug draw estimate 300 primitives. Active enemy projectiles 37. Map refresh 56.8 ms. Hero candidates scanned 63.

That level of evidence will make the optimization target unambiguous.

### Phase 1: Fast Relief With Low Design Risk

These are the safest near-term changes.

1. Default combat debug off in Development, or gate it behind an explicit setting.

   Keep the debug system, but stop drawing all hitboxes every frame by default.

2. Add debug filters.

   Useful filters:

   - nearest N enemies
   - enemies currently damaging or targeting the hero
   - projectiles only
   - traps only
   - selected/locked enemy only
   - draw every N frames instead of every frame

3. Disable debug labels by default.

   Labels are useful for diagnosis but expensive and visually noisy at high counts.

4. Temporarily reduce max alive if needed.

   If playability is the immediate goal, test 45-60 live enemies with the current rich actor model. Keep 90 as a target only after optimization.

5. Add active enemy count and projectile count to dev logs.

   This gives immediate visibility into whether the board is saturated.

Expected effect:

- If debug draw is the main issue, this may recover a large amount of FPS immediately.
- It will not solve the deeper 90-character cost if that remains too high.

### Phase 2: Optimize The Existing Actor Model

These changes keep the current enemy architecture but make it scale better.

1. Add enemy significance tiers.

   Suggested tiers:

   - near and visible: full tick
   - near but off camera: reduced behavior tick
   - far: low-frequency movement/behavior update
   - dormant/off-floor: no behavior tick

2. Reduce per-enemy tick frequency.

   Family behavior does not necessarily need to run at 60 FPS. Movement can interpolate while decisions update at 5-15 Hz.

3. Make hit zone collision cheaper.

   Audit whether body/head hit zones need overlap events, query-only collision, or trace-only use. If they are only used as attack targets, they may not need expensive overlap behavior.

4. Simplify enemy movement collision.

   If enemies do not need full `CharacterMovement` features, consider a cheaper movement component or custom movement for basic mobs.

5. Disable or throttle widget components.

   `LockIndicatorWidget` should tick/render only when relevant, visible, selected, or near the player.

6. Prewarm the enemy pool.

   Prewarm common enemy classes to the expected cap per floor. This reduces construction spikes, although it does not solve steady-state live cost.

7. Pool projectiles aggressively.

   Enemy, trap, idol, and hero projectiles should be pooled if high fire rates are expected.

8. Reduce ranged enemy log noise.

   Repeated fire/line-of-sight logs should be VeryVerbose or sampled during performance testing.

9. Throttle HUD map refresh.

   Use dirty flags, capped marker rebuilds, or lower-frequency refreshes. Avoid rebuilding all markers during normal gameplay if the map is not open.

10. Batch debug rendering.

   If combat debug must stay visible, batch or simplify it:

   - lower sphere segment count
   - draw only outlines close to the hero
   - draw only damage volumes, not all hurtboxes
   - draw every 0.1-0.25 seconds instead of every frame

Expected effect:

- This preserves the current code shape.
- It can probably support a larger enemy count than today.
- It may still struggle if the target is truly "hundreds of enemies."

### Phase 3: Architectural Swarm Solution

If the design goal is to support very dense enemy swarms, the current full-character model should probably be split.

Recommended direction:

- keep full `ACharacter` enemies for elites, bosses, and nearby priority targets
- represent basic distant mobs as lightweight swarm entities
- batch movement in a manager
- use instanced static meshes or VAT visuals for large groups
- store simple combat state in arrays
- promote an entity to a full actor only when it needs rich interaction

Possible implementation options:

1. Unreal MassEntity.

   Pros:

   - built for many entities
   - data-oriented
   - can scale better for swarm behavior

   Cons:

   - bigger architecture shift
   - more learning and integration work
   - must integrate with existing combat, hitbox, projectile, loot, and debug systems

2. Custom `UT66EnemySwarmSubsystem`.

   Pros:

   - tailored to current combat rules
   - smaller conceptual surface than Mass
   - can use existing actor registry/combat APIs as adapters

   Cons:

   - more custom code to maintain
   - risk of recreating engine systems poorly
   - needs careful editor/debug tooling

3. Hybrid actor manager.

   Pros:

   - incremental
   - keep actors but disable ticks and update them from one manager
   - easier to migrate from current code

   Cons:

   - still pays actor/component overhead
   - less scalable than true data-oriented swarm

Recommendation:

Do Phase 0 and Phase 1 first. If 90 live enemies is still too expensive with debug off and basic throttling, start Phase 2. If the long-term target is more than roughly 60-90 active basic enemies, plan Phase 3 instead of repeatedly optimizing full characters.

## Concrete Profiling Questions For Claude

1. Should the project keep full `ACharacter` for all regular enemies, or split basic mobs into lightweight swarm entities?

2. Is the biggest immediate win likely to be combat debug draw gating, enemy tick throttling, movement/collision simplification, or HUD map refresh throttling?

3. What is the cleanest way to add grouped enemy timing to the existing PerformanceSystem without making instrumentation overhead worse?

4. Should enemy hit zones stay as `UPrimitiveComponent` spheres, or should combat target detection move toward cheaper query data for basic enemies?

5. Should projectile clarity stay actor-based for now, or should temporary projectile visuals be pooled/instanced immediately?

6. What live enemy cap is realistic with the current architecture before a swarm architecture is justified?

7. How should debug visibility be designed so hitboxes remain available for diagnosing invisible damage, without becoming a default performance cost?

## Suggested Immediate Test Matrix

Run the same floor scenario with these conditions:

| Test | Enemy Budget | Combat Debug | Debug Labels | Goal |
| --- | ---: | --- | --- | --- |
| A | 30 wave / 90 cap | on | on | Current worst-case Development behavior |
| B | 30 wave / 90 cap | off | off | Separate gameplay cost from debug cost |
| C | 30 wave / 45 cap | off | off | Test whether live cap is the main limiter |
| D | 10 wave / 30 cap | off | off | Baseline against previous scale |
| E | 30 wave / 90 cap | projectiles only | off | Test visible projectile clarity without all hitboxes |

For each run, capture:

- average FPS
- 1 percent low FPS
- 0.1 percent low FPS
- live enemy count at hitches
- active projectile count at hitches
- map refresh stalls
- total enemy tick group time
- combat debug primitive estimate

## Recommended First Patch Set

If the next pass is implementation rather than investigation, the smallest practical patch set would be:

1. Add PerformanceSystem counters for live enemies, active projectiles, pending spawns, and combat debug mode.
2. Add grouped enemy tick timing per frame.
3. Make combat debug visibility explicit and default it off for normal Development standalone play.
4. Keep a one-key or console-command path to re-enable hitbox/projectile visualization when diagnosing damage.
5. Throttle or dirty-flag HUD map enemy marker refresh.
6. Re-run the same floor and compare PerformanceSystem sessions.

This preserves the combat clarity work while separating "debug visibility mode" from "normal high-enemy-count play."

## Bottom Line

The lag is probably not caused by one broken enemy. It is a scaling problem created by raising live enemy pressure into a range where several systems become expensive at the same time:

- full `ACharacter` enemies
- per-frame enemy tick and movement
- high live cap
- default-on combat debug draw and labels
- ranged projectile actors
- hero combat scans at dense enemy counts
- HUD map refresh work

The fastest way to get back to playable performance is to disable or filter default combat debug draw and verify the same 90-enemy scenario with debug off. The correct long-term solution depends on the intended enemy density. If T66 wants 90+ live basic enemies as a normal state, the project should either aggressively throttle the current actor model or introduce a lightweight swarm layer for basic mobs.
