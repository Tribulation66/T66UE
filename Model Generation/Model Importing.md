# Model Importing

This is the dedicated Unreal import guide for post-TRELLIS `T66` model assets.

Read this after [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md) once a Blender-side model, rig, or static mesh export is ready for Unreal.

## Scope

Use this file for:

- importing rigged character FBXs and animations
- repairing character materials after Unreal import
- updating `DT_CharacterVisuals`
- importing generated static mesh kits
- running Unreal-side verification and staged visual checks

Do not use this file as the art approval gate. Model quality, source image constraints, head/body assembly, and rigging checks live in:

- [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md)
- [Rigging Process.md](C:/UE/T66/Model%20Generation/Rigging%20Process.md)
- [MESH_APPROVAL_CHECKLIST.md](C:/UE/T66/Model%20Generation/MESH_APPROVAL_CHECKLIST.md)

## Golden Rules

- Normalize models in Blender before Unreal import.
- Character height should be `2.0` meters in Blender.
- Character mesh origin should be centered at the feet.
- Character bottom should sit on ground level before export.
- Import normalized characters into Unreal at uniform scale `1.0`.
- Do not use an Unreal import scale map as the production fix for size or grounding.
- Use the full editor wrapper for skeletal mesh plus animation imports.
- Do not use `UnrealEditor-Cmd.exe -run=pythonscript` for UE 5.7 skeletal mesh plus animation batches; that path hit a Slate assertion.
- Do not use `-ExecCmds="py path"` for import wrappers; Unreal can treat the script path as Python source text.
- Reload the affected DataTable after CSV changes.
- If assets, materials, DataTables, maps, or cooked content changed, do a full stage/cook before packaging-facing verification.
- Verify visually from the staged executable before calling a game-facing import done.

## Command Style

Use full editor commands for import wrappers:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe' `
  'C:\UE\T66\T66.uproject' `
  -ExecutePythonScript='C:/UE/T66/Scripts/RunImportMikeRigPrototypeAndExit.py' `
  -unattended -nop4 -nosplash
```

If a script wrapper exits the editor, wait for `UnrealEditor.exe` to close, then inspect:

- [Saved/Logs/T66.log](C:/UE/T66/Saved/Logs/T66.log)
- the import report emitted by the specific script

## Character Import: Mike Rig Prototype

This is the current Mike-only process-test import path. It intentionally rewires only `Hero_3_Chad` for in-game testing and does not redo the completed 12-hero Chad/Beachgoer batch.

Primary scripts:

- [RunImportMikeRigPrototypeAndExit.py](C:/UE/T66/Scripts/RunImportMikeRigPrototypeAndExit.py)
- [ImportMikeRigPrototype.py](C:/UE/T66/Scripts/ImportMikeRigPrototype.py)
- [RepairMikeRigPrototypeMaterials.py](C:/UE/T66/Scripts/RepairMikeRigPrototypeMaterials.py)
- [inspect_mike_rig_materials_blender.py](C:/UE/T66/Model%20Generation/Scripts/inspect_mike_rig_materials_blender.py)

Expected source comes from the Mike rig prototype export under:

- [Mike_Chad_RigPrototype_A03_LiftedNeckBridge](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge)

Run the import:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe' `
  'C:\UE\T66\T66.uproject' `
  -ExecutePythonScript='C:/UE/T66/Scripts/RunImportMikeRigPrototypeAndExit.py' `
  -unattended -nop4 -nosplash
