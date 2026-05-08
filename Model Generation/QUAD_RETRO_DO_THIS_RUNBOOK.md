# Quad Retro Do-This Runbook

This is the practical execution guide for the current one-image, one-model
Quad Retro character pipeline. Use this file when an agent needs to run
Trellis output through Quad Remesher and the Blender bake/texture pipeline.
It was proven first on Chad/Stacy heroes, and the same retro pass should be
used for enemy and boss characters after their raw Trellis models are approved.

If another doc conflicts with this runbook, follow this runbook for execution.
Older Type A, split head/body, voxel remesh, direct decimate, white-background,
and first-pass dither-heavy instructions are historical.

## Read First

1. [TRELLIS_SOURCE_IMAGE_RULES.md](C:/UE/T66/Model%20Generation/TRELLIS_SOURCE_IMAGE_RULES.md)
2. [HERO_CHAD_STACY_PROMPT_GUIDE.md](C:/UE/T66/Model%20Generation/HERO_CHAD_STACY_PROMPT_GUIDE.md)
3. [RETRO_CHARACTER_PIPELINE.md](C:/UE/T66/Model%20Generation/RETRO_CHARACTER_PIPELINE.md)
4. This file.

For enemy/boss model batches, also read:

- [EnemyBossRoster_DataContract_2026-05-07.md](C:/UE/T66/Docs/Systems/EnemyBossRoster_DataContract_2026-05-07.md)
- [Enemies.csv](C:/UE/T66/Content/Data/Enemies.csv)
- [Bosses.csv](C:/UE/T66/Content/Data/Bosses.csv)
- [BossEncounters.csv](C:/UE/T66/Content/Data/BossEncounters.csv)
- [BossEncounterMembers.csv](C:/UE/T66/Content/Data/BossEncounterMembers.csv)

The enemy/boss CSVs are the live roster source of truth. Do not use archived
placeholder rosters.

## Known Good Look

The accepted baseline look is the fixed-bake Medium pass:

- Output: [BoxerChad_Medium_QuadRetro.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake/Medium/BoxerChad/Models/BoxerChad_Medium_QuadRetro.glb)
- Front render: [BoxerChad_Medium_front.png](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake/Renders/Front/BoxerChad_Medium_front.png)
- Report: [BoxerChad_Medium_QuadRetro_report.json](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake/Medium/BoxerChad/Reports/BoxerChad_Medium_QuadRetro_report.json)

Baseline values from that successful report:

| Value | Baseline |
| --- | ---: |
| `target_quads` | `12000` |
| `adaptive_size` | `50` |
| `texture_size` | `512` |
| `bake_size` | `1024` |
| `palette_mode` | `none` |
| `dither_type` | `none` |
| `dither_strength` | `0` |
| actual `retopo_quads` | `13754` |
| actual `retopo_triangles` | `27669` |

Use this Medium baseline before trying more stylized color or dither passes.
The first-pass palette/dither ladder failed because it erased outfit color
identity.

## Hard Rules

- Use foreground Blender for Quad Remesher runs. Do not pass `-Background:$true`
  and do not call `blender.exe --background` for the Quad Remesher step.
- Do not launch Blender hidden, detached, or redirected through `Start-Process`
  for Quad Remesher runs. Use a normal foreground Blender process and let the
  wrapper block until it exits.
- Do not run multiple Quad Remesher jobs at once. The engine uses a shared
  temp host folder under `%TEMP%\Exoside\QuadRemesher\Blender`.
- Do not start with palette reduction or Bayer dithering. Preserve color first.
- Do not run Stacy variants during a male-only pass. Do not invent extra names.
  The current public roster is exactly the names in
  [HERO_CHAD_STACY_PROMPT_GUIDE.md](C:/UE/T66/Model%20Generation/HERO_CHAD_STACY_PROMPT_GUIDE.md).
- Do not treat a process report as art approval. Always provide at least a
  front screenshot for review.
