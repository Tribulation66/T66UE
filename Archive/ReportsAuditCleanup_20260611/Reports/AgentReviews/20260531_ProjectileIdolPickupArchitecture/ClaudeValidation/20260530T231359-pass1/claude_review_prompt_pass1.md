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

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260531_ProjectileIdolPickupArchitecture\current_projectile_idol_pickup_architecture.md
- Output scope: review of the packet below only.

<review_packet>
# Current Projectile, Idol Attack, and World Pickup Architecture

Read-only investigation packet. No code, data, asset, config, content, or save files were changed for this pass.

Working task:
- Operator: Codex
- Validator: Claude
- Scope: current projectile spawn/update/render, idol attack/proc model, world pickup rendering, recent weapon/idol infrastructure, and enemywaveperf harness evidence.
- Stop condition: every requested group A-E answered with file:line evidence, then Claude validation.

## Group A - Projectile Spawn, Update, Render

### A1. Hero weapon attacks

Hero weapon attacks route through `UT66CombatComponent::TryFire`; the selected weapon or hero category switches to Pierce, Bounce, AOE, or DOT in `Source/T66/Gameplay/T66CombatComponent.cpp:3298-3308`. The default combat cadence is 1.0 second per shot and 1000 range in `Source/T66/Gameplay/T66CombatComponent.h:50-58`.

| Attack type | Current damage delivery primitive | Current visual delivery primitive | Spawn/update path evidence |
| --- | --- | --- | --- |
| AOE | Immediate overlap/zone query and synchronous damage. It is not a projectile-manager route. | Bound Niagara for rows with a CombatVFX binding, otherwise optional visual-only `AT66HeroProjectile` temporary presentation. | `PerformSlash` uses slash/AOE target building and damage in `Source/T66/Gameplay/T66CombatComponent.cpp:2346-2443`; the underlying target query is `BuildSlashTargets` in `Source/T66/Gameplay/T66CombatComponent.cpp:1994-2081`; the category dispatch is `Source/T66/Gameplay/T66CombatComponent.cpp:3298-3308`. |
| Pierce | Immediate line/tube overlap query and synchronous damage. It is not a projectile-manager route. | Bound Niagara for rows with a CombatVFX binding, otherwise optional visual-only `AT66HeroProjectile`. | `PerformPierce` is in `Source/T66/Gameplay/T66CombatComponent.cpp:2267-2343`; the query is `BuildPierceTargets` in `Source/T66/Gameplay/T66CombatComponent.cpp:1882-1981`; Pierce dispatch is `Source/T66/Gameplay/T66CombatComponent.cpp:3301-3304`. |
| Bounce | Immediate chain target selection and synchronous damage per link. The link damage is resolved up front. | Sequential visual-only `AT66HeroProjectile` links, one link at a time, with optional Niagara carriers. | `PerformBounce` is in `Source/T66/Gameplay/T66CombatComponent.cpp:2446-2599`; comments say bounce damage and contexts are resolved before visual links in `Source/T66/Gameplay/T66CombatComponent.cpp:1300-1304`; sequential links are staged in `Source/T66/Gameplay/T66CombatComponent.cpp:1334-1478`; each link spawns an individual visual-only `AT66HeroProjectile` in `Source/T66/Gameplay/T66CombatComponent.cpp:1480-1528`. |
| DOT | Immediate initial hit plus a timed DOT payload stored in RunState. The DOT itself is not a world field actor. | A visual-only travel projectile can carry the DOT presentation; DOT is applied on arrival, or immediately if the visual shot does not spawn. | `PerformDOT` is in `Source/T66/Gameplay/T66CombatComponent.cpp:2603-2731`; DOT payload application on arrival is in `Source/T66/Gameplay/T66CombatComponent.cpp:2654-2663`; visual travel projectile creation is in `Source/T66/Gameplay/T66CombatComponent.cpp:2692-2714`; fallback immediate DOT is `Source/T66/Gameplay/T66CombatComponent.cpp:2717-2719`. |

