# Miniboss Special Boss Spawn and Integration Audit

Date: 2026-05-28  
Scope: documentation-only audit of current miniboss, special, and boss spawn logic plus integration with the lightweight mob manager, projectile manager, actor registry, HUD/minimap, damage attribution, and pooling.  
No runtime code, data, build output, or capture harness behavior was changed for this audit.

## Executive Summary

The current tier model is not a clean data hierarchy. Basic mobs are data rows in `Content/Data/Enemies.csv`; minibosses are a runtime modifier applied to one regular mob slot; specials are mostly hardcoded class paths; bosses are data-driven by `Stages.csv`, `Bosses.csv`, and boss encounter tables.

Key findings:

1. Miniboss promotion is family-neutral. The director chooses one regular mob slot for miniboss promotion before the final `MobID` is rolled, then the final `MobID` can resolve to Melee, Rush, Flying, or Ranged (`Source/T66/Gameplay/T66EnemyDirector.cpp:1175`, `Source/T66/Gameplay/T66EnemyDirector.cpp:1494`). This explains why a Ranged-family basic mob can appear on the rich path during CVar-on runs.
2. Minibosses deliberately route rich today. `ShouldRouteSpawnToLightweightMob` returns false when `bIsMiniBoss` is true (`Source/T66/Gameplay/T66EnemyDirector.cpp:724`). That is coherent with the current code, but it makes the basic-mob performance capture contract ambiguous unless miniboss and special routes are defined as included or excluded.
3. Director specials are Goblin Thief only. Goblin special slots are added per wave by chance, have no `MobID`, route rich, and are counted as special/unknown in route attribution (`Source/T66/Gameplay/T66EnemyDirector.cpp:1121`, `Source/T66/Gameplay/T66EnemyDirector.cpp:1161`, `Source/T66/Gameplay/T66EnemyDirector.cpp:1494`).
4. Normal rich Ranged, lightweight Ranged, and bosses now fire enemy-to-hero projectiles through `UT66ProjectileManagerSubsystem`; unique debuff specials still use an actor projectile class (`Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp:223`, `Source/T66/Gameplay/T66MobBase.cpp:802`, `Source/T66/Gameplay/T66BossBase.cpp:955`, `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:139`).
5. Actor registry integration is split: rich enemies and lightweight mobs feed live enemy/minimap flows; bosses register separately and drive HUD boss bars through RunState, not enemy marker caches (`Source/T66/Core/T66ActorRegistrySubsystem.cpp:95`, `Source/T66/Core/T66ActorRegistrySubsystem.cpp:147`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp:394`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp:591`).

## Source Authority Map

Current regular enemy roster authority:

- `FT66EnemyData` defines `EnemyID`, `FamilyID`, `RoleID`, `ModelStatus`, `Archetype`, `Feeling`, `Rarity`, and `StageTag` (`Source/T66/Data/T66DataTypes.h:1300`).
- `Content/Data/Enemies.csv` contains 50 regular enemy rows and no explicit miniboss, special, or boss tier column (`Content/Data/Enemies.csv:1`).
- `FT66EnemyFamilyResolver` hardcodes runtime family resolution by `MobID`; unknown rows resolve to `Special` (`Source/T66/Gameplay/Enemies/T66EnemyFamilyResolver.cpp:21`).

Current boss roster authority:

- `FBossData` defines boss identity, encounter role, optional `BossClass`, HP, awaken distance, movement, projectile speed, projectile damage, score, and boss part profile (`Source/T66/Data/T66DataTypes.h:1212`).
- `FT66BossEncounterData` defines stage-level encounter type and primary boss (`Source/T66/Data/T66DataTypes.h:1403`).
- `FT66BossEncounterMemberData` defines multi-boss encounter members and offsets (`Source/T66/Data/T66DataTypes.h:1447`).
- `FStageData` links each stage to `BossID`, `BossEncounterID`, `bBossOnlyFinale`, boss spawn location, and up to ten regular enemy IDs (`Source/T66/Data/T66DataTypes.h:1478`).

## Current Spawn Logic

### Minibosses

Current trigger:

- Runtime trickle waves may promote one regular mob slot to miniboss if no active miniboss exists (`Source/T66/Gameplay/T66EnemyDirector.cpp:1175`).
- The chance is `MiniBossChancePerWave`, defaulted to `0.10f` in the director header (`Source/T66/Gameplay/T66EnemyDirector.h:60`).
- The promotion selects one index from the current wave's regular mob slots, not from special slots (`Source/T66/Gameplay/T66EnemyDirector.cpp:1197`).