- Keep generated filenames and manifest row IDs aligned with the live data row
  IDs. Unreal import mappings key off those IDs.
- Do not start the Quad Retro/Blender pass until raw Trellis models have been
  reviewed and approved from front QA renders.

## Preflight

Check for stale Blender or Quad Remesher processes:

```powershell
Get-Process blender,xremesh -ErrorAction SilentlyContinue
```

If there is a visible Blender window from a human, leave it alone. If there is
a stale automated Blender or `xremesh.exe` process from a failed previous run,
close it before starting the next single run.

Check the Quad Remesher engine exists:

```powershell
Test-Path "C:\ProgramData\Exoside\QuadRemesher\Datas_Blender\QuadRemesherEngine_1.4\xremesh.exe"
```

If Quad Remesher reports EULA/trial/license activation, open
`xrLicenseManager.exe`, activate the trial/license, then rerun.

## Run One Character

Use this command shape. The important parts are `-Background:$false`,
`-PaletteMode "none"`, `-DitherType "none"`, and `-DitherStrength 0`.

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Model Generation\Scripts\RunQuadRetroCharacterPipeline.ps1" `
  -InputModel "C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\Raw\Trellis\RoyalChad_Source_02_S1337_D80000_Trellis2.glb" `
  -OutputDir "C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\QuadRetroPipeline\Medium\RoyalChad" `
  -Label "RoyalChad" `
  -TargetQuads 12000 `
  -AdaptiveSize 50 `
  -TextureSize 512 `
  -PaletteMode "none" `
  -DitherType "none" `
  -DitherStrength 0 `
  -BakeSize 1024 `
  -RenderQA:$true `
  -Background:$false
```

PowerShell boolean gotcha: if a command is generated as a string and launched
through another shell, loose `$true` / `$false` can become strings. Prefer
`-RenderQA:$true` and `-Background:$false`. The wrapper now also accepts
`"true"` / `"false"` strings, but colon booleans are still the clearest form.

Expected outputs:

- `Models\<Label>_QuadRetro.glb`
- `Textures\<Label>_QuadRetro_Bake1024.png`
- `Textures\<Label>_QuadRetro_Pixelated_512.png`
- `Reports\<Label>_QuadRetro_report.json`
- if `-RenderQA:$true`, `Renders\<Label>_front.png`,
  `Renders\<Label>_right.png`, `Renders\<Label>_back.png`, and
  `Renders\<Label>_oblique.png`

## Preset Ladder

For first review, use Medium only. If the user asks for a ladder, use:

| Preset | `target_quads` | `texture_size` | `palette_mode` | `dither_type` | `dither_strength` |
| --- | ---: | ---: | --- | --- | ---: |
| Low | `30000` | `1024` | `none` | `none` | `0` |
| Medium | `12000` | `512` | `none` | `none` | `0` |
| High | `3000` | `256` | `none` | `none` | `0` |

The earlier notes sometimes say Medium `10000`; the accepted Boxer Chad Medium
result was rerun at `12000` and produced about `13.7k` quads / `27.7k` tris.
Use `12000` as the current practical Medium baseline.

## Verify Every Run

Open the report JSON and confirm:

- `output_glb` exists
- `retopo_quads` and `retopo_triangles` are present
- `adjustable_values.palette_mode` is `none`
- `adjustable_values.dither_type` is `none`
- `adjustable_values.dither_strength` is `0`
- `adjustable_values.texture_size` matches the requested preset
- `qremesh_report.last_progress` is `2`

Example quick check:

```powershell
$report = Get-Content -Raw "C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\QuadRetroPipeline\Medium\RoyalChad\Reports\RoyalChad_QuadRetro_report.json" | ConvertFrom-Json
$report.adjustable_values
$report.retopo_quads
$report.retopo_triangles
$report.qremesh_report.last_progress
Test-Path $report.output_glb
```

