# T66 Final Cleanup Backlog Audit

Date: 2026-05-15  
Scope: read-only audit of remaining visual-system cleanup work after Visual Lock Iteration 02, Terrain Fix Iteration 01, and Visual Cleanup Iteration 01.  
Working directory: `C:\UE\T66`

## 1. Executive summary

This audit found 28 cleanup backlog items: 3 in **Resolve Now**, 9 in **Decide First**, 15 in **Defer with Tracking**, and 1 stale pending issue in **Already Resolved**. The highest-impact immediate items are resolving the remaining `r.HeterogeneousVolumes` project-vs-scalability warning, finishing the project-owned CVar cleanup that still leaves dead capture/restore state behind in `UT66RetroFXSubsystem`, and removing the empty `_Legacy` content folder while leaving the still-referenced migration constant alone.

The current gameplay visual baseline is already materially cleaner than the investigation state: frontend no longer invokes gameplay Retro FX, redundant runtime writes for project-owned renderer CVars were removed, disabled Retro FX features are gated before DMI/MPC loads, Track 2 test instances and unused terrain commons were archived, and the Knight preload was removed in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:9-109`. The remaining work is mostly organization, stale data cleanup, and decisions about whether archived or gated systems should be deleted permanently.

Companion and hero visual ownership is now audited. Companion rows are active as data rows because `Content/Data/Companions.csv:2-25` defines 24 companion IDs and `UT66CharacterVisualSubsystem::GetCompanionVisualID()` constructs matching visual row IDs at runtime in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1314-1321`, but those 24 IDs collapse onto 8 repeated meshes per skin in `Content/Data/CharacterVisuals.csv:27-74`. Hero rows are mostly active through body and skin construction in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1301-1311`; `Content/Data/CharacterVisuals.csv:3` (`Hero_1_Chad_QuadRetroUALQA`) is duplicate gameplay data but still referenced by Model Generation rigging tools, so it needs a decision before removal.

## 2. Categorized backlog

### Resolve Now

| Item | Location | Evidence | Recommendation | Rationale |
|---|---|---|---|---|
| Remove the empty `_Legacy` character folder only. | `Content/Characters/_Legacy/`; `Scripts/QuadRetroCharacterPipelineDefaults.py:18`; `Scripts/MigrateQuadRetroMaterialAssignment.py:136-145`; `Scripts/ImportQuadRetroEnemyVisuals.py:560`; `Audit/Reference/Mob_Production_Import/Report.md:112` | `Content/Characters/_Legacy/` is currently empty. The old `/Game/Characters/_Legacy/MaterialInstances_QuadRetro/` folder was deleted during the mob production import per `Audit/Reference/Mob_Production_Import/Report.md:112`, but `LEGACY_MI_DIR` remains live script state in `Scripts/QuadRetroCharacterPipelineDefaults.py:18`, is created/used by `Scripts/MigrateQuadRetroMaterialAssignment.py:136-145`, and is deleted by the enemy visual import workflow at `Scripts/ImportQuadRetroEnemyVisuals.py:560`. | Delete the empty folder if it exists on disk, but do not remove or rename the `LEGACY_MI_DIR` script constant until the migration workflow is retired. | Safe folder hygiene. Script ownership of the legacy material parking path is a separate decision. |
| Prune unused original CVar capture state for project-owned renderer CVars. | `Source/T66/Core/T66RetroFXSubsystem.h:203-211`; `Source/T66/Core/T66RetroFXSubsystem.cpp:1873-1899`; `Source/T66/Core/T66RetroFXSubsystem.cpp:1915-1918` | Visual Cleanup Iteration 01 removed runtime restore writes for project-owned CVars and now restores only `r.ScreenPercentage` plus `r.ScreenPercentage.MinResolutionFraction` in `Source/T66/Core/T66RetroFXSubsystem.cpp:1915-1918`. The class still stores original values for `r.SecondaryScreenPercentage.GameViewport`, `r.ScreenPercentage.MinResolution`, `r.Upscale.Quality`, `r.AntiAliasingMethod`, and `r.TemporalAA.Upsampling` in `Source/T66/Core/T66RetroFXSubsystem.h:203-211` and captures them in `Source/T66/Core/T66RetroFXSubsystem.cpp:1873-1899`. | Remove the no-longer-restored member fields and capture calls. | Low-risk code cleanup that matches the new Config-owned source of truth documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:30-51`. |
| Resolve the remaining `r.HeterogeneousVolumes` duplicate ownership warning. | `Source/T66/Core/pending_issues_Core.md:17`; `Config/DefaultEngine.ini:158`; `Config/DefaultScalability.ini:122`; `Config/DefaultScalability.ini:127`; `Config/DefaultScalability.ini:132`; `Config/DefaultScalability.ini:139` | The pending issue identifies two owners for the same rendering CVar in `Source/T66/Core/pending_issues_Core.md:17`. The project setting is in `Config/DefaultEngine.ini:158`; scalability buckets also assign it in `Config/DefaultScalability.ini:122`, `:127`, `:132`, and `:139`. Visual Cleanup Iteration 01 left this as the known remaining project-vs-scalability warning in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:129-175`. | Keep one source of truth, preferably project setting if the warning policy remains "project owns visual baseline", and remove the duplicate scalability assignments. | Direct warning cleanup with narrow config blast radius. |

### Decide First

| Item | Location | Evidence | Recommendation | Rationale |
|---|---|---|---|---|
| Decide whether `Hero_1_Chad_QuadRetroUALQA` should stay as a tooling validation row or be retired. | `Content/Data/CharacterVisuals.csv:3`; `Model Generation/Rigging and Animation/Tools/verify_arthur_quadretro_animation_in_unreal.py:24`; `Model Generation/Rigging and Animation/Tools/import_arthur_quadretro_animation_to_unreal.py:40`; `Model Generation/Rigging and Animation/02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md:163` | The row duplicates gameplay `Hero_1_Chad` assets from `Content/Data/CharacterVisuals.csv:2`, and runtime hero IDs do not produce `Hero_1_Chad_QuadRetroUALQA` in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1301-1311`. Model Generation rigging tools still default to this validation ID in `verify_arthur_quadretro_animation_in_unreal.py:24` and `import_arthur_quadretro_animation_to_unreal.py:40`; pipeline instructions call it a temporary validation row in `02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md:163`. | Decide whether to update the tooling to target `Hero_1_Chad` and delete the row, or keep the row as an explicit tool-only validation hook. | Not safe to remove blindly because external generation/animation tools still reference the row. |
| Delete archived visual assets permanently, or keep them through one more release. | `Content/Characters/_Archive/`; `Content/Materials/_Archive/`; `Content/World/Terrain/_Archive/`; `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:75-99` | Visual Cleanup Iteration 01 moved unused terrain commons, old hero imports, Knight assets, and Track 2 material test instances into `_Archive` folders in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:75-99`. | Pablo decision: keep archived assets as a rollback shelf, or delete them after a staged build and gameplay smoke verify. | Deletion is probably safe but irreversible without source control restore; this matches Pablo's archive-then-delete workflow. |
| Decide whether companion visual duplication is intentional content reuse or cleanup debt. | `Content/Data/Companions.csv:2-25`; `Content/Data/CharacterVisuals.csv:27-74`; `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1314-1321` | There are 24 active companion IDs in `Content/Data/Companions.csv:2-25`, and runtime constructs row IDs from companion ID plus optional skin in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1314-1321`. The 48 companion visual rows in `Content/Data/CharacterVisuals.csv:27-74` reuse only 8 base meshes and 8 Beachgoer meshes, with each mesh reused by 3 companion IDs. | Decide whether 24 companion data rows should remain with shared meshes, or whether the visual table should collapse to 8 true visual rows plus explicit gameplay mapping. | Not a safe blind cleanup: the rows are active even though the mesh assets are duplicated. |
| Decide Stacy Beachgoer policy. | `Content/Data/CharacterVisuals.csv:79-90`; `Source/T66/Core/T66SkinSubsystem.cpp:9-19`; `Source/T66/Core/T66CharacterVisualSubsystem.cpp:650-695` | Default and Beachgoer skins exist in `Source/T66/Core/T66SkinSubsystem.cpp:9-19`. The visual table has Stacy default rows in `Content/Data/CharacterVisuals.csv:79-90`, but no `Hero_X_Stacy_Beachgoer` rows. The fallback path rewrites `Hero_X_Stacy_Beachgoer` to a Chad fallback in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:650-695`. | Decide whether Stacy Beachgoer should intentionally fall back to Chad Beachgoer, be hidden in UI, or receive explicit rows. | Current behavior is functional but implicit. A cleanup pass should not erase or add body/skin variants without design approval. |
| Decide whether `M_GLB_ViewSpaceLit_Character` stays parked for future A/B or becomes archive/delete candidate. | `Content/Materials/pending_issues_Materials.md:3`; `Audit/Reference/Visual_Systems_Audit/Report.md:1-19`; `Audit/Reference/Gameplay_Visual_Cleanup_Investigation/Report.md:102-113` | The pending issue says the view-space-lit master still needs a visual lock decision in `Content/Materials/pending_issues_Materials.md:3`. The visual systems audit preserved it as Track 2 parked material context in `Audit/Reference/Visual_Systems_Audit/Report.md:1-19`, and the cleanup investigation explicitly kept the master while archiving only Track 2 test instances in `Audit/Reference/Gameplay_Visual_Cleanup_Investigation/Report.md:102-113`. | Pablo decision after visual lock: keep for A/B, archive, or delete. | This is a real visual direction decision, not mechanical cleanup. |
| Decide whether the old skeletal import workflow should be archived or repaired. | `Scripts/pending_issues_Scripts.md:3`; `Scripts/ImportSkeletalMeshes.py:27-32`; `Scripts/ImportSkeletalMeshes.py:47-52`; `Scripts/VerifyImportBatch.py:44-55` | The pending issue notes that legacy hero import scripts reference archived Idle/Walk paths in `Scripts/pending_issues_Scripts.md:3`. Current script defaults still point at old Hero and Companion import folders in `Scripts/ImportSkeletalMeshes.py:27-32` and `Scripts/ImportSkeletalMeshes.py:47-52`; verification defaults still include old skeletal rows in `Scripts/VerifyImportBatch.py:44-55`. | Decide whether to archive this old import workflow, or update it to current QuadRetro/Pixal3D conventions. | The scripts are not safe to delete blindly because `Scripts/README.md:14` still presents them as part of the import core. |
| Decide whether GLB/static-mesh unlit migration helpers are still active workflow or legacy. | `Scripts/MakeGLBImportsUnlit.py:2`; `Scripts/MakeGLBImportsUnlit.py:8-17`; `Scripts/ImportStaticMeshes.py:19`; `Scripts/RepairStaticMeshImportBatch.py:18` | `MakeGLBImportsUnlit.py` describes itself as a safe rerunnable helper for static-mesh GLB imports in `Scripts/MakeGLBImportsUnlit.py:2` and `Scripts/MakeGLBImportsUnlit.py:8-17`. It is still imported by `Scripts/ImportStaticMeshes.py:19` and `Scripts/RepairStaticMeshImportBatch.py:18`. | Decide whether static GLB import remains a live path. If not, archive helper and callers together; if yes, keep. | This is workflow ownership cleanup, not just dead-file deletion. |
| Decide whether one-off character texture repair scripts should move to archive. | `Scripts/RetroactivelyNormalizeCharacterTextures.py:1`; `Scripts/RepairQuadRetroHeroTexturesAndExit.py:5`; `Audit/Reference/Visual_Systems_Audit/Report.md:138-158` | The visual systems audit identifies the current texture defaults path through `SetCharacterTextureStreamingDefaults.py` and related scripts in `Audit/Reference/Visual_Systems_Audit/Report.md:138-158`. `RepairQuadRetroHeroTexturesAndExit.py` describes a one-off repair for the first GLB hero pass in `Scripts/RepairQuadRetroHeroTexturesAndExit.py:5`; `RetroactivelyNormalizeCharacterTextures.py` is a bulk retrofit script in `Scripts/RetroactivelyNormalizeCharacterTextures.py:1`. | Decide whether to archive one-off repair scripts after confirming no current process doc calls them. | Likely legacy, but the scripts are safer to archive after a workflow doc check than to delete immediately. |
| Decide whether to retire gated Retro FX feature code permanently. | `Source/T66/Core/T66RetroFXSettings.h:23-95`; `Source/T66/Core/T66RetroFXSettings.h:188-233`; `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:295-337` | Current defaults zero or disable PS1, N64 blur, chromatic, fake-resolution switches, runtime pixelation, outline, and geometry effects in `Source/T66/Core/T66RetroFXSettings.h:23-95` and `Source/T66/Core/T66RetroFXSettings.h:188-233`. The UI still exposes controls for PS1, geometry, N64, chromatic, and fake-resolution switches in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:295-337`. | Keep gated until Pablo decides feature retirement. Do not delete in the next fix pass unless the UI controls are also removed. | The code paths are disabled in the current baseline, but they remain user-facing experimentation controls. |

