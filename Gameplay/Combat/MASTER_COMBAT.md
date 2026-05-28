# T66 Master Combat

**Last updated:** 2026-05-28
**Scope:** Single-source handoff for combat runtime flow, targeting, damage routing, damage provenance logging, combat collision roles, debug visibility, hit feedback, spatial headshots, accuracy-driven aiming, and boss body-part combat.
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Backend/Anti Cheat/ANTI_CHEAT_POLICY_REFERENCE.md`, `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
**Maintenance rule:** Update this file after every material combat, targeting, damage-model, hitbox, projectile, boss-health, or combat-UI change.

Combat VFX process note: start future VFX work from `VFX_PROCESS_INDEX.md` and `CombatVFXDefinitionOfDone.md`. Production-path automation proof must use real weapon selection, RunState inventory/item stats, combat fire, VFX binding lookup, and damage paths; it is not the same as literal UI-click proof unless a packet requires UI-click validation.

## 1. Executive Summary

- Combat now supports target handles with optional hit-zone selection for standard enemies and bosses.
- Standard enemies can expose `Body` and `Head` hit zones while still sharing one HP pool.
- Standard enemy combat is now split across family classes:
  - `Melee` for direct chase/touch damage
  - `Rush` for burst charge movement
  - `Ranged` for projectile spit attacks with line-of-sight gating before firing
  - `Flying` for hovering chase behavior
- Bosses can now expose multipart hit zones with per-part HP snapshots and per-part HUD bars.
- Combat now has an explicit debug-draw role taxonomy:
  - `Hurtbox`: targetable enemy/boss hit zones.
  - `PlayerHurtbox`: the hero capsule that receives damage overlaps/queries.
  - `DamageVolume`: a primitive or query shape that can remove HP from a hero, enemy, or boss.
  - movement blockers, sensors, interact triggers, and visual-only actors are not part of the default combat debug view.
- Non-shipping builds now default to showing both combat hitboxes and damage volumes through `T66.Combat.DebugView=3`; Shipping builds default to `0`.
- Every successful hero HP loss now writes one `[CombatDamage]` line to the Unreal log from the shared `UT66RunStateSubsystem::ApplyDamage(...)` path.
- Backrooms chaser contact uses delivery method `BackroomsChaserTouch`. It bypasses Quick Revive, Saint Blessing, evasion, and invulnerability gates, restores the temporarily hidden inventory/weapon first, then kills the hero through the shared damage path.
- Last Stand, Survival Charge, and the legacy downed Quick Revive flow are removed from live lethal-save behavior. Reflected compatibility wrappers remain inert for old Blueprint/API callers. The only current quick-revive behavior is the reward-only `Item_BackroomsQuickRevive` inventory item, consumed by normal lethal damage for a one-heart revive.
- Unique unkillable chasers are authored through `Content/Data/UniqueEnemies.csv` and `/Game/Data/DT_UniqueEnemies`; the first row is `BackroomsChaser`, currently visualized with the existing `Slime` character visual.
- Hostile projectile damage now requires overlap with the actual hero capsule hurtbox. Hero-owned sensors such as the combat range sphere are logged as `RejectedNonHeroHurtbox` and cannot remove HP.
- Enemy, trap, unique/debuff, and boss projectile paths now emit `[ProjectileFired]` and `[ProjectileImpact]` logs with projectile/component locations, overlapped component identity, sweep/impact points, velocity, owner, source, and damage result.
- Temporary projectile presentation is centralized in `Source/T66/Gameplay/T66TemporaryProjectileSystem.*` so hero, idol, enemy, and trap ranged attacks use clear shape profiles until authored models replace them. Non-boss team colors are intentionally constrained: hero/idol projectiles are blue, enemy/trap projectiles are red.
- Hero auto-attacks now require an unblocked world-static line from the hero attack origin to the selected combat target handle before they fire. This prevents attacks, visual projectiles, pierce hits, AOE splash, and bounce chains from resolving through dungeon walls or around blocked corners.
- Hero 1 AOE axe attacks now resolve secondary splash targets through a target-anchored 180-degree frontal sector query, oriented from the hero toward the primary target, instead of an omnidirectional target-centered sphere. The primary target is still damaged through the normal target handle path.
- `Accuracy` now exists as a full primary hero stat and its secondary family feeds untargeted auto-attack head selection.
- The existing `Headshot` passive is no longer a random proc; it currently adds `+20%` accuracy weighting.
- `Alchemy`, `HpRegen`, and `LifeSteal` are now deprecated legacy secondary stats kept only for save/data compatibility and old authored rows.
- Adding real headshots and accuracy requires a model change from:
  - `AActor` target
  - one HP pool per enemy/boss
  - actor-center bounce logic
  - actor-root projectile homing