If `-RenderQA:$true` was not used, make a front render with the QA helper:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python "C:\UE\T66\Model Generation\Scripts\blender_glb_qa.py" -- `
  --input "C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\QuadRetroPipeline\Medium\RoyalChad\Models\RoyalChad_QuadRetro.glb" `
  --render "C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\QuadRetroPipeline\Medium\RoyalChad\Renders\RoyalChad_front.png" `
  --yaw 0 `
  --pitch 2 `
  --resolution 1200
```

Using background Blender is fine for this QA render helper. The foreground rule
applies to the Quad Remesher pipeline run.

## If It Fails

Failure: `Cannot convert value "System.String" to type "System.Boolean"`  
Fix: rerun with `-RenderQA:$true` and `-Background:$false`, or use the updated
wrapper which accepts string booleans.

Failure: Quad Remesher stops without output or host communication fails  
Fix: close stale `blender.exe` / `xremesh.exe`, rerun one character at a time
through foreground Blender, and do not redirect/hide the process.

Failure: EULA/license/trial activation  
Fix: activate through `xrLicenseManager.exe`.

Failure: character loses outfit colors  
Fix: verify `palette_mode=none`, `dither_type=none`, `dither_strength=0`, and
use the Medium baseline before any stylization pass.

Failure: model is rotated/sideways or bake streaks across the body  
Fix: use the current pipeline script. It clears the Quad Remesher FBX axis
rotation before unwrap/bake. Do not use archived first-pass outputs.

## Unreal Import Lessons From Chad/Stacy

The 24 Chad/Stacy Quad Retro hero pass found several integration issues that
future enemy/boss imports must avoid.

### Static Mesh Character Visuals

Rigging is not required for a first playable pass. The runtime now supports
unrigged imported static meshes through `FT66CharacterVisualRow.StaticMesh` and
`AT66HeroBase::StaticVisualMesh`. The model remains playable through the
existing actor/capsule; only the visual component is static.

For character imports that are not rigged yet:

- Clear `SkeletalMesh`, `LoopingAnimation`, `AlertAnimation`, and
  `RunAnimation` for the default visual row.
- Set `StaticMesh` to the imported static mesh object path.
- Use `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)` for Trellis/Blender
  character fronts produced by this pipeline unless QA proves otherwise.
- Set `bAutoGroundToActorOrigin=true`.
- Scale to the intended in-game height before writing the visual row.

Relevant implementation files:

- [T66DataTypes.h](C:/UE/T66/Source/T66/Data/T66DataTypes.h)
- [T66CharacterVisualSubsystem.cpp](C:/UE/T66/Source/T66/Core/T66CharacterVisualSubsystem.cpp)
- [T66HeroBase.cpp](C:/UE/T66/Source/T66/Gameplay/T66HeroBase.cpp)
- [T66HeroPreviewStage.cpp](C:/UE/T66/Source/T66/Gameplay/T66HeroPreviewStage.cpp)

### Unreal Editor Import Must Finish

When launching `UnrealEditor.exe -ExecutePythonScript=...`, the shell may return
before the editor/import work is actually finished. Always check the process
and the log before judging completion:

```powershell
Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue
Get-Content "C:\UE\T66\Saved\Logs\T66.log" -Tail 80
```

Do not launch a second import while the first `UnrealEditor.exe` import process
is still running.

### GLB Texture Binding

The Chad/Stacy GLB import preserved the pixel textures, but the first material
conversion pass bound material instances to Unreal's fallback `T_White_srgb`
texture. The result in-game was a flat white/gray silhouette.

Required fix for future imports:

- After converting GLB material instances to `/Game/Materials/M_GLB_Unlit`,
  explicitly set `BaseColorTexture` and `DiffuseColorMap` to the sibling
  imported `*_Pixelated_512` texture.
- Verify no material slot still points at `T_White_srgb` or
  `WhiteSquareTexture`.