Current selection:

- The wave plan first adds regular mob class slots and Goblin special slots, then shuffles the plan (`Source/T66/Gameplay/T66EnemyDirector.cpp:1161`, `Source/T66/Gameplay/T66EnemyDirector.cpp:1168`).
- After the miniboss slot is chosen, final `MobID` is rolled from the stage's `MobIDs` list (`Source/T66/Gameplay/T66EnemyDirector.cpp:1494`).
- Because final `MobID` is chosen after slot promotion, miniboss promotion is family-neutral. A promoted slot can become any family available in that stage's regular roster.

Current routing:

- `ShouldRouteSpawnToLightweightMob` rejects minibosses before family routing checks, so minibosses always stay rich even when `T66.Mob.UseLightweight=1` (`Source/T66/Gameplay/T66EnemyDirector.cpp:724`).
- The route attribution helper records this as `RoutedRich_MiniBossPromotion` for `bIsMiniBoss` (`Source/T66/Gameplay/T66EnemyDirector.cpp:132`).
- Applied miniboss behavior is scalar-only: HP, damage, and actor scale are multiplied, and `ActiveMiniBoss` is assigned (`Source/T66/Gameplay/T66EnemyDirector.cpp:1710`).

Data-driven status:

- There is no explicit miniboss roster table. `Enemies.csv` has a `Feeling=MiniBossFeel` label on some rows, but current runtime promotion is not restricted to those rows (`Content/Data/Enemies.csv:7`, `Content/Data/Enemies.csv:17`, `Content/Data/Enemies.csv:27`, `Content/Data/Enemies.csv:37`, `Content/Data/Enemies.csv:43`, `Content/Data/Enemies.csv:47`).

### Specials

Director special:

- The director owns a hardcoded `GoblinThiefClass` described as a special (`Source/T66/Gameplay/T66EnemyDirector.h:54`).
- Runtime waves roll a Goblin wave chance from RNG tuning, then roll count and clamp it to available wave slots (`Source/T66/Gameplay/T66EnemyDirector.cpp:1121`).
- Goblin slots are appended to the wave plan and shuffled with regular mob slots (`Source/T66/Gameplay/T66EnemyDirector.cpp:1161`).
- A Goblin slot is treated as not a stage mob: `MobID=NAME_None`, family defaults to `Special`, and routing stays rich (`Source/T66/Gameplay/T66EnemyDirector.cpp:1494`, `Source/T66/Gameplay/T66EnemyDirector.cpp:1578`).

Goblin behavior:

- `AT66GoblinThiefEnemy` sets `EnemyFamily=Special`, changes visuals by rarity, deals no heart damage, and steals gold on touch (`Source/T66/Gameplay/T66GoblinThiefEnemy.cpp:26`, `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp:45`, `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp:132`).
- It inherits `AT66EnemyBase`, so it registers as a rich enemy and participates in rich enemy registry/minimap flows.

Other special-like enemies:

- `AT66UniqueDebuffEnemy` sets `EnemyFamily=Special`, starts a firing timer, and spawns `AT66UniqueDebuffProjectile` actor projectiles (`Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:48`, `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:102`, `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:120`).
- Current source references this enemy from lab/test spawning paths and gameplay cleanup/lag-tracker utilities, not from the standard director wave special path (`Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:151`, `Source/T66/Core/T66LagTrackerSubsystem.cpp:373`).
- No standard director or stage progression production spawn path for `AT66UniqueDebuffEnemy` was found in the inspected source. Treat it as lab/latent until a gameplay trigger is identified.

### Bosses

Stage boss trigger:

- Stage bootstrap calls `SpawnBossForCurrentStage` during main map stage preparation and standard combat bootstrap (`Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:478`, `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:730`).
- Main-map terrain regeneration can also respawn the current stage boss (`Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:1031`).

Stage boss selection:

- `SpawnBossForCurrentStage` loads `FStageData` for the current stage or builds a fallback stage config (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:684`, `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:698`).
- If `BossEncounterID` is set, the game resolves boss encounter members; otherwise it falls back to `StageData.BossID`; if neither exists, it uses a generated fallback ID (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:728`).
- The first encounter boss becomes `StageBoss`, and additional encounter members are spawned as separate bosses (`Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:799`, `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:879`).
- Cowardice gates can defer skipped stage bosses into `RunState` owed-boss state; owed bosses spawn on the selected difficulty's final floor (`Source/T66/Gameplay/T66CowardiceGate.cpp:94`, `Source/T66/Core/RunState/T66RunStateSubsystem_TimersBoss.cpp:60`, `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:716`, `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:918`).

