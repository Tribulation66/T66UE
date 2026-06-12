Codex Approval: APPROVE

## Approved Task

Phase 4 binding/setup/validator/capture-harness work only: production-bind Hero 1 Bounce to the generated Bounce Niagara system, refresh the Combat VFX DataTable, extend structural validation, and add a deterministic gameplay capture mode for later Bounce proof video.

## Approved Scope

Claude FullOperator may edit:

- `Content/Data/CombatVFXBindings.csv`
- `Scripts/SetupCombatVFXBindingsDataTable.py`
- `Scripts/ValidateCombatVFXProductionBindings.py`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- Minimal adjacent source includes/build integration only if required by compile.

Claude may generate/update these runtime data assets through the approved setup script:

- `Content/Data/DT_CombatVFXBindings.uasset`
- `Content/Blueprints/Core/BP_T66GameInstance.uasset` only if `SetupCombatVFXBindingsDataTable.py` saves it as part of the existing binding-table assignment path.

Expected binding row:

- Row/BindingID: `Hero1Axe_Bounce_Base`
- SourceType: `WeaponBase`
- SourceID: `Hero_1_black_bounce`
- AttackCategory: `Bounce`
- NiagaraSystem: `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash.NS_Hero1AxeBounce_MeshSlash`
- EffectPacketID: `Hero1AxeBounceMechanismPacket`
- VFXProfile: `MeshSlashBounce`
- bSuppressTemporaryProjectile: `True`
- bDevelopmentFallbackAllowed: `True`
- BaseVisualRadius: `80.0`
- BasePlaybackSeconds: `0.32`
- VisualScaleMultiplier: `1.0`
- Notes should identify the small horizontal ImpactAnchored per-link Bounce slash.

Expected capture route:

- Add capture mode `hero1axebouncevfxbinding`.
- It must equip/use the Bounce weapon/category, not AOE or Pierce.
- It must spawn enough targets for at least a primary hit plus one chained second target.
- It must keep `T66.Combat.ImpactSourceVerbose 1` active for impact-context evidence.
- It should preserve existing AOE, Pierce, and Water idol capture modes.

## Approved Tool Surface

Claude may edit approved files and run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\SetupCombatVFXBindingsDataTable.py' -unattended -nop4 -nosplash -abslog='C:\UE\T66\Saved\Logs\SetupCombatVFXBindingsDataTable_Bounce_CodexApproved.log'

python Scripts\ValidateCombatVFXProductionBindings.py --self-test-root C:\UE\T66\Saved\Tmp\CombatVFXValidatorSelfTest_Bounce

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py' -unattended -nop4 -nosplash -abslog='C:\UE\T66\Saved\Logs\ValidateCombatVFXProductionBindings_Bounce_CodexApproved.log'

.\Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axebouncevfxbinding -UseHero1AxePreviewStaging -PrintOnly
```

Adjust log filenames only if the output artifact records exact paths and reason.

## Required Process Rules

- Follow `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`.
- The production binding must point at production Bounce Niagara, not lab assets.
- The capture mode must prove the Bounce category path and should not be a relabeled AOE/Pierce capture.
- Do not use desktop screenshots as proof.
- Keep Mini/minigame out of scope.

## Explicitly Excluded Actions

- No Bounce commandlet or generated VFX asset edits.
- No runtime combat behavior edits beyond capture harness selection/diagnostic code in `T66PlayerController_Overlays.cpp`.
- No gameplay capture/video yet; this phase may use `-PrintOnly` only.
- No staged standalone build.
- No Git mutation.
- No broad Git/LFS scans.
- No imagegen.
- No credentials or billing changes.
- No Mini/minigame work.

## Verification Required After Operator Run

Claude should report:

- exact files/assets changed,
- final `Hero1Axe_Bounce_Base` CSV row,
- setup script/DataTable refresh result and log path,
- validator self-test result,
- Unreal validator result and log path,
- build result,
- `CaptureT66GameplayVideo.ps1 -PrintOnly` command/arguments showing `hero1axebouncevfxbinding`,
- source anchors proving Bounce capture mode selects `ET66AttackCategory::Bounce`.

Codex will validate actual changed files, logs, and scope.

## Approval Rationale

This phase connects the already-generated Bounce production VFX to the existing Combat VFX data and proof infrastructure without yet performing the final gameplay video capture.