### Defer with Tracking

| Item | Location | Evidence | Recommendation | Rationale |
|---|---|---|---|---|
| Missing production archetype C++ classes. | `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md:3`; `Source/T66/Gameplay/Enemies/` | The pending issue lists missing production archetype classes in `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md:3`. The current source folder contains only base/flying/melee/ranged/rush enemy classes, while production archetypes such as Exploder, Strafer, Stutterer, Turret, Burrower, and Necromancer are not implemented as separate C++ classes. | Defer to a dedicated enemy-behavior implementation pass. | Real gameplay architecture work, not visual cleanup. |
| Enemy family, role, and archetype schema redundancy. | `Source/T66/Data/pending_issues_Data.md:3`; `Source/T66/Data/T66DataTypes.h:1251`; `Source/T66/Data/T66DataTypes.h:1255`; `Source/T66/Data/T66DataTypes.h:1272` | The pending issue identifies redundancy in `Source/T66/Data/pending_issues_Data.md:3`. The live data struct still carries `FamilyID`, `RoleID`, and `Archetype` fields in `Source/T66/Data/T66DataTypes.h:1251`, `:1255`, and `:1272`. | Defer to schema migration. | Requires data migration and spawn/director updates. |
| Spawn director fallback-family behavior. | `Source/T66/Gameplay/pending_issues_Gameplay.md:3`; `Source/T66/Gameplay/T66EnemyDirector.cpp:84-98`; `Source/T66/Gameplay/T66EnemyDirector.cpp:117` | The pending issue remains valid in `Source/T66/Gameplay/pending_issues_Gameplay.md:3`. The director still resolves enemy class through family fallback logic in `Source/T66/Gameplay/T66EnemyDirector.cpp:84-98` and applies the result in `Source/T66/Gameplay/T66EnemyDirector.cpp:117`. | Defer with the enemy archetype pass. | Tied to missing archetype classes and schema cleanup. |
| Hell core has no ranged mob. | `Source/T66/Gameplay/pending_issues_Gameplay.md:10`; `Content/Data/Enemies.csv` | The pending issue remains valid in `Source/T66/Gameplay/pending_issues_Gameplay.md:10`. Enemy data still encodes production rows in `Content/Data/Enemies.csv`, but the ranged-family coverage issue is a content/spawn-design gap. | Defer to roster/spawn balancing. | Gameplay roster design, not visual-system cleanup. |
| Empty stage slots in stages 1-3. | `Content/Data/pending_issues_Data.md:3`; `Content/Data/Stages.csv:2-4` | The pending issue identifies empty early-stage slots in `Content/Data/pending_issues_Data.md:3`. Stages 1-3 still contain explicit `None` entries in enemy slot columns at `Content/Data/Stages.csv:2-4`. | Defer to stage pacing/content pass. | Intent may be progressive unlock pacing; not a visual cleanup item. |
| Status effects not assigned to production mobs. | `Content/Data/pending_issues_Data.md:10`; `Content/Data/Enemies.csv` | The pending issue says production mobs still mostly use `None` for `StatusEffectOnHit` in `Content/Data/pending_issues_Data.md:10`. Current `Content/Data/Enemies.csv` still uses `None` for production mob status-effect fields. | Defer to combat/status design. | Not visual cleanup. |
| Community challenge references invalid starting item. | `Content/Data/pending_issues_Data.md:17` | The pending issue documents a staged smoke warning for an invalid starting item in `Content/Data/pending_issues_Data.md:17`. No obvious live text reference to `Item_Alchemy` was found in the inspected source/data, so the source may be generated, saved, or backend-provided. | Defer with tracking until community challenge data ownership is inspected. | Needs its own data-source investigation before deletion or rewrite. |
| No automated main-board enemy wave smoke hook. | `Source/T66/Gameplay/pending_issues_Gameplay.md:17` | The pending issue remains in `Source/T66/Gameplay/pending_issues_Gameplay.md:17`. | Defer to automation/test infrastructure. | Useful validation work, but not a cleanup pass dependency. |
| Player experience tuning can be requested before its DataTable is available. | `Source/T66/Gameplay/pending_issues_Gameplay.md:24`; `Source/T66/Core/T66PlayerExperienceSubSystem.cpp:56-85`; `Source/T66/Core/T66PlayerExperienceSubSystem.cpp:113-123` | The subsystem still queues async loading in `Source/T66/Core/T66PlayerExperienceSubSystem.cpp:56-85` and warns when tuning is requested before the table is available in `Source/T66/Core/T66PlayerExperienceSubSystem.cpp:113-123`. | Defer to gameplay initialization/data-loading pass. | Real startup-order issue outside visual cleanup. |
| Generated wall stack has a mid-height visual join. | `Source/T66/Gameplay/pending_issues_Gameplay.md:31`; `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:86-90` | Terrain Fix Iteration 01 intentionally stacked two wall pieces and documented the remaining mid-stack join in `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:86-90`; the pending issue records it in `Source/T66/Gameplay/pending_issues_Gameplay.md:31`. | Defer to terrain asset/material authoring. | Requires wall modules that tile vertically or top/bottom variants, not just code cleanup. |
| Inter-walkable-box floor seams remain possible. | `Source/T66/Gameplay/pending_issues_Gameplay.md:38`; `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:82`; `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:86-90` | The terrain fix removed internal subdivision seams, but the report still notes seams between separate walkable rectangles in `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:82` and tracks it in `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:86-90`. Pending entry is `Source/T66/Gameplay/pending_issues_Gameplay.md:38`. | Defer to a terrain geometry merge pass. | Requires unifying walkable boxes per gameplay floor or generating continuous surfaces. |
| Doorway header mesh selection lacks source run metadata. | `Source/T66/Gameplay/pending_issues_Gameplay.md:45`; `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:86-90` | Doorway headers were enabled, but source run metadata remains a known limitation in `Audit/Reference/Terrain_Fix/Iteration_01_Report.md:86-90`; pending entry is `Source/T66/Gameplay/pending_issues_Gameplay.md:45`. | Defer to terrain metadata refactor. | Visual polish issue coupled to wall-run generation metadata. |
| Legacy Lab unlock IDs in existing save games. | `Source/T66/Core/pending_issues_Core.md:3` | The pending issue remains in `Source/T66/Core/pending_issues_Core.md:3`. | Defer to save migration. | Not visual cleanup and needs player-save compatibility handling. |
| Skeletal hero rows ignore `MeshRelativeScale`. | `Source/T66/Core/pending_issues_Core.md:10` | The pending issue remains in `Source/T66/Core/pending_issues_Core.md:10`. | Defer to hero visual runtime QA. | Visual-adjacent but not low risk because hero scale affects gameplay presentation and animation. |
| Staged gameplay smoke references missing audio assets. | `Content/Audio/pending_issues_Audio.md:3` | The pending issue remains in `Content/Audio/pending_issues_Audio.md:3`. | Defer to audio/content cleanup. | Out of scope for gameplay visual cleanup. |