Boss activation:

- Bosses register with `UT66ActorRegistrySubsystem` in `BeginPlay` and unregister in `EndPlay` (`Source/T66/Gameplay/T66BossBase.cpp:1402`, `Source/T66/Gameplay/T66BossBase.cpp:1435`).
- Boss gate overlap awakens every registered boss and pauses director spawning (`Source/T66/Gameplay/T66BossGate.cpp:145`).
- Boss part state is pushed to RunState for boss HUD bars (`Source/T66/Gameplay/T66BossBase.cpp:474`).

Casino special boss:

- The casino/gambler flow has a separate non-stage boss path. `UT66CasinoGamblerTabWidget::TriggerGamblerBossIfAngry` calls the player controller path first, then has its own fallback direct spawn of `AT66GamblerBoss` (`Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Economy.cpp:822`, `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Economy.cpp:865`).
- `AT66PlayerController::TriggerCasinoBossIfAngry` also directly spawns `AT66GamblerBoss` when anger is full and no registered Gambler boss exists (`Source/T66/Gameplay/T66PlayerController_Overlays.cpp:5070`, `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:5137`).

### Non-Director Spawn Paths

Tutorial:

- Tutorial final arena spawns one mini-boss and three normal enemies using rich `SpawnActor<AT66EnemyBase>` path (`Source/T66/Gameplay/T66TutorialManager.cpp:513`, `Source/T66/Gameplay/T66TutorialManager.cpp:686`).

Lab/test room:

- Lab mode can spawn regular mobs, Goblin Thief, Unique Debuff Enemy, and bosses directly (`Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:151`, `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:204`).
- Lab mob spawns record route attribution as `RoutedRich_NonDirectorPath` (`Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp:193`).

Elite tier search:

- No implemented elite-tier routing or promotion path was found in current Gameplay/Core/Data searches for `Elite`, `ElitePromotion`, `bIsElite`, or `EliteChance`. The only implemented non-basic regular-mob promotion surfaced by this audit is miniboss promotion.

## Roster Inventory

### Miniboss Inventory

There is no authored miniboss table. Any regular stage mob row can become a miniboss if its wave slot is promoted. The rows currently carrying `Feeling=MiniBossFeel` are design-labeled but not enforced by the promotion logic:

| Row | Family | Source |
| --- | --- | --- |
| TombSpider | Melee | `Content/Data/Enemies.csv:7` |
| TreantAncient | Melee | `Content/Data/Enemies.csv:17` |
| AnglerfishStalker | Melee/Stutterer archetype | `Content/Data/Enemies.csv:27` |
| PlasmaSentinel | Ranged | `Content/Data/Enemies.csv:37` |
| BoneKnight | Melee | `Content/Data/Enemies.csv:43` |
| DemonSentinel | Melee/Stutterer archetype | `Content/Data/Enemies.csv:47` |

Ranged implication: because `PlasmaSentinel` has both `FamilyID=Ranged` and `Feeling=MiniBossFeel`, it is an obvious design candidate for a Ranged-feeling miniboss, but the current runtime can also promote any other Ranged stage mob such as `HexSlinger`, `StoneSentinel`, `BoneConjurer`, `BrimstoneMortar`, or `PlagueCultist` if they are in the current stage roster (`Content/Data/Enemies.csv:6`, `Content/Data/Enemies.csv:8`, `Content/Data/Enemies.csv:10`, `Content/Data/Enemies.csv:48`, `Content/Data/Enemies.csv:50`).

### Special Inventory

| Special | Spawn owner | Projectile behavior | Notes |
| --- | --- | --- | --- |
| Goblin Thief | Enemy director wave special | None; steals gold on touch | Hardcoded class, `EnemyFamily=Special`, not in `Enemies.csv` as a row (`Source/T66/Gameplay/T66EnemyDirector.h:54`, `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp:26`). |
| Unique Debuff Enemy | Lab/test and utility referenced special | Actor projectile `AT66UniqueDebuffProjectile` | Sets `EnemyFamily=Special`, uses its own projectile class, not routed through projectile manager (`Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:48`, `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:139`). |
| Gambler Boss | Casino anger path | Boss projectile behavior via `AT66BossBase` inheritance | Direct-spawn special boss outside stage boss tables (`Source/T66/Gameplay/T66PlayerController_Overlays.cpp:5070`, `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Economy.cpp:822`). |

