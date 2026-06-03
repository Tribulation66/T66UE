# Claude FullOperator Phase 3: Hero 1 Bounce VFX Assets

You are Claude Operator. Codex approved this bounded phase in:

`Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/codex_operator_approval_phase3_assets.md`

Read that approval artifact first and stay within it exactly.

Task: create the Hero 1 Bounce small horizontal slash asset-generation path and generate both lab and production Bounce VFX assets. This phase is asset/commandlet only.

Required source of truth:

- `Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/plan_packet.md`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.*`
- `Source/T66/Gameplay/T66Hero1AxePierceVFXCommandlet.*`
- Existing Hero 1 slash materials/assets only as references to preserve the red/blue material language.

Approved source scope:

- Add/edit `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.h`
- Add/edit `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp`
- Minimal adjacent source include/build integration only if required for compile.

Approved generated asset scope:

- `Content/VFXLab/Hero1Axe/Bounce/`
- `Content/VFX/Hero1/Axe/Bounce/`

Requirements:

- Primary carrier: small horizontal slash mesh.
- Mesh name: `SM_Hero1AxeBounce_HorizontalSlash`.
- Niagara system name: `NS_Hero1AxeBounce_MeshSlash`.
- The slash silhouette must live in the generated mesh/Niagara/material renderer path, not actor-side geometry.
- Reuse the existing Hero 1 red/blue slash material language where practical.
- Generate lab assets with the no-production commandlet run and production assets with the production flag.
- Keep Bounce binding, DataTable refresh, validators, runtime behavior, capture harness, Git, Mini/minigame, imagegen, and credentials out of scope.

Verification:

- Run focused editor build if possible:
  `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
- Run lab commandlet if build succeeds:
  `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeBounceVFX -unattended -nop4 -nosplash -log=Saved\Logs\Hero1AxeBounceVFX_Lab.log`
- Run production commandlet if lab succeeds:
  `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeBounceVFX -T66Hero1AxeBounceProduction -unattended -nop4 -nosplash -log=Saved\Logs\Hero1AxeBounceVFX_Production.log`
- Report exact changed source files, exact generated package paths, command/log paths, warnings, and pass/fail markers.
- If you need to exceed scope, stop and request Codex approval instead of continuing.