### Already Resolved

| Item | Location | Evidence | Recommendation | Rationale |
|---|---|---|---|---|
| Companion and hero variant ownership needed a dedicated audit. | `Content/Characters/pending_issues_Characters.md:3`; this report, Section 5 | The pending issue requests the audit in `Content/Characters/pending_issues_Characters.md:3`. Section 5 of this report now inventories every companion and hero visual row, the active construction paths, duplicate usage, and decision points. | Remove or replace the pending issue after Pablo reviews this backlog. | The audit gap is resolved; resulting cleanup decisions are tracked as separate backlog items above. |

## 3. `_Legacy` / `_Archive` consolidation plan

| Folder | Current contents | References found | Plan | Final disposition |
|---|---|---|---|---|
| `/Game/Characters/_Legacy/` | Empty. | Script references remain in `Scripts/QuadRetroCharacterPipelineDefaults.py:18`, `Scripts/MigrateQuadRetroMaterialAssignment.py:136-145`, and `Scripts/ImportQuadRetroEnemyVisuals.py:560`; historical audit text also references the old deleted material-instance folder in `Audit/Reference/Mob_Production_Import/Report.md:112`. | Delete the empty folder only. Keep the script constant until the legacy material migration workflow is retired or renamed. | Delete empty folder now; decide script path ownership later. |
| `/Game/Characters/_Archive/Heroes/Hero_1/Chad/IdleWalk_Legacy/` | Archived old Hero_1 Chad Idle/Walk import assets. | Archive move documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:91`. Legacy scripts still reference old source paths per `Scripts/pending_issues_Scripts.md:3`. | Keep until Pablo decides whether the old skeletal import workflow is retired. | Candidate for deletion after workflow decision and staged verification. |
| `/Game/Characters/_Archive/Heroes/Hero_3/Chad/RigPrototype/` | Archived rig prototype assets. | Archive move documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:92`. | Keep through one release or delete after Pablo approval. | Candidate for deletion after verification. |
| `/Game/Characters/_Archive/Heroes/Knight/` | Archived Knight folder and animation assets. | Archive move documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:93`; Knight preload was removed in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:100-109`. | Delete after Pablo approves archive purge. | Candidate for deletion after verification. |
| `/Game/Materials/_Archive/Track2/` | Track 2 test material instances/backdrop. | Archive move documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:95-99`; master `M_GLB_ViewSpaceLit_Character` intentionally kept per `Content/Materials/pending_issues_Materials.md:3`. | Keep until view-space-lit master decision. | Candidate for deletion if Track 2 A/B is abandoned. |
| `/Game/World/Terrain/_Archive/Common/Landscape/` | Unused landscape layer/material assets. | Archive move documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:77-79`. | Delete after one staged verification if no rollback is needed. | Candidate for deletion after verification. |
| `/Game/World/Terrain/_Archive/Common/Rocks/` | Unused rock meshes/material instances. | Archive move documented in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:80-83`. | Delete after one staged verification if no rollback is needed. | Candidate for deletion after verification. |

## 4. Pending issues hygiene plan

| Pending file | Entry | Current status | Hygiene action |
|---|---|---|---|
| `Scripts/pending_issues_Scripts.md` | Legacy Hero Import Scripts Reference Archived Idle Walk Paths, line 3 | Still valid, but needs a workflow decision. | Keep for now. After Pablo decides old skeletal import workflow fate, update or remove. |
| `Content/Audio/pending_issues_Audio.md` | Staged Gameplay Smoke References Missing Audio Assets, line 3 | Still valid and out of visual scope. | Keep. |
| `Content/Data/pending_issues_Data.md` | Empty Stage Slots In Stages 1-3, line 3 | Still valid but not visual cleanup. | Keep. |
| `Content/Data/pending_issues_Data.md` | Status Effects Not Assigned To Production Mobs, line 10 | Still valid but not visual cleanup. | Keep. |
| `Content/Data/pending_issues_Data.md` | Community Challenge References Invalid Starting Item, line 17 | Still unresolved or source not found in static grep. | Keep, but update with the eventual owning data source once found. |
| `Content/Characters/pending_issues_Characters.md` | Remaining Companion And Hero Variant Ownership Needs Dedicated Audit, line 3 | Resolved by this report. | Remove after Pablo accepts this report, or replace with specific companion/hero decision items. |
| `Content/Materials/pending_issues_Materials.md` | View-Space Lit Character Master Needs Visual Lock Decision, line 3 | Still valid. | Keep until visual lock decides material fate. |
| `Source/T66/Data/pending_issues_Data.md` | Enemy Family, Role, And Archetype Redundancy, line 3 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Spawn Director Still Uses Fallback-Family Behavior, line 3 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Hell Core Has No Ranged Mob, line 10 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | No Automated Main-Board Enemy Wave Smoke Hook, line 17 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Player Experience Tuning Can Be Requested Before DataTable Is Available, line 24 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Generated Wall Stack Has A Mid-Height Visual Join, line 31 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Inter-Walkable-Box Floor Seams Remain Possible, line 38 | Still valid. | Keep. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | Doorway Header Mesh Selection Lacks Source Run Metadata, line 45 | Still valid. | Keep. |
| `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md` | Missing Production Archetype Classes, line 3 | Still valid. | Keep. |
| `Source/T66/Core/pending_issues_Core.md` | Legacy Lab Unlock IDs In Existing Save Games, line 3 | Still valid. | Keep. |
| `Source/T66/Core/pending_issues_Core.md` | Skeletal Hero Rows Ignore MeshRelativeScale, line 10 | Still valid. | Keep. |
| `Source/T66/Core/pending_issues_Core.md` | Heterogeneous Volumes CVar Has Two Owners, line 17 | Still valid and Resolve Now. | Remove after config ownership is fixed and staged warning log confirms the warning is gone. |

## 5. Companion / hero variant ownership

### Ownership rules

Hero visual rows are active when they can be produced by `UT66CharacterVisualSubsystem::GetHeroVisualID()`, which constructs `HeroID_Body` and appends a non-default skin suffix in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1301-1311`. Hero application uses that generated ID in `Source/T66/Gameplay/T66HeroBase.cpp:1002-1031`.

