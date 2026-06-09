Draft completion:

Decision: NO. Heroes should not use PAC for ragdoll/knockback. The hero direction is pure Chaos ragdoll: launch the body, let physics carry it, keep the actor/camera following the simulated body, and recover afterward. This matches the user's stated rule that the character should not be dragged back toward a target pose or intended location.

Implemented:
- `Source/T66/Gameplay/T66KnockbackComponent.h`: default `FT66KnockbackProfile` now has `bEnablePhysicalAnimation=false` and `PhysicalAnimationDriveMode=Disabled`.
- `Source/T66/Gameplay/T66KnockbackComponent.cpp`: runtime guard now disables PAC for hero ragdoll profiles even if a profile tries to enable it. The detached-ragdoll guard remains for non-hero defense-in-depth.
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`: TestRoom wipeout-arm PAC CVars now default to off/disabled.
- `Source/T66/Gameplay/pending_issues_Gameplay.md`: updated stale note that previously described PAC as the active hero direction.

PAC infrastructure intentionally remains in code. This is a disable/policy change, not a removal. It keeps old code reachable only if a future explicit non-hero/non-detached experiment is approved.

Verification:
- Focused `T66Editor Win64 Development` build passed.
- `Scripts/StageStandaloneBuild.ps1` passed, producing `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`.
- Shortcut target verified as `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Existing unrelated compile warning remains: `T66Hero1AxeAOEVFXLabActor.cpp(704)` uses deprecated `FNiagaraEmitterInstance::IsReadyToRun`.

Caveat:
- I did not remove all PAC code from the project. Removing the subsystem entirely would be broader cleanup and is not needed to answer the hero decision.