`AT66HeroProjectile` is an individual `AActor`, has actor tick enabled, has collision/static mesh/Niagara components, and owns a `UProjectileMovementComponent` with 2400 speed and a 10 second safety lifespan (`Source/T66/Gameplay/T66HeroProjectile.cpp:18-54`). It remains live for hero visual carriers and legacy projectile behavior.

### A2. Idol attacks

Idol attacks are processed inside the same `TryFire` flow after the weapon branch. Each cached equipped idol slot is iterated in `Source/T66/Gameplay/T66CombatComponent.cpp:3438-3452`. Current idol attacks do not route through `UT66ProjectileManagerSubsystem`.

| Idol attack type | Current primitive | Projectile-manager route? | Evidence |
| --- | --- | --- | --- |
| Pierce idol | Line/tube query from the weapon impact point, immediate damage per target, optional bound or placeholder VFX. | No. | `Source/T66/Gameplay/T66CombatComponent.cpp:3509-3570` |
| Bounce idol | Immediate target chain from the weapon impact point, damage per link, optional bound or placeholder VFX. | No. | `Source/T66/Gameplay/T66CombatComponent.cpp:3572-3640` |
| DOT idol | RunState DOT timer applied to the impact target, optional bound or placeholder VFX. | No. | `Source/T66/Gameplay/T66CombatComponent.cpp:3642-3681`; RunState DOT storage/ticking is in `Source/T66/Core/RunState/T66RunStateSubsystem_Idols.cpp:25-96`. |
| AOE idol | Radius overlap from the weapon impact point, immediate damage to gathered targets, optional Water placeholder or binding. | No. | `Source/T66/Gameplay/T66CombatComponent.cpp:3684-3758` |

The current impact-presentation allowlist is four idols: Water/AOE, Light/Pierce, Electric/Bounce, Poison/DOT (`Source/T66/Gameplay/T66CombatShared.cpp:96-106`). Earth is supported as a control input but is not in the impact-presentation set (`Source/T66/Gameplay/T66CombatShared.cpp:109-118`).

### A3. `UT66ProjectileManagerSubsystem` current state