Companion visual rows are active when they can be produced by `UT66CharacterVisualSubsystem::GetCompanionVisualID()`, which returns the companion ID for default skin and appends a non-default skin suffix in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1314-1321`. Companion application uses that generated ID in `Source/T66/Gameplay/T66CompanionBase.cpp:196-220`.

The only skin IDs currently defined are `Default` and `Beachgoer` in `Source/T66/Core/T66SkinSubsystem.cpp:9-19`. The UI exposes Beachgoer selection for heroes in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp:456-472` and companions in `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:240-398`.

### Hero visual rows

| Row ID | CSV line | Status | References found | Confidence |
|---|---:|---|---|---|
| `Hero_1_Chad` | 2 | Active | Produced by hero body path in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1301-1311`; validated by gallery setup in `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1796-1811`. | High |
| `Hero_1_Chad_QuadRetroUALQA` | 3 | Duplicate, tool-referenced | No active gameplay runtime construction path found; it duplicates `Hero_1_Chad` assets at `Content/Data/CharacterVisuals.csv:2`. Model Generation rigging tools still reference it in `Model Generation/Rigging and Animation/Tools/verify_arthur_quadretro_animation_in_unreal.py:24` and `Model Generation/Rigging and Animation/Tools/import_arthur_quadretro_animation_to_unreal.py:40`. | High |
| `Hero_1_Chad_Beachgoer` | 4 | Active | Produced by non-default skin suffix path in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1301-1311`; Beachgoer skin exists in `Source/T66/Core/T66SkinSubsystem.cpp:9-19`. | High |
| `Hero_2_Chad` | 5 | Active | Same active body path as above. | High |
| `Hero_2_Chad_Beachgoer` | 6 | Active | Same active skin path as above. | High |
| `Hero_3_Chad` | 7 | Active | Same active body path as above. | High |
| `Hero_3_Chad_Beachgoer` | 8 | Active | Same active skin path as above. | High |
| `Hero_4_Chad` | 9 | Active | Same active body path as above. | High |
| `Hero_4_Chad_Beachgoer` | 10 | Active | Same active skin path as above. | High |
| `Hero_5_Chad` | 11 | Active | Same active body path as above. | High |
| `Hero_5_Chad_Beachgoer` | 12 | Active | Same active skin path as above. | High |
| `Hero_6_Chad` | 13 | Active | Same active body path as above. | High |
| `Hero_6_Chad_Beachgoer` | 14 | Active | Same active skin path as above. | High |
| `Hero_7_Chad` | 15 | Active | Same active body path as above. | High |
| `Hero_7_Chad_Beachgoer` | 16 | Active | Same active skin path as above. | High |
| `Hero_8_Chad` | 17 | Active | Same active body path as above. | High |
| `Hero_8_Chad_Beachgoer` | 18 | Active | Same active skin path as above. | High |
| `Hero_9_Chad` | 19 | Active | Same active body path as above. | High |
| `Hero_9_Chad_Beachgoer` | 20 | Active | Same active skin path as above. | High |
| `Hero_10_Chad` | 21 | Active | Same active body path as above. | High |
| `Hero_10_Chad_Beachgoer` | 22 | Active, needs QA | Same active skin path as above; table row appears to have sparse animation fields and should be checked before any hero skin cleanup. | Medium |
| `Hero_11_Chad` | 23 | Active | Same active body path as above. | High |
| `Hero_11_Chad_Beachgoer` | 24 | Active, needs QA | Same active skin path as above; table row appears to have sparse animation fields and should be checked before any hero skin cleanup. | Medium |
| `Hero_12_Chad` | 25 | Active | Same active body path as above. | High |
| `Hero_12_Chad_Beachgoer` | 26 | Active, needs QA | Same active skin path as above; table row appears to have sparse animation fields and should be checked before any hero skin cleanup. | Medium |
| `Hero_1_Stacy` | 79 | Active | Gallery setup validates Stacy body rows in `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1796-1811`; body path can produce `Hero_1_Stacy` in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1301-1311`. | High |
| `Hero_2_Stacy` | 80 | Active | Same active Stacy body path as above. | High |
| `Hero_3_Stacy` | 81 | Active | Same active Stacy body path as above. | High |
| `Hero_4_Stacy` | 82 | Active | Same active Stacy body path as above. | High |
| `Hero_5_Stacy` | 83 | Active | Same active Stacy body path as above. | High |
| `Hero_6_Stacy` | 84 | Active | Same active Stacy body path as above. | High |
| `Hero_7_Stacy` | 85 | Active | Same active Stacy body path as above. | High |
| `Hero_8_Stacy` | 86 | Active | Same active Stacy body path as above. | High |
| `Hero_9_Stacy` | 87 | Active | Same active Stacy body path as above. | High |
| `Hero_10_Stacy` | 88 | Active | Same active Stacy body path as above. | High |
| `Hero_11_Stacy` | 89 | Active | Same active Stacy body path as above. | High |
| `Hero_12_Stacy` | 90 | Active | Same active Stacy body path as above. | High |

### Companion visual rows

All companion visual rows are active by row-ID construction, because `Content/Data/Companions.csv:2-25` defines companion IDs `Companion_01` through `Companion_24`, and `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1314-1321` derives visual row IDs from the companion ID and skin. The cleanup question is ownership and duplication: the rows are active, but the assets repeat in groups of three.

| Row ID | CSV line | Status | References found | Confidence |
|---|---:|---|---|---|
| `Companion_01` | 27 | Active | Companion data row in `Content/Data/Companions.csv:2`; runtime row construction in `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1314-1321`. Mesh group A default. | High |
| `Companion_01_Beachgoer` | 28 | Active | Beachgoer skin row for `Companion_01`; skin exists in `Source/T66/Core/T66SkinSubsystem.cpp:9-19`. Mesh group A beach. | High |
| `Companion_02` | 29 | Active | Companion data row in `Content/Data/Companions.csv:3`; runtime row construction. Mesh group B default. | High |
| `Companion_02_Beachgoer` | 30 | Active | Beachgoer skin row for `Companion_02`. Mesh group B beach. | High |
| `Companion_03` | 31 | Active | Companion data row in `Content/Data/Companions.csv:4`; runtime row construction. Mesh group C default. | High |
| `Companion_03_Beachgoer` | 32 | Active | Beachgoer skin row for `Companion_03`. Mesh group C beach. | High |
| `Companion_04` | 33 | Active | Companion data row in `Content/Data/Companions.csv:5`; runtime row construction. Mesh group D default. | High |
| `Companion_04_Beachgoer` | 34 | Active | Beachgoer skin row for `Companion_04`. Mesh group D beach. | High |
| `Companion_05` | 35 | Active | Companion data row in `Content/Data/Companions.csv:6`; runtime row construction. Mesh group E default. | High |
| `Companion_05_Beachgoer` | 36 | Active | Beachgoer skin row for `Companion_05`. Mesh group E beach. | High |
| `Companion_06` | 37 | Active | Companion data row in `Content/Data/Companions.csv:7`; runtime row construction. Mesh group F default. | High |
| `Companion_06_Beachgoer` | 38 | Active | Beachgoer skin row for `Companion_06`. Mesh group F beach. | High |
| `Companion_07` | 39 | Active | Companion data row in `Content/Data/Companions.csv:8`; runtime row construction. Mesh group G default. | High |
| `Companion_07_Beachgoer` | 40 | Active | Beachgoer skin row for `Companion_07`. Mesh group G beach. | High |
| `Companion_08` | 41 | Active | Companion data row in `Content/Data/Companions.csv:9`; runtime row construction. Mesh group H default. | High |
| `Companion_08_Beachgoer` | 42 | Active | Beachgoer skin row for `Companion_08`. Mesh group H beach. | High |
| `Companion_09` | 43 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:10`; shares default mesh group A with `Companion_01` and `Companion_17`. | High |
| `Companion_09_Beachgoer` | 44 | Active, duplicate visual asset | Shares beach mesh group A with `Companion_01_Beachgoer` and `Companion_17_Beachgoer`. | High |
| `Companion_10` | 45 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:11`; shares default mesh group B with `Companion_02` and `Companion_18`. | High |
| `Companion_10_Beachgoer` | 46 | Active, duplicate visual asset | Shares beach mesh group B with `Companion_02_Beachgoer` and `Companion_18_Beachgoer`. | High |
| `Companion_11` | 47 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:12`; shares default mesh group C with `Companion_03` and `Companion_19`. | High |
| `Companion_11_Beachgoer` | 48 | Active, duplicate visual asset | Shares beach mesh group C with `Companion_03_Beachgoer` and `Companion_19_Beachgoer`. | High |
| `Companion_12` | 49 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:13`; shares default mesh group D with `Companion_04` and `Companion_20`. | High |
| `Companion_12_Beachgoer` | 50 | Active, duplicate visual asset | Shares beach mesh group D with `Companion_04_Beachgoer` and `Companion_20_Beachgoer`. | High |
| `Companion_13` | 51 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:14`; shares default mesh group E with `Companion_05` and `Companion_21`. | High |
| `Companion_13_Beachgoer` | 52 | Active, duplicate visual asset | Shares beach mesh group E with `Companion_05_Beachgoer` and `Companion_21_Beachgoer`. | High |
| `Companion_14` | 53 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:15`; shares default mesh group F with `Companion_06` and `Companion_22`. | High |
| `Companion_14_Beachgoer` | 54 | Active, duplicate visual asset | Shares beach mesh group F with `Companion_06_Beachgoer` and `Companion_22_Beachgoer`. | High |
| `Companion_15` | 55 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:16`; shares default mesh group G with `Companion_07` and `Companion_23`. | High |
| `Companion_15_Beachgoer` | 56 | Active, duplicate visual asset | Shares beach mesh group G with `Companion_07_Beachgoer` and `Companion_23_Beachgoer`. | High |
| `Companion_16` | 57 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:17`; shares default mesh group H with `Companion_08` and `Companion_24`. | High |
| `Companion_16_Beachgoer` | 58 | Active, duplicate visual asset | Shares beach mesh group H with `Companion_08_Beachgoer` and `Companion_24_Beachgoer`. | High |
| `Companion_17` | 59 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:18`; shares default mesh group A with `Companion_01` and `Companion_09`. | High |
| `Companion_17_Beachgoer` | 60 | Active, duplicate visual asset | Shares beach mesh group A with `Companion_01_Beachgoer` and `Companion_09_Beachgoer`. | High |
| `Companion_18` | 61 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:19`; shares default mesh group B with `Companion_02` and `Companion_10`. | High |
| `Companion_18_Beachgoer` | 62 | Active, duplicate visual asset | Shares beach mesh group B with `Companion_02_Beachgoer` and `Companion_10_Beachgoer`. | High |
| `Companion_19` | 63 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:20`; shares default mesh group C with `Companion_03` and `Companion_11`. | High |
| `Companion_19_Beachgoer` | 64 | Active, duplicate visual asset | Shares beach mesh group C with `Companion_03_Beachgoer` and `Companion_11_Beachgoer`. | High |
| `Companion_20` | 65 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:21`; shares default mesh group D with `Companion_04` and `Companion_12`. | High |
| `Companion_20_Beachgoer` | 66 | Active, duplicate visual asset | Shares beach mesh group D with `Companion_04_Beachgoer` and `Companion_12_Beachgoer`. | High |
| `Companion_21` | 67 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:22`; shares default mesh group E with `Companion_05` and `Companion_13`. | High |
| `Companion_21_Beachgoer` | 68 | Active, duplicate visual asset | Shares beach mesh group E with `Companion_05_Beachgoer` and `Companion_13_Beachgoer`. | High |
| `Companion_22` | 69 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:23`; shares default mesh group F with `Companion_06` and `Companion_14`. | High |
| `Companion_22_Beachgoer` | 70 | Active, duplicate visual asset | Shares beach mesh group F with `Companion_06_Beachgoer` and `Companion_14_Beachgoer`. | High |
| `Companion_23` | 71 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:24`; shares default mesh group G with `Companion_07` and `Companion_15`. | High |
| `Companion_23_Beachgoer` | 72 | Active, duplicate visual asset | Shares beach mesh group G with `Companion_07_Beachgoer` and `Companion_15_Beachgoer`. | High |
| `Companion_24` | 73 | Active, duplicate visual asset | Companion data row in `Content/Data/Companions.csv:25`; shares default mesh group H with `Companion_08` and `Companion_16`. | High |
| `Companion_24_Beachgoer` | 74 | Active, duplicate visual asset | Shares beach mesh group H with `Companion_08_Beachgoer` and `Companion_16_Beachgoer`. | High |

