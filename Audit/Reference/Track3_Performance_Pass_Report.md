# Track 3 Character Pipeline Performance Pass Report

Date: 2026-05-12  
Workspace: `C:\UE\T66`

## Working Goal

Apply the three requested character pipeline performance changes, verify each in the live project, refresh the staged standalone build, and record evidence for Claude handoff.

## Task 1 - Character Texture Streaming Defaults

### What Changed

- Added `Scripts/QuadRetroCharacterPipelineDefaults.py` as the shared character-pipeline defaults helper.
- Added `Scripts/SetCharacterTextureStreamingDefaults.py`.
- Updated character import paths so new `/Game/Characters/` texture imports apply the same defaults:
  - `Scripts/ImportQuadRetroEnemyVisuals.py:107`
  - `Scripts/ImportQuadRetroBossVisuals.py:77`
  - `Scripts/ImportQuadRetroHeroVisuals.py:99`
  - `Scripts/ImportSkeletalMeshes.py:113`
  - `Scripts/ImportStaticMeshes.py:546-558`
- The shared helper applies:
  - `LODGroup = TEXTUREGROUP_CHARACTER`
  - `NeverStream = false`
  - `VirtualTextureStreaming = false`
  - `MipGenSettings = TMGS_FROM_TEXTURE_GROUP` when the asset was set to no mipmaps
  - best-effort `texture_streaming_method` assignment when the UE Python property accepts it
- Relevant implementation references:
  - `Scripts/QuadRetroCharacterPipelineDefaults.py:136-217`
  - `Scripts/SetCharacterTextureStreamingDefaults.py:102`

### Verification

Commands run:

```powershell
python -m py_compile Scripts\QuadRetroCharacterPipelineDefaults.py Scripts\SetCharacterTextureStreamingDefaults.py Scripts\GenerateCharacterMeshLODs.py Scripts\MigrateQuadRetroMaterialAssignment.py Scripts\ImportQuadRetroEnemyVisuals.py Scripts\ImportQuadRetroBossVisuals.py Scripts\ImportQuadRetroHeroVisuals.py Scripts\ImportSkeletalMeshes.py Scripts\ImportStaticMeshes.py

& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -ExecutePythonScript="C:\UE\T66\Scripts\SetCharacterTextureStreamingDefaults.py" -unattended -nop4 -nosplash -log
```

Final commandlet report:

```text
Report: C:\UE\T66\Saved\Logs\SetCharacterTextureStreamingDefaults.json
ok: true
modified_count: 0
skipped_count: 184
errored_count: 0
```

The final run found all 184 `Texture2D` assets under `/Game/Characters/` already compliant with the new helper settings. Earlier migration passes modified character texture assets; the final clean rerun is the verification run.

Sample probe for `/Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/Dungeon_Slime_QuadRetro/Textures/Dungeon_Slime_QuadRetro_Pixelated_512`:

```text
Baseline from Wave 2/task statement: TextureStreamingMethod=TSM_NOT_STREAMED, LODGroup=TEXTUREGROUP_WORLD, resource size=32x32
After final commandlet: TextureStreamingMethod=TSM_NOT_STREAMED, LODGroup=TEXTUREGROUP_CHARACTER, NeverStream=false, MipGenSettings=TMGS_FROM_TEXTURE_GROUP, MaxTextureSize=0, LODBias=0, Compression=TC_DEFAULT, resource size=32x32
```

### Deviation

The requested `LODGroup = TEXTUREGROUP_CHARACTER` and stream-eligible flags are applied. UE 5.7 still reports `TextureStreamingMethod = TSM_NOT_STREAMED` for the Dungeon Slime sample after reload. The script now attempts an explicit streaming-method assignment, but the probe still returns the derived non-streamed value for this 32x32 pixelated resource. Standalone log confirms global texture streaming is enabled:

```text
C:\UE\T66\Saved\StandaloneLogs\Track3_DungeonSmoke.log:509 LogInit: Texture streaming: Enabled
```

## Task 2 - Auto-LOD Generation For QuadRetro Static Meshes

### What Changed

- Added `Scripts/GenerateCharacterMeshLODs.py`.
- Added LOD generation helpers in `Scripts/QuadRetroCharacterPipelineDefaults.py`.
- Updated new QuadRetro imports to apply the same LOD ladder:
  - `Scripts/ImportQuadRetroEnemyVisuals.py:153`
  - `Scripts/ImportQuadRetroBossVisuals.py:138`
  - `Scripts/ImportQuadRetroHeroVisuals.py:141`

LOD ladder applied:

```text
LOD0: unchanged, ScreenSize=1.00
LOD1: PercentTriangles=0.40, ScreenSize=0.60
LOD2: PercentTriangles=0.15, ScreenSize=0.25
LOD3: PercentTriangles=0.05, ScreenSize=0.10
```