### Boss Inventory

`Content/Data/Bosses.csv` defines 23 boss rows. Every row includes `FireIntervalSeconds`, `ProjectileSpeed`, `ProjectileDamageHearts`, and `BossPartProfile` fields (`Content/Data/Bosses.csv:1`).

| Stage range | Bosses | Encounter notes | Source |
| --- | --- | --- | --- |
| Dungeon 1-4 | Sewer Slime King, Web Matriarch, Bone Jailer, Bael Fallen Chad | Stage 4 is `FinaleChad` and boss-only finale | `Content/Data/Bosses.csv:2`, `Content/Data/Bosses.csv:3`, `Content/Data/Bosses.csv:4`, `Content/Data/Bosses.csv:5`, `Content/Data/BossEncounters.csv:5` |
| Forest 5-8 | Bramble Treant, Myconid Queen, Thorn Hive, Buer Verdant Chad | Stage 8 is `FinaleChad` and boss-only finale | `Content/Data/Bosses.csv:6`, `Content/Data/Bosses.csv:7`, `Content/Data/Bosses.csv:8`, `Content/Data/Bosses.csv:9`, `Content/Data/BossEncounters.csv:9` |
| Ocean 9-12 | Reef Crab Colossus, Abyssal Jellyfish, Drowned Captain, Focalor Drowned Chad | Stage 12 is `FinaleChad` and boss-only finale | `Content/Data/Bosses.csv:10`, `Content/Data/Bosses.csv:11`, `Content/Data/Bosses.csv:12`, `Content/Data/Bosses.csv:13`, `Content/Data/BossEncounters.csv:13` |
| Martian 13-16 | Red Sand Behemoth, Crystal Mantis, Plasma Saucer Prime, Stolas Astral Chad | Stage 16 is `FinaleChad` and boss-only finale | `Content/Data/Bosses.csv:14`, `Content/Data/Bosses.csv:15`, `Content/Data/Bosses.csv:16`, `Content/Data/Bosses.csv:17`, `Content/Data/BossEncounters.csv:17` |
| Hell 17-20 | Four Horsemen, False Prophet, Antichrist, Great Dragon | Stage 17 is `MultiBoss`; Stage 20 is `ApocalypseFinale` and boss-only finale | `Content/Data/Bosses.csv:18`, `Content/Data/Bosses.csv:19`, `Content/Data/Bosses.csv:20`, `Content/Data/Bosses.csv:21`, `Content/Data/Bosses.csv:22`, `Content/Data/Bosses.csv:23`, `Content/Data/Bosses.csv:24`, `Content/Data/BossEncounters.csv:18`, `Content/Data/BossEncounters.csv:21` |

Stage examples:

- Stage 1 links Dungeon mobs and `Dungeon_SewerSlimeKing` / `Encounter_Stage_01` (`Content/Data/Stages.csv:2`).
- Stage 4 is boss-only finale with `Dungeon_BaelFallenChad` (`Content/Data/Stages.csv:5`).
- Stage 17 is the Four Horsemen multi-boss encounter (`Content/Data/Stages.csv:18`, `Content/Data/BossEncounterMembers.csv:18`).
- Stage 20 is the Great Dragon boss-only finale (`Content/Data/Stages.csv:21`, `Content/Data/BossEncounterMembers.csv:24`).

## Infrastructure Integration Matrix