- Managed delivery types are only `EnemyProjectile` and `BossProjectile` (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:19-23`).
- Capacity is `MaxProjectiles = 512`; the subsystem is a `UWorldSubsystem` and `FTickableGameObject` (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:127-134`).
- State is stored as a flat `TArray<FT66ManagedProjectile>` with one struct per slot; each struct stores position, velocity, lifetime, radius, damage, source, delivery type, visual bucket, HISM instance index, and optional trail/impact systems (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:44-69`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:167-186`).
- Rendering uses `UHierarchicalInstancedStaticMeshComponent` buckets. Enemy spit uses one component; boss projectiles can allocate exact visual buckets up to `MaxExactBossVisualBuckets = 32`, then use overflow buckets (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:797-930`).
- Each HISM component preallocates `MaxProjectiles` hidden instances (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:933-946`). A live projectile slot maps to the same HISM instance index as the flat slot (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:440-516`).
- Movement/update is a manager batch tick, not per-projectile actor tick: the subsystem tick loops active slots and calls `TickProjectile`, then flushes HISM trees (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:712-739`).
- Movement is manual `Position += Velocity * DeltaSeconds`; hero collision uses manual segment-to-capsule distance, while non-hero/world collision uses sphere sweeps (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:1074-1292`).
- Lifetime expires by decrementing `LifetimeRemaining`; deactivation hides the HISM instance, clears trail components, and decrements active count (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:960-975`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp:1074-1169`).
- Current live routes through the manager are ranged mobs, ranged lightweight mobs, and bosses. Mob/ranged fire paths call `FireProjectile` (`Source/T66/Gameplay/T66MobBase.cpp:695-775`, `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp:166-245`); bosses call `FireBossProjectile` (`Source/T66/Gameplay/T66BossBase.cpp:931-1015`). Hero and idol attacks do not use this manager today.

### A4. Current status of projectile actor classes

- `AT66HeroProjectile`: individual actor, still live. It is used for hero weapon presentation and visual-only travel carriers; constructor evidence is `Source/T66/Gameplay/T66HeroProjectile.cpp:18-54`, and overlap/destroy behavior is in `Source/T66/Gameplay/T66HeroProjectile.cpp:406-455`.
- `AT66EnemyProjectileBase`: individual actor class retained as deprecated. The header states enemy ranged projectiles are now owned by `UT66ProjectileManagerSubsystem` and the actor remains for cleanup/asset-reference safety (`Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h:16-19`).
- `AT66SpitProjectile`: subclass of `AT66EnemyProjectileBase`; it still exists and sets spit values, but normal mob/ranged firing routes through the manager (`Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.h:9-15`, `Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.cpp:8-20`, `Source/T66/Gameplay/T66MobBase.cpp:752-753`, `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp:223-224`).

## Group B - Idol Proc and Multiplication Model

### B1. Equipped idol slot count

Current equipped idol capacity is 3 (`Source/T66/Core/T66IdolManagerSubsystem.h:22`). The same header has 12 stock slots and max idol level 4 (`Source/T66/Core/T66IdolManagerSubsystem.h:22-24`). The current repo does not contain a 16-equipped-idol slot model; any 16-idol estimate below is an extrapolation of the current slot loop to 16 equipped slots.

### B2. Idol trigger model

The idol trigger is not an independent timer. `TryFire` runs the hero attack branch, then iterates `CachedIdolSlots` once per `TryFire` timer execution (`Source/T66/Gameplay/T66CombatComponent.cpp:3298-3332`, `Source/T66/Gameplay/T66CombatComponent.cpp:3438-3452`). Overclock can run a second immediate weapon attack branch every eighth attack (`Source/T66/Gameplay/T66CombatComponent.cpp:3310-3320`, `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:481-494`), but the idol slot loop itself still appears once in the `TryFire` flow.

### B3. Per-idol creation counts

Current idol attacks create zero projectile-manager instances. Per trigger:

- Pierce idol: one idol impact context; up to `round(PropertyAtRarity) + 1` target hits; zero manager projectiles (`Source/T66/Gameplay/T66CombatComponent.cpp:3521-3549`).
- Bounce idol: one idol impact context; up to `round(PropertyAtRarity) + 1` chain targets; zero manager projectiles (`Source/T66/Gameplay/T66CombatComponent.cpp:3577-3620`).
- DOT idol: one DOT applied to the impact target in the impact-presentation path; legacy fallback can apply DOT to every weapon-hit actor (`Source/T66/Gameplay/T66CombatComponent.cpp:3648-3658`, `Source/T66/Gameplay/T66CombatComponent.cpp:3770-3795`).
- AOE idol: one radius query and damage pass over gathered targets; zero manager projectiles (`Source/T66/Gameplay/T66CombatComponent.cpp:3687-3758`).

### B4. Hero attack rate

Base fire interval is 1.0 seconds (`Source/T66/Gameplay/T66CombatComponent.h:53-54`). Effective interval is clamped to a minimum of 0.05 seconds after item, hero/passive, category, and weapon multipliers, which gives a hard timer cap of 20 `TryFire` executions per second (`Source/T66/Gameplay/T66CombatComponent.cpp:967-995`). Hero attack speed stat contributes via `GetHeroAttackSpeedMultiplier` (`Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp:982-986`); Rally can add attack speed while active (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:462-468`); Endurance can double attack speed below 30 percent HP (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:507-510`).

### B5. Worked load estimate for high-attack-speed full-idol build

Current code, current 3 equipped slots:

- Idol slot evaluations: up to 20 `TryFire`/sec * 3 slots = 60 idol slot evaluations/sec.
- Idol manager projectiles: 0/sec, because current idol attacks are line/chain/zone/DOT logic plus VFX, not manager projectiles.
- Idol visual-only hero-projectile lanes: only legacy/non-impact-presentation idol lanes can add `AT66HeroProjectile` visual carriers; the four impact-presentation proof idols use impact VFX/placeholder paths. If every equipped idol used a legacy visual lane, the upper visual-lane rate would be 60 actor lanes/sec, plus up to 20 unsuppressed weapon visual actors/sec.

Extrapolated 16-equipped-slot model using the current loop shape:

- Idol slot evaluations: 20 `TryFire`/sec * 16 slots = 320 idol slot evaluations/sec.
- If future idols keep the current impact-presentation architecture, projectile-manager idol instances remain 0/sec.
- If future idols use one legacy visual-only `AT66HeroProjectile` lane per equipped idol per `TryFire`, the visual actor creation rate would be 320 actor lanes/sec, plus up to 20 unsuppressed weapon visual actors/sec.
- A simple concurrent-actor sizing estimate using default 1000 range and 2400 projectile speed is 1000/2400 = about 0.42 seconds of travel (`Source/T66/Gameplay/T66CombatComponent.h:50-54`, `Source/T66/Gameplay/T66HeroProjectile.cpp:46-49`). At 320 visual lanes/sec, that is about 134 concurrently live visual carriers during steady travel. The safety-lifespan ceiling is 10 seconds, so the absolute unresolved-actor ceiling at that rate would be 3200 visual carriers if none arrived or cleaned up (`Source/T66/Gameplay/T66HeroProjectile.cpp:20-22`).

## Group C - World Pickups and Mob Loot Rendering

### C1. Current world pickup/interactable spawning and rendering

- Enemy loot bags are individual `AT66LootBagPickup` actors spawned on enemy death. Drop chance, count, rarity, and item ID are rolled in `Source/T66/Gameplay/T66EnemyBase.cpp:1720-1828`.
- `AT66LootBagPickup` is an actor with tick disabled, a physics root sphere, an overlap pickup sphere, a `UStaticMeshComponent` visual, and a `UProjectileMovementComponent` fall component (`Source/T66/Gameplay/T66LootBagPickup.cpp:56-107`). It registers as a loot bag and binds overlap events on `BeginPlay` (`Source/T66/Gameplay/T66LootBagPickup.cpp:109-125`).
- Chests, crates, fountains, totems, and tower loot bags are individual actors spawned through world-interactable/game-mode paths (`Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1210-1342`). The shared interactable base is an `AActor` with a trigger box, static mesh, and prompt text components (`Source/T66/Gameplay/T66WorldInteractableBase.h:18-53`, `Source/T66/Gameplay/T66WorldInteractableBase.cpp:71-115`).
- There is no separate world dropped-item actor found in this pass. Loot bags carry item IDs and show pickup cards; related references are `Source/T66/Gameplay/T66LootBagPickup.cpp:146` and pickup card HUD routes in `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:6815-6821`.

### C2. Existing mob perf capture death rate and concurrent alive

Exact peak mob deaths per second is not recorded in the current enemywaveperf frame sample schema. The schema records frame time, live regular/rich/lightweight counts, pending spawns, active enemy projectiles, and lightweight pool acquire/release counters, but no death-rate or death-count field (`PerformanceSystem/schema/board_saturation_frame_sample.schema.v8.json:6-42`). The performance subsystem writes the same fields to JSONL (`Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:1311-1378`). The lag tracker populates live enemy counts and pool release counters, not a named death metric (`Source/T66/Core/T66LagTrackerSubsystem.cpp:365-442`).

The current full-resolution sanity capture records peak concurrent alive as 90 regular enemies, 87 lightweight mobs, and 2 enemy projectiles (`Saved/Codex/Performance/RetroFXOffByDefaultFix/full_res_enemywaveperf_result.json:14-22`). The later B.13 before-baseline uses the same full-resolution `enemywaveperf` saturation contract and records 3/3 accepted captures with median FPS/lows, but its audit table does not add a death-rate metric (`PerformanceSystem/B13_MobInstancedRendering_Audit.md:62-76`).

### C3. Instanced/billboard/sprite pickup rendering

No existing instanced, billboard, sprite, or Niagara pickup renderer was found in the current pickup/interactable classes searched. `rg` over `T66LootBagPickup`, `T66WorldInteractableBase`, `T66ChestInteractable`, `T66FountainInteractable`, and `T66CrateInteractable` for `HierarchicalInstancedStaticMesh`, `InstancedStaticMesh`, `Billboard`, `SpriteComponent`, `PaperSprite`, and `NiagaraComponent` returned no matches. Current pickups and interactables are actors with static mesh components as listed above.

## Group D - Recent Weapon/Idol Infrastructure Pass

### D1. What currently exists and reuse surface

- Weapon data: `Content/Data/Weapons.csv` has 192 rows total: 12 heroes * 16 rows each. The Hero 1 rows show 4 branches per rarity across Black, Red, Yellow, and White (`Content/Data/Weapons.csv:1-18`). Each row carries weapon ID, hero ID, rarity, branch, damage/attack-speed/scale/range multipliers, and branch-specific bonuses (`Content/Data/Weapons.csv:1`).
- Idol data: `Content/Data/Idols.csv` has 12 rows total: 3 DOT, 3 Bounce, 3 AOE, and 3 Pierce idols (`Content/Data/Idols.csv:1-13`). Rows carry category, rarity icons, damage/property values, projectile speed, falloff, AOE delay/radius, and DOT timing fields (`Content/Data/Idols.csv:1`).
- Combat VFX bindings: `Content/Data/CombatVFXBindings.csv` has 4 rows, all for Hero 1 black-tier weapon-base VFX: AOE, Pierce, Bounce, and DOT (`Content/Data/CombatVFXBindings.csv:1-5`). All four set `bSuppressTemporaryProjectile=True`, so the bound Niagara system replaces the temporary projectile visual for those exact weapon rows.
- Runtime data loading: the game instance owns data table soft references for weapons and combat VFX bindings (`Source/T66/Core/T66GameInstance.cpp:165-176`) and exposes weapon lookup plus VFX binding lookup (`Source/T66/Core/T66GameInstance.cpp:885-945`).
- Attack classes/systems: there are no separate per-type hero weapon projectile classes. The reusable runtime surface is `UT66CombatComponent` category functions, `FWeaponData`/`FIdolData`, `AT66HeroProjectile` visual carriers, direct Niagara bindings, and RunState DOT application.
- Idol impact presentation: only four proof idols are impact-presentation routed today: Water/AOE, Light/Pierce, Electric/Bounce, Poison/DOT (`Source/T66/Gameplay/T66CombatShared.cpp:96-106`). These are reusable as the current proof-category paths, but they are not yet a full binding set for all idol rows.
- VFX asset form: current Hero 1 black-tier weapon bindings point to Niagara assets under `/Game/VFX/Hero1/Axe/...` with mechanism packet IDs in the CSV (`Content/Data/CombatVFXBindings.csv:2-5`). No idol modifier rows are present in the CSV.
- Projectile manager hooks: the manager is currently enemy/boss only. The managed enum has no hero or idol delivery type (`Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:19-23`), and normal hero/idol paths above do not call it.

### D2. CombatVFXBindings feed path

The four Hero 1 black-tier rows bind `WeaponBase` + exact weapon ID + attack category to a Niagara system, effect packet ID, VFX profile, suppression flag, radius, playback seconds, and scale (`Content/Data/CombatVFXBindings.csv:1-5`). Runtime lookup flows as follows:

1. `UT66GameInstance` loads `/Game/Data/DT_CombatVFXBindings` (`Source/T66/Core/T66GameInstance.cpp:167-168`).
2. `GetCombatVFXBindingData` scans rows by `SourceType`, `SourceID`, and `AttackCategory` (`Source/T66/Core/T66GameInstance.cpp:890-920`).
3. `UT66CombatComponent::ResolveCombatVFXBinding` loads the Niagara system synchronously from the binding row (`Source/T66/Gameplay/T66CombatComponent.cpp:1060-1097`).
4. `ShouldSuppressWeaponBaseProjectileVisual` checks the same binding and returns `Binding.bSuppressTemporaryProjectile` (`Source/T66/Gameplay/T66CombatComponent.cpp:1099-1114`).
5. The category functions then use the resolved binding to spawn bound weapon VFX instead of the temporary visual projectile for the rows that suppress the projectile visual (`Source/T66/Gameplay/T66CombatComponent.cpp:2334-2339`, `Source/T66/Gameplay/T66CombatComponent.cpp:2407`, `Source/T66/Gameplay/T66CombatComponent.cpp:2546-2595`, `Source/T66/Gameplay/T66CombatComponent.cpp:2674-2692`).

## Group E - Performance Harness

### E1. Enemywaveperf capture harness

- `Scripts/CaptureT66GameplayVideo.ps1` exposes capture mode, output paths, resolution, frame count/rate, capture interval, start delay, post-capture delay, timeout, exec commands, and extra args (`Scripts/CaptureT66GameplayVideo.ps1:1-45`).
- The script forwards `-T66GameplayAutoCapture=<mode>`, frame sequence options, screenshot delay, post-capture delay, automation resolution, and any `ExtraArgs` to Unreal (`Scripts/CaptureT66GameplayVideo.ps1:404-430`).
- The player-controller automation path recognizes `enemywaveperf`, `mainboardenemywave`, and `phaseaperf`, then applies command-line knobs such as `T66MobManagerTickProfileEnabled`, `T66RangedDiagnosticLogging`, and `T66AutoCaptureHeroHPOverride`; it resets mob/ranged and projectile diagnostics at capture start (`Source/T66/Gameplay/T66PlayerController_Overlays.cpp:2787-2845`).
- In non-shipping builds, hero death during `enemywaveperf` requests process exit with `T66EnemyWavePerfHeroDied` (`Source/T66/Gameplay/T66PlayerController_Overlays.cpp:6848-6853`).
- Metrics recorded today include frame time, average FPS, 1 percent low, 0.1 percent low, standard deviation, memory used/peak/total physical memory, hardware CPU/GPU brand when enabled, PerformanceSystem substep timings, write-queue timings, live mob counts, pool counters, active enemy projectiles, and projectile-manager diagnostics (`Source/T66/PerformanceSystem/T66PerformanceSubsystem.h:27-60`, `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:1465-1488`, `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:1728-1788`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h:71-95`).
- GPU frame time, draw calls, thermals, and VRAM pressure are not recorded by the current PerformanceSystem. VRAM pressure is explicitly serialized as `Unavailable` with no shipping-safe adapter bound (`Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:1758-1767`).
- The harness can be pointed at a projectile-saturation scenario by adding a new or existing capture mode and passing it as `-CaptureMode`; the script and automation parser already route modes through `-T66GameplayAutoCapture=<mode>` (`Scripts/CaptureT66GameplayVideo.ps1:5`, `Scripts/CaptureT66GameplayVideo.ps1:414`, `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:2787`). No existing projectile-only saturation mode was found in the current searched paths.

### E2. Existing per-projectile cost and full-resolution baseline data

The existing full-resolution sanity capture recorded 146.30 FPS, 72.33 1 percent low, 50.12 0.1 percent low, peak 90 live regular enemies, peak 87 lightweight mobs, peak 2 enemy projectiles, and PerformanceSystem overhead max 878.3 us (`Saved/Codex/Performance/RetroFXOffByDefaultFix/full_res_enemywaveperf_result.json:14-22`). The same result captured projectile-manager summary values: ActivePeak 2, Fired 111, HitHero 109 (`Saved/Codex/Performance/RetroFXOffByDefaultFix/full_res_enemywaveperf_result.json:101-112`).

The B.13 full-resolution before-baseline superseded the single 146.30 FPS sanity read for that pass and recorded 3/3 accepted captures, 189.65 median FPS, 156.16 median 1 percent low, 72.03 median 0.1 percent low, and max PerformanceSystem overhead 1237.8 us (`PerformanceSystem/B13_MobInstancedRendering_Audit.md:62-76`; the supersession note is in `Reports/AgentReviews/20260529_B13_MobHISM_VAT/plan_packet.md:224-233`). This is historical capture evidence, not a fresh run performed in this read-only pass.


</review_packet>