Relevant implementation references:

- `Scripts/QuadRetroCharacterPipelineDefaults.py:429-488`
- `Scripts/GenerateCharacterMeshLODs.py:84`

### Verification

Commandlet report:

```text
Report: C:\UE\T66\Saved\Logs\GenerateCharacterMeshLODs.json
ok: true
processed_count: 75
errored_count: 0
partial: false
```

Representative triangle counts:

```text
Mesh                                LOD0   LOD1   LOD2  LOD3
SM_Dungeon_BoneJailer_QuadRetro    28075  11229   4212  1404
SM_Dungeon_Slime_QuadRetro         19099   7640   2864   954
SM_Forest_BrambleTreant_QuadRetro  25159  10062   3774  1258
SM_Forest_MushroomBrute_QuadRetro  19557   7822   2933   977
SM_Gambler_QuadRetro               34364  13745   5154  1718
SM_Hell_GreatDragon_QuadRetro      18977   7590   2846   948
SM_Martian_DroneGrunt_QuadRetro    22483   8993   3372  1124
SM_Ocean_AbyssalJellyfish_QuadRetro 25716 10286   3858  1286
SM_Ocean_CrabGuard_QuadRetro       22100   8840   3315  1105
SM_Saint_QuadRetro                 44226  17690   6634  2212
```

The sampled `SM_Dungeon_Slime_QuadRetro` LOD0 count is 19,099 triangles as reported by UE after the pass, not the 21,416 value in the task note. The reduction ratios match the requested ladder.

### Screenshot Evidence

Files in `C:\UE\T66\Audit\Reference\Track3_LOD_Comparison\`:

```text
DungeonSlime_LOD0_LOD1_LOD2_LOD3.png
Standalone_DungeonSmoke.png
```

`Standalone_DungeonSmoke.png` is the valid staged gameplay smoke screenshot.  
`DungeonSlime_LOD0_LOD1_LOD2_LOD3.png` was generated by a temporary forced-LOD editor capture helper after the four LOD actors spawned successfully, but the exported SceneCapture image is black. This file is retained as evidence of the attempted side-by-side capture, not as visual confirmation. Asset-level verification for LODs is the UE triangle-count report above.

## Task 3 - QuadRetro Material Instance Consolidation

### What Changed

- Created shared material instance:
  - `/Game/Materials/MI_GLB_Unlit_Character_Shared`
  - disk path: `C:\UE\T66\Content\Materials\MI_GLB_Unlit_Character_Shared.uasset`
- Added `PixelatedTextureAssetPath` to the character visual data schema:
  - `Source/T66/Data/T66DataTypes.h:1708`
- Added the new CSV column and populated it for all QuadRetro rows:
  - `Content/Data/CharacterVisuals.csv`
  - `Content/Data/DT_CharacterVisuals.uasset`
- Extended runtime preload paths:
  - `Source/T66/Core/T66CharacterVisualSubsystem.cpp:128`
  - `Source/T66/Core/T66GameInstance.cpp:511`
  - `Source/T66/Core/T66GameInstance.cpp:646`
- Extended the static mesh QuadRetro runtime path to assign the shared MI, create DMIs, and bind the per-row texture:
  - shared MI path constant: `Source/T66/Core/T66CharacterVisualSubsystem.cpp:31`
  - QuadRetro detection: `Source/T66/Core/T66CharacterVisualSubsystem.cpp:500`
  - DMI texture binding: `Source/T66/Core/T66CharacterVisualSubsystem.cpp:521-544`
  - static mesh branch: `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1211-1226`
- Added `Scripts/MigrateQuadRetroMaterialAssignment.py`.
- Updated future import scripts to assign the shared MI and write `PixelatedTextureAssetPath` instead of creating new per-enemy MIs:
  - `Scripts/ImportQuadRetroEnemyVisuals.py:154,185-210`
  - `Scripts/ImportQuadRetroBossVisuals.py:139,175-208`
  - `Scripts/ImportQuadRetroHeroVisuals.py:142,176-209`
- Moved legacy per-enemy QuadRetro MIs to:
  - `/Game/Characters/_Legacy/MaterialInstances_QuadRetro/`
  - `C:\UE\T66\Content\Characters\_Legacy\MaterialInstances_QuadRetro\`

### Verification

Commandlet report:

```text
Report: C:\UE\T66\Saved\Logs\MigrateQuadRetroMaterialAssignment.json
ok: true
migrated_mesh_count: 75
csv_rows_verified: 75
mismatches_found: 0
```

CSV verification:

```text
Total rows: 136
QuadRetro rows: 75
PixelatedTextureAssetPath non-empty rows: 75
QuadRetro rows missing PixelatedTextureAssetPath: 0
Rows pointing at Engine DefaultTexture: 0
```

Sample mesh assignment verification:

```text
Row: Dungeon_Slime
Mesh: /Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/SM_Dungeon_Slime_QuadRetro.SM_Dungeon_Slime_QuadRetro
PixelatedTextureAssetPath: /Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/Dungeon_Slime_QuadRetro/Textures/Dungeon_Slime_QuadRetro_Pixelated_512.Dungeon_Slime_QuadRetro_Pixelated_512
Assigned material after migration: /Game/Materials/MI_GLB_Unlit_Character_Shared.MI_GLB_Unlit_Character_Shared
```

Material instance footprint:

```text
Scope                                      Count  Bytes
/Game/Characters before active migration  176    10,982,650
/Game/Characters active after migration   104     5,965,047
Legacy rollback folder                     72     5,017,603
Shared MI outside /Game/Characters          1         8,809
```

Parent breakdown before migration:

```text
Parent                                                                 Count
/Game/Materials/M_GLB_Unlit.M_GLB_Unlit                                  72
/Game/Materials/M_FBX_Unlit.M_FBX_Unlit                                  32
/Game/Materials/M_Character_Unlit.M_Character_Unlit                      63
/InterchangeAssets/Materials/FBXLegacyPhongSurfaceMaterial.FBXLegacyPhongSurfaceMaterial 4
/Game/Characters/Heroes/Hero_3/Chad/RigPrototype/M_MikeRigPrototype_ColorUnlit.M_MikeRigPrototype_ColorUnlit 1
/Game/Materials/M_Environment_Unlit.M_Environment_Unlit                   4
```

Active `/Game/Characters` after migration has no non-legacy `M_GLB_Unlit` per-enemy QuadRetro MIs. The 72 old MIs are retained under `_Legacy` for rollback.

## Build And Standalone Verification

### Native Build

Command run:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
```

