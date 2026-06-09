# T66 Master Movement

**Last updated:** 2026-06-08
**Scope:** Single-source handoff for player-hero movement runtime: input, locomotion, jump, one-button forward leap, speed multipliers, velocity-affecting stage effects, and current movement blockers and overrides.
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Gameplay/Combat/MASTER_COMBAT.md`
**Maintenance rule:** Update this file after every material change to hero movement input, locomotion tuning, jump or leap rules, stage-effect movement, movement-state gating, or run-state speed modifiers.

## 1. Executive Summary

- `AT66PlayerController` currently owns raw movement input capture and still applies normal walking via `AddMovementInput`.
- `UT66HeroMovementComponent` owns movement configuration, cached move intent, jump routing, one-button forward leap routing, and final `MaxWalkSpeed` refreshes, but it is not yet the sole owner of locomotion.
- `UCharacterMovementComponent` on `AT66HeroBase` remains the live movement authority for walking, falling, friction, rotation-to-movement, and impulse response.
- `UT66HeroPhysicsComponent` can add a bounded capsule shove during active-ragdoll obstacle reactions, but the simulated pelvis/mesh reaction remains owned by Gameplay Physics.
- `UT66HeroMovementComponent` now owns non-damaging bouncy wall contact and landing-only bouncy floor contact by issuing bounded `LaunchCharacter()` impulses from CharacterMovement state; these impulses do not route through damage, knockback, or ragdoll.
- Base walk speed starts at `600`, then is seeded from the hero foundational `Speed` stat on the successful non-preview hero-initialize path. Live walking speed is `Speed * 300 UU/s`, with explicit item, stage, and status movement modifiers layered on top.
- Jump is currently single-jump only and uses standard forward carry from the live movement state; it is not currently suppressing forward movement on takeoff.
- Leap is a one-button forward burst bound to `Leap`; it uses the hero actor's facing direction and does not require a movement-input chord.
- Movement state is split:
  - actual movement comes from `AddMovementInput`, `CharacterMovement`, and `LaunchCharacter`
  - the Hero Speed subsystem only tracks binary move intent for visuals and companion state
  - hero animation state also consults falling state, actual velocity, replicated velocity, and remote location delta
- Several movement-adjacent hooks exist but are not fully live:
  - `ApplyStatusBurn`, `ApplyStatusChill`, and `ApplyStatusCurse` are currently stubs, and the tick path says those enemy-applied status effects were removed

## 2. Canonical Files

- `Config/DefaultInput.ini`
- `Source/T66/Gameplay/T66PlayerController_Input.cpp`
- `Source/T66/Gameplay/T66PlayerController_Movement.cpp`
- `Source/T66/Gameplay/Movement/T66HeroMovementTypes.h`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.h`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
- `Source/T66/Gameplay/T66HeroBase.h`
- `Source/T66/Gameplay/T66HeroBase.cpp`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/Core/T66HeroSpeedSubsystem.h`
- `Source/T66/Core/T66HeroSpeedSubsystem.cpp`
- `Source/T66/Gameplay/T66StageEffects.h`
- `Source/T66/Gameplay/T66StageEffects.cpp`

## 3. Current Movement Runtime Spine

### 3.1 Input bindings and controller ownership

- Default live bindings are:
  - move forward and back: `W` / `S` and `Gamepad_LeftY`
  - move right and left: `D` / `A` and `Gamepad_LeftX`
  - jump: `SpaceBar` and `Gamepad_FaceButton_Bottom`
  - leap: `LeftShift` and `Gamepad_FaceButton_Right`
- `AT66PlayerController` binds those actions and axes in `T66PlayerController_Input.cpp`.
- Forward and right axis handlers in `T66PlayerController_Movement.cpp` currently do three separate jobs:
  - cache raw axis values in `RawMoveForwardValue` and `RawMoveRightValue`
  - forward filtered intent into `UT66HeroMovementComponent::SetMoveInputAxes()`
  - call `AddMovementInput()` on the possessed pawn for actual walking
- Current architecture is therefore intentionally split:
  - the movement component owns intent and ability interpretation
  - the controller still owns ordinary locomotion input application

### 3.2 Intent filtering before locomotion

- `UpdateHeroMovementIntent()` zeroes applied move intent when world dialogue is open.
- `UpdateHeroMovementIntent()` also zeroes applied move intent when the hero is mounted in a vehicle.
- `HandleMoveForward()` and `HandleMoveRight()` early-return before `AddMovementInput()` when:
  - the current level is not gameplay
  - world dialogue is open
  - the hero is vehicle-mounted
  - the axis value is nearly zero
- When the hero is mounted, the controller reroutes axis input to the tractor via `SetDriveForwardInput()` and `SetDriveRightInput()`.
- World dialogue currently repurposes the forward axis for dialogue navigation with a `0.18s` debounce.

### 3.3 Hero movement component ownership

- `UT66HeroMovementComponent` is attached directly to `AT66HeroBase`.
- On `BeginPlay()` it caches:
  - the owning hero
  - `UT66RunStateSubsystem`
  - `UT66HeroSpeedSubsystem`
- It then:
  - reapplies the default movement config
  - refreshes walk speed from RunState
  - updates the animation-state bridge
- `AT66HeroBase` also calls `ApplyDefaultMovementConfig()` in its constructor, so the default config is effectively applied once during construction and once again during component `BeginPlay()`.

### 3.4 CharacterMovement config currently applied

- The movement component currently writes these `UCharacterMovementComponent` values:
  - `MaxWalkSpeed = BaseWalkSpeed`
  - `MaxAcceleration = 9000`
  - `BrakingDecelerationWalking = 12000`
  - `GroundFriction = 8.0`
  - `bUseSeparateBrakingFriction = true`
  - `BrakingFriction = 12.0`
  - `BrakingFrictionFactor = 1.0`
  - `JumpZVelocity = 1600`
  - `AirControl = 0.40`
  - `GravityScale = 4.5`
  - `FallingLateralFriction = 0.35`
  - `BrakingDecelerationFalling = 4096`
  - `bOrientRotationToMovement = true`
  - `RotationRate = (0, 1440, 0)`
- It also currently forces:
  - `JumpMaxCount = 1`
  - `JumpMaxHoldTime = 0.08`

### 3.5 Walk-speed ownership

- The live base walk-speed variable is `UT66HeroMovementComponent::BaseWalkSpeed`.
- `BaseWalkSpeed` starts at `600`.
- During `AT66HeroBase::InitializeHero()`, current runtime seeds fallback hero walk speed with:
  - `HeroMovementComponent->SetHeroBaseSpeedStat(InHeroData.BaseSpeed)`
  - this currently happens only when character visual application succeeds and the hero is not in preview mode
- `RefreshWalkSpeedFromRunState()` then computes live `MaxWalkSpeed` as:
  - `RunState->GetSpeedStat() * 300 UU/s` when RunState exists, otherwise fallback `BaseWalkSpeed`
  - multiplied by `GetItemMoveSpeedMultiplier()`
  - multiplied by `GetStageMoveSpeedMultiplier()`
  - multiplied by `GetStatusMoveSpeedMultiplier()`
- The final result is clamped to `[100, 10000]`.

### 3.6 RunState hooks that currently affect movement speed

- Hero foundational `Speed` stat is the base movement-speed authority.
- `HeroData.MaxSpeed` is reserved metadata for future cap semantics and is not part of the current live walking-speed stack.
- Item-derived movement speed is represented by `ItemMoveSpeedMultiplier`.
- Stage speed boosts are live:
  - `ApplyStageSpeedBoost()` clamps multiplier to `[0.25, 5.0]`
  - clamps duration to `[0, 30]`
  - extends current stage boost by max-of-current semantics
  - broadcasts `HeroProgressChanged` so heroes refresh their walk speed
- When stage speed boost time expires, RunState resets `StageMoveSpeedMultiplier` to `1.0` and broadcasts `HeroProgressChanged`.
- `AT66HeroBase` currently refreshes movement speed whenever RunState fires:
  - `HeroProgressChanged`
  - `InventoryChanged`
- Status-move slowdown is structurally present through `GetStatusMoveSpeedMultiplier()`, but the current public status application functions are stubs.

## 4. Jump

- Jump input is handled by `AT66PlayerController::HandleJumpPressed()` and `HandleJumpReleased()`.
- Press flow:
  - controller resolves `UT66HeroMovementComponent`
  - component calls `TryJump()`
  - `TryJump()` requires:
    - valid hero
    - `CanUseMovementAbilities()`
    - `Hero->CanJump()`
  - success path calls `Hero->Jump()`
- Release flow:
  - controller calls `HeroMovement->StopJumping()`
  - component delegates to `Hero->StopJumping()`
- On successful jump, the controller also:
  - marks tutorial jump input in RunState
  - logs verbose jump state
  - spawns jump VFX around the hero feet
- Current ability-gate blockers for jump are:
  - preview mode
  - vehicle-mounted state
- World dialogue does not currently add a separate jump block in the movement component.

## 5. Leap

### 5.1 Current player-facing leap behavior

- Leap is a press-and-fire action, not a held modifier.
- `Config/DefaultInput.ini` binds `Leap` to `LeftShift` and `Gamepad_FaceButton_Right`.
- `AT66PlayerController::HandleLeapPressed()` calls `AT66HeroBase::Leap()` on the possessed hero.
- `Roll` input and `RollForward()` remain deprecated compatibility aliases during migration.
- There is no release handler and no two-button/chord consumption state.

### 5.2 Leap direction rules

- Player leap ignores cached move-input axes.
- `AT66HeroBase::Leap()` delegates to `UT66HeroMovementComponent::TryLeap()`.
- `TryLeap()` supplies `Hero->GetActorForwardVector()` as the desired leap direction.
- Result: a neutral leap and a moving leap both travel in the direction the hero is facing.

### 5.3 Leap execution rules

- Leap execution currently reuses the movement component's existing launch/cooldown helper:
  - valid hero
  - valid world
  - `CanUseMovementAbilities()`
  - a non-zero horizontal direction
- Cooldown is:
  - base `0.7s`
  - multiplied by `UT66RunStateSubsystem::GetDashCooldownMultiplier()` until the stat layer is renamed
  - clamped to `[0.05, 10.0]`
- Successful leap execution builds a forward-up launch velocity:
  - horizontal: `LeapDirection * LeapHorizontalStrength`
  - vertical: `LeapUpwardStrength`
  - call: `Hero->LaunchCharacter(LeapVelocity, true, true)`
- Current horizontal leap strength is resolved as the greater of:
  - tuning floor `3200`
  - `CurrentMaxWalkSpeed * 1.6`
- Current upward leap strength is resolved as the greater of:
  - tuning floor `880`
  - `JumpZVelocity * 0.5`
- Because `LaunchCharacter()` is called with both override flags set to `true`, leap currently replaces existing XY and Z launch components rather than layering gently onto prior velocity.
- If the current visual has `LeapAnimation`, `AT66HeroBase` plays it once and holds the movement animation state until the clip's play length elapses.

## 6. Velocity and Other Non-Input Movement Changes

### 6.1 Normal locomotion velocity

- Ordinary walking velocity is still the stock `ACharacter` / `UCharacterMovementComponent` path driven by `AddMovementInput()`.
- No custom acceleration curve, custom movement mode, or custom per-tick locomotion integrator currently replaces that stock path for the hero.

### 6.2 LaunchCharacter paths

- Current hero velocity can also be changed by direct launch impulses from:
  - hero leap
  - hero surface bounce while moving
  - hero enemy-touch bounce
  - shroom top bounce
  - shroom side knockback
- TestRoom wipeout-arm active-ragdoll reaction
- Hero surface bounce in `UT66HeroMovementComponent` is intentionally movement-owned:
  - ground bounce applies only on landing after an airborne jump or drop, not while simply walking
  - ground bounce scales launch height from the stronger of captured downward landing speed and fall-height-derived impact speed, then applies restitution so repeated landing bounces decay
  - forward wall contact sweeps ahead of the capsule and reflects the hero away from blocking world geometry
  - tower floor-gate covers, ceiling/underpass geometry, and actors/components tagged `T66_NoSurfaceBounce` are excluded from wall bounce so arrival spaces below gates stay walkable
  - neither path calls `UT66HeroPhysicsComponent::ApplyPhysicsReaction()` or `UT66KnockbackComponent`, so bouncy walls/floors cannot cause ragdoll by themselves
- Enemy-touch bounce in `AT66HeroBase::Tick()` currently launches the hero away from a nearby enemy with:
  - horizontal strength `420`
  - vertical strength `120`
- `AT66Shroom` stage effects currently launch the hero with:
  - top trigger bounce: `LaunchForwardVelocity = 1800`, `LaunchZVelocity = 2400`
  - side trigger knockback: `KnockbackForce = 2800` with `35%` of that value applied upward
- The TestRoom wipeout-arm prototype lives in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` and is intentionally local to TestRoom. When `UT66HeroPhysicsComponent` is initialized, impact routes through the Stage 3 active-ragdoll profile: simulated-body impulse plus pose/anchor loosen, with only a bounded capsule shove for playable displacement. If active ragdoll is unavailable, the legacy `UT66KnockbackComponent` path remains a fallback.