- Save the material instances and the static mesh after rebinding.

Current helper scripts:

- [ImportQuadRetroHeroVisuals.py](C:/UE/T66/Scripts/ImportQuadRetroHeroVisuals.py)
  now explicitly binds imported pixelated textures during import.
- [RepairQuadRetroHeroTexturesAndExit.py](C:/UE/T66/Scripts/RepairQuadRetroHeroTexturesAndExit.py)
  repairs already-imported Quad Retro hero materials.
- [VerifyQuadRetroHeroVisualsAndExit.py](C:/UE/T66/Scripts/VerifyQuadRetroHeroVisualsAndExit.py)
  verifies static mesh rows and material texture bindings.

Successful verification criteria:

- all expected visual rows exist
- all target rows load as `StaticMesh`
- every material slot has a `BaseColorTexture` or `DiffuseColorMap`
- zero fallback white texture hits

### Staged Build And Taskbar Shortcut

Runtime-facing asset changes must be cooked into the staged standalone build.
Editor import success alone is not enough.

Use:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development
```

For asset-only restage after a successful build, `-SkipBuild` is acceptable,
but do not use `-SkipCook` when new or repaired assets must reach the packaged
runtime.

Verify both shortcuts target:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Check with:

```powershell
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut("$env:APPDATA\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk")
$shortcut.TargetPath
```

## Enemy/Boss Batch Stage Plan

The next character batch is the live enemy/boss roster:

- 25 regular enemies from [Enemies.csv](C:/UE/T66/Content/Data/Enemies.csv)
- 23 boss characters from [Bosses.csv](C:/UE/T66/Content/Data/Bosses.csv)

Stage 1 is source image and raw Trellis generation only. Do not run Quad
Remesher, Blender retro processing, Unreal import, or staged build updates in
Stage 1.

Recommended shared run root:

```text
C:\UE\T66\Model Generation\Runs\EnemyBosses\EnemyBossBatch01
```

Recommended Stage 1 layout:

```text
Inputs\Prompts\Enemies.csv-derived prompts
Inputs\Prompts\Bosses.csv-derived prompts
Inputs\SourceImages\Enemies\<EnemyID>.png
Inputs\SourceImages\Bosses\<BossID>.png
Raw\Trellis\Enemies\<EnemyID>\<EnemyID>_Trellis.glb
Raw\Trellis\Bosses\<BossID>\<BossID>_Trellis.glb
QA\TrellisFront\Enemies\<EnemyID>_front.png
QA\TrellisFront\Bosses\<BossID>_front.png
Reports\Stage01_TrellisManifest.json
Notes\STAGE01_STATUS.md
```

Work split for a two-chat/two-RunPod pass:

- Main/local chat owns regular enemies: all 25 rows from `Enemies.csv`.
- Secondary RunPod chat owns bosses: all 23 rows from `Bosses.csv`, including
  all four Stage 17 horsemen as separate model rows.
- Both chats must use the exact row ID as the asset ID and filename prefix.
- Both chats must update the same manifest schema, or provide a manifest shard
  that can be merged without renaming.

Minimum manifest fields per row:

```json
{
  "row_id": "Dungeon_Slime",
  "source_table": "Enemies.csv",
  "display_name": "Dungeon Slime",
  "difficulty_id": "Easy",
  "theme_id": "Dungeon",
  "family_or_role": "Melee",
  "visual_concept": "...",
  "image_prompt": "...",
  "source_image": "Inputs/SourceImages/Enemies/Dungeon_Slime.png",
  "raw_trellis_glb": "Raw/Trellis/Enemies/Dungeon_Slime/Dungeon_Slime_Trellis.glb",
  "qa_front_render": "QA/TrellisFront/Enemies/Dungeon_Slime_front.png",
  "status": "PendingReview"
}
```

After Stage 1, review front QA renders and only then run the Quad Retro Medium
pipeline from this runbook on approved raw Trellis GLBs.