Result:

```text
Result: Succeeded
```

### Full Staged Standalone Refresh

Command run:

```powershell
& .\Scripts\StageStandaloneBuild.ps1
```

The staging command ran a full build/cook/stage flow and did not use `-SkipCook`.

Staged executable:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
Size: 305,870,848 bytes
LastWriteTime: 2026-05-12 18:15:41 local
```

Shortcut verification:

```text
Project shortcut: C:\UE\T66\T66 Standalone.lnk
Target: C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe

Taskbar shortcut: C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk
Target: C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

### Standalone Smoke

Command run:

```powershell
Start-Process "C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe" -ArgumentList "/Game/Maps/GameplayLevel -windowed -ResX=1280 -ResY=720 -forcelogflush -abslog=""C:\UE\T66\Saved\StandaloneLogs\Track3_DungeonSmoke.log"" -T66GameplayAutoScreenshot=""C:\UE\T66\Audit\Reference\Track3_LOD_Comparison\Standalone_DungeonSmoke.png"" -T66GameplayAutoScreenshotDelay=12 -T66GameplayAutoCapture=hudreview -T66AutomationResX=1280 -T66AutomationResY=720 -T66AutomationWindowed" -Wait
```

Result:

```text
Exit code: 0
Screenshot: C:\UE\T66\Audit\Reference\Track3_LOD_Comparison\Standalone_DungeonSmoke.png
Log: C:\UE\T66\Saved\StandaloneLogs\Track3_DungeonSmoke.log
```

Standalone log evidence:

```text
Texture streaming enabled: yes
QuadRetro visual loads with Loaded=YES: 75
Dungeon_Slime visual loaded: yes
Forest_MushroomBrute visual loaded: yes
Hell_GreatDragon visual loaded: yes
Start gallery spawned: heroes=24, enemies=25, bosses=23, npcs=3, interactables=25, traps=4
Stage 1 boss spawned: Dungeon_SewerSlimeKing
Fatal/Unhandled exception: none found
Texture/material missing errors: none found
```

### Standalone Verification Deviation

The automated standalone smoke loaded the gameplay map, spawned the start gallery and boss, and resolved all 75 QuadRetro visual rows with `Loaded=YES`. It did not enter a live moving combat wave during the screenshot window; the log says main-board combat waits for the player to enter the board. This means the smoke verified staged load/render presence for the migrated assets, but not a manual zoom-out LOD transition during active combat.

## Open Issues Or Deviations

```text
1. Dungeon_Slime_QuadRetro_Pixelated_512 still reports derived TextureStreamingMethod=TSM_NOT_STREAMED after reload, despite LODGroup=TEXTUREGROUP_CHARACTER, NeverStream=false, and TMGS_FROM_TEXTURE_GROUP.
2. The forced side-by-side LOD screenshot export is black. The four LOD actors spawned and the assets report valid LOD triangle counts, but the image is not usable visual confirmation.
3. The standalone automation did not spawn a live moving wave. It did verify staged gameplay load, boss/gallery visual loads, texture presence, and no missing texture/material errors.
```
