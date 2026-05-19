# T66 Gameplay Visual Cleanup Investigation

Date: 2026-05-15  
Scope: read-only investigation of gameplay visual systems, Retro FX backend separation, visual cleanup candidates, and recent visual-related console warnings.

Primary evidence:
- Recent visual reports: `Audit/Reference/Visual_Systems_Audit/Report.md`, `Audit/Reference/Visual_Lock/Iteration_01_Report.md`, `Audit/Reference/Visual_Lock/Iteration_02_Report.md`, `Audit/Reference/Terrain_Fix/Iteration_01_Report.md`.
- Representative staged gameplay log: `Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log`.
- Additional latest editor/runtime log for transient Retro FX preload warnings: `Saved/Logs/T66.log`.
- Read-only Unreal asset reference audits:
  - `Scripts/AuditCharacterModelDataAndExit.py` writes `Saved/Audits/CharacterModelDataAudit.json` and uses Unreal package referencers via `EditorAssetLibrary.find_package_referencers_for_asset` (`Scripts/AuditCharacterModelDataAndExit.py:77-82`).
  - `Scripts/AuditWorldAssetsAndExit.py` writes `Saved/Audits/WorldAssetAudit.json` and uses the same referencer path (`Scripts/AuditWorldAssetsAndExit.py:43-45`).

## Executive Summary

### Part 1 - Retro FX backend separation

Verdict: **partial separation**. The frontend toggle is a real frontend/UI CRT switch: it writes `FT66RetroFXSettings::UIFullScreenCRTEnabled` from the settings UI (`Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:209-214`, `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:213-218`) and the frontend root applies it only to the UI retainer material (`Source/T66/UI/T66FrontendUIRootWidget.cpp:350-373`). The gameplay toggle writes `FT66RetroFXSettings::bEnableRetroFXMaster` (`Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:219-224`, `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:220-225`), and the Retro FX subsystem uses that as a global post-process master for whatever world calls `ApplySettings` (`Source/T66/Core/T66RetroFXSubsystem.cpp:366-420`, `Source/T66/Core/T66RetroFXSubsystem.cpp:638-684`). The backend does not have a clean frontend/gameplay context source of truth; both `AT66GameMode` and `AT66FrontendGameMode` call the same subsystem with the same saved settings (`Source/T66/Gameplay/T66GameMode.cpp:1461-1475`, `Source/T66/Gameplay/T66FrontendGameMode.cpp:80-91`).

### Part 2 - Visual code/asset cleanup audit