| Tier/path | Projectile firing | Actor registry | HUD/minimap | Damage attribution | Pooling | Lightweight coexistence |
| --- | --- | --- | --- | --- | --- | --- |
| Basic rich Ranged | Uses `UT66ProjectileManagerSubsystem::FireProjectile` (`Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp:223`) | Registers as rich enemy (`Source/T66/Gameplay/T66EnemyBase.cpp:630`) | Included in minimap enemy cache (`Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp:394`) | `AT66EnemyBase` source resolves to `MobID` (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:47`) | Director can acquire/release via exact-class enemy pool (`Source/T66/Gameplay/T66EnemyDirector.cpp:1644`, `Source/T66/Core/T66EnemyPoolSubsystem.cpp:10`) | Rich path remains available for CVar-off and excluded routes |
| Basic lightweight Ranged | Uses same projectile manager (`Source/T66/Gameplay/T66MobBase.cpp:802`) | Registers as lightweight mob (`Source/T66/Gameplay/T66MobBase.cpp:148`) | Included in minimap mob cache (`Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp:403`) | `AT66MobBase` source resolves to `MobID` (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:52`) | Managed by mob manager pool, not enemy pool | Intended CVar-on path for basic mobs |
| Runtime miniboss | Uses the rich class for its resolved family; Ranged miniboss uses rich Ranged manager path | Registers as rich enemy | Included as enemy marker, not distinct miniboss marker | Resolves as `MobID` because it is a rich enemy | Director rich path can use enemy pool | Always rich due to `bIsMiniBoss` routing rejection |
| Goblin Thief special | No projectile; gold steal on overlap | Registers as rich enemy through `AT66EnemyBase` | Included as enemy marker | No heart damage; gold spend has no damage source path | Director special path can use enemy pool | Always rich special |
| Unique Debuff special | Uses `AT66UniqueDebuffProjectile`, actor/projectile-movement path (`Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:139`) | Registers as rich enemy | Included as enemy marker if spawned in gameplay registry | Damage call uses owner, but owner `MobID` is likely none, so damage log source falls back to `Enemy` (`Source/T66/Gameplay/T66UniqueDebuffProjectile.cpp:212`, `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:47`) | Direct actor path in lab; no director special pool path identified | Separate from basic lightweight manager |
| Stage/encounter/owed bosses | Uses `UT66ProjectileManagerSubsystem::FireBossProjectile`; `AT66BossProjectile` is deprecated compatibility code (`Source/T66/Gameplay/T66BossBase.cpp:955`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp`) | Registers in separate boss array (`Source/T66/Gameplay/T66BossBase.cpp:1402`) | Boss bar via RunState; minimap enemy cache does not include bosses (`Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp:591`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Map.cpp:394`) | `AT66BossBase` source resolves to `BossID` and manager damage uses `Delivery=BossProjectile` (`Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp:57`) | Direct spawned; no `BossPool*`, `AcquireBoss*`, `TryAcquireBoss*`, or `ReleaseBoss*` path was found in current source searches | Independent of lightweight mob routing |
| Gambler boss | Boss inheritance path; direct boss actor spawn | Registers as boss via `AT66BossBase` | Boss bar follows RunState boss state; spawn path is separate from stage boss flow | Boss source resolves as `GamblerBoss` if `BossID` is set (`Source/T66/Gameplay/T66GamblerBoss.cpp:16`) | Direct spawned; no pool | Independent special boss |
| Tutorial/lab non-director enemies | Rich actor spawns | Rich enemy registry | Enemy markers if spawned in active gameplay world | Rich enemy source IDs if `MobID` configured | No pool for tutorial/lab direct spawns observed | Explicit non-director rich paths |

## Gaps and Risks

### [Major] Miniboss Promotion Is Family-Neutral and Runtime-Only

What is wrong: there is no authored miniboss roster or per-stage miniboss rule. A wave slot is promoted first, then final `MobID` is rolled. This can create rich Ranged minibosses during CVar-on basic-mob captures (`Source/T66/Gameplay/T66EnemyDirector.cpp:1175`, `Source/T66/Gameplay/T66EnemyDirector.cpp:1494`).

Why it matters: basic-mob acceptance can reject on `RichSpawns` even when no basic route leak exists, because the route is a planned rich miniboss route.

Deferred follow-up would entail: choose whether minibosses are excluded from `enemywaveperf`, counted as expected rich routes, or authored as deliberate encounter spawns with an explicit miniboss roster.

### [Major] `Feeling=MiniBossFeel` Is Not Enforced

What is wrong: several `Enemies.csv` rows are tagged with `MiniBossFeel`, but the director does not restrict miniboss promotion to those rows.

Why it matters: design intent in data and runtime behavior can diverge silently.

Deferred follow-up would entail: either remove the misleading label from spawn semantics, or make miniboss selection use an explicit eligible row list / data flag.

### [Major] Specials Are Mostly Hardcoded, Not Data-Driven

What is wrong: Goblin Thief is hardcoded in the director, Unique Debuff Enemy is class-based, and Gambler Boss is direct-spawned from UI/player-controller paths. There is no unified special roster table.

Why it matters: "special" does not currently mean one thing across gameplay systems.

Deferred follow-up would entail: define a special-tier data model or explicitly document that Goblin, Unique Debuff, and Gambler Boss are separate systems.

### [Major] Unique Debuff Projectiles Still Bypass the Projectile Manager