- The clean target model is:
  - actor selection
  - optional hit-zone selection inside that actor
  - damage routing through a hit-zone result
  - optional per-part HP for bosses

## 2. Current Combat Runtime Spine

### 2.1 Data and stat schema

- `Source/T66/Data/T66DataTypes.h`
  - Defines hero attack category, passives, hero/base combat data, boss data, item line schemas, and secondary stat types.
- Current hero/item stat schema now includes:
  - primary `Accuracy` hero stat data
  - `BaseAccuracy` as the combat head-targeting baseline
  - Accuracy-family secondaries:
    - `CritChance`
    - `CritDamage`
    - `AttackRange`
    - `Accuracy`
- Deprecated live-stat removals:
  - `Alchemy`
  - `HpRegen`
  - `LifeSteal`
- Deprecated secondary stats still remain in enums and legacy rows so older saves and payloads can still deserialize safely.
- Current boss schema now includes `BossPartProfile` for generic stage-boss layout authoring.
- `Content/Data/Bosses.csv`
  - Generic stage bosses now author multipart layout through `BossPartProfile`.
  - Current shipped profiles are:
    - `HumanoidBalanced`
    - `Sharpshooter`
    - `Juggernaut`
    - `Duelist`
  - Runtime safety fallback:
    - if the loaded `Bosses` data table asset is stale and `BossPartProfile` still resolves to `UseActorDefault`, `AT66BossBase` infers a profile from `BossID` for generic stage bosses.

### 2.2 Runtime combat state

- `Source/T66/Core/T66RunStateSubsystem.h/.cpp`
  - Owns derived hero stats, secondary stat multipliers, DOT application, passive queries, and the current boss HP state used by the HUD.
  - Boss state now includes aggregate HP plus `BossPartSnapshots`, which preserves multipart boss HP across save/load and HUD refreshes.
  - `GetAccuracyStat()` now resolves a real primary stat alongside Damage / Attack Speed / Attack Scale / Armor / Evasion / Luck.
  - `GetAccuracyChance01()` now resolves a real combat value from hero base accuracy, Accuracy-family multipliers, and the `Headshot` passive bonus.

### 2.2A Current primary and secondary stat map

- `Damage`
  - `AoeDamage`
  - `BounceDamage`
  - `PierceDamage`
  - `DotDamage`
- `AttackSpeed`
  - `AoeSpeed`
  - `BounceSpeed`
  - `PierceSpeed`
  - `DotSpeed`
- `AttackScale`
  - `AoeScale`
  - `BounceScale`
  - `PierceScale`
  - `DotScale`
- `Accuracy`
  - `CritChance`
  - `CritDamage`
  - `AttackRange`
  - `Accuracy`
- `Armor`
  - `Taunt`
  - `DamageReduction`
  - `ReflectDamage`
  - `Crush`
- `Evasion`
  - `EvasionChance`
  - `CounterAttack`
  - `Invisibility`
  - `Assassinate`
- `Luck`
  - `TreasureChest`
  - `Cheating`
  - `Stealing`
  - `LootCrate`

### 2.3 Auto attack and damage dispatch