### 6.3 Stage slide

- `AT66HeroBase::ApplyStageSlide()` does not inject velocity directly.
- Instead, it temporarily reduces braking and friction to create a slide feel:
  - `GroundFriction = 0.15`
  - `BrakingFrictionFactor = 0.05`
  - `BrakingDecelerationWalking = 128`
- `AT66HeroBase` caches the baseline walking friction values on `BeginPlay()` and restores them when `StageSlideSecondsRemaining` reaches zero.

### 6.4 Sky-drop and terrain recovery

- `BeginSkyDrop()` teleports the hero upward by `5000` units and disables player input until landing.
- `Landed()` re-enables player input if the hero was sky-dropping.
- Hero tick also tracks last safe grounded transform.
- If the hero falls far below the last safe ground position or below world floor thresholds and no nearby ground can be recovered:
  - the hero is teleported back to the last safe ground transform
  - movement is stopped immediately
  - movement mode is restored to `MOVE_Walking`

## 7. Movement State, Velocity, and Animation

### 7.1 Hero Speed subsystem

- `UT66HeroSpeedSubsystem` currently does not own actual locomotion.
- It only stores:
  - resolved movement speed under a legacy `MaxSpeed` API name
  - `CurrentSpeed`
  - binary last-input movement state
- Current behavior is intentionally simple:
  - if there is move input, `CurrentSpeed = MaxSpeed`
  - if there is no move input, `CurrentSpeed = 0`
  - `GetMovementAnimState()` returns `0` for idle or `1` for moving