```

Expected Unreal destination:

```text
/Game/Characters/Heroes/Hero_3/Chad/RigPrototype
```

Expected data wiring:

- [Content/Data/CharacterVisuals.csv](C:/UE/T66/Content/Data/CharacterVisuals.csv) row `Hero_3_Chad`
- `/Game/Data/DT_CharacterVisuals`

Expected reports:

- [Saved/MikeRigPrototypeImportReport.json](C:/UE/T66/Saved/MikeRigPrototypeImportReport.json)
- [Saved/MikeRigPrototypeMaterialRepairReport.json](C:/UE/T66/Saved/MikeRigPrototypeMaterialRepairReport.json)

Mike Pass02 material finding:

- FBX import can create material instances without imported texture assets.
- When that happens, the staged executable can render a black silhouette even though the mesh, skeleton, animations, and DataTable row are correct.
- Run the targeted repair path so packed Blender base-color images are extracted/imported, assigned to unlit material instances, and rebound to the skeletal mesh slots.

## Character Import: Completed Chad Batch01

This is the historical completed import and wiring pass for the 12 active heroes, default Chad plus Beachgoer variants. Do not rerun this batch unless explicitly asked.

The current front-end naming direction is `Chad`, `Stacy`, and `Beachgoer`. Some scripts and old asset paths still say `TypeA`; treat those as legacy implementation names unless the active DataTable or asset path still requires them.

Primary scripts:

- [RunImportTypeABatch01RiggedHeroesAndExit.py](C:/UE/T66/Scripts/RunImportTypeABatch01RiggedHeroesAndExit.py)
- [ImportTypeABatch01RiggedHeroes.py](C:/UE/T66/Scripts/ImportTypeABatch01RiggedHeroes.py)
- [RepairTypeABatch01Materials.py](C:/UE/T66/Scripts/RepairTypeABatch01Materials.py)
- [VerifyTypeABatch01HeroVisuals.py](C:/UE/T66/Scripts/VerifyTypeABatch01HeroVisuals.py)

Import command, only when intentionally rerunning the batch:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe' `
  'C:\UE\T66\T66.uproject' `
  -ExecutePythonScript='C:/UE/T66/Scripts/RunImportTypeABatch01RiggedHeroesAndExit.py' `
  -unattended -nop4 -nosplash
```

Expected report:

- [Saved/TypeABatch01RiggedHeroImportReport.json](C:/UE/T66/Saved/TypeABatch01RiggedHeroImportReport.json)

Verification command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\UE\T66\T66.uproject' `
  -run=pythonscript `
  -script='C:/UE/T66/Scripts/VerifyTypeABatch01HeroVisuals.py' `
  -unattended -nop4 -nosplash
```

Expected verification report:

- [Saved/TypeABatch01HeroVisualVerification.json](C:/UE/T66/Saved/TypeABatch01HeroVisualVerification.json)

The verification report must prove:

- every expected default and Beachgoer row loads
- skeletal meshes load
- idle animations load
- mesh and animation skeletons match
- material slots are texture-backed
- character material alpha path is masked correctly
- mesh height is near `200` cm
- bottom Z is near `0` cm

## Character Material Requirements

Character imports should use the existing unlit character material path:

- `M_Character_Unlit` remains the master material.
- Material instances must be texture-backed.
- `DiffuseColorMap` must be assigned.
- `DiffuseColorMap.A` must drive `Opacity Mask`.
- Masked alpha source cards must not render as opaque poster panels.

Texture presence alone is not enough. Verify that the material slot actually uses the texture and that the alpha mask is wired.

If imported characters render black, opaque, or card-like in the staged executable, check materials before rerigging or reimporting the mesh.

## DataTable Reloads

Character import scripts that edit [CharacterVisuals.csv](C:/UE/T66/Content/Data/CharacterVisuals.csv) must reload `/Game/Data/DT_CharacterVisuals`.

Relevant scripts:

- [SetupCharacterVisualsDataTable.py](C:/UE/T66/Scripts/SetupCharacterVisualsDataTable.py)
- [ReloadActiveHeroRosterDataTables.py](C:/UE/T66/Scripts/ReloadActiveHeroRosterDataTables.py)

Use the narrow reload path when only character visuals changed. Avoid broad reloads that touch unrelated authored data unless the task requires it.

Stacy note:

- Current Stacy entries fall back to Chad until real Stacy models exist.
- Do not import fake Stacy meshes just to fill those rows.

## Static Mesh And Environment Kit Import

Use this path for generated static meshes such as modular world kit pieces, not for rigged heroes.

Current coherent environment kit authority:

- [CoherentThemeKit01 run root](C:/UE/T66/Model%20Generation/Runs/Environment/CoherentThemeKit01)
- [batch_manifest.json](C:/UE/T66/Model%20Generation/Runs/Environment/CoherentThemeKit01/batch_manifest.json)
- [Raw/Trellis](C:/UE/T66/Model%20Generation/Runs/Environment/CoherentThemeKit01/Raw/Trellis)

Preparation script:

- [export_coherent_themekit_unreal_ready.py](C:/UE/T66/Model%20Generation/Scripts/export_coherent_themekit_unreal_ready.py)

Unreal import scripts:

- [RunImportCoherentThemeKit01AndExit.py](C:/UE/T66/Scripts/RunImportCoherentThemeKit01AndExit.py)
- [ImportStaticMeshes.py](C:/UE/T66/Scripts/ImportStaticMeshes.py)
- [RunImportStaticMeshesAndExit.py](C:/UE/T66/Scripts/RunImportStaticMeshesAndExit.py)
- [VerifyCoherentThemeKit01AndExit.py](C:/UE/T66/Scripts/VerifyCoherentThemeKit01AndExit.py)
- [VerifyImportBatch.py](C:/UE/T66/Scripts/VerifyImportBatch.py)

Expected source for Unreal-ready static imports:

```text
C:\UE\T66\SourceAssets\Import\WorldKit\CoherentThemeKit01
```

Expected Unreal destination:

```text
/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01
```

Run the kit import:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe' `
  'C:\UE\T66\T66.uproject' `
  -ExecutePythonScript='C:/UE/T66/Scripts/RunImportCoherentThemeKit01AndExit.py' `
  -unattended -nop4 -nosplash