- `Source/T66/Gameplay/T66CombatComponent.h/.cpp`
  - Owns auto-attack heartbeat, target acquisition, pierce/slash/bounce/DOT behavior, crit resolution, range modifiers, idol procs, and damage dispatch.
  - Primary target resolution now works in `FT66CombatTargetHandle` space:
    - locked target handle if valid
    - otherwise closest valid target handle in `EnemiesInRange`
  - Damage is routed by hit-zone-aware actor handlers:
    - `AT66EnemyBase::TakeDamageFromHeroHitZone(...)`
    - `AT66BossBase::TakeDamageFromHeroHitZone(...)`
  - Bounce and chain-lightning style effects can now chain between boss parts instead of only actor centers.

### 2.4 Mouse targeting and scoped firing

- `Source/T66/Gameplay/T66PlayerController.h`
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp`
- `Source/T66/Gameplay/T66PlayerController_ScopedUlt.cpp`
  - Manual lock now stores a generic combat actor plus hit-zone intent/state.
  - Attack-lock traces still use `ECC_Visibility`, but can now lock `UT66CombatHitZoneComponent` targets on standard enemies and bosses.
  - Arthur's ultimate is now a single crosshair-driven line attack: the controller deprojects the gameplay reticle into world space, the combat component damages targets along that path, and a standalone giant sword visual follows the traced route.
  - Scoped sniper shots currently trace world space, then fire a line-based ultimate against actors in the beam path.
  - Scoped/manual part-specific ult routing is still incomplete outside the shared combat-component paths.

### 2.5 Enemy runtime target surface

- `Source/T66/Gameplay/T66EnemyBase.h/.cpp`
- Enemy damage can now route through `TakeDamageFromHeroHitZone(...)` using a combat target handle.
- When combat hit zones are enabled, the root capsule ignores `ECC_Visibility` and the body/head subcomponents own the targeting trace.
  - HP is stored per enemy actor:
    - `MaxHP`
    - `CurrentHP`
  - World-space enemy UI is still one health bar plus one lock indicator.

### 2.6 Boss runtime target surface

- `Source/T66/Gameplay/T66BossBase.h/.cpp`
  - Bosses now own optional `BossPartDefinitions` that build runtime hit zones, per-part HP, and multipart damage routing.
  - Generic stage bosses can now swap among authored multipart profiles from `Bosses.csv`.
  - `AT66VendorBoss` and `AT66GamblerBoss` now override the generic profile path with bespoke part layouts tuned to their special encounters.
  - Unconfigured bosses now fall back to a default six-part layout:
    - `Core`
    - `Head`
    - `LeftArm`
    - `RightArm`
    - `LeftLeg`
    - `RightLeg`
  - Boss HP is synchronized through `UT66RunStateSubsystem` as both aggregate HP and per-part snapshots.
  - Bosses still do not own:
    - part-specific ability loss/break rules
    - part-specific destruction visuals
    - per-part victory rules beyond aggregate HP reaching zero

### 2.7 Projectile and overlap path

- `Source/T66/Gameplay/T66HeroProjectile.h/.cpp`
  - Hero projectile actors can be true damage projectiles or visual-only projectile bodies.
  - Auto-attack projectile visuals spawned by `T66CombatComponent` are normally visual-only; the combat component has already resolved the hit and damage path.
  - Auto-attack target acquisition runs `HasUnblockedAutoAttackPath(...)` before a locked or range-sphere target can fire. The trace checks world-static blockers, ignores the hero and target actor, and uses the target handle aim point rather than only the actor root.
  - Visual-only weapon shots now use the temporary projectile system:
    - `Pierce`: blue cone
    - `AOE`: blue sphere
    - `Bounce`: blue cube
    - `DOT`: blue cylinder
    - idol payloads add a smaller blue overlay shape on top of the weapon attack shape.
  - The active hero/idol auto-attack path no longer emits separate particle-only attack/idol VFX. The blue temporary projectile body and optional blue idol overlay are the readable attack presentation.
  - Non-visual-only hero projectiles still own a sphere damage volume and actor-based overlap fallback.
  - Hero 1 AOE axe splash target selection is now a query-only sector DamageVolume: a broad-phase pawn overlap around the primary impact point followed by a 2D frontal sector test. The sector is oriented from the hero attack origin toward the primary target, and secondary hits still preserve `HasUnblockedAutoAttackPath(...)`.
  - Current projectile limitations:
    - visual projectiles still home to the target actor/root rather than a hit-zone component
    - old overlap fallback paths are actor-based and default to body damage
    - hit-zone intent is still strongest in `T66CombatComponent` target handles, not in projectile payloads

### 2.8 Combat collision roles and debug visibility

The current standard is to debug combat semantics, not raw Unreal collision.

- `Hurtbox`
  - Means "where this actor can be targeted or receive hero combat damage."
  - Runtime owner:
    - `Source/T66/Gameplay/T66CombatHitZoneComponent.h/.cpp`
  - Current users:
    - enemy body/head zones in `AT66EnemyBase`
    - boss part zones in `AT66BossBase`
  - Debug color examples:
    - body: cyan/blue
    - head: yellow
    - core/weak point: pink/purple
    - inactive/dead part: grey

- `PlayerHurtbox`
  - Means "the hero body shape that receives hostile overlap/query damage."
  - Current owner:
    - `AT66HeroBase` capsule
  - Debug color:
    - bright cyan

- `DamageVolume`
  - Means "this primitive or query shape can apply damage when active."
  - Current debug-enabled examples:
    - enemy touch capsule intent in `AT66EnemyBase`
    - non-visual-only hero projectile sphere in `AT66HeroProjectile`
    - Hero 1 AOE axe sector query in `UT66CombatComponent`
    - enemy projectile sphere in `AT66EnemyProjectileBase`
    - boss projectile sphere in `AT66BossProjectile`
    - unique debuff projectile sphere in `AT66UniqueDebuffProjectile`
    - trap arrow projectile box in `AT66TrapArrowProjectile`
    - boss ground AOE sphere in `AT66BossGroundAOE`
    - boss lane blocker box in `AT66BossLaneBlockerHazard`
    - floor flame and spike patch trap spheres
    - lava patch box
    - miasma tile box
  - Debug color examples:
    - active damage: red
    - warning/telegraph shape before activation: orange

Temporary projectile visuals:

- Runtime owner:
  - `Source/T66/Gameplay/T66TemporaryProjectileSystem.h/.cpp`
- Purpose:
  - provide a shared, obvious, non-particle visual language for ranged attacks during development
  - keep temporary shape profiles centralized so they can be replaced by authored projectile meshes without rewriting every attack source
- Current users:
  - hero weapon auto-attack visuals
  - idol overlay payload visuals
  - ranged enemy spit projectiles
  - unique/debuff enemy projectiles
  - wall-arrow trap projectiles
- Current non-boss color contract:
  - hero and idol projectile profiles resolve to blue inside `FT66TemporaryProjectileSystem`
  - enemy, unique/debuff, and trap projectile profiles resolve to red inside `FT66TemporaryProjectileSystem`
  - callers may pass legacy tint values for compatibility, but known temporary projectile profiles override those values to the team color
- Hostile temporary projectile profiles are intentionally oversized relative to their damage volumes so fast enemy/trap shots remain readable before authored projectile meshes replace them.
- These visuals are presentation, not authority. Damage still comes from the owning `DamageVolume` or already-resolved combat path.
- Transient query-only DamageVolumes, such as the Hero 1 AOE axe sector, use parameter-based debug drawing rather than hidden collision components.

- `TriggerContainer`
  - Means "this trap-owned primitive does not deal damage directly, but it arms or triggers a trap that can deal damage."
  - Current debug-enabled example:
    - pressure-plate trigger box in `AT66TrapPressurePlate`
  - Debug color examples:
    - armed trigger: green
    - pressed/resetting trigger: grey-green

- `VisualOnly`
  - Means "combat-looking actor with no authority to apply damage."
  - Current example:
    - auto-attack projectile visuals spawned after `T66CombatComponent` has already resolved damage.
  - These are intentionally not drawn as damage volumes.

- Hidden by default:
  - movement/blocking colliders
  - terrain/world collision
  - generic interaction triggers
  - pickup sensors
  - camera/support volumes

Debug controls:

- `T66.Combat.DebugView`
  - `0`: off
  - `1`: hitboxes and hero hurtbox only
  - `2`: damage volumes only
  - `3`: both hitboxes and damage volumes
- `T66.Combat.DebugLabels`
  - `0`: no labels
  - `1`: label visible combat debug shapes
- `T66.Combat.DebugThickness`
  - line thickness for debug draw

Damage provenance logging:

- Runtime owner:
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp`
- Every successful `UT66RunStateSubsystem::ApplyDamage(...)` call emits one `LogT66DamageReceived` line tagged `[CombatDamage]`.
- Hostile projectile sources also emit projectile-specific logs before damage routing:
  - `[ProjectileFired]` records the projectile actor, owner/source, spawn location, direction, speed, damage shape size, and whether the temporary visual meshes are visible.
  - `[ProjectileImpact] Result=HeroHit` records the exact overlap component, projectile actor location, damage primitive location, hero location, sweep location, impact point, normal, velocity, and requested damage.
  - `[ProjectileImpact] Result=RejectedNonHeroHurtbox` records ignored overlaps against hero-owned sensors or non-hurtbox components. These events must not call `ApplyDamage(...)`.
