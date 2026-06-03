# Claude Phase 5 Prompt - Hero 1 Bounce Visual Revision

You are the Claude Operator for `C:\UE\T66`. Codex is the Validator/integrator. Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/README.md`, `Reports/AGENTS.md`, and the combat VFX process docs already used for this task. Mini/minigame scope is excluded.

Working task: revise the Hero 1 Bounce weapon VFX so the production Niagara small horizontal slash is visibly readable at each Bounce impact point in gameplay proof, while preserving the already-passing Bounce target selection, damage, and per-link ImpactAnchored runtime routing.

Current live state:
- Bounce routing and damage are passing.
- `Source/T66/Gameplay/T66CombatComponent.cpp` publishes one Bounce impact context per chain link and spawns `Hero1Axe_Bounce_Base` at each impact.
- Capture log from `C:\UE\T66\Saved\Logs\T66.log` shows three production spawns:
  - Chain 0 at `ImpactPoint=V(X=360.00, Z=64.00)`
  - Chain 1 at `ImpactPoint=V(X=360.00, Y=150.00, Z=64.00)`
  - Chain 2 at `ImpactPoint=V(X=510.00, Y=150.00, Z=64.00)`
  - All have `VisualAnchorModel=ImpactAnchored`, `ImpactOffsetFromDamageCenter=0.00`, `VisualScaleVec=V(X=1.00,Y=1.00,Z=1.00)`.
- Damage proof in the same log passes: primary, ChainSecond, ChainThird hit; two neutral targets miss.
- Current video evidence at `Saved\VideoCaptures\hero1axebouncevfxbinding_20260529_074645\` does not show a clear red/blue horizontal slash at those impact points. Selected frames mainly show damage numbers; later frames show unreadable/possibly misplaced colored remnants.

Likely defect to investigate:
- `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp` appears to build the Bounce static mesh in normalized local units (`MaxHalfLength=1.30`, `MaxHalfHeight=0.22`, tiny fixed bounds) while runtime ImpactAnchored scale uses `VisualScaleMultiplier=1.0` and assumes the authored carrier is already gameplay-sized.
- A valid fix may be to author the mesh and Niagara fixed bounds in Unreal centimeters for the intended small slash footprint, or to adjust the ImpactAnchored scale/binding contract so `BaseVisualRadius=80` actually maps to the mesh. Choose the smaller repo-consistent fix.

Important constraints:
- Do not change Bounce gameplay behavior, target selection, chain count, damage, or non-Bounce attack behavior.
- Do not replace the Niagara carrier with debug C++ geometry or a temporary projectile.
- Preserve the process class: primary silhouette must remain authored by Niagara/material/renderer mesh/emitter logic.
- Keep the visual a small horizontal slash, using Hero 1 red/blue/white vocabulary.
- Do not touch Mini/minigame files.

Allowed edit scope:
- `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp`
- Generated Bounce VFX lab/production assets under `/Game/VFXLab/Hero1Axe/Bounce` and `/Game/VFX/Hero1/Axe/Bounce`
- Only if needed for scale consistency, the binding/setup/validator rows for `Hero1Axe_Bounce_Base`
- Your operator report under `Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/`

Expected result:
- A production Bounce Niagara mesh slash with an authored footprint large enough to read from the existing proof camera, centered on each impact point, with sensible fixed bounds.
- The slash should remain compact relative to the enemy hit zone; do not turn it into the AOE crescent or the Pierce lane.

Verification to run:
1. Focused build if required after C++ changes:
   `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
2. Regenerate lab and production Bounce assets:
   `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeBounceVFX -unattended -nop4 -nosplash -abslog='C:\UE\T66\Saved\Logs\Hero1AxeBounceVFX_Lab_Phase5VisualRevision.log'`
   `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeBounceVFX -T66Hero1AxeBounceProduction -unattended -nop4 -nosplash -abslog='C:\UE\T66\Saved\Logs\Hero1AxeBounceVFX_Production_Phase5VisualRevision.log'`

Deliverable:
- Write a report file under `Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/` describing:
  - Exact files changed
  - Root cause found
  - Exact scale/bounds values chosen and why
  - Commands run and pass/fail evidence
  - Any caveats Codex must validate with gameplay capture