## 6. One-off migration/import script recommendations

| Script | Last known role | Active references found | Recommendation |
|---|---|---|---|
| `Scripts/SetCharacterTextureStreamingDefaults.py` | Current texture group/filter maintenance for character assets. | Referenced as active in `Audit/Reference/Visual_Systems_Audit/Report.md:138-158` and used during mob import validation in `Audit/Reference/Mob_Production_Import/Report.md:130`. | Keep. |
| `Scripts/GenerateCharacterMeshLODs.py` | Current character LOD maintenance. | Referenced as active in `Audit/Reference/Visual_Systems_Audit/Report.md:156-158` and mob import validation in `Audit/Reference/Mob_Production_Import/Report.md:130`. | Keep. |
| `Scripts/MigrateQuadRetroMaterialAssignment.py` | Current material assignment migration path for QuadRetro assets. | Referenced as active in `Audit/Reference/Visual_Systems_Audit/Report.md:156-158` and mob import validation in `Audit/Reference/Mob_Production_Import/Report.md:130`. | Keep. |
| `Scripts/ImportQuadRetroEnemyVisuals.py` and runners/validators | Current 50-mob production import path. | Mob production import report used this workflow in `Audit/Reference/Mob_Production_Import/Report.md:130-132`. | Keep. |
| `Scripts/MakeGLBImportsUnlit.py` | Static GLB unlit helper. | Still imported by `Scripts/ImportStaticMeshes.py:19` and `Scripts/RepairStaticMeshImportBatch.py:18`. | Decide first. |
| `Scripts/MakeCharacterMaterialsUnlit.py` | Old skeletal import material helper. | Still imported by `Scripts/ImportSkeletalMeshes.py:18` and used after import in `Scripts/ImportSkeletalMeshes.py:125`. | Decide first. |
| `Scripts/ImportSkeletalMeshes.py` and `Scripts/RunImportSkeletalMeshesAndExit.py` | Old skeletal import workflow. | Defaults still reference old import folders in `Scripts/ImportSkeletalMeshes.py:27-32` and `Scripts/ImportSkeletalMeshes.py:47-52`; wrapper imports it in `Scripts/RunImportSkeletalMeshesAndExit.py:14-20`. | Decide first: archive or repair. |
| `Scripts/VerifyImportBatch.py` | Old import verification workflow. | Defaults include old hero/companion rows in `Scripts/VerifyImportBatch.py:44-55`; pending issue flags archived Idle/Walk references in `Scripts/pending_issues_Scripts.md:3`. | Decide first. |
| `Scripts/RetroactivelyNormalizeCharacterTextures.py` | Bulk retrofit tool for character texture normalization. | Superseded for active imports by `SetCharacterTextureStreamingDefaults.py` per `Audit/Reference/Visual_Systems_Audit/Report.md:138-158`. | Decide first: likely archive after process-doc check. |
| `Scripts/RepairQuadRetroHeroTexturesAndExit.py` | One-off hero texture repair. | Header describes first-pass GLB texture repair in `Scripts/RepairQuadRetroHeroTexturesAndExit.py:5`; no active import report depends on it. | Decide first: likely archive. |