- The line records:
  - applied HP loss after reductions
  - requested HP from the caller
  - incoming HP after difficulty/passive pre-armor modifiers
  - source ID
  - delivery method, e.g. `EnemyTouch`, `EnemyProjectile`, `TrapProjectile`, `TrapFloorBurst`, `MiasmaTile`
  - source actor name/class
  - damage causer actor/class, e.g. projectile actor when the owning enemy gets source credit
  - hero HP before/after
  - hero/source/causer world locations
  - source-to-hero and causer-to-hero 2D/3D distances
  - causer line-of-sight status and blocker name for impossible-through-wall or stale-overlap diagnosis
  - stage and world time
- Current explicitly tagged direct/abstract sources:
  - `MiasmaBoundary`
  - `MiasmaCoverage`
  - `MiasmaTile`
  - `LavaPatch`
  - `LoanShark`
  - `TutorialScriptedDamage`
- Non-shipping verification hook:
  - `-T66GameplayAutoCapture=combatdamagelog` applies one controlled 20 HP hit through `ApplyDamage(...)` so agents can prove the log line in a staged run.
  - `-T66GameplayAutoCapture=trapprojectilehitbox` spawns a stationary trap projectile so agents can verify the drawn damage box against the projectile body.
  - `-T66GameplayAutoCapture=trapcontainers` spawns representative wall projectile, floor burst, area-control, and pressure-plate trap visuals so agents can verify visual layer plus container layer together.
  - `-T66GameplayAutoCapture=hero1axeaoehitbox` equips the Hero 1 black AOE weapon, spawns fixed inside/outside proof targets, fires one real auto-attack AOE, draws the sector `DamageVolume`, and logs HP before/after pass/fail results for each target.