- `UT66HeroMovementComponent` updates this subsystem through `UpdateAnimationStateBridge()`.
- Current live usage is visual-state signaling, especially for companion behavior and movement-adjacent animation logic.

### 7.2 Hero animation state selection

- Hero animation state is not driven solely by cached move input.
- In `AT66HeroBase::Tick()` the current hero visual state is chosen as:
  - `Leap` while a one-shot leap animation is active
  - `Jump` if `CharacterMovement->IsFalling()`
  - else `Walk` if any of these are true:
    - movement component says there is move input
    - live velocity has horizontal magnitude
    - replicated velocity has horizontal magnitude
    - remote actor location delta indicates movement
  - else `Idle`
- Current visual implication:
  - Hero Speed subsystem is not authoritative for hero animation by itself
  - actual or replicated movement can still produce walk animation even when local input is absent

## 8. Hard Movement Blockers and State Overrides

- Preview mode:
  - disables camera
  - disables movement
  - `CanUseMovementAbilities()` returns false
- Vehicle mounted:
  - hero movement stops immediately
  - character movement is disabled
  - movement input vector is consumed
  - capsule collision is disabled
  - on exit, movement mode is restored to `MOVE_Walking` and walk speed is refreshed
- World dialogue:
  - normal move intent is zeroed
  - normal walking input path early-returns
  - leap still shares the movement-ability gate, but its direction does not depend on live move input
  - jump is not separately gated here