```

Run the kit verification:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\UE\T66\T66.uproject' `
  -run=pythonscript `
  -script='C:/UE/T66/Scripts/VerifyCoherentThemeKit01AndExit.py' `
  -unattended -nop4 -nosplash
```

Static mesh rules:

- Treat `batch_manifest.json` as the coverage authority.
- Normalize pivot and scale before import.
- Use the generated Unreal-ready FBX path, not raw TRELLIS GLBs directly, unless a script is explicitly written for raw GLB import.
- Verify destination assets, material instances, and theme coverage after import.

## Staged Visual Verification

Do not call a game-facing import done from script success alone.

Run a full stage/cook when imported content, material assets, DataTables, maps, or cooked content changed:

```powershell
& 'C:\UE\T66\Scripts\StageStandaloneBuild.ps1' `
  -ClientConfig Development `
  -SkipShortcutRefresh
```

Use `-SkipBuild` only when code did not change. Do not use `-SkipCook` when new or changed assets need packaging verification.

Current staged executable:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Useful Mike staged capture command:

```powershell
& 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe' `
  -T66FrontendScreen=HeroSelection `
  -T66HeroSelectionPreviewHero=Hero_3 `
  -T66HeroSelectionBody=Chad `
  -T66AutoScreenshot='C:\UE\T66\Saved\MikeRigPrototypeUnrealCheck\MikeRigPrototype_StagedHeroSelection_Textured_QA.png' `
  -T66AutoScreenshotDelay=7 `
  -T66AutomationResX=1600 `
  -T66AutomationResY=900 `
  -T66AutomationWindowed `
  -windowed -ResX=1600 -ResY=900 -forcelogflush
```

Visual acceptance checks:

- expected hero and body variant appears
- mesh is visible and textured
- idle animation loads
- no black silhouette from missing textures
- no opaque source cards or poster panels
- feet are grounded
- scale reads correctly in the hero-selection camera
- imported asset is the one the DataTable row resolves

## Common Failure Modes

- `UnrealEditor-Cmd.exe -run=pythonscript` crashes during skeletal mesh plus animation import in UE 5.7.
- `-ExecCmds="py path"` parses the script path incorrectly.
- FBX material instances import without texture assets.
- Masked alpha is missing, so TRELLIS card fragments render as solid panels.
- A mesh skeleton is replaced while old animations still point to an incompatible skeleton.
- The mesh is scaled in Unreal instead of being normalized in Blender.
- DataTable CSV changes are made but `/Game/Data/DT_CharacterVisuals` is not reloaded.
- A staged executable test uses stale cooked content because `-SkipCook` was used after asset changes.

## Minimal Import Checklist

Before Unreal import:

- exported FBX/GLB reimported in Blender
- Blender screenshot proves scale, grounding, and material visibility
- character export is normalized to `2.0` meters and origin at feet
- expected scripts and destination paths are identified

After Unreal import:

- import report exists and has no blocking errors
- DataTable row points to the imported assets
- mesh and animations load
- skeletons match
- material slots are texture-backed
- bounds are near `200` cm tall and bottom Z is near `0` cm

Before handoff:

- full stage/cook completed when content changed
- staged executable screenshot was captured
- visual check confirms expected model, materials, grounding, and animation
