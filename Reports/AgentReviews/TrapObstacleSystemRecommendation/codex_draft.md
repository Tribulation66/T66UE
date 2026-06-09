Codex draft recommendation for cross-review:

Short version: do not merely retune the existing damage traps. Keep UT66TrapSubsystem and AT66TrapBase as the floor/lifecycle/spawn ownership layer, but add a new obstacle-trap contract whose gameplay payload is a physics reaction/incapacitation profile, not HP damage.

Evidence from live repo:
- Gameplay/Traps/MASTER_TRAPS.md says the current tower trap families are WallProjectile, FloorBurst, and AreaControl; concrete actors currently damage through UT66RunStateSubsystem::ApplyDamage.
- Source/T66/Gameplay/Traps/T66TrapBase.h and .cpp expose damage flags and progression scalars, but no obstacle reaction profile.
- Source/T66/Gameplay/Traps/T66TrapDamageUtils.cpp is damage-only.
- The TestRoom wipeout arm in Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp already calls UT66HeroPhysicsComponent::ApplyPhysicsReaction on arm impact, but it is not an AT66TrapBase actor and is not managed by UT66TrapSubsystem.
- Gameplay/Physics docs and Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.* show the current live ragdoll owner: hit-triggered full skeletal ragdoll via ApplyPhysicsReaction, with Normal/Ragdoll/GettingUp states.
- Source/T66/Gameplay/T66TowerMapTerrain.* already has floor bounds, walkable boxes, maze wall boxes, trap-eligible wall boxes, and spawn queries, but it does not have generalized obstacle lane/socket/clearance descriptors.
- Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp currently spawns interactables and then calls TrapSubsystem->SpawnTowerStageTraps, so existing trap generation is late and damage-trap-oriented.
- Live source currently uses start floor 1, mob floors 2-3, boss floor 4, while trap/map docs/config still refer to gameplay floors 2-4 and boss floor 5. Any new budget logic should use live layout metadata rather than hardcoded stale floor numbers.

Recommended architecture:
1. Create a new obstacle-trap layer under the existing trap system:
   - Add AT66ObstacleTrapBase or AT66PhysicsObstacleTrapBase derived from AT66TrapBase.
   - Default bDamagesHeroes=false and replace the primary effect with FT66ObstacleReactionProfile / FT66PhysicsReactionProfile.
   - Keep registration, active-floor gating, progression hooks, debug labels, and floor compatibility in UT66TrapSubsystem.
   - Add an obstacle reaction helper, e.g. FT66TrapObstacleReactionUtils, that wraps UT66HeroPhysicsComponent::ApplyPhysicsReaction, applies cooldowns/source tags, checks same-floor rules, and logs proof markers.
2. Treat the old damage trap families as legacy or secondary. Wall arrows/flames/spikes can remain for now, but new Fall Guys-style traps should not call ApplyTrapDamageToOverlaps as their main effect.
3. Add data-authored trap families:
   - SweeperArm / JumpOverArm: horizontal rotating arm, low enough to jump, primary reaction on hit.
   - CeilingHammer / Pendulum: ceiling anchor, cross-lane swing arc, readable windup.
   - Bumper / Popper: small radial/upward impulse source, can be scattered safely.
   - LaunchPad / Flipper: floor trigger that launches the hero upward/forward.
   - Fan/GustStrip/Conveyor: sustained push, likely second phase because it is continuous-force behavior rather than hit impulse only.
   - Tilting platform/spinning disc: later phase because it touches moving surfaces and navigation/rescue complexity.
4. Add obstacle placement descriptors to tower layout rather than placing all traps at random points:
   - Extend T66TowerMapTerrain::FFloor or a sidecar generated descriptor with major obstacle lanes, ceiling anchors, pad sockets, bumper sockets, clearance boxes, and no-spawn reserves.
   - Size traps by normalized layout units: PlacementCellSize, floor BoundsHalfExtent, room class, and min approach/exit clearance.
   - Major obstacles require lane-shaped authored sockets; small bumpers/pads can use scatter sockets.
   - Do not place major obstacle lanes across descent holes, vendors, chests, NPCs, required interactables, or spawn safe zones.
5. Change tower generation order:
   - Layout builds floor geometry and obstacle sockets first.
   - Reserve major obstacle footprints before optional interactables.
   - Place optional interactables and loot in remaining space.
   - Spawn obstacle trap actors through UT66TrapSubsystem, then activate only the current floor.
6. Preserve the intended enemy follow-up:
   - Ragdoll/incapacitated state must remain damageable/targetable by enemies. Current hero physics disables capsule collision while ragdolled, so implementation needs a proof gate for whether enemies can actually hit the incapacitated hero. If not, add a damage proxy or incapacitation target component rather than relying on the disabled capsule.

Recommended implementation phases:
1. Documentation/source reconciliation: update trap docs for obstacle-trap direction and fix floor-number drift.
2. Extract the TestRoom wipeout arm concept into a reusable obstacle trap actor using the current UT66HeroPhysicsComponent reaction path.
3. Add tower obstacle socket/reservation generation and spawn one major sweeper arm on normal mob floors.
4. Add bumper and launch pad families.
5. Add ceiling hammer/pendulum family after ceiling anchor/headroom descriptors exist.
6. Add encounter templates/obstacle-room presets and tuning budgets.
7. Verify with focused compile, staged standalone, and Unreal-owned multi-frame capture showing hit, ragdoll, enemy attack while incapacitated, get-up, and no cross-floor false positives.

Risks and caveats:
- Current Physics pending issues say ragdoll feel/PhysicsAsset tuning is not production-final, so the trap system can be built against the current reaction API but subjective Fall Guys feel will need tuning.
- The docs/config/source floor-count drift should be cleaned before hardcoding trap budgets.
- Random scatter alone will produce clutter, not Fall Guys-style obstacles. The generator needs authored obstacle shapes/sockets.