## 7. Dead code path candidates inside active visual files

| Feature | Current state | Evidence | Recommendation |
|---|---|---|---|
| PS1 stack | Default weights are zero, loading/apply is gated. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:23-50`; gates in `Source/T66/Core/T66RetroFXSubsystem.cpp:361-392` and preload gates in `Source/T66/Core/T66RetroFXSubsystem.cpp:602-636`; UI controls still exist in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:295`. | Keep gated until visual lock decides. |
| N64 blur | Default weights are zero, loading/apply is gated. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:65-74`; UI control in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:323`. | Keep gated. |
| Chromatic aberration | Default weights are zero, loading/apply is gated. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:77-83`; UI control in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:329`; UI style also has separate chromatic treatment fields in `Source/T66/UI/Style/T66Style.cpp:467-502`. | Keep gated; do not delete while UI and style systems still expose chromatic controls. |
| Runtime outline | Default disabled. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:86-95`; outline was intentionally off for baked-outlined mobs in Visual Lock reports `Audit/Reference/Visual_Lock/Iteration_02_Report.md:110-126`. | Keep gated for future A/B only. |
| Fake-resolution material switches | Defaults are zero; real low-res framebuffer is the active baseline. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:53-62`; UI controls still exist in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:336-337`. | Keep gated until UI controls are retired. |
| Geometry effects: vertex snap, noise, affine | Default world/character geometry disabled and percents zero. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:188-233`; UI controls in `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp:302-312`; geometry collection still applies disabled values in `Source/T66/Core/T66RetroFXSubsystem.cpp:1071-1162`. | Keep gated; deletion needs a visual-direction call because these are user-facing settings. |
| Runtime pixelation | Default pixelation levels are zero; separate from real low-res framebuffer. | Defaults in `Source/T66/Core/T66RetroFXSettings.h:86-95`; pixelation material load is gated in `Source/T66/Core/T66PixelationSubsystem.cpp:50-100`. | Keep gated for possible future iteration. |