The live gameplay visual path is still broad, but the active core is clear: production mobs are data-row-driven under `/Game/Characters/Mobs/` (`Content/Data/CharacterVisuals.csv:114-163`), active mob material binding uses `/Game/Materials/MI_GLB_Unlit_Character_Shared` and runtime texture overrides (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:30-33`, `Source/T66/Core/T66CharacterVisualSubsystem.cpp:516-575`), and terrain generated-kit assets are selected by code from `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01` (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:13-14`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:223-315`). The main cleanup candidates are conservative: unused terrain common rocks/landscape assets reported by the asset referencer (`Saved/Audits/WorldAssetAudit.json:1763-1820`), hero RigPrototype/Knight/old import assets with no data references (`Saved/Audits/CharacterModelDataAudit.json:2637-2647`, `Saved/Audits/CharacterModelDataAudit.json:5054-5067`), and parked Track 2/test materials that overlap the active shared-material path.

### Part 3 - Console warnings

Visual warning root cause is confirmed: Iteration 2 put key low-res/upscale CVars in `Config/DefaultEngine.ini` as project settings (`Config/DefaultEngine.ini:107-111`), while `UT66RetroFXSubsystem::ApplyResolutionRuntime()` also tries to set the same CVars at runtime with `ECVF_SetByGameSetting` (`Source/T66/Core/T66RetroFXSubsystem.cpp:121-135`, `Source/T66/Core/T66RetroFXSubsystem.cpp:942-953`). Unreal treats `SetByProjectSetting` as higher priority than `SetByGameSetting` (`C:/Program Files/Epic Games/UE_5.7/Engine/Source/Runtime/Core/Public/HAL/IConsoleManager.h:149-153`), so the runtime calls are ignored and logged by `ConsoleManager.cpp` (`C:/Program Files/Epic Games/UE_5.7/Engine/Source/Runtime/Core/Private/HAL/ConsoleManager.cpp:274-282`). The DMI/MPC/material warnings are transient async preload timing: the subsystem queues async loads, attempts immediate application, logs nulls, then reapplies when preload completes (`Source/T66/Core/T66RetroFXSubsystem.cpp:514-518`, `Source/T66/Core/T66RetroFXSubsystem.cpp:554-615`; `Source/T66/Core/T66PixelationSubsystem.cpp:51-59`, `Source/T66/Core/T66PixelationSubsystem.cpp:156-160`).

## Part 1 - Retro FX Backend Separation

### Settings storage and UI wiring

| Setting / toggle | Backend field | Evidence | Interpretation |
|---|---|---|---|
| Gameplay Retro FX | `FT66RetroFXSettings::bEnableRetroFXMaster` | Default in `Source/T66/Core/T66RetroFXSettings.h:19-20`; saved mirror in `Source/T66/Core/T66PlayerSettingsSaveGame.h:232-237`; save/load sync in `Source/T66/Core/T66PlayerSettingsSubsystem.cpp:958-976` | This is the master post-process/runtime Retro FX switch. It is not explicitly named "gameplay" in the backend. |
| Frontend Retro FX | `FT66RetroFXSettings::UIFullScreenCRTEnabled` | Default in `Source/T66/Core/T66RetroFXSettings.h:160-179`; toggle row in `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:213-218` | This controls the frontend UI CRT/retainer path, not the gameplay post-process volume. |
| Compact settings buttons | Lambdas write the same two fields | Frontend button writes `UIFullScreenCRTEnabled` at `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:209-214`; gameplay button writes `bEnableRetroFXMaster` at `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:219-224`; display state reads both at `Source/T66/UI/Screens/Settings/T66SettingsScreen_Build.cpp:241-252` | UI label separation exists. Backend separation depends on which consumer reads each field. |
| Full settings tab apply | Saves then applies same struct | `ApplyPendingRetroFX()` saves via `SetRetroFXSettings` and calls `RetroFX->ApplySettings(PendingRetroFXSettings, GetWorld())` at `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:365-396` | Settings changes immediately hit the same world-level Retro FX subsystem. |

### Frontend path

`UIFullScreenCRTEnabled` is consumed by the frontend root widget, not the world post-process stack. `UT66FrontendUIRootWidget::ApplySettingsToRetainer()` reads `CurrentSettings.UIFullScreenCRTEnabled` and drives `CRTEnabled`, scanline, phosphor mask, bloom, chromatic, barrel distortion, vignette, and color quantization parameters on the retainer material (`Source/T66/UI/T66FrontendUIRootWidget.cpp:350-373`). `UT66UIManager` reads saved Retro FX settings, applies the optional `-T66DisableFrontendCRT` command-line bypass, then passes the settings to the frontend root (`Source/T66/UI/T66UIManager.cpp:35-45`, `Source/T66/UI/T66UIManager.cpp:734-741`).

This is a genuine frontend visual path, but it is UI-retainer scoped rather than `UT66RetroFXSubsystem` scoped.

### Gameplay/post-process path

`UT66RetroFXSubsystem::ApplySettings()` resolves a world, builds effective settings, ensures an unbound post-process volume for that world, adds blendables, applies weights/parameters, applies runtime low-resolution CVars, updates geometry materials, and forwards pixelation levels to `UT66PixelationSubsystem` (`Source/T66/Core/T66RetroFXSubsystem.cpp:638-696`). `BuildEffectiveSettings()` only checks `bEnableRetroFXMaster`: if true it returns the full struct; if false it zeroes or disables PS1, real low-res, N64, chromatic, pixelation, outline, UI treatment, and geometry fields (`Source/T66/Core/T66RetroFXSubsystem.cpp:366-420`, `Source/T66/Core/T66RetroFXSubsystem.cpp:428-443`).

The subsystem does not ask "frontend or gameplay" before applying post-process effects. The only scoping is the `UWorld*` passed into `ApplySettings`, and `EnsureBlendablesInWorld()` creates a transient unbound `APostProcessVolume` with priority `5000` in that target world (`Source/T66/Core/T66RetroFXSubsystem.cpp:706-731`).

### World callers and context source of truth

| Caller | File / lines | Behavior | Separation result |
|---|---|---|---|
| Gameplay game mode | `Source/T66/Gameplay/T66GameMode.cpp:1461-1475` | Applies neutral world visual setup, then applies saved Retro FX settings to the gameplay world. | Gameplay world receives post-process backend according to `bEnableRetroFXMaster`. |
| Frontend game mode | `Source/T66/Gameplay/T66FrontendGameMode.cpp:50-64`, `Source/T66/Gameplay/T66FrontendGameMode.cpp:80-91` | Applies neutral world visual setup and calls `RetroFX->ApplyCurrentSettings(World)`. | Frontend shell can also receive the same post-process backend if `bEnableRetroFXMaster` is true. |
| Gameplay transition preload | `Source/T66/Core/T66GameInstance.cpp:1756-1770` | Reads saved Retro FX settings and applies them to the current world around gameplay transition work. | Same global settings struct, no context branch. |
| Shared neutral PP setup | `Source/T66/Gameplay/T66WorldVisualSetup.cpp:178-195`, `Source/T66/Gameplay/T66WorldVisualSetup.cpp:222-225` | Creates/finds an unbound post-process volume and applies neutral rendering setup. | Shared setup is intentionally reused by frontend and gameplay; it does not provide Retro FX separation. |

### What each toggle actually does

- Turning off **Gameplay Retro FX** sets `bEnableRetroFXMaster=false`. In `UT66RetroFXSubsystem`, this disables all post-process/runtime/geometry Retro FX for whichever world calls `ApplySettings` (`Source/T66/Core/T66RetroFXSubsystem.cpp:366-420`). It is effectively a global world-post-process master, not strictly gameplay-only.
- Turning off **Frontend Retro FX** sets `UIFullScreenCRTEnabled=false`. It disables frontend UI CRT parameters in `UT66FrontendUIRootWidget` (`Source/T66/UI/T66FrontendUIRootWidget.cpp:364-373`). It does not disable the world-level post-process Retro FX subsystem.
- There is no clean backend context enum or source of truth such as `ERetroFXContext::Frontend/Game`. Context is inferred from which world/game mode calls the subsystem, and both frontend and gameplay callers currently use the same `ApplyCurrentSettings`/`ApplySettings` backend (`Source/T66/Gameplay/T66GameMode.cpp:1461-1475`, `Source/T66/Gameplay/T66FrontendGameMode.cpp:80-91`).

### Verdict

**Partial separation.** Frontend UI CRT is separately implemented and separately toggled. Gameplay post-process Retro FX is not cleanly separated at the backend; `bEnableRetroFXMaster` is a global world-post-process master consumed by `UT66RetroFXSubsystem`, and the frontend game mode also calls that subsystem. A cleanup/fix pass should either stop applying `UT66RetroFXSubsystem` in frontend worlds, or add an explicit context-aware settings resolver before blendable/CVar application.

## Part 2 - Visual Code/Asset Cleanup Audit

### Method

- Text/reference search used `rg` across `Source`, `Config`, `Content/Data`, `Scripts`, `Model Generation`, and prior reports.
- Unreal package referencer checks were run through existing read-only scripts:
  - Character/hero/UI hero roots: `Scripts/AuditCharacterModelDataAndExit.py`, output `Saved/Audits/CharacterModelDataAudit.json`.
  - World roots: `Scripts/AuditWorldAssetsAndExit.py`, output `Saved/Audits/WorldAssetAudit.json`.
- `CharacterModelDataAudit` reports 162 character visual rows, 12 hero rows, zero missing data refs, zero redirectors, and zero unused hero folder candidates (`Saved/Audits/CharacterModelDataAudit.json:6-7`, `Saved/Audits/CharacterModelDataAudit.json:259-261`, `Saved/Audits/CharacterModelDataAudit.json:360`).
- `WorldAssetAudit` reports 279 `/Game/World` assets and 14 orphan candidates, including 8 terrain common assets (`Saved/Logs/T66.log:927-941`).

### `/Game/Characters/`

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `/Game/Characters/Mobs/<50 production mobs>` static meshes/textures | Active | `Content/Data/CharacterVisuals.csv` rows `Slime` through `HellWyrm` reference `/Game/Characters/Mobs/<Mob>/SM_<Mob>` and `/Game/Characters/Mobs/<Mob>/T_<Mob>` (`Content/Data/CharacterVisuals.csv:114-163`). Runtime identifies `/Game/Characters/Mobs/` static visuals as QuadRetro and applies the shared material override (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:505-575`). | Keep |
| `/Game/Characters/MobsVAT/<10 EasyMobVAT mobs>` | Active, experimental/parallel | `Content/Data/MobVertexAnimations.csv` rows reference `/Game/Characters/MobsVAT/...` static meshes/materials and source mob textures (`Content/Data/MobVertexAnimations.csv:1-11`). Runtime loads `DT_MobVertexAnimations` and only applies enabled rows (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:831-872`). | Keep, but keep separate from production static-mob cleanup |
| `/Game/Characters/Heroes/Hero_1/Chad/QuadRetroUALQA` | Active | `Hero_1_Chad` and `Hero_1_Chad_QuadRetroUALQA` rows reference the skeletal mesh and animation assets (`Content/Data/CharacterVisuals.csv:2-3`). | Keep |
| `/Game/Characters/Heroes/*/QuadRetro` and Beachgoer/current hero variants | Active / Investigate further | Character audit found 37 hero visual rows and no missing data refs (`Saved/Audits/CharacterModelDataAudit.json:6-7`, `Saved/Audits/CharacterModelDataAudit.json:259`). Example Beachgoer row is active data (`Content/Data/CharacterVisuals.csv:4`). | Keep; investigate only if consolidating hero visual variants later |
| `/Game/Characters/Heroes/Hero_1/Chad/Idle` and `/Walk` old Arthur imports | Possibly-Legacy | Audit shows `ArthurAIdle` has no package referencers and is not referenced by character data, but still has text references from scripts/docs (`Saved/Audits/CharacterModelDataAudit.json:485-498`). `Scripts/ImportSkeletalMeshes.py` still targets the old Idle/Walk folders (`Scripts/ImportSkeletalMeshes.py:27-32`). | Archive after confirming the old import script is retired |
| `/Game/Characters/Heroes/Hero_3/Chad/RigPrototype` | Possibly-Legacy | Example `AM_Hero_3_Mike_Chad_RigPrototype_Idle` has zero package referencers, zero text refs, and no character-data reference (`Saved/Audits/CharacterModelDataAudit.json:2637-2647`). Folder exists as `RigPrototype` under Hero_3 (`Get-ChildItem` inspection; path listed in shell output). | Archive or investigate further before delete |
| `/Game/Characters/Heroes/Knight/*` | Possibly-Legacy | `KnightClip` has no package referencers and no character-data reference, but two text references (`Saved/Audits/CharacterModelDataAudit.json:5054-5067`). `T66GameInstance` still preloads `/Game/Characters/Heroes/Knight/KnightClip.KnightClip` (`Source/T66/Core/T66GameInstance.cpp:572`). | Investigate further; likely remove stale preload before archiving assets |
| `/Game/Characters/Enemies/Bosses/*` | Active | 23 character visual rows reference `/Game/Characters/Enemies/Bosses/` assets per CSV scan; boss import/validation scripts also target that root (`Scripts/ImportQuadRetroBossVisuals.py:28`, `Scripts/RunImportQuadRetroBossVisualsAndExit.py:11-17`). | Keep |
| `/Game/Characters/NPCs/Gambler/Saint/Ouroboros` QuadRetro/runtime assets | Active | NPC rows are present in `Content/Data/CharacterVisuals.csv` scan; source directly loads Gambler QuadRetro mesh (`Source/T66/Gameplay/T66GamblerNPC.cpp:20`). World/NPC import verification scripts reference Saint/Ouroboros/Gambler roots (`Scripts/VerifyWorldNpcInteractablesRetroBatch01AndExit.py:40-46`). | Keep |
| `/Game/Characters/Companions/*` | Active / Investigate further | CSV scan found 48 companion visual rows; old visibility reports show older `Companion1/2/3` FBX-style material instances and newer `Companion_01..08` variants coexisting. Current audit scope did not run companion-specific ownership checks. | Investigate further before any archive/delete |
| `/Game/Characters/_Legacy` | Possibly-Legacy | Top-level `_Legacy` folder exists; pipeline defaults still name `/Game/Characters/_Legacy/MaterialInstances_QuadRetro` as `LEGACY_MI_DIR` (`Scripts/QuadRetroCharacterPipelineDefaults.py:13-19`). | Keep as rollback archive; do not delete without a dedicated rollback policy |

### `/Game/Materials/`

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `/Game/Materials/MI_GLB_Unlit_Character_Shared` | Active | Runtime shared mob material path is hardcoded (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:30-33`) and loaded/applied in `T66ApplyQuadRetroStaticMaterialOverrides()` (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:516-575`). | Keep |
| `/Game/Materials/M_GLB_Unlit` | Active | Parent/canonical GLB unlit material path in pipeline defaults (`Scripts/QuadRetroCharacterPipelineDefaults.py:13-18`); also referenced by Retro FX geometry replacement base paths (`Source/T66/Core/T66RetroFXSubsystem.cpp:33-40`). | Keep |
| `/Game/Materials/M_Character_Unlit` | Active, Possibly-Legacy for production mobs | Character fallback path is hardcoded (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:30`). Current production static mobs use the shared GLB MI path instead (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:516-575`). | Keep as fallback; mark for later consolidation |
| `/Game/Materials/M_FBX_Unlit` | Active fallback/import | Character subsystem has FBX base material path (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:31`); import/material scripts still reference it (`Scripts/MakeGLBImportsUnlit.py:30`, `Scripts/VerifyImportBatch.py:30-32`). | Keep until old FBX/companion paths are cleaned |
| `/Game/Materials/M_Environment_Unlit` | Active | Terrain/theme utilities load it directly (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:13`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:115-118`); miasma/lava/main map paths also load it (`Source/T66/Gameplay/T66MiasmaBoundary.cpp:61`, `Source/T66/Gameplay/T66LavaShared.h:8`, `Source/T66/Gameplay/T66MainMapTerrain.cpp:878-884`). | Keep |
| `/Game/Materials/M_CameraWallOccluderFade` | Active | Player controller loads it for camera wall occluder fading (`Source/T66/Gameplay/T66PlayerController.cpp:788-794`). | Keep |
| `/Game/Materials/M_EasyMobVAT_Unlit_UV2` | Active, experimental | VAT import tooling names it as master material (`Model Generation/Rigging and Animation/Tools/import_easy_mob_vat_to_unreal.py:39`); VAT rows point to per-mob MIs (`Content/Data/MobVertexAnimations.csv:1-11`). | Keep while MobsVAT path remains enabled |
| `/Game/Materials/Retro/M_Character_Unlit_RetroGeometry`, `/M_Environment_Unlit_RetroGeometry`, `/M_FBX_Unlit_RetroGeometry`, `/M_GLB_Unlit_RetroGeometry`, `/MPC_T66_RetroGeometry` | Active code path, currently disabled by settings | Paths are hardcoded in Retro FX (`Source/T66/Core/T66RetroFXSubsystem.cpp:37-40`, `Source/T66/Core/T66RetroFXSubsystem.cpp:54`). Defaults disable world/character geometry and all vertex/affine/noise percents (`Source/T66/Core/T66RetroFXSettings.h:188-233`). | Investigate further; dead-path candidate if geometry FX is cut |
| `/Game/Materials/Retro/M_T66_OutlinePostProcess` | Active code path, disabled in current baseline | Path is hardcoded (`Source/T66/Core/T66RetroFXSubsystem.cpp:51`), DMI is ensured (`Source/T66/Core/T66RetroFXSubsystem.cpp:734-737`), weight comes from `bEnableCharacterOutline` (`Source/T66/Core/T66RetroFXSubsystem.cpp:772-790`), default is false (`Source/T66/Core/T66RetroFXSettings.h:91-95`). | Keep for A/B unless visual lock permanently rejects runtime outline |
| `/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess` | Active code path, currently zeroed | Path hardcoded (`Source/T66/Core/T66RetroFXSubsystem.cpp:52`), default chromatic percents are zero (`Source/T66/Core/T66RetroFXSettings.h:76-83`). | Investigate further; dead-path candidate if chromatic is removed |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C*_S*_B*` | Active code path, currently zeroed | Variant path generated in `GetPs1VariantMaterialPath` (`Source/T66/Core/T66RetroFXSubsystem.cpp:190`) and queued for preload (`Source/T66/Core/T66RetroFXSubsystem.cpp:570-577`); PS1 settings default to zero (`Source/T66/Core/T66RetroFXSettings.h:22-50`). | Investigate further; archive only if PS1 stack is retired |
| `/Game/Materials/M_GLB_ViewSpaceLit_Character`, `/MI_GLB_ViewSpaceLit_Character_Test`, `/MI_TestSlime_ViewSpaceLit`, `/MI_TestSlime_Unlit`, `/M_Track2_NeutralBackdrop` | Possibly-Legacy / Duplicate test assets | Prior Track 2 report identifies these as view-space lit test/parked assets, not active production (`Audit/Reference/Track2_Visibility/Track2_Report.md:17-24`, `Audit/Reference/Track2_Visibility/Track2_Report.md:92-102`). Visual Systems Audit confirms `M_GLB_ViewSpaceLit_Character` is parked and not active on production mobs (`Audit/Reference/Visual_Systems_Audit/Report.md:29`, `Audit/Reference/Visual_Systems_Audit/Report.md:421`). | Archive test instances after any planned A/B; keep master if visual lock still wants a lit-character experiment |
| `/Game/Materials/Generated/M_Unlit_DiffuseColorMap` | Investigate further | Present in materials folder listing; no direct source reference found in current grep. Could be referenced by imported assets. | Investigate further with material referencer audit before archive |

### `/Game/World/Terrain/`

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/*` | Active | Theme root is hardcoded (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:13-14`); each theme selects wall/floor module names from this root (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:223-315`); `T66GameInstance` preloads generated kit paths by module id (`Source/T66/Core/T66GameInstance.cpp:1423-1439`, `Source/T66/Core/T66GameInstance.cpp:1496-1500`). Asset audit shows generated-kit assets have text references and zero orphan candidates (`Saved/Audits/WorldAssetAudit.json:1963-1972`, `Saved/Audits/WorldAssetAudit.json:2283-2292`). | Keep |
| `/Game/World/Terrain/TowerForest/*` and `/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof/T_TowerDungeonRoof` fallback assets | Active | Theme and main map code reference these fallback materials/textures (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:31-66`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:218-225`, `Source/T66/Gameplay/T66MainMapTerrain.cpp:878-884`). Game instance preloads them (`Source/T66/Core/T66GameInstance.cpp:1434-1439`). | Keep |
| `/Game/World/Terrain/Common/Landscape/Grass_LayerInfo`, `/MI_Landscape` | Unused | World asset audit reports no package referencers and no text refs (`Saved/Audits/WorldAssetAudit.json:1763-1780`). | Archive, not delete, unless a map-specific landscape pass confirms no hidden map references |
| `/Game/World/Terrain/Common/Rocks/MI_Rock1..3`, `/SM_Rock1..3` | Unused | World asset audit reports no package referencers and no text refs for rock assets (`Saved/Audits/WorldAssetAudit.json:1783-1820`; remaining rock rows continue through `Saved/Audits/WorldAssetAudit.json:1822-1840`). | Archive |

### `/Game/UI` shared visual materials

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `/Game/UI/M_PixelationPostProcess` | Shared (gameplay+frontend capable), currently inactive by settings | Pixelation subsystem hardcodes the path (`Source/T66/Core/T66PixelationSubsystem.cpp:15`) and adds it to an unbound post-process volume when pixelation levels are applied (`Source/T66/Core/T66PixelationSubsystem.cpp:96-182`). Current defaults set pixelation percents to zero (`Source/T66/Core/T66RetroFXSettings.h:85-92`). | Keep; gate load/apply when both levels are zero to avoid warnings |
| `/Game/UI/Materials/M_T66_UI_CRTPostProcess` | Active frontend | Frontend root widget hardcodes this material path (`Source/T66/UI/T66FrontendUIRootWidget.cpp:20`) and applies settings via retainer (`Source/T66/UI/T66FrontendUIRootWidget.cpp:350-373`). | Keep |
| `/Game/UI/Materials/M_UI_RetroRetainer` | Shared UI legacy/support | `T66Style` hardcodes the retainer material path (`Source/T66/UI/Style/T66Style.cpp:362`). | Keep unless UI chrome cleanup explicitly retires retained surfaces |
| `/Game/UI/Materials/M_UI_Glow` | Shared UI legacy/support | `T66Style` loads `M_UI_Glow` for button glow (`Source/T66/UI/Style/T66Style.cpp:986`); flat style comment says newer flat helpers intentionally avoid `M_UI_Glow` (`Source/T66/UI/T66FlatStyle.h:151`). | Investigate further; possibly legacy for old UI widgets |

### `Source/T66/Core` visual systems

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `Source/T66/Core/T66RetroFXSubsystem.cpp/.h` | Active, shared backend | Game modes and settings UI call `ApplySettings`/`ApplyCurrentSettings` (`Source/T66/Gameplay/T66GameMode.cpp:1461-1475`, `Source/T66/Gameplay/T66FrontendGameMode.cpp:80-91`, `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp:392-396`). | Keep; add context separation before cleanup |
| `Source/T66/Core/T66RetroFXSettings.h` | Active | SaveGame struct holds all gameplay and frontend Retro FX fields (`Source/T66/Core/T66RetroFXSettings.h:14-20`, `Source/T66/Core/T66RetroFXSettings.h:160-179`, `Source/T66/Core/T66RetroFXSettings.h:188-233`). | Keep; later split frontend CRT settings from gameplay post-process settings |
| `Source/T66/Core/T66PixelationSubsystem.cpp/.h` | Active code path, currently inactive by settings | Runtime path hardcodes `/Game/UI/M_PixelationPostProcess` and applies blendables (`Source/T66/Core/T66PixelationSubsystem.cpp:15`, `Source/T66/Core/T66PixelationSubsystem.cpp:81-96`, `Source/T66/Core/T66PixelationSubsystem.cpp:185-213`). | Keep; avoid material load if levels are zero |
| `Source/T66/Core/T66CharacterVisualSubsystem.cpp/.h` | Active | Owns `DT_CharacterVisuals`, `DT_MobVertexAnimations`, shared material overrides, and brightness (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:26-33`, `Source/T66/Core/T66CharacterVisualSubsystem.cpp:831-872`, `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1034-1060`). | Keep |

### `Source/T66/Gameplay` visual systems

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `T66WorldVisualSetup.cpp/.h` | Active, shared gameplay/frontend setup | Removes sky/lights/fog and applies neutral PP setup (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:178-195`), exposes shared runtime PP lookup (`Source/T66/Gameplay/T66WorldVisualSetup.cpp:222-225`). | Keep; separation cleanup should happen in Retro FX caller/backend, not here |
| `T66TowerMapTerrain.cpp/.h` | Active | Current generated terrain uses constants such as kit unit size, wall depth, and cell size (`Source/T66/Gameplay/T66TowerMapTerrain.cpp:39-53`), and Terrain Fix Iteration 01 changed this path. | Keep |
| `T66TowerThemeVisuals.cpp/.h` | Active | Theme root, fallback materials, and module arrays are hardcoded here (`Source/T66/Gameplay/T66TowerThemeVisuals.cpp:13-14`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:31-66`, `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:223-315`). | Keep |
| `T66PlayerController.cpp` camera wall fade path | Active | Loads `/Game/Materials/M_CameraWallOccluderFade` for occluder fading (`Source/T66/Gameplay/T66PlayerController.cpp:788-794`). | Keep |

### `Model Generation/Scripts/Core/QuadRetro/`

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `t66_quad_retro_character_pipeline.py` | Possibly-Legacy for current production mobs; active as research/pipeline tool | Script describes full Quad Remesher/bake/pixelate workflow (`Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py:1-20`) with defaults that differ from current imported production mobs (`Model Generation/Scripts/Core/QuadRetro/t66_quad_retro_character_pipeline.py:83-134`). Visual Systems Audit concluded the 50 production mobs were not processed through this script (`Audit/Reference/Visual_Systems_Audit/Report.md:230-234`). | Keep as pipeline reference; do not delete |
| `RunQuadRetroCharacterPipeline.ps1` | Active wrapper/tooling | Wrapper points to the Python pipeline and sets safer current defaults: target quads `12000`, texture size `512`, palette/dither disabled (`Model Generation/Scripts/Core/QuadRetro/RunQuadRetroCharacterPipeline.ps1:1-29`, `Model Generation/Scripts/Core/QuadRetro/RunQuadRetroCharacterPipeline.ps1:34`). Pixal3D smoke tooling still references the wrapper (`Model Generation/Pixal3D/Scripts/run_pixal3d_smoke.py:30`). | Keep |

### `Scripts/` visual-related scripts

| Asset / file | Status | References found | Recommendation |
|---|---|---|---|
| `QuadRetroCharacterPipelineDefaults.py` | Active helper | Imported by import/validation/default scripts (`Scripts/ImportQuadRetroEnemyVisuals.py:15`, `Scripts/ImportQuadRetroBossVisuals.py:14`, `Scripts/ImportQuadRetroHeroVisuals.py:14`, `Scripts/SetCharacterTextureStreamingDefaults.py:19`). Defines shared material path and LOD ladder (`Scripts/QuadRetroCharacterPipelineDefaults.py:13-31`). | Keep |
| `SetCharacterTextureStreamingDefaults.py` | Active maintenance | Iterates `/Game/Characters` textures and applies shared defaults (`Scripts/SetCharacterTextureStreamingDefaults.py:22-34`, `Scripts/SetCharacterTextureStreamingDefaults.py:51-55`, `Scripts/SetCharacterTextureStreamingDefaults.py:99-110`). | Keep |
| `GenerateCharacterMeshLODs.py` | Active maintenance | Imports shared defaults and is documented in Track 3 performance pass (`Audit/Reference/Track3_Performance_Pass_Report.md:73-99`). | Keep |
| `MigrateQuadRetroMaterialAssignment.py` | Active maintenance / migration | Imports shared defaults and is documented in Track 3 material migration (`Audit/Reference/Track3_Performance_Pass_Report.md:157-171`). | Keep until migration is no longer needed |
| `ImportQuadRetroEnemyVisuals.py`, `RunImportQuadRetroEnemyVisualsAndExit.py`, `ValidateImportedEnemyVisualsAndExit.py` | Active import/validation | Enemy import script targets `/Game/Characters/Mobs` (`Scripts/ImportQuadRetroEnemyVisuals.py:27`); production import report says 50 imported, 0 skipped, 0 failures (`Audit/Reference/Mob_Production_Import/Report.md:130-132`). | Keep |
| `ImportQuadRetroBossVisuals.py`, `ImportQuadRetroHeroVisuals.py`, runners/validators | Active import/validation | Boss/hero scripts import shared defaults and target the corresponding character roots (`Scripts/ImportQuadRetroBossVisuals.py:14`, `Scripts/ImportQuadRetroBossVisuals.py:28`, `Scripts/ImportQuadRetroHeroVisuals.py:14`). | Keep |
| `MakeGLBImportsUnlit.py`, `MakeCharacterMaterialsUnlit.py`, `MakeGLBImportsUnlit.py` family | Possibly-Legacy one-off material migration | Scripts create/reparent unlit materials and are safe to rerun per headers (`Scripts/MakeGLBImportsUnlit.py:8-17`, `Scripts/MakeCharacterMaterialsUnlit.py:26-42`). Current runtime uses shared MI overrides for mobs (`Source/T66/Core/T66CharacterVisualSubsystem.cpp:516-575`). | Archive after confirming no future imports need them |
| `CleanupUnusedHeroCharacterAssetsAndExit.py` | Active audit/cleanup tool, not a gameplay runtime dependency | Existing tool logs deletions when run (`Scripts/CleanupUnusedHeroCharacterAssetsAndExit.py:60-79`), but this pass was read-only. | Keep as tool; do not run in cleanup mode without a separate prompt |
| `AuditCharacterModelDataAndExit.py`, `AuditWorldAssetsAndExit.py` | Active read-only audit tools | Used in this report; source says read-only and combines DataTable, referencer, and text scans (`Scripts/AuditCharacterModelDataAndExit.py:1-7`, `Scripts/AuditWorldAssetsAndExit.py:1-7`). | Keep |

### Dead code path candidates inside active files

| Candidate | Status | Evidence | Recommendation |
|---|---|---|---|
| PS1 blend/dither/LUT/fog stack | Currently dead in Iteration 2 baseline | Defaults are all zero (`Source/T66/Core/T66RetroFXSettings.h:22-50`), but `ApplySettings` still ensures PS1 DMI and applies PS1 params (`Source/T66/Core/T66RetroFXSubsystem.cpp:671-680`, `Source/T66/Core/T66RetroFXSubsystem.cpp:802-835`). | Gate DMI creation/parameter writes behind nonzero PS1 weights; remove only if PS1 stack is retired |
| N64 blur stack | Currently dead | Defaults zero/false (`Source/T66/Core/T66RetroFXSettings.h:64-74`), but N64 materials are preloaded/ensured (`Source/T66/Core/T66RetroFXSubsystem.cpp:49-50`, `Source/T66/Core/T66RetroFXSubsystem.cpp:734-741`). | Gate or remove after visual lock |
| Chromatic aberration/distortion | Currently dead | Defaults zero (`Source/T66/Core/T66RetroFXSettings.h:76-83`), but DMI creation and parameters still run (`Source/T66/Core/T66RetroFXSubsystem.cpp:737`, `Source/T66/Core/T66RetroFXSubsystem.cpp:850-870`). | Gate to avoid null warning and unnecessary work |
| Runtime pixelation post-process | Currently dead, kept for future iteration | Defaults zero (`Source/T66/Core/T66RetroFXSettings.h:85-92`), yet subsystem can load/apply material (`Source/T66/Core/T66PixelationSubsystem.cpp:81-96`, `Source/T66/Core/T66PixelationSubsystem.cpp:156-160`). | Keep but skip material path when both levels are zero |
| Runtime outline | Currently dead because source mobs have baked outlines | Default `bEnableCharacterOutline=false` (`Source/T66/Core/T66RetroFXSettings.h:91-95`), but outline DMI is loaded and parameterized (`Source/T66/Core/T66RetroFXSubsystem.cpp:736-737`, `Source/T66/Core/T66RetroFXSubsystem.cpp:837-848`). | Keep for A/B unless visual lock permanently rejects it |
| Geometry material replacement / vertex snap / noise / affine | Currently dead | Defaults disabled/zero (`Source/T66/Core/T66RetroFXSettings.h:188-233`); code still computes and applies collection/material state (`Source/T66/Core/T66RetroFXSubsystem.cpp:991-1070`, `Source/T66/Core/T66RetroFXSubsystem.cpp:1127-1165`). | Gate aggressively; remove only if geometry FX is cut from the product |
| UI chrome/text/background Retro FX treatment fields | Mostly dead outside frontend CRT | Defaults zero for UI chrome/text/background treatment fields (`Source/T66/Core/T66RetroFXSettings.h:97-158`); `BuildEffectiveSettings` also zeros them when master is false (`Source/T66/Core/T66RetroFXSubsystem.cpp:399-419`). | Split from gameplay Retro FX settings or archive after UI fidelity cleanup |

## Part 3 - Console Warnings

### Representative log warning inventory

`Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log` contains 55 warnings:

| Category | Count | Scope |
|---|---:|---|
| Visual CVar priority warnings | 21 | Visual / IN SCOPE |
| Steam/online/socket warnings | 16 | Out of scope |
| Audio package/soundclass warnings | 10 | Out of scope |
| DataTable item/community content warnings | 5 | Out of scope |
| Player experience DataTable timing warning | 1 | Out of scope |
| Gameplay spawn recovery warnings | 2 | Out of scope |

### Visual / IN SCOPE warnings

#### `r.Upscale.Quality` / `r.HeterogeneousVolumes` with `SetByScalability` ignored

Exact representative lines:

```text
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:343:[2026.05.14-19.36.49:279][  0]LogConsoleManager: Warning: Setting the console variable 'r.Upscale.Quality' with 'SetByScalability' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '1'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:344:[2026.05.14-19.36.49:295][  0]LogConsoleManager: Warning: Setting the console variable 'r.HeterogeneousVolumes' with 'SetByScalability' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '1'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:370:[2026.05.14-19.36.50:140][  0]LogConsoleManager: Warning: Setting the console variable 'r.Upscale.Quality' with 'SetByScalability' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '1'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:371:[2026.05.14-19.36.50:155][  0]LogConsoleManager: Warning: Setting the console variable 'r.HeterogeneousVolumes' with 'SetByScalability' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '1'
```

Emitter:
- Unreal logs this warning in `ConsoleManager.cpp` when a lower-priority CVar write loses to an earlier higher-priority write (`C:/Program Files/Epic Games/UE_5.7/Engine/Source/Runtime/Core/Private/HAL/ConsoleManager.cpp:274-282`).
- Priority ordering confirms `SetByScalability < SetByGameSetting < SetByProjectSetting` (`C:/Program Files/Epic Games/UE_5.7/Engine/Source/Runtime/Core/Public/HAL/IConsoleManager.h:149-153`).

Root cause:
- `r.Upscale.Quality=1` is set in project config (`Config/DefaultEngine.ini:109`), so later scalability attempts cannot override it.
- `r.HeterogeneousVolumes=True` is set in project config (`Config/DefaultEngine.ini:158`), while scalability config also sets `r.HeterogeneousVolumes` at multiple quality buckets (`Config/DefaultScalability.ini:122`, `Config/DefaultScalability.ini:127`, `Config/DefaultScalability.ini:132`, `Config/DefaultScalability.ini:139`). Project setting wins.

Recommended fix:
- Pick one source of truth. For the low-res visual lock, `r.Upscale.Quality` should probably stay in `[SystemSettings]` because the point/nearest upscale behavior must be stable at boot. Remove any runtime/scalability duplicate if warning noise matters.
- For `r.HeterogeneousVolumes`, decide whether it is intentionally project-forced. If yes, remove duplicate scalability entries. If no, remove the project-level line and let scalability own it.

#### `r.AntiAliasingMethod` / `r.TemporalAA.Upsampling` / `r.SecondaryScreenPercentage.GameViewport` / `r.ScreenPercentage.MinResolution` with `SetByGameSetting` ignored

Exact representative lines:

```text
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:582:[2026.05.14-19.36.53:379][  0]LogConsoleManager: Warning: Setting the console variable 'r.SecondaryScreenPercentage.GameViewport' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '100'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:583:[2026.05.14-19.36.53:391][  0]LogConsoleManager: Warning: Setting the console variable 'r.TemporalAA.Upsampling' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '0'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:615:[2026.05.14-19.36.53:765][  0]LogConsoleManager: Warning: Setting the console variable 'r.AntiAliasingMethod' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '0'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:616:[2026.05.14-19.36.53:776][  0]LogConsoleManager: Warning: Setting the console variable 'r.Upscale.Quality' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '1'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:617:[2026.05.14-19.36.53:788][  0]LogConsoleManager: Warning: Setting the console variable 'r.TemporalAA.Upsampling' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '0'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:618:[2026.05.14-19.36.53:800][  0]LogConsoleManager: Warning: Setting the console variable 'r.SecondaryScreenPercentage.GameViewport' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '100'
Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:619:[2026.05.14-19.36.53:812][  0]LogConsoleManager: Warning: Setting the console variable 'r.ScreenPercentage.MinResolution' with 'SetByGameSetting' was ignored as it is lower priority than the previous 'SetByProjectSetting'. Value remains '0'
```

Emitter:
- Same engine warning path in `ConsoleManager.cpp` (`C:/Program Files/Epic Games/UE_5.7/Engine/Source/Runtime/Core/Private/HAL/ConsoleManager.cpp:274-282`).

Root cause:
- Iteration 2 set these values in `Config/DefaultEngine.ini` `[SystemSettings]`: `r.TemporalAA.Upsampling=False`, `r.AntiAliasingMethod=0`, `r.Upscale.Quality=1`, `r.SecondaryScreenPercentage.GameViewport=100`, `r.ScreenPercentage.MinResolution=0` (`Config/DefaultEngine.ini:107-111`).
- Runtime still calls `SetRetroCVarInt/Float`, which uses `ECVF_SetByGameSetting` (`Source/T66/Core/T66RetroFXSubsystem.cpp:121-135`).
- `ApplyResolutionRuntime()` writes those same CVars when real low resolution is active (`Source/T66/Core/T66RetroFXSubsystem.cpp:942-953`), and restore code also writes them on cleanup (`Source/T66/Core/T66RetroFXSubsystem.cpp:1822-1831`).

Recommended fix:
- Keep the `.ini` values as the visual-lock source of truth for boot-critical low-res rendering, and remove or conditionally skip the runtime writes for CVars already project-owned.
- Alternative: remove these lines from `[SystemSettings]` and keep runtime ownership, but this is less stable for fresh staged boot and would not match the Iteration 2 reason for adding them.
- `r.ScreenPercentage` itself is still runtime-owned and should remain runtime-controlled because it depends on viewport height and `TargetResolutionHeightPercent` (`Source/T66/Core/T66RetroFXSubsystem.cpp:938-953`).

#### `ApplyPs1Parameters: PS1 post-process DMI was null`

Exact line:

```text
Saved/Logs/T66.log:1087:[2026.05.15-07.09.21:507][274]LogT66RetroFXRuntime: Warning: ApplyPs1Parameters: PS1 post-process DMI was null
```

Emitter:
- `UT66RetroFXSubsystem::ApplyPs1Parameters()` logs this when `Ps1PostProcessDMI` is null (`Source/T66/Core/T66RetroFXSubsystem.cpp:802-807`).

Root cause:
- Retro asset preload is async: initialization queues preloads (`Source/T66/Core/T66RetroFXSubsystem.cpp:514-518`, `Source/T66/Core/T66RetroFXSubsystem.cpp:554-607`).
- Object resolution only checks already-loaded objects and does not synchronously load if absent (`Source/T66/Core/T66RetroFXSubsystem.cpp:58-72`).
- `ApplySettings()` calls `EnsurePs1PostProcessDMI()` and immediately calls `ApplyPs1Parameters()` (`Source/T66/Core/T66RetroFXSubsystem.cpp:671-680`, `Source/T66/Core/T66RetroFXSubsystem.cpp:744-756`). If the PS1 material is not loaded yet, the first apply logs this warning.
- Preload completion reapplies settings (`Source/T66/Core/T66RetroFXSubsystem.cpp:609-615`), so this is likely transient rather than a missing-asset failure.

Recommended fix:
- If all PS1 weights are zero, skip PS1 DMI creation and parameter application entirely. If PS1 becomes active, defer warning until after async preload has completed or promote missing-asset state to a single error if the asset is still absent.

#### `ApplyChromaticAberrationParameters: chromatic aberration DMI was null`

Exact line:

```text
Saved/Logs/T66.log:1088:[2026.05.15-07.09.21:507][274]LogT66RetroFXRuntime: Warning: ApplyChromaticAberrationParameters: chromatic aberration DMI was null
```

Emitter:
- `UT66RetroFXSubsystem::ApplyChromaticAberrationParameters()` logs this when `ChromaticAberrationDMI` is null (`Source/T66/Core/T66RetroFXSubsystem.cpp:850-856`).

Root cause:
- Same async preload timing as PS1. Chromatic material path is hardcoded (`Source/T66/Core/T66RetroFXSubsystem.cpp:52`) and loaded through `LoadChromaticAberrationMaterial()` (`Source/T66/Core/T66RetroFXSubsystem.cpp:1640-1643`), but defaults leave chromatic strength and distortion at zero (`Source/T66/Core/T66RetroFXSettings.h:76-83`). The subsystem still ensures and applies the chromatic DMI (`Source/T66/Core/T66RetroFXSubsystem.cpp:737`, `Source/T66/Core/T66RetroFXSubsystem.cpp:678`).

Recommended fix:
- Gate chromatic DMI load/parameter application behind nonzero `ChromaticAberrationPercent` or `ChromaticDistortionPercent`. Treat null as verbose while the effect is disabled.

#### `ApplyResolutionCollection: resolution MPC was null`

Exact line:

```text
Saved/Logs/T66.log:1089:[2026.05.15-07.09.21:507][274]LogT66RetroFXRuntime: Warning: ApplyResolutionCollection: resolution MPC was null
```

Emitter:
- `UT66RetroFXSubsystem::ApplyResolutionCollection()` logs this if `LoadResolutionCollection()` returns null (`Source/T66/Core/T66RetroFXSubsystem.cpp:883-895`).

Root cause:
- The resolution MPC path points to `/Game/UE5RFX/Materials/UE5RFX_MaterialParameterCollection` (`Source/T66/Core/T66RetroFXSubsystem.cpp:53`), and the loader only resolves already-loaded objects after queuing async preload (`Source/T66/Core/T66RetroFXSubsystem.cpp:1646-1653`). The warning fires during the first immediate apply before preload finishes.
- This is separate from real low-resolution framebuffer rendering: `ApplyResolutionRuntime()` sets `r.ScreenPercentage` and associated CVars directly (`Source/T66/Core/T66RetroFXSubsystem.cpp:919-963`). The MPC appears to support fake resolution switch material parameters (`Source/T66/Core/T66RetroFXSubsystem.cpp:905-912`).

Recommended fix:
- Skip `ApplyResolutionCollection()` while both fake resolution switches are zero, or make the MPC load synchronous only if fake-resolution material features are enabled. Keep `ApplyResolutionRuntime()` for real low-res framebuffer work.

#### `[Pixelation] Material at /Game/UI/M_PixelationPostProcess... is not loaded yet`

Exact line:

```text
Saved/Logs/T66.log:1095:[2026.05.15-07.09.21:508][274]LogT66Pixelation: Warning: [Pixelation] Material at /Game/UI/M_PixelationPostProcess.M_PixelationPostProcess is not loaded yet - pixelation disabled until async preload completes.
```

Emitter:
- `UT66PixelationSubsystem::GetOrCreatePixelationMaterial()` logs the warning when the soft object path cannot resolve/find the material (`Source/T66/Core/T66PixelationSubsystem.cpp:37-47`).

Root cause:
- Pixelation preload is async (`Source/T66/Core/T66PixelationSubsystem.cpp:22-26`, `Source/T66/Core/T66PixelationSubsystem.cpp:51-59`).
- `UT66RetroFXSubsystem::ApplySettings()` forwards pixelation levels even when current defaults are zero (`Source/T66/Core/T66RetroFXSubsystem.cpp:686-696`), and `UT66PixelationSubsystem::SetPixelationLevels()` immediately ensures a blendable in the current world (`Source/T66/Core/T66PixelationSubsystem.cpp:81-96`).
- If the material is still loading, `EnsureBlendableInWorld()` queues preload again and returns (`Source/T66/Core/T66PixelationSubsystem.cpp:156-160`).

Recommended fix:
- If both world and character pixelation levels are zero, return before `EnsureBlendableInWorld()` and do not load the material. If nonzero, keep async preload but log once or at verbose until the handle completes.

### Out-of-scope warnings

Grouped from `Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log`:

| Group | Count | Representative lines |
|---|---:|---|
| Steam/online/socket setup | 16 | Steam API failed at `:254-268`; Steam socket deprecated/missing keys at `:274-276`; Steam helper unavailable at `:579-580`. |
| Audio missing packages/sound classes | 10 | Missing `/Game/Audio/...` packages at `:570-577`; missing `SC_SFX` / `SC_Music` sound classes at `:903-904`. |
| Item/community content data rows | 5 | `Item_GamblersToken` missing at `:568`; `Item_Alchemy` and invalid challenge item warnings at `:920-923`. |
| Player experience DataTable timing | 1 | `DT_PlayerExperience` unavailable at `:578`. |
| Gameplay spawn recovery | 2 | Pawn spawn recovery/duplicate prevention at `:621` and `:850`. |

No other visual-related warning class was found hidden in the representative staged log beyond the CVar warnings.

## Pending Issues Created

None. This was a read-only investigation pass. New cleanup candidates and visual-warning root causes are documented in this report rather than added as code-folder pending issues.