What is wrong: basic enemy projectiles and boss projectiles are now manager/HISM owned, while unique debuff projectiles are still an actor/projectile-movement system (`Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp:139`).

Why it matters: the remaining projectile architecture split is now limited to the latent/lab-only Unique Debuff special path. Boss projectile firing, rendering, damage attribution, and HISM capacity accounting are unified with the enemy projectile manager.

Deferred follow-up would entail: decide whether the Unique Debuff special path should stay actor-owned because it is lab/latent, or add a special projectile type to `UT66ProjectileManagerSubsystem`.

### [Minor] Bosses Are Not Part of General Damageable Target Queries

What is wrong: `ForEachDamageableTarget` iterates rich enemies and lightweight mobs, but not boss registry entries (`Source/T66/Core/T66ActorRegistrySubsystem.cpp:123`).

Why it matters: systems using registry damageable targets will ignore bosses unless they have separate boss logic.

Deferred follow-up would entail: audit callers of `ForEachDamageableTarget` / `GetAllDamageableTargets` and choose whether bosses belong there or require separate boss-target APIs.

### [Minor] Bosses Register Separately and Do Not Broadcast `EnemiesChanged`

What is wrong: enemy and mob registration broadcasts `EnemiesChanged`; boss registration does not (`Source/T66/Core/T66ActorRegistrySubsystem.cpp:46`, `Source/T66/Core/T66ActorRegistrySubsystem.cpp:64`, `Source/T66/Core/T66ActorRegistrySubsystem.cpp:147`).

Why it matters: current minimap caches ignore bosses, so this is harmless for existing map behavior, but any future map or target cache expecting boss changes from the same event will miss them.

Deferred follow-up would entail: keep bosses deliberately separate, or add a boss-changed broadcast and update map/target consumers.

### [Minor] Casino Gambler Boss Has Two Direct Spawn Paths

What is wrong: the widget calls the player-controller spawn path first, then has a fallback direct spawn path of its own (`Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Economy.cpp:822`, `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:5070`).

Why it matters: duplicate prevention exists through registered boss checks, but spawn ownership is split.

Deferred follow-up would entail: centralize Gambler boss spawn ownership in one gameplay path and make UI call only that path.

## Deferred Decision Areas

Future restructure decision areas:

1. Define the intended tier taxonomy in data/process terms: basic, elite modifier if it exists, miniboss, special, boss.
2. Decide `enemywaveperf` contract for non-basic tiers: disable miniboss/special spawns, or accept and separately count expected rich routes.
3. Replace family-neutral miniboss promotion with a deliberate rule: stage trigger, eligible roster, fixed encounter, wave threshold, or explicit data row.
4. Choose whether remaining special projectiles, specifically Unique Debuff, remain actor-owned or move into `UT66ProjectileManagerSubsystem` with separate projectile type definitions.
5. Centralize special spawn ownership if Goblin, Unique Debuff, and Gambler Boss are intended to share a tier.

Lightweight actor acceptance contract ambiguity:

- `RoutedRich_MiniBossPromotion` and `RoutedRich_SpecialOrMiniBoss` are expected only when the capture contract includes miniboss/special tiers.
- For a basic-mob-only CVar-on acceptance contract, interpreting `RichSpawns` as actual route leakage depends on an explicit miniboss/special exclusion or filter.

## Verification Performed

- Read project router instructions from `AGENTS.md` supplied in the session.
- Read performance instructions from `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.
- Inspected live source for director spawn logic, non-director tutorial/lab spawns, boss flow, boss gate activation, casino boss spawn, projectile paths, actor registry, HUD/minimap, damage attribution, and pooling.
- Searched current Gameplay/Core/Data source for elite-tier terms; no implemented elite promotion or routing tier was found.
- Searched current source for boss-pooling terms (`BossPool`, `AcquireBoss`, `TryAcquireBoss`, `ReleaseBoss`); no boss pool path was found.
- Searched `EnemyFamily=Special` assignments; only Goblin Thief and Unique Debuff Enemy define themselves as `Special` classes in inspected gameplay source.
- Inspected live CSV data under `Content/Data/Enemies.csv`, `Content/Data/Stages.csv`, `Content/Data/Bosses.csv`, `Content/Data/BossEncounters.csv`, and `Content/Data/BossEncounterMembers.csv`.
- No compile, cook, staged build, or runtime capture was run because this pass is audit-only and made no production behavior changes.