Implementation rule for future agents:

- New combat damage sources must either:
  - expose a clear `DamageVolume` debug draw at the owning actor/component, or
  - be documented as direct/abstract damage in `Source/T66/Gameplay/pending_issues_Gameplay.md` until a debug provider exists.
- New hostile projectile or touch paths must pass an explicit delivery method and damage causer into `UT66RunStateSubsystem::ApplyDamage(...)`; do not rely on source-actor-only logs.
- New hostile projectile damage must validate the overlapped hero component with `T66CombatShared::IsHeroHurtboxComponent(...)` before applying HP loss. Do not damage the hero from overlaps with `CombatRangeSphere`, range rings, pickup sensors, interaction triggers, or other hero-owned non-hurtbox components.
- New hostile projectile paths must emit `[ProjectileFired]` and `[ProjectileImpact]` logs at the projectile owner/path, including `OtherComp`, primitive location, sweep location, impact point, and velocity, so `[CombatDamage]` can be correlated with the exact visible projectile contact.
- New temporary projectile visuals must use `FT66TemporaryProjectileSystem` profiles first. Do not add one-off particle-only ranged attacks while this replacement layer is active, and do not reintroduce non-boss per-idol/per-element projectile colors outside the blue-hero/red-hostile contract.
- New hero auto-attack, chain, or splash logic must preserve the `HasUnblockedAutoAttackPath(...)` gate, or explicitly document why that attack is allowed to ignore dungeon wall line of sight.
- New irregular attack visuals, such as slashes or arcs, must keep VFX as presentation and use an explicit logical query shape for damage. Do not use Niagara particle collision, render-mesh per-poly collision, or visual material opacity as combat authority.
- New targetable sub-parts must use `UT66CombatHitZoneComponent` or a compatible hurtbox abstraction, not a generic hidden collider with ad hoc trace handling.

