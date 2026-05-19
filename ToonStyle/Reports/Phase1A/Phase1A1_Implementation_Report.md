# Phase 1A.1 Implementation Report

## Scope

Track A fixed the Phase 1A.0 TestRoom usability issues:

- Diagnosed and fixed the movement lock.
- Expanded the room from 1000 x 1000 x 600 UU to 10000 x 10000 x 600 UU.
- Brightened the neutral evaluation lighting.
- Preserved the staged standalone shortcut workflow.

## Movement Bug

Root cause: `AT66GameMode::MaintainPlayerTerrainSafety()` in `Source/T66/Gameplay/T66GameMode.cpp` was still running for `ET66RunCategory::TestRoom`.

That safety loop is correct for procedural tower stages: it disables player movement while terrain collision is not ready. TestRoom intentionally skips normal terrain generation, so `bTerrainCollisionReady` never becomes true. The result was:

- pawn spawned correctly,
- HUD and targeting systems were active,
- but the movement component was repeatedly forced to `MOVE_None`.

Lab did not show the bug because the function already returned early for `IsLabRun()`.

Fix: added the matching TestRoom early return:

- `Source/T66/Gameplay/T66GameMode.cpp`

The fix is deliberately narrow. It does not change Lab, Tower, Tutorial, or normal terrain setup.

## Room Changes

Updated `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`:

- Interior half width: `500` -> `5000`
- Interior half depth: `500` -> `5000`
- Interior size: `10000 x 10000 x 600 UU`
- Wall thickness remains `40 UU`
- Player spawn remains centered at `(0, 0, 220)`

## Lighting Changes

Updated TestRoom evaluation lighting:

- Directional light intensity: `3.0` -> `4.0`
- Sky light intensity: `0.35` -> `1.5`
- Directional rotation remains `(-60, -35, 0)`
- Light color remains neutral white

This is evaluation lighting for asset comparison, not final ToonStyle lighting. Phase 5 should replace it with the final cel-atmosphere setup.

## Cook Inclusion

The Lu Bu matrix meshes are loaded by path from C++ at runtime, so the cooker did not include `/Game/ToonStyle/TestAssets/LuBu_Matrix` on the first stage pass. I added a narrow packaging rule in `Config/DefaultGame.ini`:

- `+DirectoriesToAlwaysCook=(Path="/Game/ToonStyle")`

This ensures the TestRoom comparison assets are present in standalone builds. Verification confirmed cooked files under:

- `Saved/Cooked/Windows/T66/Content/ToonStyle/TestAssets/LuBu_Matrix/`

## Verification

Build:

- Command: `"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Result: succeeded.

Standalone refresh:

- Command: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`
- Result: succeeded.
- Staged saves were preserved and restored by the script.

Shortcut verification:

- `C:\UE\T66\T66 Standalone.lnk`
- Pinned taskbar shortcut at `%APPDATA%\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
- Both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Standalone smoke:

- Command launched the staged exe with `-nullrhi -nosound -ExecCmds="quit"`.
- Exit code: `0`
- Log: `C:\UE\T66\Saved\StandaloneLogs\Phase1A1_StandaloneSmoke.log`
- Log contains clean `LogExit: Exiting`.

Cook verification:

- Confirmed cooked Lu Bu matrix static mesh packages exist under `Saved/Cooked/Windows/T66/Content/ToonStyle/TestAssets/LuBu_Matrix/`.

Limitations:

- I did not perform a manual visual walk test from the packaged UI. The code path that suppressed movement was identified, fixed, compiled, cooked, and staged; Pablo should do the final eyeball/walk check in the TestRoom.
- I did not manually click ENTER, TUTORIAL, CHALLENGES, MODS, or LAB in the packaged UI during this pass. No code in those handlers was changed in Phase 1A.1, and the build/stage smoke passed.

## Notes

The import pass exposed a reusable tooling issue: full-editor GLB import returned no object paths for the normalized Lu Bu GLBs. I documented that separately in `Scripts/pending_issues_Scripts.md`.

## Follow-Up: Lu Bu Matrix Scale Fix

After Pablo's first staged visual check, the Lu Bu labels were visible but the meshes appeared as huge malformed geometry above and beside the test area. Inspection showed the normalized GLB/FBX files were sane in Blender at about 180 units tall, but the imported UE static mesh bounds were 100x larger: approximately 18000 UU tall with bounds origin around Z=9000.

Root cause: the FBX fallback import interpreted units differently than the normalized source files. The TestRoom matrix is an evaluation-only display path, so the narrow fix is to scale each Lu Bu matrix actor to `0.01` at spawn time in `T66GameMode_TestRoom.cpp`. That brings the visible meshes back to about 180 UU tall without rewriting the raw Pixal3D GLBs.

Verification after this follow-up:

- `Build.bat T66 Win64 Development` succeeded.
- `StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild -SkipCook` succeeded after closing the running staged build that held `dbghelp.dll`.
- Staged smoke `Phase1A1_LuBuScaleFixSmoke.log` exited with code `0`.

## Follow-Up: Lu Bu Matrix Texture Fix

After Pablo's second staged visual check, the six variants were correctly scaled but rendered blue/teal with no visible character texture.

Root cause: the FBX fallback wrote each Pixal3D texture as an extensionless `Image_0` file under the matching `.fbm` folder. The payloads are WebP images, but Unreal's FBX import did not bind them as usable material textures. The import log warning matched the visual result:

- `Texture2D /Game/ToonStyle/TestAssets/LuBu_Matrix/Image_0 contains no miplevels`

Fix:

- Converted the six extensionless WebP payloads to PNG under `SourceAssets/ToonStyle/Pixal3D/Phase1A/LuBu_Matrix/ExtractedTextures/`.
- Added `ToonStyle/Source/ImportLuBuMatrixTexturesAndBindMaterials.py`.
- Imported the six PNG textures to `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/`.
- Created one material instance per Lu Bu variant under `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/`, parented to `/Game/Materials/M_GLB_Unlit`.
- Set both `DiffuseColorMap` and `BaseColorTexture` on each material instance, then assigned the instance to static mesh slot 0.

Verification after this follow-up:

- `Saved/LuBuTextureBindingVerify.json` confirms all six static meshes load, slot 0 points to the expected `MI_LuBu_*` material instance, and `DiffuseColorMap` points to the expected `T_LuBu_*` texture.
- `StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild` succeeded and preserved staged saves.
- Cook verification confirmed all six `MI_LuBu_*` material packages and all six `T_LuBu_*` texture packages exist under `Saved/Cooked/Windows/T66/Content/ToonStyle/TestAssets/LuBu_Matrix/`.
- Staged smoke `Phase1A1_LuBuTextureFixSmoke.log` exited with code `0` and clean `LogExit: Exiting`.