## 9. Current Live Numbers

- Base walk speed fallback: `600` before hero data is applied
- Speed conversion: `1 Speed` = `300 UU/s`
- Hero initialize walk speed assignment on successful non-preview visual setup: `HeroData.BaseSpeed * 300`
- Walk speed clamp after multiplier stack: `100` to `10000`
- Max acceleration: `9000`
- Walking braking deceleration: `12000`
- Ground friction: `8.0`
- Use separate braking friction: `true`
- Braking friction: `12.0`
- Braking friction factor: `1.0`
- Jump count: `1`
- Jump hold time: `0.08`
- Jump Z velocity: `1600`
- Air control: `0.40`
- Gravity scale: `4.5`
- Falling lateral friction: `0.35`
- Falling braking deceleration: `4096`
- Leap cooldown base: `0.7`
- Leap cooldown clamp: `0.05` to `10.0`
- Leap strength floor: `3200`
- Leap upward strength floor: `880`
- Leap speed multiplier over current walk speed: `1.6`
- Rotation rate yaw: `1440`
- Surface bounce enabled CVar: `t66.HeroMovement.SurfaceBounceEnabled = 1`
- Surface ground bounce restitution: `0.55`
- Surface ground bounce minimum impact speed: `520`
- Surface ground bounce minimum fall height: `70`
- Surface ground bounce minimum launch Z: `260`
- Surface ground bounce maximum launch Z: `1650`
- Surface ground bounce duplicate-fire cooldown: `0.12`
- Surface wall bounce horizontal floor: `2200`
- Surface wall bounce Z: `420`
- Surface wall bounce cooldown: `0.24`
- Surface wall trace distance: `220`
- Surface bounce moving-speed floor: `160`
- Stage slide friction override: `0.15`
- Stage slide braking friction factor: `0.05`
- Stage slide braking deceleration: `128`
- Sky-drop altitude: `5000`

## 10. Known Drift and Improvement Targets

- Ordinary locomotion authority is split between controller and movement component.
  - this is the biggest architectural simplification target if movement is refactored
- Status-effect move-speed plumbing exists structurally, but current public status application functions are stubs
- Hero Speed subsystem comments imply movement-speed ownership, but current live behavior is cosmetic or animation-facing only
- Leap currently reuses the legacy dash-named stat multiplier (`GetDashCooldownMultiplier()`) until item/stat data contracts are ready for a broader rename.
  - these names should be migrated when item/stat data contracts are ready for a broader rename
- Jump and leap share the same movement-ability gate helper, but ordinary walking is blocked by separate controller and state-management logic rather than one central movement-state policy

## 11. Source-of-Truth Rules

- If movement authority changes, update this file in the same change.
- If walking is moved fully into `UT66HeroMovementComponent`, record the old split-controller model here as historical context and rewrite Sections 3, 5, and 10.
- If leap, jump, or stage-effect velocity behavior changes, update both the runtime path description and the numeric tuning section.
- If movement-related RunState multipliers become live, remove the stale-hook notes and document the exact multiplier stack order here.