### 2.9 Combat UI and feedback

- `Source/T66/UI/T66GameplayHUDWidget.h/.cpp`
  - Owns the center crosshair, aggregate boss bar, and per-part boss bar presentation sourced from `BossPartSnapshots`.
- `Source/T66/UI/T66EnemyHealthBarWidget.h/.cpp`
  - Enemy-only health bar widget with locked-state visuals.
- `Source/T66/UI/T66EnemyLockWidget.h/.cpp`
  - Manual lock indicator.
- `Source/T66/Core/T66FloatingCombatTextSubsystem.h/.cpp`
- `Source/T66/UI/T66FloatingCombatTextWidget.cpp`
  - Supports event text such as `Crit`, `Headshot`, `DoT`, `CloseRange`, `LongRange`.
  - `Headshot` currently colors the damage number event; standalone status-word popups remain globally suppressed.

## 3. Current Limitations Blocking Full Body-Part Combat

### 3.1 Projectile and ultimate paths are not fully hit-zone-aware

- Core auto-attack damage now resolves enemy hit zones, but several projectile/ultimate/idol helper paths still enter through actor wrappers and therefore default to body targeting.

### 3.2 Bounce/body-part rules are still incomplete

- Standard bounce logic can now resolve enemy and boss-part aim points, but dedicated per-boss exclusion/preference rules do not exist yet.

### 3.3 Boss multipart data and rules are still at the runtime-foundation stage

- The runtime now ships with authored generic boss profiles in `Bosses.csv` plus bespoke multipart layouts for `VendorBoss` and `GamblerBoss`.
- The remaining major work is deeper boss-rule design:
  - break-state gameplay effects
  - projectile/ultimate path cleanup
  - per-boss phase rules and destruction visuals

## 4. Required Design Decisions

These decisions should be made before implementation begins, because they affect schema, UI, and tuning.

### 4.1 How `Accuracy` is earned

Resolved:

- `Accuracy` is now a primary hero stat.
- It participates in:
  - hero base stat tuning
  - hero per-level gain ranges
  - item line-1 primary stat bonuses
  - permanent buff progression
  - summary/backend snapshots
  - stat-panel presentation
- Accuracy-family item/buff ownership now lives under the Accuracy primary track.

### 4.2 How accuracy maps to head selection

Define one central rule, for example:

- chance to select head when the target exposes a head zone
- aim offset blend from body center toward head anchor
- lock-on assistance threshold for head selection

This logic must live in one shared combat targeting path, not in scattered special cases.

### 4.3 What headshots do

Decide whether headshots:

- only change aim point
- add bonus damage
- guarantee crits
- trigger unique floating text
- interact with the old `Headshot` passive

### 4.4 Which enemies support head/body zones

- The system should be optional per enemy type.
- Not every enemy should be forced into the two-zone model.
- Recommended default:
  - enemies without configured zones still use legacy actor/body targeting
  - enemies with configured zones expose at least `Body` and `Head`

### 4.5 Boss part rules

Decide:

- whether destroying a part removes attacks/abilities
- whether total boss death requires all parts or a core part
- whether damaged parts stay targetable after break
- whether bounce prefers different parts on the same boss before jumping away

## 5. Recommended Target Model

### 5.1 Add an explicit hit-zone abstraction

Recommended new runtime concepts:

- `ET66HitZoneType`
  - `Body`
  - `Head`
  - boss-specific values such as `Core`, `Arm`, `Leg`, `WeakPoint`
- `FT66CombatTargetHandle`
  - owning actor
  - hit-zone type
  - hit primitive / scene component
  - world aim point
