Result: OK

## Operator Draft

Implemented the requested enemy-damage reaction threshold.

Scope:
- Enemy-style damage sources are identified by `AT66EnemyBase`, `AT66MobBase`, or delivery names beginning with `Enemy`.
- Enemy hits still apply damage percent through `UT66RunStateSubsystem::ApplyDamage`.
- Enemy hits at or below `t66.HealthPercent.EnemyDisableStartPercent` (default 50) now apply `LaunchCharacter` knockback only and do not call `UT66HeroPhysicsComponent::ApplyPhysicsReaction`.
- Enemy hits whose resulting percent is above the threshold call the hero physics ragdoll path.
- Enemy ragdoll launch scale and duration scale now ramp from `t66.HealthPercent.EnemyDisableStartPercent` (default 50) through `t66.HealthPercent.EnemyDisableFullPercent` (default 99).
- Death remains owned by `HeroDamagePercent >= 100`.
- Non-enemy ragdoll paths keep their existing duration curve by using the new hero-physics duration-window parameters only when the run-state passes enemy-specific values.

Files changed:
- `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Gameplay/Physics/CURRENT_STATE.md`
- `Gameplay/Physics/MASTER_PHYSICS.md`
- `Gameplay/Combat/MASTER_COMBAT.md`

Verification performed:
- Focused editor compile passed:
  `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -FromMsBuild`
- Editor gameplay proof passed:
  `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode combatdamagelog ...`
  Log evidence in `Saved\Logs\T66.log`:
  - `Mode=EnemyKnockbackOnly` for 20 percent enemy hit, `PercentAfter=20.00`, `HeroKnockbackAfterFirst=0`, `BelowThresholdNoRagdoll=1`
  - `Mode=EnemyRagdoll` for 60 percent enemy hit, `PercentAfter=60.00`, `ThrowAlpha=0.204`, `DurationScaleStart=50.00`, `DurationScaleFull=99.00`, `AboveThresholdRagdoll=1`
- Staged readiness wrapper was run:
  `Scripts\RunStagedBuildReadinessGate.ps1 -OutputRoot C:\UE\T66\Saved\StagedBuildReadiness\EnemyDamageReactionThreshold_20260609_045804 -ContinueOnFailure`
  Stage passed, staged exe exists, project-root shortcut and pinned taskbar shortcut target the refreshed staged exe. Wrapper status is FAIL because the child smoke suite reports `BUILD_CONFIG_UNSUPPORTED` for durable-save and lifecycle gates in this Development staged run; frontend tag-click passed.
- Targeted staged executable proof passed:
  `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe /Game/Maps/GameplayLevel -T66GameplayAutoCapture=combatdamagelog ...`
  Exit code 0. Log evidence in `Saved\StagedProof\EnemyDamageReactionThreshold_20260609_0507\run.log`:
  - `Mode=EnemyKnockbackOnly` for 20 percent enemy hit.
  - `Mode=EnemyRagdoll` for 60 percent enemy hit with `DurationScaleStart=50.0` and `DurationScaleFull=99.0`.
  - `[HealthPercentQA] ... BelowThresholdNoRagdoll=1 AboveThresholdRagdoll=1 ... FullHealReset=1`.

Known caveat:
- Boss deliveries are intentionally not included in the enemy threshold unless they use an `Enemy*` delivery name. The prompt said enemies, and boss/trap/lava/miasma paths remain outside this scoped change.
