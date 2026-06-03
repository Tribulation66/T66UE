# Phase 1B - TestRoom And Exposure Report

Date: 2026-05-17

## Scope

Workstream E updated the TestRoom to display the Phase 1B cel-rendering foundation: toon materials on room surfaces and lineup assets, paired outline actors, explicit atmosphere parameter registration, reduced advisory lights, and locked exposure.

## TestRoom Lineup

The full lineup spawner now uses the Phase 1B shading and outline meshes.

Lineup assets shown:

- Lu Bu validation
- ARIA
- Gambler
- Slime
- TombSpider
- CaveBat
- Idol Altar
- Arcade Machine
- Loot Chest
- Loot Crate

Loot Bag Yellow remains deferred because its Pixal3D generation issue is still tracked separately.

For each lineup entry the TestRoom spawns:

- One shading `AStaticMeshActor`
- One outline `AStaticMeshActor`
- One label `ATextRenderActor`

The old Lu Bu matrix CVar remains available but defaults off:

- `t66.TestRoom.SpawnLuBuMatrix=0`

The full lineup defaults on:

- `t66.TestRoom.SpawnFullLineup=1`

## Materials

Lineup shading actors use:

- `MI_lubu_validation`
- `MI_aria`
- `MI_gambler`
- `MI_slime`
- `MI_tombspider`
- `MI_cavebat`
- `MI_idolaltar`
- `MI_arcademachine`
- `MI_lootchest`
- `MI_lootcrate`

Outline actors use matching `_Outline` material instances.

Room surfaces use:

- Floor: `MI_TestRoom_Floor`
- Walls: `MI_TestRoom_Wall`
- Ceiling: `MI_TestRoom_Ceiling`

`UVTileScale` starts at `10.0` for wall, floor, and ceiling materials.

## Exposure Diagnosis

The Phase 1A.2 note said manual exposure had previously blacked gameplay. The likely root cause was applying manual exposure to the older, dim PBR-lit scene where auto exposure was implicitly compensating for low average luminance. Removing eye adaptation without switching the visual base to predictable emissive cel output made the frame appear black or near-black.

Phase 1B changes the relevant TestRoom surfaces and lineup materials to Unlit emissive toon materials. In that context, manual exposure is safe because the materials output their computed color directly.

Implemented TestRoom exposure:

- Auto exposure method: Manual
- Physical camera exposure: disabled
- Bias: `0.0`
- Fixed min/max brightness fallback: `1.0` / `1.0`
- Ambient occlusion intensity: `0.0`

The manual exposure field named in the prompt is not present in UE 5.7's `FPostProcessSettings` headers in this project install, so the implementation uses the available manual method and fixed bounds fields.

## Lighting

Because the cel materials are Unlit, scene lights are advisory only.

Final TestRoom light values:

- Skylight intensity: `0.1`
- Directional light intensity: `1.0`
- Directional rotation remains `(-60, -35, 0)`

## Gate G7

G7 passed.

Command:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe /Game/Maps/GameplayLevel -T66AutomationTestRoom -windowed -ResX=1280 -ResY=720 -abslog=C:\UE\T66\Saved\StandaloneLogs\Phase1B_TestRoomSmoke_Final_PostOutlineFix.log -forcelogflush -ExecCmds=Quit
```

Result:

- Exit code `0`
- TestRoom BeginPlay reached
- 26 toon material components registered
- G6 parameter probe executed inside the staged TestRoom
- No `Failed to compile Material`
- No `invalid ShaderMap`
- No `uncooked shader map`
- No missing lineup asset messages

The `-T66AutomationTestRoom` command-line hook was added to `UT66GameInstance` for staged smoke only. It avoids Windows DPI/click automation flakiness by starting the GameplayLevel as a TestRoom run category when the explicit automation flag is present.

## Build And Stage

Build/stage command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development
```

Result:

- BuildCookRun succeeded.
- Staged build refreshed at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- `C:\UE\T66\T66 Standalone.lnk` refreshed to the staged executable.
- Pinned taskbar `T66 Standalone.lnk` refreshed to the staged executable.
- Regular staged smoke boot also exited with code `0` using `Saved/StandaloneLogs/Phase1B_SmokeBoot_Final.log`.

## Notes

`T66GameMode_TestRoom.cpp` and several C++ runtime files were locked by Windows mapped sections during part of this pass. Edits were made with full-file replacement after closing runtime processes rather than same-size binary patching.

The final technical smoke confirms assets and materials load. Pablo still needs to do the aesthetic evaluation pass in the live TestRoom to decide whether the foundation reads in the intended Hi-Fi Rush / Guilty Gear Xrd direction.