- `UT66CombatHitZoneComponent`
  - reusable component for enemy/boss sub-targets
  - stores zone type, optional damage multiplier, health-sharing mode, and targeting flags

This keeps targeting, projectile homing, damage routing, and UI all speaking the same language.

### 5.2 Separate actor selection from zone selection

Recommended flow:

1. Select target actor.
2. Resolve best hit zone on that actor.
3. Build a `FT66CombatTargetHandle`.
4. Route damage using that handle.

This prevents the current actor-only assumptions from leaking everywhere.

### 5.3 Keep enemies and bosses on one combat contract

Recommended shared interface behavior:

- resolve available hit zones
- resolve default body zone
- resolve preferred auto-aim zone from accuracy
- apply damage to zone
- return combat feedback data

Enemies can use:

- shared HP with head/body damage multipliers

Bosses can use:

- true per-part HP with a boss-specific ruleset

## 6. Files That Need To Change

## 6.1 Data and schema

- `Source/T66/Data/T66DataTypes.h`
  - extend the now-live `Accuracy` stat wherever balance/data authoring requires it
  - add hit-zone enums/structs
  - extend `FBossData` or add dedicated boss-part config structs
- `Content/Data/Heroes.csv`
  - add base accuracy fields if hero-driven
- `Content/Data/Items.csv`
  - add item support if accuracy is item-driven
- `Content/Data/Bosses.csv`
  - add body-part definitions or references to a part config table
- likely new data source(s)
  - `EnemyHitZones.csv`
  - `BossParts.csv`
  - or equivalent per-archetype config assets

## 6.2 Run-state and stat plumbing

- `Source/T66/Core/T66RunStateSubsystem.h/.cpp`
  - maintain accuracy getters and multiplier plumbing
  - keep headshot/accuracy math centralized
  - extend boss runtime state if HUD stays `RunState`-driven for part bars

## 6.3 Combat targeting and hit resolution

- `Source/T66/Gameplay/T66CombatComponent.h/.cpp`
  - replace actor-only primary target handling with target-handle resolution
  - resolve head/body aim for auto attacks from accuracy
  - route manual lock to specific hit zones when requested
  - update bounce to support boss-part chaining
  - update damage application to call hit-zone-aware damage entry points

## 6.4 Projectile path

- `Source/T66/Gameplay/T66HeroProjectile.h/.cpp`
  - add support for targeting a scene component or explicit aim point, not only an actor root
  - carry hit-zone intent so impact resolution knows whether this is a head/body/part shot
  - keep visual-only projectiles visually readable but excluded from `DamageVolume` debug draw

## 6.5 Enemy runtime surface

- `Source/T66/Gameplay/T66EnemyBase.h/.cpp`
  - add optional head/body hit primitives
  - expose hit-zone query helpers
  - apply head/body damage rules
  - keep legacy single-hitbox fallback for enemies that do not define zones
  - adjust `ECC_Visibility` routing so the trace can hit the correct sub-target

## 6.6 Boss runtime surface

- `Source/T66/Gameplay/T66BossBase.h/.cpp`
  - replace single-HP-body assumption with part definitions and per-part state
  - add part damage entry points
  - publish part-health updates to the HUD/state layer
  - define destruction and victory rules

## 6.7 Player controller input and lock-on