## 8. File/folder organization findings

| Finding | Location | Evidence | Recommendation |
|---|---|---|---|
| `Source/T66/Core/` owns both persistent settings and visual runtime subsystems. | `Source/T66/Core/T66RetroFXSubsystem.*`; `Source/T66/Core/T66PixelationSubsystem.*`; `Source/T66/Core/T66CharacterVisualSubsystem.*` | Gameplay visual runtime code lives in Core while terrain/world visual setup lives in `Source/T66/Gameplay/`. | Do not move now. If a larger organization pass happens, consider a `Source/T66/Visual/` module or folder, but only after visual lock stabilizes. |
| `Content/Materials/` now has a clear active/archive split, except the parked view-space-lit master. | `Content/Materials/M_GLB_ViewSpaceLit_Character`; `Content/Materials/_Archive/Track2/`; `Content/Materials/pending_issues_Materials.md:3` | Track 2 test instances were archived in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:95-99`; master remains pending decision. | Keep as-is until the material decision. |
| `Content/Characters/` has archive and active production folders; `_Legacy` is empty noise. | `Content/Characters/Mobs/`; `Content/Characters/_Archive/`; `Content/Characters/_Legacy/` | Production mobs are active in `/Game/Characters/Mobs/` per `Audit/Reference/Mob_Production_Import/Report.md`; `_Archive` holds intentional rollback assets; `_Legacy` is empty. | Delete empty `_Legacy` in Resolve Now. |
| Terrain archive is organized, but unresolved terrain geometry polish remains tracked in Gameplay pending issues. | `Content/World/Terrain/_Archive/`; `Source/T66/Gameplay/pending_issues_Gameplay.md:31-45` | Unused commons were archived in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:77-83`; terrain geometry issues remain in pending entries at `Source/T66/Gameplay/pending_issues_Gameplay.md:31-45`. | Keep archive until deletion decision; keep terrain pending issues. |

## 9. Miscellaneous findings

| Finding | Location | Evidence | Recommendation |
|---|---|---|---|
| Visual warning cleanup is mostly complete; remaining warnings are source-of-truth conflicts, not missing assets. | `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:110-133` | Tracked visual CVar priority warnings dropped from 21 to 4, and DMI/MPC/pixelation null warnings were eliminated in `Audit/Reference/Visual_Cleanup/Iteration_01_Report.md:110-133`. | Resolve the remaining repo-owned CVar duplicate where possible; keep `r.Upscale.Quality=1` if it remains required for nearest-style upscaling per `Audit/Reference/Visual_Lock/Iteration_02_Report.md:53`. |
| Current real low-res visual baseline depends on Config ownership. | `Config/DefaultEngine.ini:107-111`; `Audit/Reference/Visual_Lock/Iteration_02_Report.md:34-53` | Visual Lock Iteration 02 moved renderer CVars into `[SystemSettings]` and documented `r.Upscale.Quality=1` as the nearest-style choice in UE 5.7. | Avoid moving these CVars back into runtime code during cleanup. |
| No new pending issue file was created during this audit. | N/A | This was a read-only inventory pass. | N/A |
