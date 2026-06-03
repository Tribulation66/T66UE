# T66 Master Camera

**Last updated:** 2026-05-23
**Scope:** Single-source handoff for gameplay camera ownership, zoom behavior, scoped-camera overrides, occluder fading, and frontend preview cameras. Mini battle now runs in Slate and does not own a world camera path.  
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Gameplay/Movement/MASTER_MOVEMENT.md`, `Gameplay/Combat/MASTER_COMBAT.md`
**Maintenance rule:** Update this file after every material camera change to gameplay framing, boom collision behavior, zoom/FOV tuning, camera-driven aiming, or frontend preview framing.

## 1. Executive Summary

- Main gameplay camera ownership is currently split between:
  - `AT66HeroBase`, which constructs the gameplay `CameraBoom` and `FollowCamera`
  - `AT66PlayerController`, which applies player zoom and Hero One scoped-camera overrides
- Main gameplay camera defaults are currently hard-coded in `AT66HeroBase`:
  - spring-arm length `2300`
  - spring-arm relative location `(0, 0, 60)`
  - boom collision enabled on `ECC_Camera` with probe size `12`
- Normal mouse-wheel zoom adjusts boom length only and clamps to `[1200, 2400]`.
- Hero One scoped mode is a dedicated camera override path, not a generic camera-mode system:
  - boom length forced to `0`
  - boom location forced to `(0, 0, 78)`
  - boom collision disabled
  - FOV forced to `18`
  - scoped zoom wheel clamps FOV to `[8, 30]`
- Camera obstruction mitigation currently uses two overlapping systems:
  - spring-arm collision pull-in
  - tag-based occluder fading for traversal barriers and tower ceilings
- Frontend preview cameras are separate spawned `ACameraActor` instances and do not reuse gameplay spring-arm behavior.

## 2. Canonical Files

- `Config/DefaultInput.ini`
- `Source/T66/Gameplay/T66HeroBase.h`
- `Source/T66/Gameplay/T66HeroBase.cpp`
- `Source/T66/Gameplay/T66PlayerController.h`
- `Source/T66/Gameplay/T66PlayerController.cpp`
- `Source/T66/Gameplay/T66PlayerController_Input.cpp`
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp`
- `Source/T66/Gameplay/T66PlayerController_ScopedUlt.cpp`
- `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`
- `Source/T66/Gameplay/T66FrontendGameMode.cpp`
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`

## 3. Current Main Gameplay Camera Runtime Spine

### 3.1 Hero camera component ownership

- `AT66HeroBase` constructs the gameplay camera stack directly.
- `CameraBoom` is a `USpringArmComponent` attached to the hero root.
- Current constructor defaults are:
  - `TargetArmLength = 2300`
  - relative location `FVector(0, 0, 60)`
  - `bUsePawnControlRotation = true`
  - `bDoCollisionTest = true`
  - `ProbeSize = 12`
  - `ProbeChannel = ECC_Camera`
- `FollowCamera` is attached to the boom socket and currently sets:
  - `bUsePawnControlRotation = false`
- Gameplay code treats this as the live third-person follow camera, while `PlayerCameraManager` is usually used as the first source of truth for traces and camera-space queries.

### 3.2 Player zoom and camera-driven aiming

- `AT66PlayerController::HandleZoom()` owns runtime camera zoom for standard gameplay.
- Normal gameplay zoom behavior:
  - scroll input changes `CameraBoom->TargetArmLength`
  - zoom speed is `25`
  - zoom range is clamped to `[1200, 2400]`
- Current comment in input code says:
  - `1200` is the closest allowed gameplay zoom
  - `2400` is the farthest allowed gameplay zoom
- Combat aiming already depends on the camera viewpoint:
  - `T66PlayerController_Combat.cpp` resolves aim origin and direction from `PlayerCameraManager`
  - it falls back to `FollowCamera` when needed
- Camera changes therefore affect not just presentation but also projectile and hit-trace behavior.

### 3.3 Hero One scoped camera override

- Scoped mode is handled in `AT66PlayerController::SetHeroOneScopeViewEnabled()`.
- Entering scope currently saves and later restores:
  - boom arm length
  - boom relative location
  - current FOV
  - boom collision-test state
  - `bUseControllerRotationYaw`
  - movement `bOrientRotationToMovement`
  - placeholder mesh visibility
  - skeletal mesh visibility
- Enter-scope override then forces:
  - `TargetArmLength = 0`
  - boom relative location `FVector(0, 0, 78)`
  - `bDoCollisionTest = false`
  - `FollowCamera` FOV `18`
  - hero controller-yaw rotation enabled
  - movement orient-to-movement disabled
- Scoped mouse-wheel input no longer changes arm length; it changes FOV instead:
  - min FOV `8`
  - max FOV `30`
  - step `2`
- Scoped shot traces use the live camera direction and currently cap at range `20000`.
- This is the clearest existing example of camera behavior living inside ability code rather than a shared camera-mode abstraction.

### 3.4 Occluder handling and collision behavior

- Main gameplay camera collision is still spring-arm based.
- `AT66HeroBase` also runs `UpdateCameraOccluderFade()` for the locally controlled, non-preview hero.
- Occluder fading currently:
  - sweeps from camera position toward five hero-body trace targets
  - uses a sphere trace radius of `18`
  - only considers actors tagged `T66_Map_TraversalBarrier` or `T66_Tower_Ceiling`
  - fades matching mesh materials toward opacity `0.22`
  - interpolates with speed `10`
- The current player-controller wall fade path traces from the gameplay camera to the hero and swaps only qualifying generated wall visuals to `/Game/Materials/M_CameraWallOccluderFade`.
- A visual must be tagged `T66_CameraOccludingWallVisual`, must also be a traversal barrier, must not be a tower ceiling, and must have vertical wall-like bounds before the fade material can be applied. Floors, ceilings, and hidden collision proxies are intentionally excluded.
- Generated tower wall visual components with `T66_CameraOccludingWallVisual` keep `ECC_Camera` query blocking even when gameplay camera collision is otherwise ignored, so the fade trace can discover the specific visible wall section without relying on hard hiding.
- `ResetCameraOccluderFade()` restores full opacity when the path clears or the camera-fade path is not valid.
- Current implication:
  - only tagged vertical wall visuals can fade
  - ordinary world blockers still rely on spring-arm pull-in unless they are explicitly authored as camera wall visuals
- `T66TowerMapTerrain.cpp` configures camera-wall visuals to block `ECC_Camera` for visibility tracing while leaving non-wall terrain out of the fade path.

## 4. Frontend Preview Cameras

- Frontend character preview does not use the hero gameplay spring arm.
- `AT66FrontendGameMode` spawns a dedicated `ACameraActor` for menu/world preview.
- Current preview camera spawn defaults are:
  - location `(-400, 0, 350)`
  - rotation `(-15, 0, 0)`
  - FOV `90`
  - unconstrained aspect ratio
- The game mode then repositions that camera from the preview stage's `GetIdealCameraLocation()` and `GetIdealCameraRotation()`.
- `AT66PlayerController` also maintains a local frontend preview camera path with the same basic setup:
  - spawned `ACameraActor`
  - FOV `90`
  - unconstrained aspect ratio
  - view-target handoff to the preview camera
- Current architecture therefore has two separate preview-camera orchestration paths:
  - authoritative/in-world frontend game mode path
  - local controller preview path


- The board converts 2D battle coordinates into widget coordinates and clamps gameplay to the board-space arena.
- Sprite facing, shadows, hit flashes, telegraphs, and combat text are simulation/presentation state in the widget runtime.
- Future Mini presentation tuning belongs in the Mini battle board transform and `AnimatedStyle.Mini.*` rendering tags, not in the main gameplay camera stack.

## 6. Current Improvement Priorities

- Centralize gameplay camera tuning. Current camera numbers are spread across hero construction, player input, scoped ability code, and preview-camera code.
- Decide the intended baseline gameplay framing. The constructor default arm length is `2300`, while normal zoom allows a wider range of `[1200, 2400]`.
- Reconcile spring-arm collision with manual occluder fading. The current hybrid approach can still snap on blockers that are not tagged for fade.
- Move ability-specific camera behavior toward a reusable camera-mode stack. Scoped ult currently hand-saves and restores multiple booleans, transforms, and visibility flags.
- Keep main-game and frontend preview camera rules intentionally documented. Preview cameras currently mirror the gameplay FOV target of `90` but do not share gameplay collision or zoom behavior.

## 7. Immediate Working Checklist

- Capture the desired main gameplay camera target:
  - preferred default distance
  - preferred pitch/yaw feel
  - acceptable minimum and maximum zoom
  - desired behavior indoors, near ceilings, and near traversal barriers
- Validate camera behavior in at least these contexts before changing live tuning:
  - open outdoor gameplay
  - tower interior or low-ceiling spaces
  - traversal barriers
  - Hero One scoped mode
  - hero selection preview
  - companion selection preview
- Record any camera changes here with:
  - old value
  - new value
  - files touched
  - reason for change