- `Source/T66/Gameplay/T66PlayerController.h`
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp`
- `Source/T66/Gameplay/T66PlayerController_ScopedUlt.cpp`
  - current lock state only stores `AT66EnemyBase`
  - this needs to become combat-target-aware
  - click/reticle selection must:
    - detect head/body hit components
    - allow explicit head lock
    - fall back cleanly to body/actor when no part is available
  - scoped shots should also resolve concrete body parts

## 6.8 HUD and world UI

- `Source/T66/UI/T66GameplayHUDWidget.h/.cpp`
  - add boss-part bars or a boss-part panel
  - add target-zone feedback near the crosshair/lock state
- `Source/T66/UI/T66EnemyHealthBarWidget.h/.cpp`
  - optionally indicate head-targeted state, not just locked state
- likely new widget(s)
  - `T66BossPartHealthWidget`
  - `T66TargetPartReticleWidget`

## 6.9 Combat feedback and text

- `Source/T66/Core/T66FloatingCombatTextSubsystem.h/.cpp`
- `Source/T66/UI/T66FloatingCombatTextWidget.cpp`
  - add explicit `Headshot` event text if desired
  - differentiate spatial headshots from crits

## 6.10 Combat debug role layer

- `Source/T66/Gameplay/T66CombatDebugDraw.h/.cpp`
  - owns the debug role vocabulary and cvars
  - draws hurtbox spheres, player hurtbox capsules, damage spheres, damage boxes, damage capsules, and transient query-only damage sectors
  - defaults on in non-shipping builds for this combat-readability pass
  - remains off in Shipping builds

## 7. Recommended Implementation Phases

## Phase 1: Combat target model

- Implemented:
  - hit-zone enums/structs/components
  - enemy target handles for auto-attacks and manual lock
  - shared-HP enemy head/body damage routing

## Phase 2: Standard enemies with optional head/body

- Implemented:
  - optional `Body`/`Head` hit zones on `AT66EnemyBase`
  - shared HP for enemies
  - accuracy-driven head selection for untargeted auto-attacks
  - manual head/body lock selection when the trace hits a combat hit-zone component

## Phase 3: Replace fake headshot passive behavior

- Implemented:
  - removed random `RollHeadshot()` proc logic
  - real headshots now come from hit-zone selection
- If a hero/passive still modifies headshots, make it modify:
  - headshot chance weighting
  - headshot damage
  - or headshot crit behavior

## Phase 4: Boss parts and per-part HP

- Add boss part definitions and runtime part state.
- Route boss damage through part-aware entry points.
- Add boss-part HUD bars and destruction rules.

## Phase 5: Bounce and advanced targeting

- Update bounce to chain across boss parts.
- Define whether enemy head/body also count as separate bounce nodes.
- Update scoped shots and ultimate traces to respect part targeting.

## Phase 6: Data and tuning pass

- Author per-enemy zone data.
- Author per-boss part data.
- Add accuracy stat tuning and UI labels.
- Rebalance headshot damage vs crit damage vs attack categories.

## 8. Practical Risks

### 8.1 Trace conflict risk

- If the root capsule keeps swallowing `ECC_Visibility`, manual part targeting will feel broken.
- Sub-target collision ownership has to be designed deliberately.

### 8.2 Combat code sprawl risk

- `T66CombatComponent.cpp` is already large.
- Do not bury hit-zone logic directly into the existing lambdas without extracting shared helpers/types.

### 8.3 Boss UI coupling risk

- Boss HP is currently simple because `RunState` owns one flat value.
- Multi-part bosses will either:
  - significantly expand `RunState`
  - or require the HUD to read boss-part state more directly

### 8.4 Naming conflict risk

- Existing `Headshot` passive naming will confuse future work unless it is changed early.

## 9. Current Rollout Status

Completed:

1. Combat hit-zone types/components and target handles are live.
2. Standard enemies support spatial `Body` / `Head` targeting while still sharing one HP pool.
3. `Accuracy` is now a full primary stat with item, buff, summary, backend, and UI integration.
4. Random passive `Headshot` proc logic was replaced with spatial head-targeting support plus accuracy weighting.
5. Bosses now support multipart targeting, per-part HP snapshots, authored part profiles, and per-part HUD bars.
6. Combat debug visibility now follows explicit roles instead of raw colliders:
   - hitboxes/hurtboxes
   - player hurtbox
   - active or warning damage volumes
   - visual-only projectiles excluded from damage-volume rendering

Remaining follow-up work is tuning and specialization rather than core plumbing:

1. audit remaining projectile / ultimate helper paths for full hit-zone parity
2. tune per-hero `BaseAccuracy` and per-boss part weighting
3. add deeper boss-part break-state gameplay and visuals where desired
4. consolidate direct/global damage rules into the same debug-provider/event contract as geometric damage volumes
5. resolve the enemy touch-damage overlap/proximity split documented in `Source/T66/Gameplay/pending_issues_Gameplay.md`
