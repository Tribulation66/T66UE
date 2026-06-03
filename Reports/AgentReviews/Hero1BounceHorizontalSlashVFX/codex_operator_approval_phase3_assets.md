Codex Approval: APPROVE

## Approved Task

Phase 3 asset/commandlet work only: create the Hero 1 Bounce horizontal slash Niagara/mesh generation path and generate lab plus production Bounce VFX assets.

## Approved Scope

Claude FullOperator may edit or add:

- `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.h`
- `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp`
- Minimal adjacent source include/build integration only if required by compile.

Claude may generate assets under:

- `Content/VFXLab/Hero1Axe/Bounce/`
- `Content/VFX/Hero1/Axe/Bounce/`

Expected asset direction:

- Primary carrier: small horizontal slash mesh.
- Niagara system: `NS_Hero1AxeBounce_MeshSlash`.
- Mesh: `SM_Hero1AxeBounce_HorizontalSlash`.
- Reuse existing Hero 1 slash material family where practical, especially the AOE/Pierce shared red/blue slash-layer materials.
- Author production and lab asset paths through the commandlet, using a production flag comparable to Pierce.

## Approved Tool Surface

Claude may edit approved source files and run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeBounceVFX -unattended -nop4 -nosplash -log=Saved\Logs\Hero1AxeBounceVFX_Lab.log

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=T66Hero1AxeBounceVFX -T66Hero1AxeBounceProduction -unattended -nop4 -nosplash -log=Saved\Logs\Hero1AxeBounceVFX_Production.log
```

Adjust commandlet flag names only if the implementation records the exact names and reason.

## Required Process Rules

- Follow `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`.
- Primary silhouette must live in Niagara/material/mesh renderer assets.
- Do not use actor-side geometry as the primary carrier.
- Reuse the Hero 1 slash material language where practical.
- Generated assets must be under the approved lab/production Bounce paths.

## Explicitly Excluded Actions

- No runtime behavior edits beyond minimal compile integration for the new commandlet.
- No `Content/Data/CombatVFXBindings.csv` edits.
- No DataTable refresh.
- No production binding validator edits.
- No capture harness edits.
- No gameplay capture.
- No Mini/minigame work.
- No Git mutation.
- No broad Git/LFS scans.
- No imagegen.
- No credential or billing changes.

## Verification Required After Operator Run

Claude should report:

- changed/added source files,
- exact build command and result,
- exact commandlet commands and results,
- generated asset package paths,
- any warnings or skipped checks.

Codex will validate source scope and asset file existence.

## Approval Rationale

This phase creates the Bounce carrier assets independently from production binding, so runtime dispatch and DataTable changes remain separately reviewable.
