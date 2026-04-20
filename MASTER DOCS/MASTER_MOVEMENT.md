# T66 Master Movement

**Last updated:** 2026-04-17  
**Scope:** Single-source handoff for player-hero movement runtime: input, locomotion, jump, dash, speed multipliers, velocity-affecting stage effects, and current movement blockers and overrides.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/T66_PROJECT_CATALOGUE.md`, `MASTER DOCS/MASTER_COMBAT.md`  
**Maintenance rule:** Update this file after every material change to hero movement input, locomotion tuning, jump or dash rules, stage-effect movement, movement-state gating, or run-state speed modifiers.

## 1. Executive Summary

- `AT66PlayerController` currently owns raw movement input capture and still applies normal walking via `AddMovementInput`.
- `UT66HeroMovementComponent` owns movement configuration, cached move intent, jump routing, dash routing, and final `MaxWalkSpeed` refreshes, but it is not yet the sole owner of locomotion.
- `UCharacterMovementComponent` on `AT66HeroBase` remains the live movement authority for walking, falling, friction, rotation-to-movement, and impulse response.
- Base walk speed starts at `1800`, then is overwritten with `InHeroData.MaxSpeed * 2.0f` only on the successful non-preview hero-initialize path, then multiplied by RunState-derived movement modifiers.
- Jump is currently single-jump only and uses standard forward carry from the live movement state; it is not currently suppressing forward movement on takeoff.
- Dash is currently an 8-direction, held-shift directional burst that requires live movement input and only consumes the held input on a successful dash.
- Movement state is split:
  - actual movement comes from `AddMovementInput`, `CharacterMovement`, and `LaunchCharacter`
  - the Hero Speed subsystem only tracks binary move intent for visuals and companion state
  - hero animation state also consults falling state, actual velocity, replicated velocity, and remote location delta
- Several movement-adjacent hooks exist but are not fully live:
  - `FT66HeroMovementTuning::DefaultWalkSpeed` is declared but not used by current speed authority
  - `UT66RunStateSubsystem::GetMovementSpeedSecondaryMultiplier()` currently returns `1.0f`
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
  - dash: `LeftShift`
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

- The live base walk-speed variable is `UT66HeroMovementComponent::BaseWalkSpeed`, not `FT66HeroMovementTuning::DefaultWalkSpeed`.
- `BaseWalkSpeed` starts at `1800`.
- During `AT66HeroBase::InitializeHero()`, current runtime sets hero base walk speed with:
  - `HeroMovementComponent->SetHeroBaseWalkSpeed(InHeroData.MaxSpeed * 2.0f)`
  - this currently happens only when character visual application succeeds and the hero is not in preview mode
- `RefreshWalkSpeedFromRunState()` then computes live `MaxWalkSpeed` as:
  - `BaseWalkSpeed`
  - multiplied by `GetHeroMoveSpeedMultiplier()`
  - multiplied by `GetItemMoveSpeedMultiplier()`
  - multiplied by `GetMovementSpeedSecondaryMultiplier()`
  - multiplied by `GetLastStandMoveSpeedMultiplier()`
  - multiplied by `GetStageMoveSpeedMultiplier()`
  - multiplied by `GetStatusMoveSpeedMultiplier()`
- The final result is clamped to `[200, 10000]`.

### 3.6 RunState hooks that currently affect movement speed

- Hero foundational `Speed` stat currently feeds `GetHeroMoveSpeedMultiplier()` with:
  - `1.0 + (SpeedStat - 1) * 0.01`
- Item-derived movement speed is represented by `ItemMoveSpeedMultiplier`.
- Stage speed boosts are live:
  - `ApplyStageSpeedBoost()` clamps multiplier to `[0.25, 5.0]`
  - clamps duration to `[0, 30]`
  - extends current stage boost by max-of-current semantics
  - broadcasts `HeroProgressChanged` so heroes refresh their walk speed
- When stage speed boost time expires, RunState resets `StageMoveSpeedMultiplier` to `1.0` and broadcasts `HeroProgressChanged`.
- `AT66HeroBase` currently refreshes movement speed whenever RunState fires:
  - `HeroProgressChanged`
  - `SurvivalChanged`
  - `InventoryChanged`
- The declared secondary-stat movement hook is not live yet:
  - `GetMovementSpeedSecondaryMultiplier()` currently returns `1.0f`
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
  - quick-revive downed state
- World dialogue does not currently add a separate jump block in the movement component.

## 5. Dash

### 5.1 Current player-facing dash behavior

- Dash is treated as a held modifier, not a press-and-fire action with its own stored direction.
- `HandleDashPressed()` calls `SetDashModifierHeld(true)`.
- `HandleDashReleased()` calls `SetDashModifierHeld(false)`.
- While the modifier is held:
  - dash can be consumed once
  - releasing shift resets `bDashConsumedThisHold`
  - changing move input while shift is still held can trigger dash if it has not yet been consumed for that hold
  - failed dash attempts no longer consume the hold

### 5.2 Dash direction rules

- The movement component caches forward and right input axes.
- `HasMovementInput()` requires absolute axis magnitude greater than `0.1`.
- `GetCurrentDashDirection()` converts the cached 2D input vector into 8 octants.
- `GetWorldMoveDirectionFromAxes()` transforms those axes into world space using controller yaw only.
- `GetQuantizedWorldDashDirection()` quantizes the world yaw to 45-degree increments.
- Result: current player-input dash is an 8-direction world-space dash based on current move intent and controller yaw.
- Neutral dash from the standard player input path does not currently exist:
  - if there is no live move input, `TryConsumeHeldDash()` does nothing
- There is also a direct helper path:
  - `AT66HeroBase::DashForward()` calls `TryDashInWorldDirection(GetActorForwardVector())`
  - this bypasses the "must have current move input" rule because it supplies a direction explicitly

### 5.3 Dash execution rules

- `TryDashInWorldDirection()` requires:
  - valid hero
  - valid world
  - `CanUseMovementAbilities()`
  - a non-zero horizontal direction
- Cooldown is:
  - base `0.7s`
  - multiplied by `UT66RunStateSubsystem::GetDashCooldownMultiplier()`
  - clamped to `[0.05, 10.0]`
- Successful dash execution uses:
  - `Hero->LaunchCharacter(DashDirection * DashStrength, true, true)`
- Current dash strength is resolved as the greater of:
  - tuning floor `3200`
  - `CurrentMaxWalkSpeed * 1.6`
- Because `LaunchCharacter()` is called with both override flags set to `true`, dash currently replaces existing XY and Z launch components rather than layering gently onto prior velocity.

## 6. Velocity and Other Non-Input Movement Changes

### 6.1 Normal locomotion velocity

- Ordinary walking velocity is still the stock `ACharacter` / `UCharacterMovementComponent` path driven by `AddMovementInput()`.
- No custom acceleration curve, custom movement mode, or custom per-tick locomotion integrator currently replaces that stock path for the hero.

### 6.2 LaunchCharacter paths

- Current hero velocity can also be changed by direct launch impulses from:
  - hero dash
  - hero enemy-touch bounce
  - shroom top bounce
  - shroom side knockback
- Enemy-touch bounce in `AT66HeroBase::Tick()` currently launches the hero away from a nearby enemy with:
  - horizontal strength `420`
  - vertical strength `120`
- `AT66Shroom` stage effects currently launch the hero with:
  - top trigger bounce: `LaunchForwardVelocity = 1800`, `LaunchZVelocity = 2400`
  - side trigger knockback: `KnockbackForce = 2800` with `35%` of that value applied upward

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
  - `MaxSpeed`
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
- Quick-revive downed:
  - hero movement stops immediately
  - character movement is disabled
  - movement input vector is consumed
  - on recovery, movement mode is restored to `MOVE_Walking` if not vehicle-mounted and walk speed is refreshed
- World dialogue:
  - normal move intent is zeroed
  - normal walking input path early-returns
  - dash cannot consume through the standard input path because there is no live move input
  - jump is not separately gated here

## 9. Current Live Numbers

- Base walk speed fallback: `1800`
- Hero initialize walk speed assignment on successful non-preview visual setup: `HeroData.MaxSpeed * 2.0`
- Walk speed clamp after multiplier stack: `200` to `10000`
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
- Dash cooldown base: `0.7`
- Dash cooldown clamp: `0.05` to `10.0`
- Dash strength floor: `3200`
- Dash speed multiplier over current walk speed: `1.6`
- Rotation rate yaw: `1440`
- Stage slide friction override: `0.15`
- Stage slide braking friction factor: `0.05`
- Stage slide braking deceleration: `128`
- Sky-drop altitude: `5000`

## 10. Known Drift and Improvement Targets

- Ordinary locomotion authority is split between controller and movement component.
  - this is the biggest architectural simplification target if movement is refactored
- `FT66HeroMovementTuning::DefaultWalkSpeed` is currently dead configuration.
  - live speed authority uses `BaseWalkSpeed`
- `UT66RunStateSubsystem::GetMovementSpeedSecondaryMultiplier()` advertises item-driven movement-speed secondary behavior but currently returns `1.0f`
- Status-effect move-speed plumbing exists structurally, but current public status application functions are stubs
- Hero Speed subsystem comments imply movement-speed ownership, but current live behavior is cosmetic or animation-facing only
- Dash currently depends on live movement input for the standard player-input path.
  - there is no neutral in-place dash or last-input dash memory
- Jump and dash share the same movement-ability gate helper, but ordinary walking is blocked by separate controller and state-management logic rather than one central movement-state policy

## 11. Source-of-Truth Rules

- If movement authority changes, update this file in the same change.
- If walking is moved fully into `UT66HeroMovementComponent`, record the old split-controller model here as historical context and rewrite Sections 3, 5, and 10.
- If dash, jump, or stage-effect velocity behavior changes, update both the runtime path description and the numeric tuning section.
- If movement-related RunState multipliers become live, remove the stale-hook notes and document the exact multiplier stack order here.
