# T66 Project Cleanup Inventory

Created: 2026-05-07
Scope: first-pass inventory for `C:\UE\T66` project cleanup before deleting files.

Rules for this pass:
- Do not delete during inventory.
- Classify each top-level folder from the project root.
- Prefer deleting or consolidating aggressively when evidence says the folder is scratch, obsolete, duplicated, generated, or recoverable from git.
- Mark any required tweaks that would make deletion possible.
- Use current repo evidence, git tracking, Unreal references, and packaging behavior instead of guessing from folder names alone.

## 01. `_codex_previews`

Path: `C:\UE\T66\_codex_previews`

Inventory:
- Git status: untracked (`?? _codex_previews/`).
- Git ignore status: not currently ignored.
- Direct subfolders: `arthur_ultrakill_pass`, `old_arthur_extract`, `refs`, `simple_boxes_20260505_2342`, `simple_boxes_20260505_2351`, `simple_boxes_20260505_2356`, `simple_boxes_20260506_0000`.
- Recursive contents: 11 directories, 61 files, about 155.67 MB.
- File types: 44 PNG previews, 14 JSON reports, 2 FBX files, 1 HTML gallery.
- Largest files are old Arthur FBX/source texture extracts under `old_arthur_extract`, including `ArthurIdle.fbx`, `ArthurWalk.fbx`, and extracted `.fbm` textures.

General purpose:
- Local Codex scratch preview workspace for visual experiments, screenshot checks, and old Arthur/Ultrakill-style model pass comparisons.
- This is not a normal Unreal project folder and does not appear to be part of live source, content, config, or build logic.

Reference evidence:
- Search references to `_codex_previews` only found references inside `_codex_previews` itself and inside `Model Generation/Runs/Heroes/_Archive/Obsolete_2026-05-06_QuadRetroFirstPass`.
- That model-generation archive is also currently untracked.
- No live `Source`, `Config`, `Content`, or project build file reference was found.

Recommendation:
- Delete the entire `_codex_previews` folder during cleanup.

Tweaks that make cleanup stronger:
- Add `_codex_previews/` to `.gitignore` so future Codex preview output does not keep reappearing as untracked root noise.
- When the pass reaches `Model Generation`, delete or consolidate `Model Generation/Runs/Heroes/_Archive/Obsolete_2026-05-06_QuadRetroFirstPass` too, because it only preserves obsolete reports that point back to this scratch folder.

Risk:
- Low. The folder is untracked and appears to be disposable preview output.
- If any images are still wanted as human reference, keep only a single contact sheet or move it into a deliberate reference folder before deleting the rest.

Decision:
- Accepted delete: yes.
- Proposed immediate tweak: add `_codex_previews/` to `.gitignore` when cleanup changes begin.

## 02. `ANTI_CHEAT`

Path: `C:\UE\T66\ANTI_CHEAT`

Inventory:
- Git status: tracked.
- Direct subfolders: none.
- Recursive contents: 0 directories, 2 files, about 25.52 KB.
- Files:
  - `MASTER_ANTI_CHEAT.md`
  - `ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`

General purpose:
- Active documentation folder for ranked anti-cheat policy, backend authority, integrity attestation, provenance validation, moderation, and an implementation checklist.
- This is documentation only. It is not a runtime Unreal folder and does not appear in the staged standalone build.

Reference evidence:
- `MASTER_ANTI_CHEAT.md` states that it is the single-source anti-cheat handoff and points to `MASTER DOCS/MASTER_BACKEND.md`.
- `ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md` is a phased execution checklist from 2026-04-18.
- Current docs intentionally reference this folder from:
  - `Docs/README.md`
  - `Docs/Systems/T66_Community_Mods_And_Challenges.md`
  - `MASTER DOCS/README.md`
  - `MASTER DOCS/T66_MASTER_GUIDELINES.md`
  - `MASTER DOCS/MASTER_BACKEND.md`
  - `MASTER DOCS/MASTER_COMBAT.md`
  - `MASTER DOCS/MASTER_PLAYER_EXPERIENCE.md`
  - `MASTER DOCS/T66_DECISION_LOG.md`
- No live `Source`, `Config`, `.uproject`, or packaging reference depends on the folder path.

Recommendation:
- Delete the top-level `ANTI_CHEAT` folder by moving its active docs under `MASTER DOCS/Backend Anti Cheat/`.
- Do not keep a dedicated root folder for two markdown files. It contributes to top-level project clutter and duplicates the master-doc system.

Tweaks required before deletion:
- Move `MASTER_ANTI_CHEAT.md` to `MASTER DOCS/Backend Anti Cheat/MASTER_ANTI_CHEAT.md`.
- Move `ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md` to `MASTER DOCS/Backend Anti Cheat/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`.
- Update markdown references that currently point at `ANTI_CHEAT/MASTER_ANTI_CHEAT.md` or `ANTI_CHEAT/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`.
- Update `MASTER DOCS/T66_MASTER_GUIDELINES.md` so root-level docs no longer list `ANTI_CHEAT/` as a required project root.

Good-to-delete candidates:
- Delete `ANTI_CHEAT/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md` from its old path after moving it into `MASTER DOCS/Backend Anti Cheat/`.
- Delete `ANTI_CHEAT/MASTER_ANTI_CHEAT.md` from its old path after moving it into `MASTER DOCS/Backend Anti Cheat/`.
- Delete the empty `ANTI_CHEAT` folder after both files are removed.

Risk:
- Medium if deleted raw, because active docs still point here and the content contains current ranked/integrity policy.
- Low after consolidation and reference updates.

Decision:
- Accepted cleanup: move docs into `MASTER DOCS/Backend Anti Cheat/`, update references, then remove the root `ANTI_CHEAT` folder.

## 03. `Archive`

Path: `C:\UE\T66\Archive`

Inventory:
- Git status: untracked (`?? Archive/`).
- Direct subfolders:
  - `DataReorg_2026-05-07_EnemyBossPlaceholderArchive`
  - `DataReorg_2026-05-07_PropsRemoval`
  - `DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01`
- Recursive contents: 3 directories, 13 files, about 132.62 KB.
- File types: CSV, JSON, INI, and README snapshots.

General purpose:
- Local backup/archive folder created by recent data reorganization work.
- It preserves pre-change copies of old placeholder enemy/boss rows, deprecated prop data, and pre-batch NPC/interactable visual references.
- It is not a live Unreal `Content`, `Config`, `Source`, `SourceAssets`, or packaging folder.

Subfolder notes:
- `DataReorg_2026-05-07_EnemyBossPlaceholderArchive`: old placeholder roster sources removed from live data. README says live roster source is now under `Content/Data` with contiguous stages `1..20`, five enemies per difficulty, named boss encounters, and status-effect rows.
- `DataReorg_2026-05-07_PropsRemoval`: old `Content/Data/Props.csv` contents for generated world props that are no longer live.
- `DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01`: pre-batch copies of arcade interactables and character visuals before replacing older mesh paths with Quad Retro assets.

Reference evidence:
- No live `Source`, `Config`, `.uproject`, packaging, or runtime asset path depends on this folder.
- Current references are documentation/comments only:
  - `Docs/Systems/EnemyBossRoster_DataContract_2026-05-07.md` points to the enemy/boss placeholder archive.
  - `Scripts/ImportStaticMeshes.py` has a comment saying retired generated world prop row data lives in this archive.
  - `Model Generation/Runs/Interactables/WorldNpcInteractablesRetroBatch01/Notes/STATUS.md` lists the pre-batch archive files.
  - `Docs/README.md` has a broad `Archive/` entry that should probably mean `Docs/Archive/`, not a new root archive folder.

Recommendation:
- Delete the whole root `Archive` folder after reference cleanup.
- Do not preserve root-level archive folders long-term. They encourage keeping old snapshots outside the committed project structure and conflict with the cleanup goal.

Tweaks required before deletion:
- Update `Docs/Systems/EnemyBossRoster_DataContract_2026-05-07.md` to say the old placeholder roster was intentionally removed and is recoverable from git history if needed, rather than pointing to root `Archive`.
- Update the `Scripts/ImportStaticMeshes.py` comment to remove the root archive dependency.
- Update `Model Generation/Runs/Interactables/WorldNpcInteractablesRetroBatch01/Notes/STATUS.md` to state the pre-batch backup files were cleanup-deleted after inventory, or move only a summary into that status file.
- Update `Docs/README.md` to clarify that archived documentation belongs under `Docs/Archive/`, not root `Archive/`.

Good-to-delete candidates:
- Delete all three `Archive/DataReorg_2026-05-07_*` folders.
- Delete the root `Archive` folder after it is empty.
- Delete all 13 files currently inside `Archive`:
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/Bosses.placeholder-boss-number-roster.csv`
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/CharacterVisuals.with-placeholder-enemy-rows.csv`
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/DefaultT66PlayerExperience.old-stage-anchors.ini`
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/DefaultT66StageProgression.old-impossible-3-stage-comment.ini`
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/PlayerExperience.old-stage-anchors.json`
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/README.md`
  - `Archive/DataReorg_2026-05-07_EnemyBossPlaceholderArchive/Stages.placeholder-23row.csv`
  - `Archive/DataReorg_2026-05-07_PropsRemoval/Props_Deprecated_2026-05-07.csv`
  - `Archive/DataReorg_2026-05-07_PropsRemoval/README.md`
  - `Archive/DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01/ArcadeInteractables.pre_world_npc_retro_batch01.json`
  - `Archive/DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01/CharacterVisuals.pre_world_npc_retro_batch01.csv`
  - `Archive/DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01/PreBatchLiveVisualReferences.json`
  - `Archive/DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01/README.md`

Risk:
- Low for runtime/build behavior because nothing live depends on it.
- Medium for historical convenience because the folder is untracked, so these exact backup snapshots are not recoverable from GitHub unless committed first. The live tracked files they were copied from may still be recoverable from git history, but these archive copies themselves are local-only.

Decision:
- Accepted delete: yes.
- Accepted delete scope: every file and subfolder under root `Archive`, then the empty root `Archive` folder itself.
- Required paired edits before deletion: update/remove the few docs and comments that point to this root archive.

## 04. `ArchivedBuilds`

Path: `C:\UE\T66\ArchivedBuilds`

Inventory:
- Git status: ignored by `.gitignore` via `ArchivedBuilds/`.
- Direct subfolders:
  - `Windows`
- Recursive contents: 84 directories, 357 files, about 2493.22 MB / 2.43 GB.
- Top-level packaged tree under `ArchivedBuilds/Windows`:
  - `Engine`
  - `T66`
  - `Manifest_DebugFiles_Win64.txt`
  - `Manifest_NonUFSFiles_Win64.txt`
  - `Manifest_UFSFiles_Win64.txt`
  - `T66.exe`
- `ArchivedBuilds/Windows/T66` contains packaged `Binaries`, `Content`, `RuntimeDependencies`, and `SourceAssets`.
- Largest files:
  - `ArchivedBuilds/Windows/T66/Content/Paks/T66-Windows.ucas`: about 1591.09 MB.
  - `ArchivedBuilds/Windows/T66/Binaries/Win64/T66.pdb`: about 368.91 MB.
  - `ArchivedBuilds/Windows/T66/Binaries/Win64/T66.exe`: about 287.45 MB.

General purpose:
- Old local packaged Windows build archive from 2026-04-09.
- This is generated runtime output, not source, content-authoring, documentation, or a live staging location.

Reference evidence:
- No repo references found for `ArchivedBuilds`.
- `.gitignore` already treats `ArchivedBuilds/` as a build artifact.
- Current active staged standalone is elsewhere:
  - `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Current staged executable timestamp is 2026-05-07, while the archived executable is from 2026-04-09.

Recommendation:
- Delete the entire `ArchivedBuilds` folder.
- Keep relying on GitHub/history for old source states and on `Saved/StagedBuilds/Windows/T66` for the current local packaged build.

Tweaks required before deletion:
- None. It is already ignored and not referenced.

Good-to-delete candidates:
- Delete `ArchivedBuilds/Windows` and everything under it.
- Delete the empty root `ArchivedBuilds` folder after the build archive is removed.

Risk:
- Low for source/project/runtime behavior.
- Low-to-medium for historical convenience only: deleting removes an old local playable packaged build from April 9, 2026. It does not remove source history.

Decision:
- Accepted delete: yes.
- Accepted delete scope: `ArchivedBuilds/Windows` and everything under it, then the empty root `ArchivedBuilds` folder.

## 05. `Art reference images`

Path: `C:\UE\T66\Art reference images`

Inventory:
- Git status: tracked.
- Direct subfolders:
  - `Toon Cel Shading`
- Recursive contents: 1 directory, 3 files, about 7.93 MB.
- File types: PNG only.
- Files:
  - `Art reference images/Toon Cel Shading/01_Reference_5_Original.png`: 2046x1152, about 3.08 MB.
  - `Art reference images/Toon Cel Shading/02_Cardboard_House_Forest.png`: 1672x941, about 2.43 MB.
  - `Art reference images/Toon Cel Shading/03_Graduates_Diplomas.png`: 1672x941, about 2.42 MB.

General purpose:
- Visual reference images for a toon/cel-shading art direction pass.
- This is human reference material, not an Unreal runtime/content folder and not an asset pipeline source folder.

Reference evidence:
- No live references found for `Art reference images`, `Toon Cel Shading`, or the three PNG filenames.
- No `Source`, `Config`, `.uproject`, packaging, data-table, or script path depends on this folder.

Recommendation:
- Delete the entire root `Art reference images` folder.
- The files are tracked in git, so they are recoverable from repository history if the style references are needed later.

Alternative if the user wants to preserve the style reference:
- Move the three PNGs under a deliberate docs/reference path such as `Docs/Art/Reference/Toon Cel Shading/`, then delete the root `Art reference images` folder.
- This keeps the project root clean while preserving the images.

Tweaks required before deletion:
- None, if deleting outright.
- If preserving, create the new docs/reference folder and update no current references, because none were found.

Good-to-delete candidates:
- Delete all three tracked PNG files.
- Delete `Art reference images/Toon Cel Shading`.
- Delete the empty root `Art reference images` folder.

Risk:
- Low for runtime/build behavior.
- Low-to-medium for art-direction convenience. These may still be useful human references for future stylization work, but they are not connected to live project systems.

Decision:
- Accepted delete: yes.
- Accepted delete scope: all three PNG files, the `Toon Cel Shading` subfolder, then the empty root `Art reference images` folder.

## 06. `Audit`

Path: `C:\UE\T66\Audit`

Inventory:
- Git status: mixed tracked and untracked. The current checkout already has pending audit files moved out of `Pending` into `Finished` as unrelated existing work.
- Direct subfolders:
  - `Finished`
  - `Inventory Cleanup`
  - `Pending`
  - `Reference`
- Root files:
  - `README.md`
- Recursive contents: 7 directories, 17 files, about 0.91 MB.
- Current subfolder sizes:
  - `Finished`: 6 files, about 207.67 KB.
  - `Inventory Cleanup`: 1 file, about 14.32 KB.
  - `Pending`: 0 files.
  - `Reference`: 9 files, about 708.13 KB.

General purpose:
- Audit and cleanup-control folder.
- `Audit/README.md` defines the current status taxonomy:
  - `Pending`: unresolved work.
  - `Finished`: closed audits retained as audit trail.
  - `Reference`: background/history, not an active fix queue.
- `Inventory Cleanup/T66_Project_Cleanup_Inventory.md` is the active inventory file for this cleanup pass.

Reference evidence:
- Memory and current `Audit/README.md` agree that this folder should use `Pending`, `Finished`, and `Reference` rather than vague archive buckets.
- Current `Pending` is empty.
- `MASTER DOCS/T66_MASTER_GUIDELINES.md` references `Audit/Reference/T66_UI_AUDIT.md`.
- `MASTER DOCS/README.md` still references the old `Audit/Pending/T66_MASTER_OPTIMIZATION_AUDIT_V5.md` path, which is stale against the current folder state.
- `Audit/Reference/Historical` contains superseded optimization drafts and review passes.
- `Audit/Finished` contains closed audit trails, not active work.

Recommendation:
- Keep the root `Audit` folder.
- Keep `Audit/README.md`.
- Keep `Audit/Inventory Cleanup/T66_Project_Cleanup_Inventory.md` as the active cleanup control file.
- Aggressively delete or consolidate old closed/historical audit material after paired reference updates.

Tweaks required before deletion/consolidation:
- Update `MASTER DOCS/README.md` so it no longer points to `../Audit/Pending/T66_MASTER_OPTIMIZATION_AUDIT_V5.md`.
- Update `Audit/README.md` after any `Finished` or `Reference` files are deleted.
- If deleting `Audit/Reference/T66_UI_AUDIT.md`, first extract any still-current UI policy into `Docs/UI/UI_GENERATION.md`, `MASTER DOCS/T66_MASTER_GUIDELINES.md`, or a future `MASTER DOCS/MASTER_UI.md`.
- Update stale links inside old audit docs only if those docs are kept. If the old docs are deleted, no internal stale-link cleanup is needed.

Good-to-delete candidates:
- Delete `Audit/Reference/Historical/2026-04-16-optimization/` entirely. These are superseded V1-V4 optimization drafts and Claude review passes.
- Delete `Audit/Reference/Historical/2026-04-17-optimization-wave/T66_OPTIMIZATION_AGENT_ASSIGNMENTS.md`. It explicitly says it is old operational handoff material.
- Delete the closed files in `Audit/Finished/` after keeping only short closeout summaries in `Audit/README.md` or the cleanup inventory:
  - `Audit/Finished/PERFORMANCE_AUDIT.md`
  - `Audit/Finished/T66_ARCHIVE_ASSET_CLEANUP_LEDGER.md`
  - `Audit/Finished/T66_DOCS_CLEANUP_LEDGER.md`
  - `Audit/Finished/T66_FULL_AUDIT_2026-05-04.md`
  - `Audit/Finished/T66_MASTER_OPTIMIZATION_AUDIT_V5.md`
  - `Audit/Finished/T66_PACKAGING_CLEANUP_TRACKER.md`
- Delete empty `Pending`, `Finished`, or `Reference/Historical` directories after their contents are removed, unless we decide to keep `.gitkeep` placeholders.
- Candidate for paired consolidation: `Audit/Reference/T66_UI_AUDIT.md`. It is large and useful only as a detailed historical UI audit. Delete it after extracting still-current UI rules/findings into the active UI/master docs.

Risk:
- Keep/delete risk varies by subfolder.
- Low runtime/build risk. Audit docs are not runtime assets.
- Medium documentation risk if `T66_UI_AUDIT.md` or finished packaging/optimization trackers are deleted before their still-current rules are represented in master docs.
- Low risk for deleting historical optimization drafts once current master docs and finished closeout summaries are updated.

Decision:
- Keep root folder: yes.
- Accepted cleanup: yes.
- Proposed cleanup: reduce `Audit` to README plus active inventory, with only genuinely active pending/reference files kept.
- Proposed delete now after paired reference updates: historical reference folders and closed finished audit files.
- Proposed delete after consolidation: `Audit/Reference/T66_UI_AUDIT.md`.

## 07. `Binaries`

Path: `C:\UE\T66\Binaries`

Inventory:
- Git status: ignored by `.gitignore` via `Binaries/`.
- Git tracked files: none.
- Direct subfolders:
  - `Win64`
- Recursive contents: 5 directories, 37 files, about 1327.63 MB / 1.24 GB.
- File types/counts:
  - 22 `.dll`
  - 8 `.pdb`
  - 2 `.target`
  - 1 `.exe`
  - 1 `.exp`
  - 1 `.lib`
  - 1 `.modules`
  - 1 `.txt`
- Largest files:
  - `Binaries/Win64/T66.pdb`: about 400.29 MB.
  - `Binaries/Win64/T66.exe`: about 291.71 MB.
  - `Binaries/Win64/UnrealEditor-T66.pdb`: about 157.61 MB.
  - module PDBs for `T66Mini`, `T66TD`, `T66Deck`, `T66Idle`, `T66Editor`, and `T66Versus`: about 58-71 MB each.
- Third-party/runtime DLL copies are present under `Binaries/Win64`, including Boost, TBB, WebView2Loader, DirectML, and D3D12 files.
- `Binaries/Win64/steam_appid.txt` contains `4464300`.

General purpose:
- Unreal-generated local build output for the project and editor modules.
- This folder is not source. It is regenerated by Unreal builds and editor/module compilation.
- The current taskbar shortcut rule points to the staged standalone executable at `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`, not this root `Binaries` folder.

Reference evidence:
- `.gitignore` explicitly ignores `Binaries/`.
- No files under `Binaries` are tracked by git.
- `MASTER DOCS/T66_MASTER_GUIDELINES.md` already says `Binaries/`, `Intermediate/`, `DerivedDataCache/`, and parts of `Saved/` are generated by Unreal and may reappear after builds or editor sessions.
- `Source/T66/Core/T66RunIntegritySubsystem.cpp` inspects `ProjectDir/Binaries` for local executable/module file stamps. That means the folder is relevant to runtime integrity reporting when running from the project directory, but it does not make the folder source-controlled or manually maintained.
- Current active staged standalone path is `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- A few active UI workflow docs still reference the root development exe:
  - `Docs/UI/UI_GENERATION.md`
  - `UI/SCREEN_WORKFLOW.md`
  - `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md`
  - `UI/Reference/SCREEN_MODAL_TASK.md`
- Many additional references are historical UI proof manifests under `UI/Reference/**` and old audit/docs archives. Those should not block deleting generated binaries.

Recommendation:
- Do not manually delete the root `Binaries` folder during this inventory cleanup. It is generated, ignored, and will come back after local Unreal builds anyway.
- Keep `.gitignore` as-is so Unreal can regenerate this folder locally without polluting git status.
- Prefer the staged standalone executable for taskbar and screenshot workflows.

Tweaks required:
- Update active UI workflow docs that point at `C:\UE\T66\Binaries\Win64\T66.exe` so they use `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` or the default in `Scripts/CaptureT66UIScreen.ps1`.
- No extra Steam app id preservation is required: root `steam_appid.txt`, `Config/DefaultEngine.ini`, `MASTER DOCS/MASTER_STEAMWORKS.md`, and `Source/T66/T66.Build.cs` already make AppID `4464300` durable. The ignored copy under `Binaries/Win64/steam_appid.txt` can be treated as generated output.

Good-to-delete candidates:
- None for the active cleanup pass. The folder is generated and ignored, so leaving it in place avoids a rebuild without adding source-control clutter.

Risk:
- Low for source control and packaged/taskbar gameplay because the folder is ignored and not the taskbar target.
- Low for local development convenience because the current generated build artifacts remain in place.
- Low for Steam local testing because AppID `4464300` is already preserved outside this generated folder.

Decision:
- Accepted cleanup: update active UI capture docs away from the root development exe.
- Accepted keep: do not manually delete `Binaries`; treat it as ignored generated output that may be regenerated by Unreal.
- Cleanup completed:
  - Updated `Docs/UI/UI_GENERATION.md`.
  - Updated `UI/SCREEN_WORKFLOW.md`.
  - Updated `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md`.
  - Updated `UI/Reference/SCREEN_MODAL_TASK.md`.

## 08. `Build`

Path: `C:\UE\T66\Build`

Inventory:
- Git status: ignored by `.gitignore` via `Build/`.
- Git tracked files: none.
- Direct subfolders:
  - `Windows`
- Recursive contents: 2 directories, 2 files, about 1.90 MB.
- Files:
  - `Build/Windows/FileOpenOrder/CookerOpenOrder.log`: about 642 KB.
  - `Build/Windows/FileOpenOrder/EditorOpenOrder.log`: about 1.18 MB.

General purpose:
- Unreal-generated build/cook metadata.
- The current files are file-open-order logs generated by editor and cooker activity.
- These logs are not source, content, config, packaging scripts, or project documentation.

Reference evidence:
- `.gitignore` explicitly ignores `Build/`.
- No files under `Build` are tracked by git.
- No live repo references were found for `Build/Windows`, `FileOpenOrder`, `CookerOpenOrder`, or `EditorOpenOrder` outside the folder itself.

Recommendation:
- Do not manually delete `Build` during this inventory cleanup.
- Treat it the same way as `Binaries`: ignored generated Unreal output that can reappear after editor/cook/build activity.

Tweaks required:
- None.

Good-to-delete candidates:
- None for the active cleanup pass. The folder is small, ignored, and generated.
- If we later want a physically clean working tree, `Build/Windows/FileOpenOrder` can be deleted with the editor closed, but it is not worth prioritizing because Unreal can regenerate it.

Risk:
- Low if left alone.
- Low if deleted manually later, but it may remove cook/editor ordering hints and reappear after future Unreal activity.

Decision:
- Accepted keep: do not manually delete `Build`; treat it as ignored generated output.

## 09. `CodexSkills`

Path: `C:\UE\T66\CodexSkills`

Inventory:
- Git status: untracked empty directory. Git does not track empty folders, so `git status` shows no files.
- Git ignore status: not ignored.
- Direct subfolders: none.
- Recursive contents: 0 directories, 0 files, 0 bytes.

General purpose:
- No active purpose found.
- The folder name suggests a possible old placeholder for repo-local Codex skills, but it currently contains nothing.
- Current project agent guidance now lives in root `AGENTS.md`, not this folder.

Reference evidence:
- No references found for `CodexSkills`, `Codex Skills`, `codex skill`, or `codexskills` outside the folder.
- No source, docs, config, tooling, or build file depends on it.

Recommendation:
- Delete the empty `CodexSkills` folder during cleanup.
- Do not add it to `.gitignore` unless a future workflow intentionally regenerates it.

Tweaks required:
- None.

Good-to-delete candidates:
- Delete the empty root `CodexSkills` folder.

Risk:
- None for runtime/build behavior.
- Very low for workflow behavior because the folder is empty and unreferenced.

Decision:
- Accepted delete: empty root `CodexSkills` folder.

## 10. `Config`

Path: `C:\UE\T66\Config`

Inventory:
- Git status: mixed tracked, modified, and untracked active config work.
  - Modified tracked files: `DefaultGame.ini`, `DefaultT66PlayerExperience.ini`, `DefaultT66Rng.ini`, `DefaultT66StageProgression.ini`, `DefaultT66TrapTuning.ini`.
  - Untracked files: `DefaultDeviceProfiles.ini`, `DefaultScalability.ini`.
  - Tracked unchanged files: `DefaultEditor.ini`, `DefaultEngine.ini`, `DefaultInput.ini`, and the four `Localization/*.ini` files.
- Direct subfolders:
  - `Layouts`
  - `Localization`
- Recursive contents: 2 directories, 14 files, about 106.71 KB.
- Root files:
  - `DefaultDeviceProfiles.ini`
  - `DefaultEditor.ini`
  - `DefaultEngine.ini`
  - `DefaultGame.ini`
  - `DefaultInput.ini`
  - `DefaultScalability.ini`
  - `DefaultT66PlayerExperience.ini`
  - `DefaultT66Rng.ini`
  - `DefaultT66StageProgression.ini`
  - `DefaultT66TrapTuning.ini`
- `Config/Layouts` is empty.
- `Config/Localization` contains:
  - `T66_Compile.ini`
  - `T66_Gather.ini`
  - `T66_Gather_Assets.ini`
  - `T66_Gather_Source.ini`

General purpose:
- Required Unreal project configuration root.
- Contains runtime maps, rendering, Steam AppID, online subsystem, input, packaging/cook settings, localization gather/compile settings, platform/scalability tuning, and T66-specific authored tuning.
- This folder cannot be moved out of the project root without breaking standard Unreal behavior.

Reference evidence:
- `MASTER DOCS/T66_MASTER_GUIDELINES.md` lists `Config/` as a required stable root item.
- `DefaultEngine.ini` owns core project maps, renderer settings, UI DPI policy, Python plugin settings, Steam AppID `4464300`, and online subsystem setup.
- `DefaultGame.ini` owns packaging/cook coverage, loose runtime content root declarations, backend dev settings, and explicit staged config files.
- `DefaultInput.ini` owns project input mappings.
- `DefaultDeviceProfiles.ini` and `DefaultScalability.ini` are untracked current performance/render-quality config files; they should be treated as active project work, not cleanup trash.
- `DefaultT66Rng.ini`, `DefaultT66StageProgression.ini`, and `DefaultT66TrapTuning.ini` are loaded by runtime C++ config classes and are explicitly staged by `DefaultGame.ini`.
- `Config/Localization/*.ini` files are Unreal localization commandlet configs for T66.
- `DefaultT66PlayerExperience.ini` is now suspicious: `UT66PlayerExperienceSubSystem` currently loads `/Game/Data/DT_PlayerExperience`, generated from `Content/Data/PlayerExperience.json`, but this config file is still documented, staged, and included in validation placeholder checks.
- `DefaultEditor.ini` only appears to contain `AdvancedPreviewScene.SharedProfiles` entries for editor asset-preview profiles: `Epic Headquarters`, `Grey Wireframe`, and `Grey Ambient`.

Recommendation:
- Keep the root `Config` folder.
- Keep all live runtime, packaging, input, localization, Steam, scalability, device-profile, trap, RNG, and stage-progression configs.
- Delete only clearly empty root clutter now: `Config/Layouts`.
- Consider deleting or consolidating `DefaultEditor.ini` if we agree that shared editor asset-preview profiles are not worth keeping in source control.
- Consider deleting `DefaultT66PlayerExperience.ini` only after paired source/docs/script cleanup confirms `Content/Data/PlayerExperience.json` and `/Game/Data/DT_PlayerExperience` are the sole player-experience tuning source.

Tweaks required before deletion/consolidation:
- For `Config/Layouts`: none.
- For `DefaultEditor.ini`: decide whether losing the shared asset-preview profiles is acceptable. No runtime tweak appears required.
- For `DefaultT66PlayerExperience.ini`:
  - Remove `T66/Config/DefaultT66PlayerExperience.ini` from `[Staging] +AllowedConfigFiles` in `Config/DefaultGame.ini`.
  - Update `Scripts/ValidateEnemyBossRosterData.py` so live placeholder validation no longer reads the deleted config.
  - Update docs that still describe `DefaultT66PlayerExperience.ini` as the tuning authority, especially `MASTER DOCS/MASTER_PLAYER_EXPERIENCE.md`.
  - Update the comment in `Config/DefaultT66Rng.ini` that still names `Config/DefaultT66PlayerExperience.ini`.
  - Verify `Content/Data/PlayerExperience.json` and `/Game/Data/DT_PlayerExperience` cover the full current tuning surface before deletion.

Good-to-delete candidates:
- Delete empty `Config/Layouts`.
- Optional delete: `Config/DefaultEditor.ini`, if editor asset-preview profiles are not needed as shared source-controlled project defaults.
- Optional delete after paired cleanup: `Config/DefaultT66PlayerExperience.ini`, if the DataTable/JSON player-experience pipeline is confirmed as the sole source of truth.

Risk:
- High if deleting the `Config` folder or core files raw.
- Low for deleting empty `Config/Layouts`.
- Low runtime risk but medium editor-workflow risk for deleting `DefaultEditor.ini`.
- Medium gameplay/docs risk for deleting `DefaultT66PlayerExperience.ini` before removing its staging/docs/validation hooks and verifying the DataTable pipeline.

Decision:
- Accepted keep: root `Config` folder and all active runtime/project config.
- Accepted delete: empty `Config/Layouts`.
- Accepted follow-up review: `Config/DefaultEditor.ini` after confirming editor preview profiles are disposable.
- Accepted follow-up review with paired cleanup: `Config/DefaultT66PlayerExperience.ini`.

## 11. `Content`

Path: `C:\UE\T66\Content`

Inventory:
- Git status: heavily mixed tracked, modified, deleted, and untracked active asset work.
- Git tracked files: about 3007.
- Recursive contents: 3238 files, about 4586.33 MB / 4.27 GB.
- File types/counts:
  - 2980 `.uasset`
  - 105 `.png`
  - 55 `.csv`
  - 22 `.archive`
  - 22 `.locres`
  - 15 `.umap`
  - 9 `.fbx`
  - 9 `.md`
  - 6 `.wav`
  - 5 `.ogg`
  - 4 `.json`
  - 1 `.mp4`
  - 1 `.jpg`
  - 1 `.txt`
  - 1 `.gitkeep`
  - 1 `.locmeta`
  - 1 `.manifest`
- Direct subfolders:
  - `__Debug`: 4 current files, about 9.33 MB.
  - `__ExternalActors__`: 41 files, about 0.16 MB.
  - `__ExternalObjects__`: 6 files, about 0.01 MB.
  - `Audio`: 154 files, about 94.31 MB.
  - `Blueprints`: 13 files, about 0.23 MB.
  - `Characters`: 803 files, about 2819.77 MB.
  - `Collections`: empty.
  - `Data`: 42 files, about 1.01 MB.
  - `Deck`: 12 files, about 0.00 MB.
  - `Developers`: empty nested `DoPra/Collections` folder.
  - `Idle`: 10 files, about 0.01 MB.
  - `Idols`: 64 files, about 3.57 MB.
  - `Items`: 120 files, about 4.01 MB.
  - `Localization`: 46 files, about 6.60 MB.
  - `Maps`: 5 files, about 9.14 MB.
  - `Materials`: 20 files, about 0.23 MB.
  - `Mini`: 49 files, about 12.26 MB.
  - `Movies`: 1 file, about 19.74 MB.
  - `Slate`: 8 files, about 0.09 MB.
  - `SourceAssets`: 87 files, about 34.25 MB.
  - `Stylized_VFX_StPack`: 591 files, about 471.94 MB.
  - `T66MapAssets`: 9 current files, about 0.13 MB.
  - `TD`: 11 files, about 0.07 MB.
  - `UE5RFX`: 289 files, about 101.58 MB.
  - `UI`: 258 files, about 178.23 MB.
  - `ULTS`: 1 file, about 1.63 MB.
  - `VFX`: 15 files, about 0.88 MB.
  - `Weapons`: 308 files, about 88.04 MB.
  - `World`: 271 current files, about 516.63 MB.

General purpose:
- Required Unreal runtime/editor asset root.
- Contains all cooked project assets: maps, data tables, localization, characters, world assets, UI textures, VFX packs, audio, minigame content, and external actor/object data.
- This folder cannot be moved or broadly deleted. Cleanup must happen in targeted sub-passes using Unreal asset-registry referencers plus text/data references.

Reference evidence:
- `Config/DefaultGame.ini` always-cooks many `/Game/...` directories, including `/Game/Audio`, `/Game/Characters`, `/Game/Data`, `/Game/Maps`, `/Game/Mini`, `/Game/UI`, `/Game/VFX`, `/Game/World`, and `/Game/SourceAssets`.
- Prior cleanup memory for this repo specifically warns that filename matching is not enough for `Content/World`; use Unreal asset-registry referencers plus source/data text search.
- `Content/__Debug` is only text-referenced by `Scripts/DebugGLBRawImport.py`, which writes scratch imports under `/Game/__Debug/GLBRaw`.
- `Content/Collections` and `Content/Developers/DoPra/Collections` are empty; current logs only show Unreal scanning them.
- `Content/Movies/ArthurPreview.mp4` has no exact live text reference found. Current code references `KnightClip.mp4`, not `ArthurPreview.mp4`.
- `Content/SourceAssets/UI/DeletedTheme/Generated` is still text-referenced by `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp` as a runtime PNG fallback directory, so the parent folder is live.
- `Content/SourceAssets/UI/DeletedTheme/Generated/debug_downscaled` has no live text references found and appears to contain debug/small/test variants.
- `Content/T66MapAssets` has existing tree asset deletions in the working tree and a README updated on 2026-05-07. It still contains landscape and rock assets, and old captured logs show maps previously loading `T66MapAssets` rocks/trees. Treat this as its own asset-registry sub-pass, not a raw delete.

Recommendation:
- Keep root `Content`.
- Do not make broad deletes inside `Content` during the top-level pass.
- Mark only clearly disposable root-level cleanup now:
  - delete empty `Content/Collections`;
  - delete empty `Content/Developers`;
  - delete `Content/__Debug` after confirming no current debug import work is in progress;
  - delete `Content/Movies/ArthurPreview.mp4` unless the user wants to keep old Arthur video preview media.
- Mark `Content/SourceAssets/UI/DeletedTheme/Generated/debug_downscaled` as a delete candidate after a small asset-registry check confirms no cooked assets reference the debug variants.
- Keep `Content/SourceAssets/UI/DeletedTheme/Generated` parent for now because runtime code still uses it.
- Split the rest of `Content` into sub-passes later: `Characters`, `World`, `UI`, `SourceAssets`, VFX packs, maps/external actors, and minigame folders.

Tweaks required before deletion/consolidation:
- For `Content/Collections`: none.
- For `Content/Developers`: none.
- For `Content/__Debug`: keep `Scripts/DebugGLBRawImport.py` if the debug import workflow is still useful; the script can recreate scratch imports. Delete only the generated `/Game/__Debug/GLBRaw` assets.
- For `Content/Movies/ArthurPreview.mp4`: none found by text search; if deleting, verify no MediaPlayer/MediaSource asset references it in the Unreal asset registry.
- For `Content/SourceAssets/UI/DeletedTheme/Generated/debug_downscaled`: run a small Unreal asset-registry referencer check before deleting the 28 debug/small/test files.
- For any `Content/World`, `Characters`, `UI`, `T66MapAssets`, or pack-level deletion: run Unreal asset-registry referencer audit and source/config/data text search first.

Good-to-delete candidates:
- Delete empty `Content/Collections`.
- Delete empty `Content/Developers/DoPra/Collections`, then empty `Content/Developers/DoPra`, then empty `Content/Developers`.
- Delete `Content/__Debug/GLBRaw` and the root `Content/__Debug` folder after confirming no debug import pass is active.
- Delete `Content/Movies/ArthurPreview.mp4` after confirming no media asset uses it.
- Delete `Content/SourceAssets/UI/DeletedTheme/Generated/debug_downscaled` after asset-registry confirmation.

Risk:
- High for broad raw deletion anywhere in `Content`.
- Low for empty `Collections` and `Developers`.
- Low-to-medium for `__Debug`: runtime risk appears low, but it removes import-debug evidence.
- Low-to-medium for `ArthurPreview.mp4`: likely obsolete preview media, but confirm no MediaSource asset points to it.
- Medium for generated UI/debug-downscaled assets until Unreal referencers are checked.
- High for `Characters`, `World`, `UI`, VFX packs, maps, and external actor/object folders without asset-registry evidence.

Decision:
- Proposed keep: root `Content` folder and live asset subfolders.
- Proposed delete now: empty `Content/Collections` and `Content/Developers`.
- Proposed delete after quick confirmation: `Content/__Debug` and `Content/Movies/ArthurPreview.mp4`.
- Proposed delete after asset-registry check: `Content/SourceAssets/UI/DeletedTheme/Generated/debug_downscaled`.
- Proposed follow-up: handle the rest of `Content` as dedicated subfolder-level cleanup passes, not as one top-level delete.

Implementation update:
- Superseded by the Alpha 0.1 implementation notes below. The later cleanup pass removed the DeletedTheme UI fallback path, audited the full old UI target set, and deleted all of `Content/SourceAssets/UI`, not just `debug_downscaled`.

### Deeper `Content` follow-up: stale maps, external actors, characters, props, and UI

Additional user-raised cleanup targets:
- `Content/__ExternalActors__`
- `Content/__ExternalObjects__`
- stale hero folders beyond the reduced 12-hero roster
- stale enemy folders that no longer match the current enemy/boss roster
- `Content/Data/Props.csv` and `Content/Data/DT_Props.uasset`
- `Content/Maps/Gameplay_Coliseum.umap`
- `Content/SourceAssets/UI`
- `Content/T66MapAssets`
- old `Content/UI` DeletedTheme/retro wood assets

External actor/object evidence:
- `git ls-files Content/__ExternalActors__ Content/__ExternalObjects__` shows both folders are only under `ThirdPerson/Maps/ThirdPersonMap`.
- Text search found no live source/config/data reference to `ThirdPersonMap`, `__ExternalActors__`, or `__ExternalObjects__` outside this inventory and the project catalogue.
- Current `Content/Maps` does not include `ThirdPersonMap.umap`.

External actor/object recommendation:
- Mark `Content/__ExternalActors__` and `Content/__ExternalObjects__` for deletion.
- These do not appear to belong to current T66 maps. If they belonged to a current map, the right cleanup would be to open the owning map in Unreal, convert/disable one-file-per-actor or resave the map so Unreal owns the references. Here the owning map is missing/obsolete, so raw deletion is appropriate after a final asset-registry check.

Map evidence:
- `Content/Maps/Gameplay_Coliseum.umap` exists, but exact text search found no live map load/cook reference.
- `Config/DefaultGame.ini` cooks the active map folders broadly but does not point specifically at `Gameplay_Coliseum`.
- Remaining `Coliseum` text hits are generic feature comments/guards in source and old catalogue prose, not the map package itself.

Map recommendation:
- Mark `Content/Maps/Gameplay_Coliseum.umap` for deletion.
- Later gameplay cleanup can remove or rename old `Coliseum` wording in code/docs if the whole feature is gone, but that is separate from deleting the obsolete map package.

Hero evidence:
- `Content/Data/Heroes.csv` has 12 active hero rows: `Hero_1` through `Hero_12`.
- `Content/Characters/Heroes` still has `Hero_13`, `Hero_14`, and `Hero_15`.
- `Content/Data/CharacterVisuals.csv` still references:
  - `Hero_10_Chad_Beachgoer` -> `/Game/Characters/Heroes/Hero_13/...`
  - `Hero_11_Chad_Beachgoer` -> `/Game/Characters/Heroes/Hero_14/...`
  - `Hero_12_Chad_Beachgoer` -> `/Game/Characters/Heroes/Hero_15/...`
- `Content/Data/Heroes.csv` also still uses portrait paths under `/Game/UI/Sprites/Heroes/Hero_13`, `/Game/UI/Sprites/Heroes/Hero_14`, and `/Game/UI/Sprites/Heroes/Hero_15` for active `Hero_10`, `Hero_11`, and `Hero_12`.
- `Knight` is separate and should stay for now. Current source references `KnightClip.mp4`, and the user explicitly expects 12 heroes plus Knight.

Hero recommendation:
- Do not delete `Hero_13` through `Hero_15` raw yet because active data still points at them.
- Mark for delete after data redirect:
  - redirect `CharacterVisuals.csv` beachgoer rows for `Hero_10` through `Hero_12` to active `Hero_10`, `Hero_11`, and `Hero_12` assets, or delete those beachgoer rows if the variants are no longer used;
  - regenerate/reload `DT_CharacterVisuals`;
  - redirect `Heroes.csv` portrait references for active `Hero_10` through `Hero_12` away from `/Game/UI/Sprites/Heroes/Hero_13..15`, or rename/import the portraits into `Hero_10..12`;
  - regenerate/reload `DT_Heroes`;
  - then delete `Content/Characters/Heroes/Hero_13`, `Hero_14`, `Hero_15`, and stale UI portrait folders `Content/UI/Sprites/Heroes/Hero_13`, `Hero_14`, `Hero_15`.

Arthur/Hero_1 legacy evidence:
- `CharacterVisuals.csv` now uses `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro` for the active `Hero_1_Chad` visual.
- `Content/Characters/Heroes/Hero_1/Chad/Idle` and `Content/Characters/Heroes/Hero_1/Chad/Walk` still contain old `ArthurIdle`, `ArthurAIdle`, `ArthurWalk`, and `ArthurAWalk` skeletal assets.
- Current text references to those old skeletal assets are import/export/inspection/verification scripts, not the live `CharacterVisuals.csv` active Hero_1 visual row.
- `Content/Movies/ArthurPreview.mp4` is already marked for deletion after media-reference confirmation.
- `Content/VFX/Projectiles/Hero1/Arthur_Sword.uasset` is still live through `T66ArthurSwordVisuals`, Hero 1 attack/ultimate VFX, and wall-arrow fallback code.
- `Content/Mini/Sprites/Heroes/Arthur.uasset` is still potentially live in Mini visual code/docs.

Arthur/Hero_1 legacy recommendation:
- Mark the old Hero_1 skeletal Arthur import folders for deletion after scripts are retired:
  - `Content/Characters/Heroes/Hero_1/Chad/Idle`
  - `Content/Characters/Heroes/Hero_1/Chad/Walk`
- Required tweak:
  - retire or update `Scripts/ImportSkeletalMeshes.py`, `Scripts/VerifyImportBatch.py`, `Scripts/ExportArthurRuntimeMeshForBlender.py`, `Scripts/ExportArthurRuntimeMeshForBlenderGLTF.py`, and inspection scripts that still require `ArthurAIdle` / `ArthurAWalk`;
  - keep `Content/Characters/Heroes/Hero_1/Chad/QuadRetro` and `Content/Characters/Heroes/Hero_1/Stacy/QuadRetro`;
  - do not delete `Content/VFX/Projectiles/Hero1/Arthur_Sword` until Hero 1 sword VFX is renamed/replaced;
  - do not delete `Content/Mini/Sprites/Heroes/Arthur.uasset` until Mini hero visual naming is updated.

Enemy evidence:
- `Content/Data/Enemies.csv` and `Content/Data/Bosses.csv` define the current roster around stage/theme IDs such as `Dungeon_Slime`, `Forest_Boar`, `Hell_Gargoyle`, and named boss IDs.
- `Content/Characters/Enemies` has both current-looking folders (`Regular`, `Bosses`) and old folders (`Boss`, `Cow`, `Enemy1`, `Enemy2`, `Enemy3`, `Goat`, `GoblinThief`, `Pig`, `Roost`).
- `Content/Data/CharacterVisuals.csv` still references old rows/assets:
  - `Boss` -> `/Game/Characters/Enemies/Boss/SK_Boss.SK_Boss`
  - `GoblinThief_Black`
  - `GoblinThief_Red`
  - `GoblinThief_Yellow`
  - `GoblinThief_White`
- Current themed enemy/boss rows use the newer `Regular` and `Bosses` static mesh structure.
- `Cow`, `Pig`, `Goat`, and `Roost` are referenced in Mini tooling/docs, but that appears to be minigame sprite preparation context, not proof that the old `Content/Characters/Enemies/*` 3D folders are live in the main game.

Enemy recommendation:
- Keep `Content/Characters/Enemies/Regular` and `Content/Characters/Enemies/Bosses`.
- Mark for delete after data cleanup:
  - `Content/Characters/Enemies/Boss`
  - `Content/Characters/Enemies/GoblinThief`
  - `Content/Characters/Enemies/Cow`
  - `Content/Characters/Enemies/Enemy1`
  - `Content/Characters/Enemies/Enemy2`
  - `Content/Characters/Enemies/Enemy3`
  - `Content/Characters/Enemies/Goat`
  - `Content/Characters/Enemies/Pig`
  - `Content/Characters/Enemies/Roost`
- Required tweak: remove or redirect old `Boss` and `GoblinThief_*` rows from `CharacterVisuals.csv`, reload `DT_CharacterVisuals`, then run an Unreal asset-registry referencer pass before deleting the folders. If Mini still needs farm animals, it should use `Content/Mini` sprites or explicit Mini assets, not stale main-game enemy model folders.

Props evidence:
- `Content/Data/Props.csv` and `Content/Data/DT_Props.uasset` still exist.
- Source still contains `UT66PropSubsystem`, and `T66GameMode_MainMap.cpp` still calls `SpawnPropsForStage`.
- `UT66PropSubsystem.cpp` still references `/Game/Data/DT_Props.DT_Props`.
- Scripts and docs still reference `Props.csv`, `DT_Props`, and `SetupPropsDataTable.py`.
- Many old `Content/World/Props` assets are already deleted in the working tree.

Props recommendation:
- Mark `Content/Data/Props.csv` and `Content/Data/DT_Props.uasset` for deletion, but only after code cleanup.
- Required tweak:
  - remove or disable prop spawning from `T66GameMode_MainMap.cpp`;
  - remove or retire `UT66PropSubsystem` if props are fully gone;
  - remove stale `FT66PropRow` only if no code remains dependent on it;
  - delete or archive `Scripts/SetupPropsDataTable.py`;
  - update validation scripts that currently expect `DT_Props` to exist with zero rows;
  - update `MASTER DOCS` imports/catalogue references.

`T66MapAssets` evidence:
- `Content/T66MapAssets` now only contains a small `Landscape` folder, a `Rocks` folder, and `README.txt`.
- Exact source/config search found no live source reference to `/Game/T66MapAssets` or `T66MapAssets`.
- Old UI proof logs show previous runtime loads from `/Game/T66MapAssets/Rocks` and `/Game/T66MapAssets/Trees`, but those are historical logs. The tree assets are already deleted in the working tree.

`T66MapAssets` recommendation:
- Mark all of `Content/T66MapAssets` for deletion.
- Required tweak: update stale project catalogue/docs. If an editor-only procedural hills/map-assets setup tool still exists, retire it or point it at the current `Content/World/Terrain` kit before deleting this folder.

`Content/SourceAssets/UI` evidence:
- This folder is entirely DeletedTheme-generated loose fallback art plus an empty `MainMenu` folder.
- Current live runtime UI code now loads most shared chrome from `SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements`.
- However `T66RuntimeUIBrushAccess.cpp` still has `GetDeletedThemeGeneratedSourceDir()` pointing at `Content/SourceAssets/UI/DeletedTheme/Generated`, and `ResolveDeletedThemeButtonPlateBrush()` still tries cooked `T_UI_DeletedTheme*` assets and DeletedTheme loose PNG fallbacks.
- `Scripts/GenerateChestRewardAssets.py` still points at `SourceAssets/UI/DeletedTheme/Generated/frontend_topbar_achievement_coins_icon.png`.

`Content/SourceAssets/UI` recommendation:
- Mark all of `Content/SourceAssets/UI` for deletion after UI brush cleanup.
- Required tweak:
  - remove the DeletedTheme fallback directory and DeletedTheme plate helper paths from `T66RuntimeUIBrushAccess`;
  - route any remaining no-override button-plate calls to the Ultrakill/reference element paths or simple color fallback;
  - update/remove `Scripts/GenerateChestRewardAssets.py` if it still needs an icon source;
  - confirm `Config/DefaultGame.ini` does not need to cook `/Game/SourceAssets` after other `Content/SourceAssets` users are reviewed.

`Content/UI` evidence:
- `Content/UI` contains 258 files, about 178.23 MB.
- `Content/UI/Assets` contains cooked DeletedTheme plates, topbar icons, medals, `ButtonLight_*`, `PanelLight`, and retro wood trim textures.
- `Content/UI/Materials` contains `M_UI_Glow` plus old `M_UI_RetroSkyBorder*`, `M_UI_RetroWoodTrim*`, and their material instances.
- `T66ButtonVisuals.cpp` directly loads the old retro sky/wood assets, but `FT66Style::IsDeletedThemeTheme()` currently always returns true, so default shared buttons/panels do not route through retro wood unless call sites explicitly request that visual.
- `T66RuntimeUIBrushAccess.cpp` directly loads cooked DeletedTheme plate assets under `/Game/UI/Assets/T_UI_DeletedTheme*`.
- Current runtime code directly uses some `Content/UI` assets:
  - `Content/UI/M_PixelationPostProcess.uasset` via `UT66PixelationSubsystem`;
  - `Content/UI/Preview/*` via preview material code;
  - `Content/UI/Leaderboard/T_LB_*` via leaderboard code and preload code;
  - `Content/UI/Sprites/Heroes/*` via `Heroes.csv`, hero selection, and leaderboard portrait logic;
  - `Content/UI/Sprites/Companions/*` via `Companions.csv`;
  - `Content/UI/Sprites/UI/Hearts/*`, `SKULL`, `CLOWN`, ability icons, quick-revive icon, gambler icons/cards/RPS/coin assets via HUD/gambler UI code.
- `Content/UI/MainMenu` old assets are preloaded by `T66GameInstance.cpp`, but current main-menu chrome/background path is largely loose `SourceAssets/UI/Reference/...`. These should be reviewed as likely obsolete after confirming nothing still binds them in UI screens.
- Existing working tree already deletes old vendor UI sprites under `Content/UI/Sprites/NPCs/Vendor`.

`Content/UI` recommendation:
- Keep for now:
  - `Content/UI/M_PixelationPostProcess.uasset`
  - `Content/UI/Preview`
  - `Content/UI/Leaderboard`
  - `Content/UI/Sprites/Abilities`
  - `Content/UI/Sprites/Companions`
  - `Content/UI/Sprites/Games`
  - `Content/UI/Sprites/Interactables`
  - `Content/UI/Sprites/NPCs/Gambler`
  - `Content/UI/Sprites/PowerUp`
  - `Content/UI/Sprites/UI`
  - active hero portrait folders needed by `Heroes.csv`
- Mark for delete after UI style cleanup:
  - `Content/UI/Assets/T_UI_DeletedThemeDangerButtonPlate.uasset`
  - `Content/UI/Assets/T_UI_DeletedThemeInventorySlotFrame.uasset`
  - `Content/UI/Assets/T_UI_DeletedThemeNeutralButtonPlate.uasset`
  - `Content/UI/Assets/T_UI_DeletedThemePrimaryButtonPlate.uasset`
  - `Content/UI/Assets/T_UI_RetroWoodTrim*_V2_*.uasset`
  - old `Content/UI/Materials/M_UI_Retro*` and `MI_UI_Retro*`
  - `Content/UI/Sprites/Heroes/Hero_13`, `Hero_14`, `Hero_15` after active hero portrait paths are redirected
- Mark likely delete after source/preload cleanup:
  - `Content/UI/MainMenu`
  - `Content/UI/PartyPicker`
  - `Content/UI/Assets/TopBar`
  - `Content/UI/Assets/ButtonLight_*`
  - `Content/UI/Assets/PanelLight.uasset`
  - `Content/UI/Obsidian.uasset`
- Required tweak:
  - rename/generalize the DeletedTheme-named UI APIs to neutral T66/Ultrakill naming or keep compatibility wrappers that no longer load DeletedTheme assets;
  - remove DeletedTheme cooked asset paths from `T66RuntimeUIBrushAccess`;
  - remove or no-op retro wood/retro sky branches in `T66ButtonVisuals` and `ET66ButtonBorderVisual` if the old visual families are fully retired;
  - remove old main menu/preload soft paths from `T66GameInstance.cpp` after confirming the current main menu uses `SourceAssets/UI/Reference` assets.

Accepted Content cleanup direction:
- Accepted later data/code cleanup: redirect active data so `Content/Characters/Heroes/Hero_13`, `Hero_14`, `Hero_15` and matching UI portrait folders can be deleted.
- Accepted later Arthur cleanup: retire old Hero_1 Arthur skeletal import folders and `ArthurPreview` media after script/reference cleanup; keep live sword VFX until renamed/replaced.
- Accepted later props cleanup: remove `UT66PropSubsystem` and its main-map call sites so `Props.csv`, `DT_Props`, and remaining old prop validation/import scripts can be deleted.
- Accepted later UI cleanup: delete DeletedTheme, retro wood, retro sky, old main-menu, old topbar, and stale hero portrait assets from `Content/UI` once the style/preload code no longer points at them.

Implementation update:
- Superseded by the Alpha 0.1 implementation notes below. Hero 13-15 data ownership, props removal, stale enemies, `T66MapAssets`, old DeletedTheme/retro/main-menu/topbar UI, and the accepted script retirements were implemented and verified in later passes.

## 12. `DerivedDataCache`

Path: `C:\UE\T66\DerivedDataCache`

Inventory:
- Git status: no tracked files and no visible pending git changes.
- Git ignore status: ignored by `.gitignore` rule `DerivedDataCache/`.
- Direct subfolders: `VT`.
- Recursive contents: 1 folder, 5 files, about 1.55 MB.
- Files are virtual-texture derived cache chunks named `TEXTURE_*_VTCHUNK*`.

General purpose:
- Unreal-generated derived data cache. It stores generated/cached data for faster editor and asset loading.
- It is not source content and can be regenerated by Unreal.

Reference evidence:
- `MASTER DOCS/T66_MASTER_GUIDELINES.md` already says `DerivedDataCache/` is generated by Unreal and may reappear after builds or editor sessions.
- `.gitignore` already excludes `DerivedDataCache/`.
- Search found only generic Unreal cache logs and virtual-texture config settings, not live source files that require these cache chunks to be preserved.

Recommendation:
- Delete the whole `DerivedDataCache` folder during cleanup.

Tweaks required before deletion/consolidation:
- None.
- Keep the `.gitignore` rule as-is.

Good-to-delete candidates:
- `DerivedDataCache/VT`
- all files under `DerivedDataCache/VT`
- root `DerivedDataCache` folder if empty after deletion

Risk:
- Low. Unreal will regenerate derived data as needed.
- Expected downside is only a one-time editor/build recache cost.

Decision:
- Accepted delete: entire `DerivedDataCache` folder.

## 13. `Docs`

Path: `C:\UE\T66\Docs`

Inventory:
- Direct subfolders: `Archive`, `Deck`, `Idle`, `Implementation Plan`, `Mini`, `Minigames`, `Plans`, `Systems`, `TD`, `UI`.
- Direct files: `README.md`.
- Recursive contents: 12 folders, 33 files, about 24.58 MB.
- File types: mostly Markdown, plus 2 JSON files, 1 CSV, 1 PNG, and 1 BIN.
- Size note: almost all folder weight is `Docs/Archive/Root_Quarantine/waveletNoiseTile.bin`.

General purpose:
- Documentation workspace for implementation notes, system references, UI workflow docs, minigame docs, plans, and historical material.
- This folder is documentation-only except for archived root leftovers under `Docs/Archive/Root_Quarantine`.

Reference evidence:
- `Docs/README.md` says docs are organized by function and points active docs toward `MASTER DOCS`, `Systems`, `Minigames`, `Plans`, and `Implementation Plan`.
- `Docs/README.md` still mentions `Archive` and a nonexistent `Docs/Art` folder.
- `MASTER DOCS/T66_DECISION_LOG.md`, `MASTER DOCS/MASTER_BACKEND.md`, `MASTER DOCS/MASTER_LIGHTING.md`, and `Audit/Finished/T66_DOCS_CLEANUP_LEDGER.md` reference archived docs or quarantined root leftovers.

Keep for now:
- Nothing under `Docs` is required to stay in place if the cleanup goal is to keep active instructions in `AGENTS.md`, `MASTER DOCS`, `UI`, `Audit`, or the relevant feature folders instead.

Good-to-delete candidates:
- Delete the entire `Docs` folder.
- This includes active-looking but non-runtime docs under `Docs/Deck`, `Docs/Idle`, `Docs/Mini`, `Docs/Minigames`, `Docs/Systems`, `Docs/TD`, and `Docs/UI`.
- This also removes `Docs/Archive`, including historical backend/lighting/checklist docs and quarantined root leftovers (`Items.csv`, `T66.png`, `waveletNoiseTile.bin`).

Optional salvage before deletion:
- If any current process doc is still wanted, move only that content into `MASTER DOCS`, `UI`, `Model Generation`, or the specific feature folder before deleting `Docs`.
- The most likely salvage candidates are `Docs/UI/UI_GENERATION.md`, `Docs/Minigames/T66Minigame_CharacterAnimationProcess.md`, and `Docs/Systems/EnemyBossRoster_DataContract_2026-05-07.md`; delete them too if the goal is to rely on Git history rather than local process archives.

Tweaks required before deletion/consolidation:
- Update `MASTER DOCS/T66_MASTER_GUIDELINES.md` so it no longer lists `Docs/` as an active documentation home or points to `Docs/UI/UI_GENERATION.md`.
- Update `MASTER DOCS/T66_DECISION_LOG.md`, `MASTER DOCS/MASTER_BACKEND.md`, `MASTER DOCS/MASTER_LIGHTING.md`, and `MASTER DOCS/MASTER_TRAPS.md` to remove `Docs` references.
- Update UI workflow references that point at `C:\UE\T66\Docs\UI\UI_GENERATION.md`, especially `UI/README.md`, `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md`, `UI/Reference/SCREEN_MODAL_TASK.md`, and active UI reference manifests/prompts.
- Update `Model Generation/README.md` and `Model Generation/QUAD_RETRO_DO_THIS_RUNBOOK.md` if those runbooks should survive without `Docs`.
- Update audit cleanup references that describe `Docs` as an active or archived destination, including this inventory file where needed.

Risk:
- Runtime risk is low because `Docs` is not game content.
- Process risk is medium because several prompt/workflow docs currently tell future agents to read `Docs/UI/UI_GENERATION.md` or other `Docs` files. Those references need to be removed, replaced, or made self-contained before the actual delete pass.

Decision:
- Accepted delete: entire `Docs` folder, after paired reference edits or optional salvage into the remaining documentation homes.

## 14. `Exports`

Path: `C:\UE\T66\Exports`

Inventory:
- Direct subfolders: `_package_v5`, `_staging`, `_verify_v3`, `_verify_v4`, `_verify_v5`, `_verify_v9`, `Chadpocalypse_v6`, `Chadpocalypse_v7`, `Chadpocalypse_v8`, `Chadpocalypse_v9`.
- Direct files: `Chadpocalypse_v1.zip` through `Chadpocalypse_v9.zip`.
- Recursive contents: 1,022 folders, 3,810 files, about 43.2 GB.
- Largest contents are repeated staged/verified Windows package outputs and 9 zipped builds at about 1.75-1.77 GB each.

General purpose:
- Historical local packaged-build export/output folder.
- This is generated distribution material, not source content.

Reference evidence:
- `git status --short -- Exports` and `git ls-files Exports` returned no tracked files.
- `.gitignore` already ignores `Exports/`.
- Exact search for root export references (`C:\UE\T66\Exports`, `C:/UE/T66/Exports`, `Exports/Chadpocalypse`, and `Chadpocalypse_v#`) outside `Exports` found no live references.

Recommendation:
- Delete the whole `Exports` folder during cleanup.
- Future packaged builds should use the current staging/build process and should not preserve every exported local version inside the repo workspace.

Tweaks required before deletion/consolidation:
- None found for root `Exports`.
- Keep the `.gitignore` rule as-is so new exports stay untracked if the folder reappears.

Good-to-delete candidates:
- all `Chadpocalypse_v*.zip` files
- `_package_v5`
- `_staging`
- `_verify_v3`, `_verify_v4`, `_verify_v5`, `_verify_v9`
- `Chadpocalypse_v6`, `Chadpocalypse_v7`, `Chadpocalypse_v8`, `Chadpocalypse_v9`
- root `Exports` folder if empty after deletion

Risk:
- Low. This is ignored generated package output and can be recreated by the build/stage pipeline.
- Main impact is losing old local exported builds, which remains acceptable because older versions are recoverable from GitHub/release history if needed.

Decision:
- Proposed delete: entire `Exports` folder.

## 15. `Guidelines`

Path: `C:\UE\T66\Guidelines`

Inventory:
- Direct subfolders: `Archive`.
- Direct files: none.
- Recursive contents: 1 folder, 0 files.
- Total size: 0 KB.

General purpose:
- Former location for guideline/master handoff material.
- Active guideline content has already been consolidated under `MASTER DOCS`.

Reference evidence:
- `git status --short -- Guidelines` and `git ls-files Guidelines` returned no tracked files.
- `MASTER DOCS/T66_DECISION_LOG.md` says active master handoff docs were moved out of `Guidelines/` into `MASTER DOCS/`.
- `Guidelines/Archive` is empty.

Recommendation:
- Delete the whole `Guidelines` folder.

Tweaks required before deletion/consolidation:
- None required for runtime or source.
- Optional documentation cleanup: update historical mentions in `MASTER DOCS/T66_DECISION_LOG.md` and `Audit/Finished/T66_DOCS_CLEANUP_LEDGER.md` so they no longer imply useful files remain under `Guidelines/Archive`.

Good-to-delete candidates:
- `Guidelines/Archive`
- root `Guidelines` folder

Risk:
- Very low. This is an empty, untracked folder.

Decision:
- Accepted delete: entire `Guidelines` folder.

## 16. `Intermediate`

Path: `C:\UE\T66\Intermediate`

Inventory:
- Direct subfolders: `Build`, `PipInstall`, `ProjectFiles`, `ReimportCache`, `ShaderAutogen`, `Staging`.
- Direct files: 67 root cache/target-info files, mostly cached asset registry `.bin` files.
- Recursive contents: 88 folders, 4,409 files, about 6.95 GB.
- Largest subfolder: `Intermediate/Build` at about 6.79 GB.
- Main file types: `.rsp`, `.json`, `.obj`, `.sarif`, generated `.cpp`, generated `.h`, `.bin`, `.old`, `.res`, `.lib`, `.exp`, project-file artifacts, and UHT artifacts.

General purpose:
- Unreal-generated intermediate build/editor state.
- Contains generated build objects, Unreal Header Tool output, generated project files, shader autogen files, reimport cache, staging scratch files, and cached asset registry data.

Reference evidence:
- `git status --short -- Intermediate` and `git ls-files Intermediate` returned no tracked files.
- `.gitignore` already ignores `Intermediate/`.
- `MASTER DOCS/T66_MASTER_GUIDELINES.md` and `MASTER DOCS/T66_DECISION_LOG.md` already describe `Intermediate/` as Unreal-generated and expected to reappear.
- Outside references found are generic cleanup/troubleshooting notes and logs, not source dependencies.

Recommendation:
- Delete the whole `Intermediate` folder during cleanup.
- It will regenerate on the next editor launch, project-file generation, compile, cook, or package.

Tweaks required before deletion/consolidation:
- None required.
- Keep the `.gitignore` rule as-is.

Good-to-delete candidates:
- `Intermediate/Build`
- `Intermediate/ProjectFiles`
- `Intermediate/ReimportCache`
- `Intermediate/ShaderAutogen`
- `Intermediate/Staging`
- `Intermediate/PipInstall`
- root cached asset registry files under `Intermediate`
- root `Intermediate` folder if empty after deletion

Risk:
- Low. This is generated local state.
- Expected downside is only one-time regeneration cost: Unreal/UBT may rebuild project files, UHT output, cached asset registry, shaders, and object files.

Decision:
- Accepted delete: entire `Intermediate` folder.

## 17. `MASTER DOCS`

Path: `C:\UE\T66\MASTER DOCS`

Inventory:
- Direct subfolders: `Backend Anti Cheat`.
- Direct files: 15 Markdown files.
- Recursive contents: 1 folder, 17 Markdown files, about 243 KB.
- Tracked files: 15 existing master docs.
- Current pending changes: several tracked master docs are already modified from the anti-cheat move/reference update; `MASTER DOCS/Backend Anti Cheat/` is currently untracked because the anti-cheat docs were moved there during this cleanup pass.

General purpose:
- Canonical long-lived project reference and policy layer.
- This is the folder that should survive after deleting old `Docs`, `Guidelines`, and root `ANTI_CHEAT`.
- It is documentation-only, but future agents and audits currently reference it heavily.

Reference evidence:
- `MASTER DOCS/README.md` identifies this folder as the canonical home for active master handoff and policy documents.
- Search outside the folder finds many references from `Audit`, `World Generation`, `Model Generation`, and prior docs.
- Several master files still contain stale references to folders now marked for deletion: `Docs`, `Guidelines`, old `ANTI_CHEAT`, and old `Audit/Pending` paths.

Keep:
- `README.md`, but update it as the compact index for the surviving master docs.
- `T66_MASTER_GUIDELINES.md`, but remove `Docs/` from active-root rules and remove references to deleted UI docs.
- Core runtime master docs, moved out of the flat root into domain subfolders.
- Online/backend docs, moved into a backend/release domain structure.
- Pipeline/reference docs, moved into the domain that owns the workflow.

Target organization:
- Updated direction: disband `MASTER DOCS` as a long-term folder after the surviving docs are moved to the component that owns them.
- Root `AGENTS.md` remains the tiny always-read agent rule file.
- Component folders under `C:\UE\T66` become the canonical doc homes:
  - `Backend/`: backend, APIs, persistence, leaderboards, and account docs.
  - `Backend/Anti Cheat/`: anti-cheat and run-integrity docs.
  - `Gameplay/Movement/`: movement docs.
  - `Gameplay/Combat/`: combat docs.
  - `Gameplay/Stats/`: stats and player-experience docs.
  - `Gameplay/Camera/`: camera docs.
  - `Gameplay/World/`: maps, traps, lighting, tower/world docs unless `World Generation` owns a specific generation workflow.
  - `Release/Steam/` or `Steam/`: Steamworks/release docs.
  - `UI/`: UI workflow, reference generation, sprite/icon process docs, and current UI component documentation.
  - `Model Generation/Instructions/`: model-generation and import-pipeline docs that belong to the asset pipeline.
- Exact folder names should be finalized during implementation, but the key rule is that docs live next to their owning component instead of in a central master-doc pile.

Good-to-delete / consolidate candidates:
- `Backend Anti Cheat/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`: implementation-plan style doc. Delete after any still-current anti-cheat requirements are summarized into `Backend Anti Cheat/MASTER_ANTI_CHEAT.md` or confirmed obsolete.
- `T66_DECISION_LOG.md`: mostly historical decisions and cleanup history. Delete or heavily compress after active exceptions are moved into `T66_MASTER_GUIDELINES.md`; Git history can carry the detailed timeline.
- `T66_PROJECT_CATALOGUE.md`: large architecture snapshot that may go stale quickly. Either keep only if it remains actively useful as a high-level codebase map, or replace with a much smaller `README.md`/index and delete the catalogue.
- Historical path references inside `MASTER_BACKEND.md`, `MASTER_LIGHTING.md`, and `MASTER_TRAPS.md` that point to `Docs/Archive` or `Docs/Implementation Plan` should be removed when `Docs` is deleted.

Tweaks required before deletion/consolidation:
- Update `MASTER DOCS/README.md`; it still points to `../Audit/Pending/T66_MASTER_OPTIMIZATION_AUDIT_V5.md`, which is stale against the current audit folder state.
- Update `T66_MASTER_GUIDELINES.md`; it still lists `Docs/` as an active documentation home and lists `Docs/UI/UI_GENERATION.md` as a related doc.
- Update `MASTER_TRAPS.md`; it still names `Docs/Implementation Plan/T66_Tower_Multi_Agent_Implementation_Plan.md` as a companion doc.
- Update `MASTER_BACKEND.md` and `MASTER_LIGHTING.md`; they still point at historical predecessor docs under `Docs/Archive`.
- If `T66_DECISION_LOG.md` is deleted, preserve only active exceptions that still affect current work.
- If `T66_PROJECT_CATALOGUE.md` is deleted, preserve only a compact pointer list or let agents inspect source directly.

Risk:
- Do not delete the whole folder until the surviving docs have been moved and all references updated.
- Runtime risk is none, but process risk is high if master docs are deleted without moving active policy elsewhere.
- Aggressive cleanup should reduce this folder to current policy/reference only and remove historical timelines, implementation checklists, and stale path references.

Decision:
- Accepted direction update: `MASTER DOCS` should be disbanded during the cleanup implementation after active docs are moved into component folders under the project root.
- Accepted cleanup: preserve only current master-level content, delete/consolidate historical and implementation-plan style docs, and remove stale references to folders already marked for deletion.

## 18. `Model Generation`

Path: `C:\UE\T66\Model Generation`

Inventory:
- Direct subfolders: `Archive`, `Reference`, `Runs`, `Scenes`, `Scripts`, `Tools`.
- Direct files: 23 files, mostly Markdown workflow/prompt docs, plus `ModelReference_Green.png` and `LOCAL_ACCESS.env`.
- Recursive contents: 1,523 folders, 4,339 files, about 8.01 GB.
- Largest areas:
  - `Runs`: about 5.52 GB.
  - `Scenes`: about 2.11 GB.
  - `Archive`: about 376 MB.
  - `Scripts`, `Tools`, and `Reference`: less than 1 MB combined.
- File types are mostly generated model-work artifacts: `.png`, `.glb`, `.fbx`, `.blend`, `.blend1`, `.json`, `.txt`, `.log`, plus workflow scripts/docs.

General purpose:
- Local 3D model-generation workspace for Trellis/RunPod, Blender processing, Quad Retro experiments, mesh QA, retopo/rigging/import prep, and generation run evidence.
- This folder is not runtime game content, but several import/verification scripts point at it when reimporting or validating generated assets.

Reference evidence:
- `README.md` identifies `MASTER_WORKFLOW.md` as the current production workflow and `Runs/`, `Scenes/`, and `Archive/` as artifact/work areas.
- `.gitignore` already ignores `Model Generation/Archive/`, `Model Generation/Scenes/`, large run artifacts such as `.blend`, `.fbx`, `.glb`, images, `Raw/`, `Renders/`, `Exports/`, and `LOCAL_ACCESS.env`.
- There are still tracked older run artifacts, especially under `Runs/Arthur/Raw`, `Runs/Enemies/Easy`, and `Runs/Environment/CoherentThemeKit01`.
- Memory note: `Runs/Environment/CoherentThemeKit01` was the canonical completed coherent environment-kit run, with `batch_manifest.json`, split-sheet workflow, and 40 raw Trellis GLBs as its validation surface.
- Current source/scripts outside the folder reference model runs:
  - `Scripts/ImportTypeABatch01RiggedHeroes.py`
  - `Scripts/ImportQuadRetroHeroVisuals.py`
  - `Scripts/ImportQuadRetroEnemyVisuals.py`
  - `Scripts/ImportQuadRetroBossVisuals.py`
  - `Scripts/ImportWeaponProjectileMeshesAndSetup.py`
  - `Scripts/ImportWorldNpcInteractablesRetroBatch01AndExit.py`
  - `Scripts/BuildWorldNpcInteractablesRetroBatch01ManifestAndExit.py`
  - `Scripts/ValidateBossQuadRetroVisuals.py`
  - `Scripts/VerifyWorldNpcInteractablesRetroBatch01AndExit.py`

Keep:
- Keep the `Model Generation` folder as the active pipeline home unless the whole Trellis/Blender source workflow is being retired.
- Keep `Scripts/` and `Tools/`; they are small and contain the reproducible workflow.
- Keep a compact `README.md`, `MASTER_WORKFLOW.md`, `ENVIRONMENT_LOCK.md`, `KNOWN_ISSUES.md`, `RUN_HISTORY.md`, and whichever single current runbook is still active.

Good-to-delete / consolidate candidates:
- Delete `Model Generation/Archive` entirely.
- Delete `Model Generation/Scenes` entirely, including the tracked `Scenes/Arthur_EasyEnemy_Lineup.blend`.
- Delete `LOCAL_ACCESS.env`; it is ignored local access material and should not live in cleanup inventory as durable source.
- Delete prompt/handoff docs that are not current evergreen process docs:
  - `CURRENT_HANDOFF_PROMPT.md`
  - `SECOND_ATTEMPT_PROMPT.md`
  - `NEXT_CHAT_HERO_MALE_IMAGEGEN_PROMPT.md`
  - `NEXT_STEPS.md`
- Delete or consolidate root one-off/reference docs if their useful rules are already in `MASTER_WORKFLOW.md` or the main pipeline docs:
  - `TRELLIS_DIVERGENCE_REEVALUATION_20260504.md`
  - `WALLS_FLOORS_CEILINGS.md`
  - `MESH_APPROVAL_CHECKLIST.md`
  - `Reference/TRELLIS2_RunPod_Setup_Guide.jsx`
  - `ModelReference_Green.png`
- Delete `Model Generation/Runs/Heroes/_Archive`, including `Obsolete_2026-05-06_QuadRetroFirstPass`, which was already identified while reviewing `_codex_previews`.
- Delete old/obsolete character experiment runs if their outputs were imported or superseded:
  - `Runs/Arthur`
  - `Runs/Heroes/Hero_1_Arthur`
  - `Runs/Heroes/QuadRetroSourceExploration01`
  - `Runs/Heroes/QuadRetroModelTest01`
  - `Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake`
  - older TypeA/Chad experimental run outputs if current hero-count cleanup retires Hero_13 through Hero_15 and Arthur-era source paths.
- Delete old generated run outputs under `Runs/EnemyBosses`, `Runs/Interactables`, and `Runs/Weapons` after confirming their final imported `/Game/...` assets are present and current.
- For `Runs/Environment/CoherentThemeKit01`, either keep only the lightweight manifest/notes needed to explain the completed batch, or delete the full run if we are comfortable relying on Git history and cooked/imported content. Do not treat raw Trellis GLBs as runtime dependencies.

Tweaks required before deletion/consolidation:
- If deleting run outputs, update or delete scripts that hardcode those run roots, especially import/verification scripts listed above.
- Update `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md`; it directly links to tracked GLBs in `Model Generation/Runs`.
- Update `World Generation` docs that point at `Model Generation/Runs/Environment/CoherentThemeKit01`, especially `MODULAR_DUNGEON_KIT_PROCESS.md`, `SHARED_ASSET_PIPELINE.md`, and `SETUP.md`, if those docs survive the later `World Generation` cleanup.
- Update `Model Generation/README.md` after removing handoff docs, `Reference`, `Archive`, `Scenes`, or any run roots.
- Before deleting untracked/ignored generated outputs, note that they may not be recoverable from GitHub unless they were pushed elsewhere; tracked files are recoverable from Git.

Risk:
- Runtime risk is low: this folder is not cooked game content.
- Pipeline/reproduction risk is medium to high if raw run outputs are deleted before confirming final assets are already imported into `Content`.
- Disk cleanup payoff is high: deleting `Runs`, `Scenes`, and `Archive` could remove roughly 8 GB.

Decision:
- Accepted keep folder, aggressively prune inside: keep the model-generation pipeline docs/scripts/tools, delete `Archive`, `Scenes`, obsolete handoff/prompt docs, obsolete experiment runs, and generated run outputs once paired script/doc references are cleaned.

Detailed docs consolidation proposal:
- Replace the current loose root-doc set with a small `Model Generation/Instructions/` folder and a thin root `README.md`.
- Root `README.md`: only explain what this workspace is, what to read first, and where scripts/runs live.
- `Instructions/00_MASTER.md`: canonical decision tree for the model-generation pipeline. This should replace the broad role of `MASTER_WORKFLOW.md` and point to the step docs below.
- `Instructions/01_TRELLIS_RUNPOD_SETUP.md`: merge the still-current setup parts of `MASTER_WORKFLOW.md`, `ENVIRONMENT_LOCK.md`, `LOCAL_ACCESS.md`, and `KNOWN_ISSUES.md` RunPod/environment sections. Do not keep live secrets in repo docs.
- `Instructions/02_SOURCE_IMAGE_RULES.md`: merge `TRELLIS_SOURCE_IMAGE_RULES.md`, `HERO_CHAD_STACY_PROMPT_GUIDE.md`, and the useful pass/fail rules from `MESH_APPROVAL_CHECKLIST.md`.
- `Instructions/03_QUAD_RETRO_PIPELINE.md`: merge `RETRO_CHARACTER_PIPELINE.md`, `QUAD_RETRO_DO_THIS_RUNBOOK.md`, `RETOPOFLOW_4.md`, and Quad Retro failure/verification notes from `KNOWN_ISSUES.md`.
- `Instructions/04_BLENDER_PROCESSING_AND_RIGGING.md`: merge the still-current parts of `Model Processing.md` and `Rigging Process.md`. Keep legacy TypeA/Mike rigging notes only as short historical notes if still needed.
- `Instructions/05_UNREAL_IMPORT_AND_VALIDATION.md`: keep `Model Importing.md` as the base, plus the Quad Retro import lessons currently embedded in `QUAD_RETRO_DO_THIS_RUNBOOK.md`.
- `Instructions/06_RUN_HISTORY_AND_KNOWN_ISSUES.md`: compress `RUN_HISTORY.md` and `KNOWN_ISSUES.md` into current known-good/known-bad evidence only. Do not preserve full chat-style chronology unless it is needed for current decisions.
- Delete root docs after consolidation:
  - `CURRENT_HANDOFF_PROMPT.md`
  - `SECOND_ATTEMPT_PROMPT.md`
  - `NEXT_CHAT_HERO_MALE_IMAGEGEN_PROMPT.md`
  - `NEXT_STEPS.md`
  - `TRELLIS_DIVERGENCE_REEVALUATION_20260504.md`
  - `WALLS_FLOORS_CEILINGS.md`
  - `MESH_APPROVAL_CHECKLIST.md`
  - `LOCAL_ACCESS.md`
  - `LOCAL_ACCESS.env`
  - `Reference/TRELLIS2_RunPod_Setup_Guide.jsx`
  - `ModelReference_Green.png`

Detailed scripts organization proposal:
- Add `Model Generation/Scripts/README.md` that explains the difference between master scripts, batch drivers, legacy prototypes, and retired scripts.
- Add the script lifecycle rule to that README: master scripts should stay small, reusable, and amended when a one-off task teaches a durable improvement; task-specific scripts should be deleted after they have accomplished their task and their result is imported, verified, or documented.
- Split scripts into:
  - `Scripts/Core/Trellis/`: reusable pod/server/auth helpers such as `bootstrap_trellis2_pod.sh`, `Invoke-RunPodHfLogin.ps1`, and any future generic Trellis manifest runner.
  - `Scripts/Core/Blender/`: reusable Blender QA/render/export helpers, starting with `blender_glb_qa.py`.
  - `Scripts/Core/QuadRetro/`: the reusable Quad Retro engine and wrapper, `t66_quad_retro_character_pipeline.py` and `RunQuadRetroCharacterPipeline.ps1`.
  - `Scripts/Core/UnrealReadyExport/`: future generalized exporters that consume a manifest and produce Unreal-ready mesh/texture outputs.
  - `Scripts/Batches/<BatchName>/`: one-off or batch-specific drivers with hardcoded run roots, row orders, pod IDs, or asset maps.
  - `Scripts/Legacy/`: old Arthur, TypeA split, Mike prototype, dungeonkit/floorfix, and rejected experiment helpers that are kept only until their outputs are either imported or deleted.
- Current reusable master scripts:
  - `bootstrap_trellis2_pod.sh`
  - `Invoke-RunPodHfLogin.ps1`
  - `blender_glb_qa.py`
  - `t66_quad_retro_character_pipeline.py`
  - `RunQuadRetroCharacterPipeline.ps1`
  - possibly `split_theme_module_sheet.py`, if environment-kit sheet splitting stays in this workspace.
- Current batch/situational scripts:
  - `run_enemyboss_stage01_enemies.py`
  - `run_world_npc_interactables_stage01_trellis.py`
  - `run_world_npc_interactables_stage02_quad_retro.py`
  - `prepare_world_npc_interactables_stage01_sources.py`
  - `export_world_npc_interactables_retro_batch01_unreal_ready.py`
  - `export_interactable_batch01_unreal_ready.py`
  - `export_weapon_projectiles_unreal_ready.py`
  - `export_coherent_themekit_unreal_ready.py`
  - `build_typea_hero_batch01_assets.py`
  - review-scene/render scripts tied to a named batch.
- Current legacy/prototype candidates:
  - `assemble_typea_head_body.py`
  - `assemble_chad_head_body_pair.py`
  - `rig_typea_batch01.py`
  - `rig_typea_mike_prototype.py`
  - `rig_chad_mike_process_prototype.py`
  - `repair_mike_a03_b02_neck_hair.py`
  - `graft_mike_a04_neck_to_a03.py`
  - `preprocess_mike_a04_head_source.py`
  - `qa_mike_sword_roll_variants.py`
  - old dungeonkit/floor-slab helper scripts if `World Generation` owns that process now.
- Future master-script targets:
  - Replace hardcoded Trellis batch drivers with a generic `run_trellis_manifest_batch.py` that takes a manifest, pod connection config, output root, seed, texture size, decimation, QA flag, and retry policy.
  - Replace hardcoded Quad Retro batch drivers with a generic `run_quad_retro_manifest_batch.py` that takes a manifest and invokes the core Quad Retro pipeline per asset.
  - Replace the multiple `export_*_unreal_ready.py` scripts with one manifest-driven exporter for static mesh assets, plus small per-batch manifests instead of separate per-batch Python files.

Detailed cleanup policy for generated outputs:
- Keep only lightweight manifests, final status summaries, and any truly current source images needed for active generation.
- Delete full raw/generated folders once final content is imported into `Content` or source-import folders under `SourceAssets`.
- Do not keep both an imported `/Game/...` asset, a `SourceAssets/Import/...` copy, and several `Model Generation/Runs/...` raw copies unless the run is still active.

## 19. `output`

Path: `C:\UE\T66\output`

Inventory:
- Direct subfolders: `asset_library_20260428`, `frontend_pass09_focus`.
- Direct files: none.
- Recursive contents: 3 folders, 23 files, about 15.01 MB.
- `asset_library_20260428` contains an old generated asset-library report: `.html`, `.json`, `.csv`, `.md`, and screenshot `.png` files.
- `frontend_pass09_focus` is empty.

General purpose:
- Local generated output/scratch folder.
- Current contents are old report artifacts, not source content.

Reference evidence:
- `git status --short -- output` and `git ls-files output` returned no tracked files.
- `.gitignore` already ignores `output/`.
- Exact search for `asset_library_20260428` and `frontend_pass09_focus` found no references outside this folder.
- `Tools/Codex/powerup_statue_masks.py` uses `output/imagegen` and `output/powerup_masks` only as example scratch paths; it does not depend on the current output folders.
- References to `Tools/ChatGPTBridge/output` in `SourceAssets/Mini` manifests are a different path and do not require keeping root `output`.

Recommendation:
- Delete the whole `output` folder during cleanup.
- Keep the `.gitignore` rule as-is so scratch output remains local if the folder reappears.

Tweaks required before deletion/consolidation:
- None for current contents.
- Optional: update `Tools/Codex/powerup_statue_masks.py` examples later if the project standard moves scratch output under `Tools/Temp` instead of root `output`.

Good-to-delete candidates:
- `output/asset_library_20260428`
- `output/frontend_pass09_focus`
- root `output` folder if empty after deletion

Risk:
- Low. This is ignored generated report/scratch output.

Decision:
- Accepted delete: entire `output` folder.

## 20. `Plugins`

Path: `C:\UE\T66\Plugins`

Inventory:
- Direct subfolders: none.
- Direct files: none.
- Recursive contents: 0 folders, 0 files, 0 MB.
- No tracked files.

General purpose:
- Standard Unreal project-local plugin folder.
- Currently empty. No project-local `.uplugin` files are present.

Reference evidence:
- `T66.uproject` enables engine plugins: `ModelingToolsEditorMode`, `PythonScriptPlugin`, `EditorScriptingUtilities`, `OnlineSubsystemSteam`, `SocketSubsystemSteamIP`, and `ProceduralMeshComponent`.
- Those are engine/plugin-manager dependencies, not files under `C:\UE\T66\Plugins`.
- Search found no references to project-local plugin files or plugin directories under `C:\UE\T66\Plugins`.
- `MASTER DOCS/T66_MASTER_GUIDELINES.md` currently lists `Plugins/` as a required stable root item, but this is stale if the folder remains empty.

Recommendation:
- Delete the empty `Plugins` folder.
- If project-local plugins are added later, Unreal or the developer can recreate the folder.

Tweaks required before deletion/consolidation:
- Update `MASTER DOCS/T66_MASTER_GUIDELINES.md` so `Plugins/` is listed as optional/project-local rather than a required stable root while empty.
- Do not remove the enabled engine plugin entries from `T66.uproject`.

Good-to-delete candidates:
- root `Plugins` folder

Risk:
- Low. The folder is empty and untracked.
- Do not confuse this with disabling engine plugins in `T66.uproject`; those should stay.

Decision:
- Accepted delete: empty `Plugins` folder, after updating master guidelines wording.

## 21. `RuntimeDependencies`

Path: `C:\UE\T66\RuntimeDependencies`

Inventory:
- Direct subfolders: `T66`.
- Direct files: none.
- Current working-tree contents: 11 folders, 68 files, about 23.92 MB.
- Tracked files: 70, with two already deleted in the working tree: `RuntimeDependencies/T66/UI/Minimap/Icons/support_vendor.png` and `RuntimeDependencies/T66/UI/Minimap/Icons/vendor.png`.
- Main subfolders:
  - `RuntimeDependencies/T66/Fonts`: `Jersey10-Regular.ttf` plus license text.
  - `RuntimeDependencies/T66/UI/HeroSelection`: hero-selection medals, rank badge, crossed-swords challenge icon, and one older loose unproven-medal image.
  - `RuntimeDependencies/T66/UI/Minimap`: minimap frame, marker sheet, theme backgrounds, theme walls, icon PNGs, and small README files.
  - `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs`: secondary-stat/buff icon PNGs.

General purpose:
- Staged loose runtime dependency assets for file-backed Slate UI.
- These are not generated build products like `Binaries`, `Intermediate`, or `DerivedDataCache`; the current game code and packaging rules intentionally stage and load these files by path.

Reference evidence:
- `Source/T66/T66.Build.cs` stages `RuntimeDependencies/T66/Fonts/...` and `RuntimeDependencies/T66/UI/...` via `AddLooseRuntimeDependency`.
- `Config/DefaultGame.ini` declares both roots as `LooseRuntimeContentRoots` with owners `T66RuntimeUIFontAccess` and `T66RuntimeUITextureAccess`.
- Live code references the folder:
  - `T66RuntimeUIFontAccess.cpp` loads `RuntimeDependencies/T66/Fonts/Jersey10-Regular.ttf`.
  - `T66Style.cpp` loads `RuntimeDependencies/T66/UI/Minimap/minimap_frame.png`.
  - `T66GameplayHUDWidget_Private.h` references minimap backgrounds, walls, icons, chest reward paths, currency paths, and an old minimap sheet name.
  - `T66HeroSelectionScreen_Private.h` references hero-selection medals, rank badge, and crossed-swords challenge icon.
  - `T66PowerUpScreen.cpp` and `T66TemporaryBuffUIUtils.cpp` build secondary-buff icon paths under `RuntimeDependencies/T66/UI/PowerUp/SecondaryBuffs`.

Recommendation:
- Do not delete the whole folder in the first cleanup pass. It is live runtime content.
- Treat this as a consolidation target: migrate the remaining loose UI/font assets to cooked Unreal assets under `Content/UI` or another canonical UI asset root, update the C++ loaders to use those cooked assets, then remove the loose-runtime staging rules and delete the folder.
- Use this folder as evidence that some UI cleanup should happen in code, not only in the file tree.

Tweaks required before deletion/consolidation:
- Replace file-path-based font loading with a cooked font asset, or keep only the font root if the file-backed Slate font approach remains intentional.
- Move minimap art, hero-selection medals, and secondary-buff icons to canonical cooked UI assets, then update all C++ references.
- Fix stale runtime paths that currently point to missing files:
  - `RuntimeDependencies/T66/UI/ChestRewards/*.png`
  - `RuntimeDependencies/T66/UI/Currency/*.png`
  - `RuntimeDependencies/T66/UI/Minimap/#1 - Transparent Icons.png`
- Review `T66RuntimeUITextureAccess.cpp` remaps for `MainMenu`, `Currency`, `ChestRewards`, and `PowerUp/Statues`; those runtime folders are not present in the current `RuntimeDependencies` tree.
- After migration, remove the `RuntimeDependencies/T66/...` entries from `Source/T66/T66.Build.cs` and `Config/DefaultGame.ini`.
- Refresh and verify the staged standalone build after any playable-build-affecting migration.

Good-to-delete candidates:
- `RuntimeDependencies/T66/UI/Minimap/Icons/support_vendor.png` and `RuntimeDependencies/T66/UI/Minimap/Icons/vendor.png`; already deleted in the working tree and no current source reference was found.
- `RuntimeDependencies/T66/UI/HeroSelection/medal_unproven_imagegen_20260426_v1.png`; appears to be an older duplicate because current source uses `Medals/medal_unproven_imagegen_20260427_v2.png`.
- `RuntimeDependencies/T66/UI/Minimap/**/README.txt` after any useful notes are moved into a canonical UI/runtime-assets doc.
- Eventually, `RuntimeDependencies/T66/UI` and possibly `RuntimeDependencies/T66/Fonts`, but only after the code/config migration above.

Risk:
- High if deleted immediately. The packaged game can lose UI font rendering, minimap art, hero-selection medals, and power-up/buff icons.
- Medium for the stale missing-path fixes, because those paths already do not exist but the related UI fallback behavior should still be tested.

Decision:
- Accepted cleanup path: keep for now, mark the stale/deleted files above for deletion, and plan a later migration from loose runtime files to cooked UI assets so the whole folder can eventually be removed.

## 22. `Saved`

Path: `C:\UE\T66\Saved`

Inventory:
- Direct subfolders: 41.
- Direct files: 147.
- Recursive contents: 3,057 folders, 20,100 files, about 22,939.27 MB.
- Tracked files: 0.
- `.gitignore` already ignores `Saved/`.
- Largest direct entries:
  - `StagedBuilds`: about 8,340.76 MB.
  - `Cooked`: about 4,950.33 MB.
  - `UploadBuilds`: about 2,751.35 MB.
  - `StandaloneValidation`: about 2,748.22 MB.
  - `Autosaves`: about 1,668.10 MB.
  - `Crashes`: about 590.28 MB.
  - `TikTokWebView2`: about 484.30 MB.
  - `Shaders`: about 213.30 MB.
  - `Screenshots`: about 191.23 MB.
  - Generated visual-review folders such as `ChadBatch01VisualCheck`, `TypeABatch01VisualCheck`, `Worker2VisualReview`, `MirrorShots`, `IdolReview`, `ZibraReview`.

General purpose:
- Unreal-generated local workspace state: cooked output, staged packages, autosaves, logs, crash dumps, screenshots, shader/debug output, temporary import reports, local save games, web cache, and validation artifacts.
- This is not source content and should not be part of the repo inventory beyond temporary local evidence.

Reference evidence:
- `Saved/` is ignored in `.gitignore`.
- `git ls-files Saved` returned zero tracked files.
- Current active standalone executable exists at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- `AGENTS.md`, UI docs, Steam docs, and build scripts intentionally point at `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe` as the local playable standalone target.
- `Scripts/StageStandaloneBuild.ps1` writes the staged build under `Saved/StagedBuilds` and standalone logs under `Saved/StandaloneLogs`.
- Various docs reference `Saved/Logs/T66.log` as a transient verification log, not as a preserved source artifact.

Recommendation:
- Delete most of `Saved` during the cleanup stage.
- Do not leave the taskbar shortcut broken: either preserve `Saved/StagedBuilds/Windows/T66` through the cleanup or delete/regenerate it immediately with `Scripts/StageStandaloneBuild.ps1`, then verify `T66 Standalone.lnk` still points at the refreshed exe.
- Keep only short-term evidence that is actively needed for the current cleanup decision. Any important audit conclusion should be copied into `Audit` or `MASTER DOCS`; raw generated evidence in `Saved` should not be kept long-term.

Tweaks required before deletion/consolidation:
- Before deleting `Saved/StagedBuilds`, confirm whether the current taskbar shortcut should remain usable during the cleanup. If yes, rebuild/restage immediately after deletion.
- Preserve no raw `Saved/Logs` history unless an active bug investigation specifically needs it; summarize useful findings into docs instead.
- If local player progress matters, manually export or intentionally preserve `Saved/SaveGames`; otherwise it can be deleted like the rest of `Saved`.
- If Steam upload evidence matters, keep the relevant notes in `MASTER DOCS/MASTER_STEAMWORKS.md`, not the generated `Saved/UploadBuilds` folder.

Good-to-delete candidates:
- `Saved/Cooked`
- `Saved/UploadBuilds`
- `Saved/StandaloneValidation`
- `Saved/Autosaves`
- `Saved/Crashes`
- `Saved/TikTokWebView2`
- `Saved/webcache_*`
- `Saved/Shaders`
- `Saved/ShaderDebugInfo`
- `Saved/Screenshots`
- `Saved/Logs`
- `Saved/StandaloneLogs`
- `Saved/Temp`
- `Saved/MaterialStats`
- `Saved/Interchange`
- `Saved/Automation`, `Saved/AutomationScreens`
- Generated review/proof folders such as `Saved/ChadBatch01VisualCheck`, `Saved/TypeABatch01VisualCheck`, `Saved/Worker2VisualReview`, `Saved/MirrorShots`, `Saved/IdolReview`, `Saved/ZibraReview`, `Saved/MikeRigPrototypeUnrealCheck`.
- Root-level `.tmp`, temporary report `.json`, contact-sheet `.png`, and inspect `.txt` files after any useful conclusions are moved to canonical docs.
- `Saved/StagedBuilds` can also be deleted, but only if it is immediately regenerated so the taskbar shortcut remains valid.

Risk:
- Low for repo/source integrity because nothing under `Saved` is tracked.
- Medium for local workflow if `Saved/StagedBuilds` is deleted without restaging; it would break the current taskbar shortcut.
- Medium for local-only save/progress state if `Saved/SaveGames` is deleted.

Decision:
- Accepted cleanup: delete nearly all of `Saved`.
- Preserve or immediately regenerate `Saved/StagedBuilds/Windows/T66` so `T66 Standalone.lnk` continues launching the current playable standalone.

## 23. `Scripts`

Path: `C:\UE\T66\Scripts`

Inventory:
- Direct subfolders: none.
- Direct files: 153.
- Recursive contents: 153 files, about 0.75 MB.
- File types: 148 `.py`, 5 `.ps1`.
- Tracked files: 124.
- Current working tree has modified scripts and many untracked recent scripts from the latest import/model/UI work; do not revert these as part of cleanup.

General purpose:
- Project automation scripts for Unreal editor tasks, data-table reloads, asset imports, packaged-build staging, UI capture, material repair, runtime dependency guards, and one-off audits/probes.
- This folder is source/tooling, not generated output, so it should not be deleted wholesale.
- The current problem is that all reusable tools, temporary experiment scripts, old migration scripts, and batch-specific scripts are mixed in one flat directory.

Reference evidence:
- `AGENTS.md`, `MASTER DOCS/T66_MASTER_GUIDELINES.md`, UI docs, audit docs, and model-generation docs all reference `Scripts/StageStandaloneBuild.ps1`.
- UI workflow docs reference `Scripts/CaptureT66UIScreen.ps1`.
- Audit/packaging docs reference `Scripts/GuardT66RuntimeAssetContract.ps1`.
- `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md` references the generic import pipeline scripts, but still includes stale prop pipeline entries.
- `Source/T66Editor/T66UISetupSubsystem.cpp` references current setup scripts such as `SetupArcadeInteractablesDataTable.py` and `SetupWeaponsDataTable.py`.
- Source comments reference missing scripts `Scripts/ImportLeaderboardIcons.py` and `Scripts/ReparentToFBXUnlit.py`; those comments should be updated or the intended current script names should be restored.

Recommendation:
- Keep `Scripts`, but reorganize it aggressively and delete obsolete one-off scripts after any useful lessons are moved into canonical docs.
- Add a `Scripts/README.md` with the same lifecycle rule used in `Model Generation/Scripts`: master scripts are reusable project tools; one-off task scripts should be deleted after the task is proven complete; durable lessons from those one-offs should be folded into an existing master script, a manifest format, a new reusable tool, or a canonical doc.
- Introduce a folder structure:
  - `Scripts/Build/`: standalone staging, runtime contract guard, smoke/capture helpers.
  - `Scripts/DataTables/`: setup/reload/validate scripts for live data tables.
  - `Scripts/Import/Core/`: generic mesh/UI import, repair, flatten, and validation helpers.
  - `Scripts/Import/Batches/`: temporary current batch drivers that are still needed until their imported content is verified.
  - `Scripts/UI/`: UI capture/resample/import helpers that remain relevant to the current Ultrakill-inspired UI process.
  - `Scripts/Maintenance/`: active audits/cleanup scripts that are still useful.
- Longer-term: replace batch-specific scripts with manifest-driven master scripts, so new asset batches use a manifest instead of a new custom Python file each time.

Tweaks required before deletion/consolidation:
- Update docs and source comments after moving scripts so references use the new folder paths.
- Remove or rewrite stale prop instructions in `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md` once the prop data/system cleanup is done.
- Split `ReloadPropsAndArcadeInteractablesAndExit.py` if the arcade half is still needed, then delete the props half.
- Consolidate combat visual validators into fewer scripts, for example one `ValidateCombatRosterVisuals.py` instead of separate boss/enemy/quad-retro variants.
- Consolidate static-mesh import batch drivers into a generic manifest runner instead of per-batch import scripts.
- Consolidate Quad Retro import wrappers into one manifest-driven `ImportQuadRetroVisualsAndExit.py`.

Keep candidates:
- `StageStandaloneBuild.ps1`
- `CaptureT66UIScreen.ps1`
- `GuardT66RuntimeAssetContract.ps1`
- `ImportStaticMeshes.py`
- `ImportSkeletalMeshes.py`
- `VerifyImportBatch.py`
- `MakeGLBImportsUnlit.py`
- `MakeCharacterMaterialsUnlit.py`
- `FlattenInterchangeAssets.py`
- `FixCookWarningRoots.py`
- current live data-table setup scripts: audio events, weapons, player experience, arcade interactables, house NPCs, combat roster, character visuals, items, idols, companions.
- current import/verify scripts for the active Quad Retro, combat roster, world NPC/interactable, coherent theme kit, weapon projectile, and arcade replacement work until those are consolidated.

Good-to-delete candidates:
- Probe scripts after their findings are no longer needed:
  - `ProbeBgSockets.py`
  - `ProbeBlenderFluidAPI.py`
  - `ProbeDomainDisplayAttrs.py`
  - `ProbeVolumeNodes.py`
  - `ProbeVolumeNodes2.py`
  - `ProbeVolumePrincipled.py`
- One-off inspect scripts after useful facts are moved into docs:
  - `InspectDefaultSmokeMaterial.py`
  - `InspectExpressionInputs.py`
  - `InspectGLTFExporterPython.py`
  - `InspectMaterialExpressions.py`
  - `InspectMaterialFlags.py`
  - `InspectMpcStruct.py`
  - `InspectPlaceholderMaterial.py`
  - `InspectQuickSmokeDefaults.py`
  - `InspectRetroBaseParams.py`
  - `InspectRetroFunctionInputs.py`
  - `InspectRetroFunctions.py`
  - `InspectRetroMasters.py`
  - `InspectUnrealMaterialAsset.py`
  - `InspectUnrealMeshApis.py`
- Arthur-specific scripts, aligned with the accepted Arthur removal:
  - `CreateArthurVfxPreviewMap.py`
  - `ExportArthurRuntimeMeshForBlender.py`
  - `ExportArthurRuntimeMeshForBlenderGLTF.py`
- Prop-specific scripts, aligned with the accepted prop subsystem/data removal:
  - `SetupPropsDataTable.py`
  - `RunImportPropsAndExit.py`
  - prop half of `ReloadPropsAndArcadeInteractablesAndExit.py`
- Old TypeA/Mike prototype scripts after the current hero pipeline is documented and the live hero assets are verified:
  - `ImportMikeRigPrototype.py`
  - `RunImportMikeRigPrototypeAndExit.py`
  - `RepairMikeRigPrototypeMaterials.py`
  - `ImportTypeABatch01RiggedHeroes.py`
  - `RunImportTypeABatch01RiggedHeroesAndExit.py`
  - `RepairTypeABatch01Materials.py`
  - `VerifyTypeABatch01HeroVisuals.py`
  - `CaptureTypeABatch01VisualLineup.py`
  - `MigrateBodyTypeNomenclatureToChadStacy.py`
- Old retro-material experiment scripts after current materials are confirmed stable:
  - `AppendRetroGeometryMaterial.py`
  - `SetupRetroGeometryMaterials.py`
  - `SetupRetroGeometryOneMaterial.py`
  - `SetupRetroGeometryVariantMaterials.py`
  - `SetupIntegratedRetroGeometryMasters.py`
  - `SetupIntegratedRetroVariants.py`
  - `SetupRetroPs1Variants.py`
  - `RestoreCoreUnlitMasters.py`
  - `PromoteCurrentMastersToRetroVariants.py`
  - `DisableRetroGeometryWPO.py`
  - `FixRetroGeometryTextureDefaults.py`
  - `RepairRetroGeometryVisibility.py`
  - `CreateRetroChromaticAberrationMaterial.py`
- Old UI/source-art generation scripts after the current UI library is canonical:
  - `SliceImageGenWoodUILibrary.ps1`
  - `GenerateNewMMBackgroundLayers.py`
  - `GenerateChestRewardAssets.py`
- `BuildTutorialMap.py` if the tutorial map is confirmed obsolete during Content/map cleanup.

Risk:
- Medium. Deleting the wrong script will not break cooked game runtime directly, but it can break the build/import/verification workflow.
- High for `StageStandaloneBuild.ps1`, `CaptureT66UIScreen.ps1`, and `GuardT66RuntimeAssetContract.ps1`; those should stay.

Decision:
- Accepted keep: `Scripts` as a source/tooling folder.
- Accepted cleanup: reorganize into subfolders, add a README with the script lifecycle rule, consolidate batch wrappers into manifest-driven master scripts, and delete the obsolete probe/inspect/Arthur/props/TypeA/Mike/old-retro/old-UI scripts listed above after references are updated.

## 24. `Source`

Path: `C:\UE\T66\Source`

Inventory:
- Direct subfolders: `T66`, `T66Editor`, `T66Mini`, `T66TD`, `T66Idle`, `T66Deck`, `T66Versus`.
- Direct files: `T66.Target.cs`, `T66Editor.Target.cs`.
- Recursive contents: 85 folders, 682 files, about 7.42 MB.
- File types: 337 `.h`, 336 `.cpp`, 9 `.cs`.
- Tracked files: 693.
- `T66.uproject` declares all seven modules: `T66`, `T66Mini`, `T66TD`, `T66Idle`, `T66Deck`, `T66Versus`, and `T66Editor`.
- Current working tree has many modified/deleted/untracked `Source` files from recent work. Do not revert them as part of inventory cleanup.

General purpose:
- Core game runtime, editor tooling module, and minigame runtime modules.
- This is essential project source, not generated output.
- The cleanup target is obsolete code paths and stale naming, not deleting the `Source` folder.

Reference evidence:
- `T66.uproject` includes each source module as a runtime/editor module.
- `Source/T66/T66.Build.cs` stages WebView2, Steam app id, loose runtime UI/font roots, and source-art exceptions; this directly ties `Source` cleanup to the accepted `RuntimeDependencies` and `SourceAssets/UI` migration plans.
- `Source/T66/Core/T66PropSubsystem.cpp`, `Source/T66/Core/T66PropSubsystem.h`, `FT66PropRow` in `T66DataTypes.h`, and `T66GameMode_MainMap.cpp` still reference the old props pipeline.
- `T66RunStateSubsystem_Private.h`, `T66RunStateSubsystem_Stats.cpp`, `T66LocalizationSubsystem.cpp`, and multiple gameplay VFX files still contain Arthur-specific special cases/names.
- `Source/T66/UI/DeletedTheme` exists and `ET66UITheme::DeletedTheme` is still the forced theme from `UT66PlayerSettingsSubsystem::GetUITheme()`.
- `T66VisualUtil.cpp` points at a missing `Scripts/ReparentToFBXUnlit.py`, and `T66LeaderboardPanel.cpp` comments point at missing `Scripts/ImportLeaderboardIcons.py`.
- `Source/T66/Core/T66LegacyRuntimeTextureAccess.cpp/.h` appear to have no references outside themselves.

Recommendation:
- Keep `Source`.
- Do a code cleanup pass after the file/folder inventory is accepted, grouped by system so each deletion has paired compile/build validation.
- Do not remove minigame modules just because they are small or shell-like; `T66Mini`, `T66TD`, `T66Idle`, `T66Deck`, and `T66Versus` are declared modules and wired into the Minigames screen. Remove a minigame module only if the feature itself is intentionally cut.

Tweaks required before deletion/consolidation:
- Props removal:
  - Delete `UT66PropSubsystem` source files.
  - Remove `FT66PropRow` from `T66DataTypes.h`.
  - Remove `Core/T66PropSubsystem.h` include from `T66GameModePrivate.h`.
  - Remove the `PropSub->ClearProps()` path from `T66GameMode_MainMap.cpp`.
  - Pair this with deletion of `Content/Data/Props.csv`, `/Game/Data/DT_Props`, and related scripts/docs.
- Hero roster / Arthur cleanup:
  - Remove Hero_13 through Hero_15 data mappings where no longer needed.
  - Remove or rename Arthur-specific stat boost code; do not leave a hardcoded hidden Hero_1 test boost.
  - Rename or replace Arthur-specific VFX helper names if Hero_1 still uses the sword-style VFX; delete them only if the effect is no longer used.
  - Update localization so `Hero_1` is no longer named Arthur if Arthur is being removed.
- UI/DeletedTheme cleanup:
  - Replace `ET66UITheme::DeletedTheme` and `Source/T66/UI/DeletedTheme` with the current canonical UI theme naming.
  - Remove `DeletedThemeScreenClasses`, `DeletedThemeGameplayHUDClass`, DeletedTheme overlay class properties, and `SetUseDeletedThemePlateOverlay`/`SetDeletedThemePlateOverrideBrush` naming once equivalent current-theme code exists.
  - Replace DeletedTheme/old loose UI asset paths with the accepted Ultrakill-inspired canonical UI asset library.
- Runtime UI migration:
  - After `RuntimeDependencies` and `SourceAssets/UI` are migrated into cooked UI assets, remove or shrink `T66RuntimeUITextureAccess`, `T66RuntimeUIBrushAccess`, and `T66RuntimeUIFontAccess`.
  - Delete `T66LegacyRuntimeTextureAccess.cpp/.h` if still unreferenced.
  - Keep `T66LegacyRuntimeDataAccess` only if leaderboard summary fallback still needs it; otherwise move it into a clearer backend/leaderboard utility name or delete it.
- Script-reference cleanup:
  - Update stale comments/logs that mention missing scripts `Scripts/ReparentToFBXUnlit.py` and `Scripts/ImportLeaderboardIcons.py`.
- Data/header cleanup:
  - Consider splitting the large `T66DataTypes.h` into smaller data-domain headers after obsolete structs are removed, but do this as a low-risk refactor only after runtime cleanup stabilizes.

Good-to-delete candidates:
- Prop system:
  - `Source/T66/Core/T66PropSubsystem.cpp`
  - `Source/T66/Core/T66PropSubsystem.h`
  - `FT66PropRow` block in `Source/T66/Data/T66DataTypes.h`
- Already-deleted/obsolete world-interactable and NPC classes currently visible in git status:
  - `Source/T66/Gameplay/T66CasinoInteractable.cpp/.h`
  - `Source/T66/Gameplay/T66FountainOfLifeInteractable.h`
  - `Source/T66/Gameplay/T66ItemPickup.cpp/.h`
  - `Source/T66/Gameplay/T66StageCatchUpGoldInteractable.cpp/.h`
  - `Source/T66/Gameplay/T66StageCatchUpLootInteractable.cpp/.h`
  - `Source/T66/Gameplay/T66TeleportPadInteractable.cpp/.h`
  - `Source/T66/Gameplay/T66TreeOfLifeInteractable.cpp/.h`
  - `Source/T66/Gameplay/T66TricksterNPC.cpp/.h`
  - `Source/T66/Gameplay/T66VendorBoss.cpp/.h`
  - `Source/T66/Gameplay/T66VendorNPC.cpp/.h`
  - `Source/T66/Gameplay/T66WheelSpinInteractable.cpp/.h`
  - `Source/T66/UI/T66VendorOverlayWidget.cpp/.h`
- DeletedTheme/old theme code after current theme migration:
  - `Source/T66/UI/DeletedTheme/T66DeletedThemeTheme.cpp`
  - `Source/T66/UI/DeletedTheme/T66DeletedThemeTheme.h`
  - DeletedTheme enum values/properties/method names that remain only for the old theme.
- Arthur-specific code after current hero/VFX decisions are made:
  - `Source/T66/Gameplay/T66ArthurSwordVisuals.cpp/.h`
  - `Source/T66/Gameplay/T66ArthurUltimateSword.cpp/.h`
  - Arthur-specific stat boost and localization strings.
- Runtime loose-asset helpers after cooked UI migration:
  - `Source/T66/Core/T66LegacyRuntimeTextureAccess.cpp`
  - `Source/T66/Core/T66LegacyRuntimeTextureAccess.h`
  - possibly parts of `T66RuntimeUITextureAccess`, `T66RuntimeUIBrushAccess`, and `T66RuntimeUIFontAccess`.
- `Source/T66/UI/Style/T66CasinoVendorTabReferenceLayout.generated.h` if the vendor tab is removed and no code includes it.

Risk:
- High if deleted casually. This is compiled source.
- Deletion should be grouped into small compile-verified passes: props, hero/Arthur, UI theme/runtime assets, obsolete interactables/NPCs, then docs/scripts references.
- Any playable-build-affecting source cleanup needs a fresh staged standalone build and taskbar shortcut verification.

Decision:
- Accepted keep: entire `Source` folder and active modules.
- Accepted cleanup: remove obsolete systems and stale code paths listed above after paired data/content/script references are cleaned and compile/staged-build validation passes.

## 25. `SourceAssets`

Purpose:
- Raw/source art, source audio, generated sprite sheets, model-import staging, UI reference images, and loose runtime assets used before or alongside cooked Unreal assets.
- This folder is not just an archive today. Some children are still explicitly staged into packaged builds or read through runtime loose-file helpers.

Direct contents:
- Folders: `Arcade`, `Audio`, `Deck`, `Example VFX Full`, `Fab`, `FinalPortraits`, `GeneratedPortraits`, `Idle`, `IdolSprites`, `Import`, `ItemSprites`, `Mini`, `Preview Videos`, `ReferenceImage`, `Shikashi's Fantasy Icons Pack v2`, `Skies`, `TD`, `Trellis2`, `UI`, `WeaponSprites`.
- Files: `KnightPortrait.png`, `NewMMBackground.png`, `OuterWallTexture.png`, `Reference 5.png`.

Inventory snapshot:
- Recursive size: about 2.75 GB.
- Recursive file count: about 2,711 files across about 415 folders.
- Largest children:
  - `Import`: about 1.71 GB.
  - `Fab`: about 472 MB.
  - `UI`: about 155 MB.
  - `ItemSprites`: about 98 MB.
  - `Mini`: about 90 MB.
  - `Audio`: about 69 MB.
  - `Trellis2`: about 48 MB.
  - `TD`: about 36 MB.
  - `Preview Videos`: about 20 MB.
- Current git status inside this folder includes user/work-in-progress changes:
  - modified `SourceAssets/Mini/NPCs/Sheets/npcs_batch_01/batch_manifest.json`
  - deleted `SourceAssets/Mini/NPCs/Singles/Vendor.png`

Reference evidence:
- `.gitignore` ignores `SourceAssets/Import/`, `SourceAssets/UI/`, and `SourceAssets/Fab/VaultCache/`, but there are still tracked files inside ignored `SourceAssets/Import` and `SourceAssets/UI`.
- `Config/DefaultGame.ini` still declares `LooseRuntimeContentRoots` under:
  - `SourceAssets/Arcade`
  - `SourceAssets/UI/HeroSelection`
  - `SourceAssets/UI/Reference`
  - `SourceAssets/UI/PowerUp/Diplomas`
- `Source/T66/T66.Build.cs` stages loose runtime UI/font roots and multiple `SourceAssets/UI` paths.
- `Source/T66Mini/T66Mini.Build.cs` stages `SourceAssets/Mini` and `SourceAssets/ItemSprites`.
- `Source/T66TD/T66TD.Build.cs` stages `SourceAssets/TD`.
- `Source/T66Idle/T66Idle.Build.cs` stages `SourceAssets/Idle`.
- `Source/T66Deck/T66Deck.Build.cs` stages `SourceAssets/Deck`.
- Runtime code reads specific loose files under `SourceAssets/Arcade`, `SourceAssets/UI/Reference`, `SourceAssets/UI/HeroSelection`, `SourceAssets/UI/PowerUp/Diplomas`, `SourceAssets/ItemSprites`, `SourceAssets/Mini`, and `SourceAssets/TD`.
- `Source/T66Mini/Private/Core/T66MiniVisualSubsystem.cpp` still contains stale missing candidates such as `SourceAssets/UI/HUDGenerated/arthur_ultimate_colossal_sword.png` and old `SourceAssets/UI/MainMenuReference/...` references.
- `Source/T66/UI/Style/T66RuntimeUITextureAccess.cpp` still remaps `SourceAssets/Shikashi's Fantasy Icons Pack v2` into `RuntimeDependencies/T66/UI/Minimap`.
- `Scripts/GenerateWeaponSpritesFromSheet.py` writes to `SourceAssets/WeaponSprites`; current runtime source references were not found.
- `Scripts/GenerateNewMMBackgroundLayers.py` uses root `NewMMBackground.png`; current runtime source references were not found.
- `Scripts/SetupOuterWallTexture.py` uses root `OuterWallTexture.png`; current runtime source references were not found.

Target state:
- Keep the `SourceAssets` folder itself only as a temporary import inbox or empty placeholder.
- No permanent assets should live in this folder.
- Every current child should be handled in one of four ways:
  - import into cooked Unreal content under `Content/...`;
  - move durable process/reference documentation into `MASTER DOCS`, `UI`, or `Model Generation`;
  - move reusable source-generation workflow material into `Model Generation` if it is still part of the active pipeline;
  - delete it after verification.

Move/import/delete matrix:
- `Arcade`:
  - Current situation: live loose runtime PNGs are loaded by `T66QuickArcadeWidget`, `T66GoldMinerArcadeWidget`, `T66WhackAMoleArcadeWidget`, `T66TopwarArcadeWidget`, and `T66ArcadeSelectionWidget`; `DefaultGame.ini` also stages `SourceAssets/Arcade/...`.
  - Required work: import all arcade panel/sprite PNGs into a cooked folder such as `/Game/UI/Arcade` or `/Game/Arcade/Sprites`, update arcade widgets to load cooked textures, remove the `SourceAssets/Arcade/...` loose runtime root, then delete `SourceAssets/Arcade`.
- `Audio`:
  - Current situation: `SourceAssets/Audio/HeltonPixelCombat/Selected` has 138 WAV files, and `Content/Audio/HeltonPixelCombat` already has 138 imported assets. `SourceAssets/Audio/Arcade` has 6 WAVs and `Content/Audio/Arcade` exists.
  - Required work: verify the arcade WAVs are imported, preserve required third-party attribution/license notes outside `SourceAssets`, update `SetupAudioEventsDataTable.py` so source selection is no longer expected to live here, then delete `SourceAssets/Audio`.
- `Deck`:
  - Current situation: only a README, but `T66Deck.Build.cs` stages `SourceAssets/Deck/...`.
  - Required work: move any useful note into the deck/minigame docs that survive cleanup, remove `SourceAssets/Deck/...` staging from `T66Deck.Build.cs`, then delete `SourceAssets/Deck`.
- `Example VFX Full`:
  - Current situation: Godot-style/project sample files only; no runtime code references.
  - Required work: delete, after removing or rewriting old VFX setup scripts that mention it.
- `Fab`:
  - Current situation: 472 MB of marketplace/cache-like `.uasset`/`.umap` material with no current source/code references.
  - Required work: delete the whole folder.
- `FinalPortraits`:
  - Current situation: source portrait PNGs, including old Arthur/extra-hero variants; current hero data already points at cooked `/Game/UI/Sprites/Heroes/...` assets, though the hero roster cleanup still needs to remove Hero_13 through Hero_15 and Arthur paths.
  - Required work: verify/import the current 12 hero portraits into `/Game/UI/Sprites/Heroes`, move only a small current portrait-source note into UI/model-generation docs if needed, then delete `SourceAssets/FinalPortraits`.
- `GeneratedPortraits`:
  - Current situation: Arthur invincible generated outputs with no live references.
  - Required work: delete.
- `Idle`:
  - Current situation: only `.gitkeep`, but `T66Idle.Build.cs` stages `SourceAssets/Idle/...`.
  - Required work: remove `SourceAssets/Idle/...` staging from `T66Idle.Build.cs`, then delete `SourceAssets/Idle`.
- `IdolSprites`:
  - Current situation: fallback source PNGs still referenced by `T66MiniVisualSubsystem` for idol effects.
  - Required work: import current idol effect sprites into cooked Mini or Idol content, update `T66MiniVisualSubsystem` to use cooked paths, remove `SourceAssets/IdolSprites` fallback, then delete `SourceAssets/IdolSprites`.
- `Import`:
  - Current situation: 1.71 GB import staging. `WorldKit/CoherentThemeKit01` has 40 source modules that already correspond to 120 imported assets under `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01`. Root imports include old `Cow.glb`, `FullBody.glb`, `RoboCow.glb`, and `TeleportPad.glb`; tracked old VFX import files include `Arthur_Sword`.
  - Required work: verify each imported asset family in `Content`, update import scripts so their temporary source queue is under the cleaned `Model Generation` pipeline or a disposable task folder instead of permanent `SourceAssets/Import`, then delete `SourceAssets/Import`. `TeleportPad.glb` and `Arthur_Sword.*` are direct delete candidates aligned with earlier accepted cleanup.
- `ItemSprites`:
  - Current situation: 120 live source PNGs match 120 imported assets in `Content/Items/Sprites`, but `T66Mini.Build.cs` still stages `SourceAssets/ItemSprites/...` and `T66GameplayHUDWidget_Private.h` plus `T66MiniVisualSubsystem` still use `SourceAssets/ItemSprites` fallback paths.
  - Required work: update HUD/minigame code to use `/Game/Items/Sprites/...`, remove `SourceAssets/ItemSprites/...` staging from `T66Mini.Build.cs`, keep `ImportItemSprites.py` as a reusable importer with a non-SourceAssets input convention, then delete `SourceAssets/ItemSprites`.
- `Mini`:
  - Current situation: partly imported. `Content/Mini/Sprites` has cooked assets for Heroes, Enemies, Bosses, NPCs, and Interactables, but live code still falls back to loose `SourceAssets/Mini` for background, hero/companion animations, projectiles, companions, idol effects, effects, HUD icons, and item fallbacks. `T66Mini.Build.cs` stages the folder.
  - Required work: import missing Mini runtime art into cooked `Content/Mini` or `Content/Mini/Sprites` folders, update `T66MiniVisualSubsystem` to load cooked assets with no loose fallback, remove stale Arthur/missing `HUDGenerated` and `MainMenuReference` candidates, remove `SourceAssets/Mini/...` staging, then delete `SourceAssets/Mini`.
  - Current user work to preserve while planning: `SourceAssets/Mini/NPCs/Sheets/npcs_batch_01/batch_manifest.json` is modified and `SourceAssets/Mini/NPCs/Singles/Vendor.png` is already deleted in git status. Do not revert those.
- `Preview Videos`:
  - Current situation: one MP4, no live references.
  - Required work: delete, unless the video is intentionally moved to a docs/reference folder before the final cleanup.
- `ReferenceImage`:
  - Current situation: one source PNG, no live references.
  - Required work: delete.
- `Shikashi's Fantasy Icons Pack v2`:
  - Current situation: only still relevant through a legacy remap in `T66RuntimeUITextureAccess` to minimap runtime dependencies.
  - Required work: move/import minimap icons/backgrounds/walls into cooked `/Game/UI/Minimap` assets as part of the accepted `RuntimeDependencies` cleanup, remove the remap, then delete this folder.
- `Skies`:
  - Current situation: empty.
  - Required work: delete.
- `TD`:
  - Current situation: not imported enough. `Content/TD` currently holds data, while `Content/TD/Data/T66TD_Maps.csv` still stores `SourceAssets/TD/Maps/Backgrounds/...` paths; `T66TDVisualSubsystem` and `T66TDMainMenuScreen` load `SourceAssets/TD/...` loose PNGs; `T66TD.Build.cs` stages the folder.
  - Required work: import TD heroes, enemies, bosses, map backgrounds, and main-menu UI into cooked `/Game/TD/...` assets; update `T66TD_Maps.csv`, `T66TDVisualSubsystem`, and `T66TDMainMenuScreen` to cooked asset paths; remove `SourceAssets/TD/...` staging; then delete `SourceAssets/TD`.
- `Trellis2`:
  - Current situation: Arthur-specific Trellis output with no live references.
  - Required work: delete.
- `UI`:
  - Current situation: `Reference`, `HeroSelection`, and `PowerUp/Diplomas` are live loose runtime assets. `Cursors` appears redundant because cursor PNGs already exist under `Content/Slate`. `Reference/archive` and old compatibility paths contain stale UI-generation history.
  - Required work: import accepted UI runtime art into cooked `/Game/UI/...` assets, update `T66RuntimeUITextureAccess`, `T66RuntimeUIBrushAccess`, Slate helpers, screen code, and build/config staging to use cooked assets, move only useful UI-generation process docs into the surviving `UI` or `MASTER DOCS` documentation, delete old `Reference/archive`, delete `Cursors`, and finally delete `SourceAssets/UI`.
- `WeaponSprites`:
  - Current situation: generated source PNGs with imported assets already present under `Content/Weapons/Sprites`; current runtime source references were not found.
  - Required work: verify weapon UI/runtime data uses cooked `/Game/Weapons/Sprites` assets, update `GenerateWeaponSpritesFromSheet.py` to output to a temporary task folder or direct import queue, then delete `SourceAssets/WeaponSprites`.
- Root files:
  - `OuterWallTexture.png`: already has an import script targeting `/Game/World/Cliffs`; verify imported asset, update/remove script dependency, then delete.
  - `NewMMBackground.png`: generated input only; verify current main-menu screen art exists in cooked/current UI reference path, move any useful source note into `UI` docs, then delete.
  - `KnightPortrait.png` and `Reference 5.png`: no live references; delete.

Implementation order for the later cleanup:
1. Delete obvious no-reference content first: `Fab`, `GeneratedPortraits`, `Preview Videos`, `ReferenceImage`, `Skies`, `Trellis2`, `KnightPortrait.png`, `Reference 5.png`.
2. Remove empty/placeholder staging: `Deck` and `Idle` Build.cs staging, then delete those folders.
3. Finish imports for already mostly migrated assets: `Audio`, `ItemSprites`, `WeaponSprites`, `OuterWallTexture.png`.
4. Migrate runtime loose art: `Arcade`, `Mini`, `TD`, and `UI`.
5. Delete `Import` only after imported Content verification and import-script path cleanup.
6. Run compile, cook/stage standalone, and taskbar shortcut verification after any playable-build-affecting migration.

Risk:
- High if deleted casually because this folder currently participates in packaged runtime staging.
- The cleanup target is still aggressive: keep the folder itself, but empty it after each live dependency has been imported, moved, or retired.

Decision:
- Accepted target-state: keep the `SourceAssets` folder only as an empty placeholder/import inbox.
- Accepted cleanup direction: all current contents are marked for deletion after the move/import/code-reference work above is completed.

## 26. `ThirdParty`

Purpose:
- External runtime/build dependencies that are not Unreal project content.
- Current actual use is only Microsoft WebView2 support for the in-game short-form video Media Viewer.

Direct contents:
- `WebView2`

Inventory snapshot:
- Recursive size: about 2.79 MB.
- Recursive folders: 4.
- Recursive files: 3.
- Files:
  - `ThirdParty/WebView2/bin/Win64/WebView2Loader.dll`
  - `ThirdParty/WebView2/include/WebView2.h`
  - `ThirdParty/WebView2/include/WebView2EnvironmentOptions.h`

Reference evidence:
- `Source/T66/T66.Build.cs` adds `ThirdParty/WebView2/include` to public include paths on Win64, stages `WebView2Loader.dll` next to the executable, links Win32/COM libraries, and defines `T66_WITH_WEBVIEW2=1`.
- `Source/T66/Core/T66WebView2Host.cpp` includes `<WebView2.h>`, loads `WebView2Loader.dll`, and falls back to `ThirdParty/WebView2/bin/Win64/WebView2Loader.dll` if the loader is not next to the executable.
- `Source/T66/Core/T66MediaViewerSubsystem.*` owns the Media Viewer and WebView2 lifecycle.
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Overlays.cpp`, `T66GameplayHUDWidget_Build.cpp`, settings screens, player-controller input, player settings, and localization all reference the TikTok/Shorts/Reels Media Viewer feature.
- `Config/DefaultInput.ini` binds `ToggleTikTok` and `ToggleMediaViewer`.
- Master docs still describe `ThirdParty/` as a root dependency folder because this WebView2 path currently exists.

Recommendation:
- Do not delete `ThirdParty` while the Media Viewer feature is still in the game.
- If we want maximum cleanup and the Media Viewer is not a release feature, delete the entire `ThirdParty` folder by cutting the Media Viewer/WebView2 feature at the same time.
- If the Media Viewer stays, keep only `ThirdParty/WebView2` and keep this folder tight: no caches, no SDK extras, no generic marketplace or source art storage.

Good-to-delete path:
- Delete the whole `ThirdParty` folder only after this paired cleanup:
  - Remove `FT66WebView2Host` source files.
  - Remove WebView2 include path, Win32/COM libraries, `WebView2Loader.dll` staging, and `T66_WITH_WEBVIEW2` definition from `T66.Build.cs`.
  - Remove or stub out `UT66MediaViewerSubsystem` if the whole Media Viewer is cut.
  - Remove Media Viewer HUD overlay, settings tab, input bindings, player settings fields, localization text, and saved setting migration/defaults.
  - Remove `ToggleTikTok` and `ToggleMediaViewer` from `Config/DefaultInput.ini`.
  - Remove `Saved/TikTokWebView2` generated data during the later `Saved` cleanup.
  - Update master docs/guidelines so `ThirdParty` is no longer listed as a required root dependency.

Keep path:
- Keep all three current WebView2 files if Media Viewer remains.
- No subfolder cleanup is needed inside `ThirdParty/WebView2`; it is already minimal.

Risk:
- Medium if deleting with the feature intentionally cut; high if deleting without code cleanup because Win64 builds and packaged Media Viewer runtime behavior depend on it.
- Any change here affects the playable standalone build and needs compile plus staged standalone/taskbar shortcut verification.

Decision:
- Accepted keep: `ThirdParty/WebView2`, because Media Viewer is the TikTok/YouTube Shorts/Instagram Reels feature and it should stay.
- Revisit deletion only if the Media Viewer feature is intentionally cut later.

## 27. `tmp`

Purpose:
- Temporary workspace, generated diagnostics, downloaded/cloned research caches, old validation screenshots/logs, and one-off helper scripts.
- This folder is ignored by git and should not contain durable project assets.

Direct contents:
- Files:
  - old Arthur HUD/UI validation screenshots and logs
  - old staged/tower packaged smoke logs
  - one-off asset-library report/helper scripts and JSON outputs
- Folders:
  - `guest_diag_102617`
  - `hud_art_preview`
  - `imagegen`
  - `latest_guest_diag`
  - `worldgen_research`

Inventory snapshot:
- Recursive size: about 692.56 MB.
- Recursive files: about 426.
- Recursive folders: about 64.
- Largest child:
  - `tmp/worldgen_research`: about 681.48 MB.
- `git ls-files tmp` returns 0 tracked files.
- `.gitignore` ignores `tmp/`.

Reference evidence:
- No current source/config/script references were found for `tmp`.
- `World Generation` docs exist and contain the durable HY-World / WorldMirror research notes:
  - `World Generation/MASTER.md`
  - `World Generation/MEMORY.md`
  - `World Generation/SETUP.md`
  - `World Generation/ROOM_GENERATION_PROCESS.md`
  - `World Generation/USEFUL_LINKS.md`
- Several `World Generation` docs still link to local cached files under `C:/UE/T66/tmp/worldgen_research`, including the cached `HY_World_2_0.pdf` and cloned `HY-World-2.0_repo`.

Recommendation:
- Delete the entire `tmp` folder.
- Before deletion, update the `World Generation` docs so any local `tmp/worldgen_research` links are replaced with official web links, instructions to reclone/redownload when needed, or a short note that the local cache was intentionally removed during cleanup.
- Do not preserve old Arthur screenshots/logs, packaged smoke logs, `guest_diag_*`, `latest_guest_diag`, empty `hud_art_preview`, or one-off asset-library scripts in their current location.

Good-to-delete candidates:
- Entire folder:
  - `tmp`
- Specific high-value cleanup:
  - `tmp/worldgen_research/HY-World-2.0_repo`
  - `tmp/worldgen_research/HY_World_2_0.pdf`
  - old Arthur validation screenshots/logs
  - old tower packaged smoke logs
  - old guest diagnostics
  - old `tmp/imagegen` generated previews
  - one-off helper scripts and JSON report outputs

Tweaks required before deletion:
- Update `World Generation/MASTER.md`, `World Generation/SETUP.md`, `World Generation/USEFUL_LINKS.md`, and `World Generation/MEMORY.md` where they point to `C:/UE/T66/tmp/worldgen_research`.
- If any world-generation research cache is needed again, recreate it outside the repo or through a documented temporary setup command instead of keeping it in project `tmp`.

Risk:
- Low. It is git-ignored, untracked, and no runtime/build references were found.
- Documentation links need cleanup first so the docs do not point at files we intentionally remove.

Decision:
- Accepted delete: entire `tmp` folder after local-cache links in `World Generation` docs are updated.

## 28. `Tools`

Purpose:
- Repo helper tools that are not Unreal runtime code.
- Current contents mix durable tools, ignored build/test output, old logs, Python caches, and one large temporary utility stack.

Direct contents:
- `Codex`
- `Items`
- `Logs`
- `Mini`
- `Steam`
- `Temp`

Inventory snapshot:
- Recursive size: about 10.34 GB.
- Recursive files: about 8,012.
- Recursive folders: about 1,061.
- Tracked files: 9.
- Ignored/generated bulk:
  - `Tools/Temp`: about 10.34 GB.
  - `Tools/Logs`: small ignored build/stage logs.
  - Python `__pycache__` folders under tracked tool folders.

Tracked files:
- `Tools/Codex/powerup_auto_mask.py`
- `Tools/Codex/powerup_statue_masks.py`
- `Tools/Items/T66ProcessReimaginedItemSheets.py`
- `Tools/Mini/T66MiniBuildWalkComparison.py`
- `Tools/Mini/T66MiniNormalizeWalkSheet.py`
- `Tools/Mini/T66MiniPrepareEnemyBossAssets.py`
- `Tools/Mini/T66MiniSplitHeroSheets.py`
- `Tools/Mini/T66MiniSplitSheets.py`
- `Tools/Steam/UploadToSteam.ps1`

Reference evidence:
- `.gitignore` ignores `Tools/Logs/`, `Tools/Temp/`, `**/__pycache__/`, and `*.pyc`.
- `MASTER DOCS/MASTER_STEAMWORKS.md` and `MASTER DOCS/MASTER_BACKEND.md` reference `Tools/Steam/UploadToSteam.ps1`.
- `UI/Sprites/Sprite Retro Process.md` references `Tools/Items/T66ProcessReimaginedItemSheets.py`.
- `Docs/Mini/T66Mini_WalksheetPipeline.md`, `Docs/Mini/T66Mini_Memory_Progression.md`, and `Docs/Minigames/T66Minigame_CharacterAnimationProcess.md` reference `Tools/Mini` scripts or planned shared `Tools/Minigames` scripts.
- `Tools/Codex/powerup_statue_masks.py` references old example scratch paths under `output/...`; earlier inventory already noted that this should be updated if the script survives.
- `MASTER DOCS/MASTER_STEAMWORKS.md` contains stale references to `Tools/ChatGPTBridge` logs, but no `Tools/ChatGPTBridge` folder exists.

Recommendation:
- Keep the `Tools` folder, but reduce it to durable reusable tools only.
- Delete all ignored/generated contents now during the later cleanup.
- Do not treat `Tools` as a generic dumping folder. It should only contain durable operator tools that are not Unreal editor/runtime scripts.
- Script-like tools must follow the same lifecycle rule accepted for `Scripts` and `Model Generation`: master/reusable scripts stay tight and documented; task-specific scripts are deleted once the task is accomplished and any durable process improvements are written into the master script/readme/docs.
- Reclassify the tracked scripts when the broader script organization pass happens:
  - Steam release wrapper should stay, moved to `Tools/Release/Steam`.
  - Mini sprite tools should move to `Tools/ArtPipeline/Minigames` if they remain reusable.
  - Item sprite reimagining tool should move to `Tools/ArtPipeline/Items` if it remains reusable, and it must stop outputting to permanent `SourceAssets` paths.
  - PowerUp mask tools should move to `Tools/ArtPipeline/UI/PowerUp` only if the statue/diploma reveal pipeline still needs them; otherwise delete them as one-off Codex helpers.

Target organization:
- `Tools/README.md`: states the tool/script lifecycle rule and lists durable tools.
- `Tools/Release/Steam/UploadToSteam.ps1`
- `Tools/ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
- `Tools/ArtPipeline/Minigames/T66MiniBuildWalkComparison.py`
- `Tools/ArtPipeline/Minigames/T66MiniNormalizeWalkSheet.py`
- `Tools/ArtPipeline/Minigames/T66MiniPrepareEnemyBossAssets.py`
- `Tools/ArtPipeline/Minigames/T66MiniSplitHeroSheets.py`
- `Tools/ArtPipeline/Minigames/T66MiniSplitSheets.py`
- `Tools/ArtPipeline/UI/PowerUp/*.py` only if still needed; otherwise delete.
- No `Tools/Temp`, `Tools/Logs`, cache folders, or task-specific one-off scripts after cleanup.

Good-to-delete candidates:
- Delete outright:
  - `Tools/Temp`
  - `Tools/Logs`
  - all `Tools/**/__pycache__`
  - all `Tools/**/*.pyc`
- Delete or move after script consolidation:
  - `Tools/Codex/powerup_auto_mask.py`
  - `Tools/Codex/powerup_statue_masks.py`
- Delete stale doc references:
  - references to missing `Tools/ChatGPTBridge` logs in `MASTER DOCS/MASTER_STEAMWORKS.md`

Keep candidates:
- `Tools/Steam/UploadToSteam.ps1` because Steam release docs rely on it.
- `Tools/Mini/*.py` if the minigame sprite pipeline remains active.
- `Tools/Items/T66ProcessReimaginedItemSheets.py` if the item sprite generation/reimagining process remains active, but it should be updated to follow the new no-permanent-SourceAssets target.

Risk:
- Low for `Tools/Temp`, `Tools/Logs`, and caches because they are ignored/generated and untracked.
- Medium for tracked scripts because docs still reference some of them and they represent reusable pipeline work.
- No staged standalone build is required for deleting ignored tool output, but changing release scripts or imported asset pipelines should be validated by the relevant workflow.

Decision:
- Accepted keep: `Tools` folder as a small durable-tools root, not a generic script dump.
- Accepted delete: `Tools/Temp`, `Tools/Logs`, caches, and stale missing-tool references.
- Accepted reorganization: move surviving tracked tools into domain folders such as `Tools/Release/Steam`, `Tools/ArtPipeline/Items`, `Tools/ArtPipeline/Minigames`, and optionally `Tools/ArtPipeline/UI/PowerUp`.
- Accepted lifecycle rule: task-specific tool scripts should be deleted after the task is accomplished, with any reusable process improvements moved into the master script/readme/docs first.

## 29. `UI`

Purpose:
- Current repo-side UI generation and reference workspace.
- It is not Unreal runtime source code and should not be a permanent dump for screenshots, rejected generations, per-screen proof images, or old prompt bundles.
- The useful part is the small current process documentation for making UI reference assets and comparing them against the staged standalone build.

Direct contents:
- `MASTER_REFERENCE_UI_GENERATION_PROMPT.md`
- `MEMORY.md`
- `README.md`
- `SCREEN_WORKFLOW.md`
- `archive`
- `generation`
- `Reference`
- `screens`

Inventory snapshot:
- Recursive size: about 4.12 GB.
- Recursive files: about 4,825.
- Recursive folders: about 1,853.
- Tracked files: 3,025.
- Current git status entries under `UI`: about 80, mostly existing deletions/reorg work that should not be reverted.
- Major size centers:
  - `UI/archive`: about 2.37 GB, 2,908 files, 791 folders.
  - `UI/screens`: about 813.82 MB, 598 files, 397 folders.
  - `UI/Reference`: about 805.02 MB, 1,224 files, 649 folders.
  - `UI/generation`: about 123.95 MB, 91 files, 12 folders.

Reference evidence:
- `.gitignore` already treats the high-churn UI output areas as generated or non-source:
  - `UI/archive/`
  - `UI/generation/`
  - `UI/screens/**/*.png`
  - `UI/screens/**/*.jpg`
  - `UI/screens/**/*.jpeg`
  - `UI/screens/**/_capture_results.json`
  - `UI/Reference/**/*.png`
- `UI/README.md`, `UI/SCREEN_WORKFLOW.md`, and `UI/Reference/SCREEN_MODAL_TASK.md` describe the current UI generation workflow.
- `Docs/UI/UI_GENERATION.md` overlaps with the root UI docs, but the `Docs` folder is already marked for deletion. Any still-useful instructions from that doc should be folded into the surviving `UI` docs or `MASTER DOCS`.
- `UI/MEMORY.md` contains a narrower old UI-generation rule. That rule should be folded into the current master UI prompt/workflow if still wanted, then the separate memory file can be deleted.
- Runtime code still depends on some loose assets under `SourceAssets/UI/Reference/...`; that belongs to the later `SourceAssets` and cooked UI migration, not to keeping root `UI` output folders forever.
- During this pass, the four live Deck/Idle minigame mockups were imported into cooked UI assets:
  - `/Game/UI/Minigames/Deck/Mockups/T_Deck_MainMenu_Mockup`
  - `/Game/UI/Minigames/Deck/Mockups/T_Deck_Gameplay_Mockup`
  - `/Game/UI/Minigames/Idle/Mockups/T_Idle_MainMenu_Mockup`
  - `/Game/UI/Minigames/Idle/Mockups/T_Idle_Gameplay_Mockup`
- `Source/T66Deck/T66Deck.Build.cs` and `Source/T66Idle/T66Idle.Build.cs` no longer stage the old `UI/screens/minigames/...` PNGs.
- `Source/T66Deck/Private/UI/Screens/T66DeckMainMenuScreen.cpp` and `Source/T66Idle/Private/UI/Screens/T66IdleMainMenuScreen.cpp` now load those cooked assets through `T66RuntimeUITextureAccess::LoadAssetTexture`.
- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` succeeded after the asset/code migration, and both `T66 Standalone.lnk` shortcuts were refreshed to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Current source/config no longer has runtime references to `UI/screens/minigames/...`; remaining `UI/screens` references are documentation/workspace history and one historical Steam proof screenshot path.
- `MASTER DOCS/MASTER_STEAMWORKS.md` has one historical proof screenshot path under `UI/screens/main_menu/outputs/...`.

Recommendation:
- Treat `UI` as the future UI component documentation/process folder in the broader component-docs cleanup plan.
- Keep the `UI` folder only as a small UI docs/process workspace plus any UI-specific process subfolders.
- Keep or consolidate:
  - `UI/README.md`
  - `UI/SCREEN_WORKFLOW.md`
  - `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md`
  - `UI/Reference/SCREEN_MODAL_TASK.md`
- Keep the moved sprite workflow doc:
  - `UI/Sprites/Sprite Retro Process.md`
- Target UI documentation shape:
  - `UI/README.md`: short index.
  - `UI/MASTER_UI.md` or a clearly named equivalent master file for current UI rules.
  - `UI/Processes/Reference Generation.md` or renamed current `MASTER_REFERENCE_UI_GENERATION_PROMPT.md` for screen/reference generation.
  - `UI/Sprites/Sprite Retro Process.md` for sprite/icon retro processing.
  - No old per-screen reference prompts, proof notes, capture histories, or generated-reference docs once durable rules are merged into the surviving masters.
- Delete or consolidate the old standalone `UI/MEMORY.md` after moving any still-useful rule into the surviving UI workflow or master prompt.
- Delete `UI/archive` entirely. It is mostly rejected/old prompt and screenshot history, and the repo already has Git history for old versions.
- Delete `UI/generation` after any currently needed locked full-screen references are either no longer needed or summarized into the active workflow. It should be treated as generated output, not source.
- Delete most or all of `UI/screens`. The Deck/Idle runtime blocker has been removed; only documentation/workspace references remain. Do not keep packaged proof images, contact sheets, capture JSON, old review screenshots, or per-screen scratch folders in the repo.
- Heavily reduce `UI/Reference`:
  - Keep only master-level docs or current process files after merging any useful information.
  - Delete `UI/Reference/Archive`.
  - Delete old per-screen prompt bundles, reference `.md` files, proof screenshots, generated candidates, history folders, and stale difference/proof artifacts after any durable lessons are folded into the master prompt/workflow.
  - Do not keep runtime image assets here; accepted runtime UI art should be migrated out of loose source paths into cooked `/Game/UI` assets during the later build cleanup stage.

Good-to-delete candidates:
- Delete outright after documenting the decision:
  - `UI/archive`
  - `UI/generation`
  - `UI/Reference/Archive`
  - generated PNG/JPG proof output under `UI/screens`
  - capture result JSON and old screen review screenshots under `UI/screens`
  - old prompt bundles under `UI/archive` and `UI/Reference/**/PROMPT.md`
- Delete or shrink after consolidation:
  - `UI/MEMORY.md`
  - `Docs/UI/UI_GENERATION.md` references from the surviving UI docs
  - old per-screen manifests that only record completed implementation history
- Delete now during the later deletion phase:
  - `UI/screens/minigames/chadpocalypse_deckbuilder/reference/*.png` because Deck now uses cooked `/Game/UI/Minigames/Deck/Mockups` assets.
  - `UI/screens/minigames/idle_chadpocalypse/reference/*.png` because Idle now uses cooked `/Game/UI/Minigames/Idle/Mockups` assets.
  - historical `UI/screens/main_menu/outputs/...` reference in `MASTER DOCS/MASTER_STEAMWORKS.md`.

Tweaks required before deletion:
- Merge any still-current content from `Docs/UI/UI_GENERATION.md` into `UI/SCREEN_WORKFLOW.md`, `UI/MASTER_REFERENCE_UI_GENERATION_PROMPT.md`, or a future `UI/MASTER_UI.md` before the `Docs` folder is deleted.
- Decide whether the locked reference full-screen set is still needed. If yes, keep a tiny curated subset in `UI/Reference` or move it to a documented external/non-repo working location; do not keep the whole generated folder.
- For the later build cleanup stage, migrate accepted loose UI art from `SourceAssets/UI/Reference/...` into cooked `/Game/UI` assets and remove loose runtime staging paths.
- Remove or update documentation references that still point at `UI/screens` as a current workflow root. Future captures should go to temporary output or a tiny current-review location, not a permanent tracked reference tree.

Risk:
- Low for deleting old archives, proof images, contact sheets, capture JSON, rejected generations, and old prompt bundles.
- Low-to-medium for deleting `UI/screens` after the current doc references are updated; source/config no longer needs it.
- Medium for deleting all `UI/Reference` immediately because it contains current workflow docs and some active manifests, even though the PNG bulk and reference docs should go.
- No staged standalone build is required for deleting old UI documentation/output only. Changing Deck/Idle runtime image paths or the loose UI runtime asset path does affect the playable standalone build and must follow the AGENTS staged-build shortcut rule.

Decision:
- Accepted keep: `UI` as a small UI component docs/process folder.
- Accepted move: root `Retro Process.md` is now `UI/Sprites/Sprite Retro Process.md`.
- Accepted cooked migration complete: Deck/Idle mockups were imported to `/Game/UI/Minigames/...`, source code now loads cooked textures, and standalone staging/shortcut refresh succeeded.
- Accepted delete: more than 90% of current `UI` contents should go during cleanup, including `UI/archive`, `UI/generation`, `UI/Reference/Archive`, old reference `.md` docs, reference screens, proof/capture images, and scratch folders.
- Accepted consolidation: keep only master/current UI process docs and the actual current UI elements used by screens; fold `Docs/UI/UI_GENERATION.md` and `UI/MEMORY.md` into the surviving UI docs before deleting them.

## 30. `Video Generation`

Purpose:
- One-off handoff workspace for generating an animated main-menu background video with Kling.
- It is not runtime game content and is not currently part of the build.
- It overlaps with the future UI component/process documentation structure and should not remain as a top-level component folder.

Direct contents:
- `API_STATUS.md`
- `NEXT_STEPS.md`
- `PROMPTS.md`
- `README.md`
- `scripts/regenerate_kling_masks.py`
- `scripts/submit_kling_motion_brush.py`

Inventory snapshot:
- Recursive size: about 0.017 MB.
- Recursive files: 6.
- Recursive folders: 1.
- Tracked files: 6.
- Current git status entries under `Video Generation`: 0.

Reference evidence:
- No repo references to `Video Generation` were found outside the folder itself.
- The docs describe an unfinished main-menu-video experiment using Kling Video 3.0 or `kling-v1` Motion Brush.
- The docs say the API path was blocked by Kling account balance, not by current game implementation.
- The two scripts are task-specific and hardcoded to `C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM`.
- That hardcoded source directory does not currently exist.
- The current main menu code still loads loose PNGs from `SourceAssets/UI/Reference/Screens/MainMenu/ScreenArt/...`, not from `Video Generation`.
- No current `Content/Movies/MainMenuBackground.mp4` asset was found.

Recommendation:
- Delete the top-level `Video Generation` folder after salvaging any still-useful process guidance.
- If animated main-menu video remains a planned UI feature, move the durable guidance into the UI component docs as something like `UI/Processes/Main Menu Video Background.md`.
- In that moved doc, update stale paths to the current main-menu source plate:
  - `SourceAssets/UI/Reference/Screens/MainMenu/ScreenArt/mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_1920.png`
  - final cooked target: `Content/Movies/MainMenuBackground.mp4`
- Do not keep `API_STATUS.md` as a long-term doc. It is a dated account/API-balance snapshot.
- Do not keep `NEXT_STEPS.md` as a long-term doc. If the video background is still wanted, turn it into a clean process checklist inside the UI docs.
- Do not keep `PROMPTS.md` as a separate top-level file. Merge the useful prompt text into the UI process doc if still relevant.
- Do not keep the two `scripts/*.py` files as-is. They are hardcoded to a missing source path and represent a specific Kling attempt.
- If Motion Brush support becomes a reusable workflow later, rewrite the scripts into a durable tool under `Tools/ArtPipeline/UI/MainMenu` with configurable paths and documented usage. Otherwise delete them under the script lifecycle rule.

Good-to-delete candidates:
- Delete after doc salvage:
  - `Video Generation/API_STATUS.md`
  - `Video Generation/NEXT_STEPS.md`
  - `Video Generation/PROMPTS.md`
  - `Video Generation/README.md`
  - `Video Generation/scripts/regenerate_kling_masks.py`
  - `Video Generation/scripts/submit_kling_motion_brush.py`
  - `Video Generation/scripts`
  - `Video Generation`

Tweaks required before deletion:
- Decide whether main-menu video generation is still a planned feature.
- If yes, create or merge into `UI/Processes/Main Menu Video Background.md` and update the stale paths before deleting the top-level folder.
- If no, delete the whole folder with no code changes.
- Later build cleanup should separately migrate the current loose main-menu PNGs from `SourceAssets/UI/Reference/...` into cooked UI assets or a movie asset path; that is not a reason to keep this top-level folder.

Risk:
- Low if the animated main-menu-video experiment is no longer pending.
- Medium if the user still wants the main-menu video soon, because useful prompt wording and API notes should be salvaged first.
- No staged standalone build is required for deleting this docs/scripts workspace only.

Decision:
- Recommended delete/rehome: remove `Video Generation` as a top-level folder.
- Recommended keep only if still wanted: a small UI process doc under `UI/Processes`, with current paths and no dated API-status snapshot.
- Recommended delete: both task-specific Kling scripts unless they are rewritten later as durable configurable tools.

## 31. `World Generation`

Purpose:
- Documentation workspace for HY-World / WorldMirror research, generated-room experiments, and the current modular dungeon/environment-kit process.
- This is not runtime Unreal content and contains no raw generated run outputs right now.
- It is more useful than the old archive/reference folders, but it should be compacted into clean component docs instead of staying as several overlapping master/memory/setup files.

Direct contents:
- `MASTER.md`
- `MEMORY.md`
- `MODULAR_DUNGEON_KIT_PROCESS.md`
- `ROOM_GENERATION_PROCESS.md`
- `SETUP.md`
- `SHARED_ASSET_PIPELINE.md`
- `USEFUL_LINKS.md`

Subfolders:
- None.

Inventory snapshot:
- Recursive size: about 68.36 KB.
- Recursive files: 7.
- Recursive folders: 0.
- Tracked files: 7.
- Current git status entries under `World Generation`: 5 modified files that already existed before this inventory pass and should not be reverted during cleanup.

Reference evidence:
- `Model Generation/README.md` and `Model Generation/WALLS_FLOORS_CEILINGS.md` point at:
  - `World Generation/MODULAR_DUNGEON_KIT_PROCESS.md`
  - `World Generation/SHARED_ASSET_PIPELINE.md`
- `MODULAR_DUNGEON_KIT_PROCESS.md` is the most current production-facing document in the folder. It describes the active modular dungeon kit direction, runtime triangle budgets, visual-mesh plus hidden-collision-proxy rule, and CoherentThemeKit01 lag fix.
- `SHARED_ASSET_PIPELINE.md` contains useful cross-cutting rules, but it overlaps with the `Model Generation` instruction cleanup plan and should probably become part of the canonical model-generation instruction set.
- `MASTER.md`, `SETUP.md`, `ROOM_GENERATION_PROCESS.md`, `MEMORY.md`, and `USEFUL_LINKS.md` still contain stale local-cache references to `C:/UE/T66/tmp/worldgen_research`, which is already marked for deletion.
- `MASTER.md` and `MEMORY.md` duplicate release-boundary and decision history.
- `SETUP.md` mixes TRELLIS modular-kit setup, HY-World/WorldMirror setup, local cache notes, and RunPod commands in one long doc.
- `ROOM_GENERATION_PROCESS.md` is a full generated-room prototype plan. It is useful only if that experiment is still planned; it is not current runtime implementation.
- `USEFUL_LINKS.md` has useful official links, but also links to local `tmp/worldgen_research` clone/PDF files that should not survive the tmp cleanup.
- No `World Generation/Runs` folder exists right now, which is good for cleanup. Future run outputs should be treated like generated artifacts and not become permanent repo clutter.

Recommendation:
- Keep `World Generation` only if it remains one of the top-level component documentation folders. Otherwise move the surviving docs under the future component-docs structure, likely `Gameplay/World/` or `Gameplay/World Generation/`.
- Keep and polish the modular dungeon kit process as the main durable doc:
  - `World Generation/MODULAR_DUNGEON_KIT_PROCESS.md`
  - or moved/renamed to `Gameplay/World/Modular Dungeon Kit Process.md`
- Merge `SHARED_ASSET_PIPELINE.md` into the canonical `Model Generation/Instructions` docs and/or the surviving world-generation master doc, then delete the duplicate standalone file.
- Replace `MASTER.md` with a much shorter index/master file after consolidation. It should point to the current modular-kit process, the model-generation instructions, and official HY-World links only.
- Delete `MEMORY.md` after moving any still-current locked decisions into the surviving master/index file. This repo should not keep multiple topical memory files now that `AGENTS.md` is the automatic always-read rule file.
- Delete or heavily shrink `ROOM_GENERATION_PROCESS.md` unless generated-room work is still active. If kept, move it under a `Research/HY-World` or `Experimental` subfolder and mark it clearly as a future/research process, not current implementation.
- Delete or heavily shrink `SETUP.md` after splitting it:
  - TRELLIS modular-kit setup belongs in `Model Generation/Instructions`.
  - HY-World/WorldMirror setup belongs in a small research doc only if that experiment remains active.
- Delete or merge `USEFUL_LINKS.md`. Keep official web links in the relevant process doc, and remove all local-cache links to `tmp/worldgen_research`.
- Update `Model Generation/README.md` and `Model Generation/WALLS_FLOORS_CEILINGS.md` if any surviving world-generation docs are moved or renamed.

Good-to-delete candidates:
- Delete after consolidation:
  - `World Generation/MEMORY.md`
  - `World Generation/SHARED_ASSET_PIPELINE.md`
  - `World Generation/USEFUL_LINKS.md`
- Delete or move to research-only docs after deciding whether generated-room HY-World work is still planned:
  - `World Generation/ROOM_GENERATION_PROCESS.md`
  - `World Generation/SETUP.md`
- Delete stale local-cache links inside:
  - `World Generation/MASTER.md`
  - `World Generation/MEMORY.md`
  - `World Generation/SETUP.md`
  - `World Generation/USEFUL_LINKS.md`
- Delete any future generated output folders under `World Generation/Runs` after accepted assets are moved/imported into the proper model-generation or Unreal content pipeline. No such folder exists currently.

Keep candidates:
- Keep the modular dungeon/environment-kit process, because it is current and referenced by `Model Generation`.
- Keep a short world-generation index/master doc only after it is rewritten as a clean component entry point.
- Keep official HY-World links only as references, not as a reason to keep the local cached repo/PDF under `tmp`.

Tweaks required before deletion:
- Decide the target component-doc location:
  - keep top-level `World Generation`, or
  - move surviving docs under `Gameplay/World` / `Gameplay/World Generation`.
- Move reusable TRELLIS/modular-kit setup rules into the canonical `Model Generation/Instructions` cleanup plan.
- Remove or replace all links to `C:/UE/T66/tmp/worldgen_research`.
- Update `Model Generation/README.md` and `Model Generation/WALLS_FLOORS_CEILINGS.md` after any move/rename.
- If HY-World work resumes later, recheck official sources before trusting the April 2026 release-status notes in these docs.

Risk:
- Low for deleting `MEMORY.md` and `USEFUL_LINKS.md` after useful decisions/links are merged.
- Low-to-medium for deleting `ROOM_GENERATION_PROCESS.md` if no generated-room experiment is pending.
- Medium for deleting or moving `MODULAR_DUNGEON_KIT_PROCESS.md` without updating `Model Generation` references, because it is the current environment-kit source of truth.
- No staged standalone build is required for documentation-only moves or deletions.

Decision:
- Recommended keep/rewrite: one clean world-generation or gameplay-world component doc set.
- Recommended keep: modular dungeon/environment-kit process.
- Recommended delete after consolidation: topical `MEMORY.md`, duplicate shared pipeline doc, useful-links-only doc, and stale local-cache references.
- Recommended move/merge: TRELLIS and shared generation rules into the canonical `Model Generation/Instructions` cleanup.

## Alpha 0.1 Worker A Implementation Notes - 2026-05-08

Documentation/component cleanup implemented for the accepted doc-folder scope. No Source, Content, Config, top-level Scripts, Tools, RuntimeDependencies, SourceAssets, or binary Unreal assets were edited by this Worker A pass. No builds or git commands were run.

Created component documentation roots:

- `Backend/`
- `Backend/Anti Cheat/`
- `Backend/Community/`
- `Gameplay/`
- `Gameplay/Audio/`
- `Gameplay/Camera/`
- `Gameplay/Combat/`
- `Gameplay/Minigames/`
- `Gameplay/Movement/`
- `Gameplay/Stats/`
- `Gameplay/Traps/`
- `Gameplay/World/`
- `Release/`
- `Release/QA/`
- `Release/Steam/`
- `UI/Processes/`
- `Model Generation/Instructions/`

Moved active master/component docs:

- Backend docs moved from `MASTER DOCS` into `Backend/`.
- Anti-cheat docs moved from `MASTER DOCS/Backend Anti Cheat` into `Backend/Anti Cheat/`.
- Steamworks docs moved from `MASTER DOCS` into `Release/Steam/`.
- Gameplay camera/combat/movement/stats/player-experience/trap/world docs moved from `MASTER DOCS` into `Gameplay` subfolders.
- Minigame docs moved from `Docs` into `Gameplay/Minigames`.
- Audio, enemy/boss roster, console-command, community mods/challenges, and tower implementation-plan docs moved from `Docs` into their owning component folders.
- UI generation docs moved from `Docs/UI` and `UI/Reference` into `UI/Processes`.
- Import pipeline and TRELLIS workflow guidance is now represented by the numbered `Model Generation/Instructions` docs, especially `00_MASTER.md`, `01_TRELLIS_RUNPOD_SETUP.md`, and `05_UNREAL_IMPORT_AND_VALIDATION.md`.

Merged or replaced docs:

- Replaced central master-doc indexing with component `README.md` files under `Backend`, `Gameplay`, `Gameplay/Minigames`, `Release`, `Model Generation`, and `Model Generation/Instructions`.
- Merged the useful `Video Generation` main-menu video process into `UI/Processes/Main Menu Video Background.md`, using the current main-menu source plate and `Content/Movies/MainMenuBackground.mp4` as the final movie target.
- Replaced the old `World Generation` master/memory/useful-links set with `Gameplay/World/HY_WORLD_RESEARCH.md`.
- Kept the modular dungeon kit process as `Gameplay/World/MODULAR_DUNGEON_KIT_PROCESS.md`.
- Folded model-generation root docs, import guidance, and shared model/world pipeline rules into the current numbered `Model Generation/Instructions` shape.

Deleted obsolete documentation roots after rehome/merge:

- `Docs/`
- `MASTER DOCS/`
- `Guidelines/`
- `World Generation/`
- `Video Generation/`
- root `ANTI_CHEAT/` was already absent before this pass

Deleted obsolete one-off docs/prompts in this scope:

- `MASTER DOCS/README.md`
- `MASTER DOCS/T66_DECISION_LOG.md`
- `MASTER DOCS/T66_PROJECT_CATALOGUE.md`
- `Docs/README.md`
- `UI/MEMORY.md`
- `World Generation/MASTER.md`
- `World Generation/MEMORY.md`
- `World Generation/ROOM_GENERATION_PROCESS.md`
- `World Generation/SETUP.md`
- `World Generation/USEFUL_LINKS.md`
- `Video Generation/API_STATUS.md`
- `Video Generation/NEXT_STEPS.md`
- `Video Generation/PROMPTS.md`
- `Video Generation/README.md`
- `Video Generation/scripts/regenerate_kling_masks.py`
- `Video Generation/scripts/submit_kling_motion_brush.py`
- `Gameplay/Minigames/OLD_Docs_Minigames_README.md`

Current notes and open risks:

- Active docs were mechanically updated away from `MASTER DOCS`, `Docs`, and `World Generation` paths where they now point at component docs.
- Some moved historical audit docs still mention deleted old prompt-pack paths as historical context; they are not live process authorities.
- UI generated-output folders such as `UI/archive`, `UI/generation`, `UI/screens`, and bulky `UI/Reference` artifacts were not removed by this Worker A pass because this pass stayed on documentation/process docs rather than generated UI asset/output cleanup.
- Model Generation appears in its current cleaned shape as `README.md`, `Instructions/`, `Scripts/`, and `Tools/`; the detailed instruction set is now the numbered `00_` through `06_` docs in `Model Generation/Instructions`.

## Alpha 0.1 Integration Notes - 2026-05-08

Implemented the accepted cleanup decisions that crossed worker ownership boundaries.

Deleted root clutter and generated-output folders:

- `_codex_previews`
- `Archive`
- `ArchivedBuilds`
- `Art reference images`
- `CodexSkills`
- `DerivedDataCache`
- `Exports`
- `output`
- `Plugins`
- `tmp`
- generated `Saved` contents except `Saved/StagedBuilds` and `Saved/StandaloneLogs`
- `UI/archive`
- `UI/generation`
- `UI/screens`
- `UI/Reference`

Cleaned active references after the folder removals:

- UI process docs now treat generated proof/reference captures as temporary `Saved/Codex/UI` output, not persistent repo roots.
- Steam documentation no longer points at the deleted old `UI/screens` capture path.
- Tool references now point at `Tools/Release/Steam`, `Tools/ArtPipeline/Items`, or `Tools/ArtPipeline/Minigames`.

Runtime/content cleanup implemented from the accepted inventory:

- Removed `UT66PropSubsystem` and the main-map prop spawn path.
- Deleted `Content/Data/Props.csv` and `Content/Data/DT_Props.uasset`.
- Removed the reusable import verifier's deleted-props data-table expectation.
- Deleted the one-off `Scripts/BuildWorldNpcInteractablesRetroBatch01ManifestAndExit.py` manifest generator because it depended on the retired props table and belonged to the completed World/NPC interactables batch.
- Deleted obsolete `Content/Characters/Enemies/Enemy1` and `Content/Characters/Enemies/Enemy2`.
- Deleted Arthur source/reference preview artifacts that were not part of the still-live Hero 1 sword VFX path.
- Removed stale `SourceAssets/UI/MainMenuReference` code paths from active UI code, replacing Mini screen backgrounds and in-run button plate lookups with the current `SourceAssets/UI/Reference` runtime asset library.

Verification status:

- `T66Editor Win64 Development` build succeeded after source cleanup.
- Full standalone stage/cook/package succeeded, followed by a source-only `-SkipCook` refresh after UI path cleanup.
- `T66 Standalone.lnk` and the pinned taskbar shortcut both resolve to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged executable smoke boot reached `BP_FrontendGameMode_C` and `Engine is initialized`; it remained running after 25 seconds and was stopped by the verification script.
- `Intermediate`, `DerivedDataCache`, and generated `Saved` folders were cleared again after verification, preserving only `Saved/StagedBuilds` and `Saved/StandaloneLogs`.

## Alpha 0.1 Content/Data Cleanup Notes - 2026-05-08

Implemented the next accepted project-content cleanup pass against the actual Unreal project tree, not the staged Windows packaged build.

Enemy cleanup implemented:

- Redirected old `GoblinThief_*` rarity visual IDs to current enemy visual IDs in `T66GoblinThiefEnemy.cpp`.
- Redirected the unique debuff enemy away from the old `Enemy3` visual and onto a current caster visual.
- Removed the generic old `Boss` preload from game-mode bootstrap.
- Removed stale `Boss` and `GoblinThief_*` rows from `Content/Data/CharacterVisuals.csv`.
- Regenerated `Content/Data/DT_CharacterVisuals.uasset` through `Scripts/SetupCharacterVisualsDataTable.py`.
- Ran an Unreal package/text audit for the legacy enemy folders and confirmed no live outside references.
- Deleted legacy enemy folders:
  - `Content/Characters/Enemies/Boss`
  - `Content/Characters/Enemies/Cow`
  - `Content/Characters/Enemies/Enemy3`
  - `Content/Characters/Enemies/Goat`
  - `Content/Characters/Enemies/GoblinThief`
  - `Content/Characters/Enemies/Pig`
  - `Content/Characters/Enemies/Roost`
- Current main enemy folders are now the active `Bosses` and `Regular` roots.

UI cleanup implemented:

- Replaced old DeletedTheme button-plate cooked-asset loading with the current `SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements` runtime fallback path.
- Removed retro sky and retro wood button-border paths from the style code.
- Removed old main-menu preload paths for `MMRed`, `sky_bg`, `fire_moon`, and `pyramid_chad`.
- Removed stale `MainMenu` and `PartyPicker` scan roots from the UI texture-quality helper.
- Ran `Saved/Audits/UIAssetCleanupTargetsAudit.json`; all 36 old UI targets had zero outside package referencers and zero source/data/script text references.
- Deleted old UI/content roots and assets:
  - `Content/SourceAssets/UI`
  - `Content/UI/MainMenu`
  - `Content/UI/PartyPicker`
  - `Content/UI/Assets/TopBar`
  - `Content/UI/Assets/Medals`
  - `Content/UI/Obsidian.uasset`
  - old DeletedTheme plate textures under `Content/UI/Assets`
  - old retro wood trim textures under `Content/UI/Assets`
  - old `ButtonLight_*` and `PanelLight` assets
  - old retro sky and retro wood materials/material instances under `Content/UI/Materials`
- Current `Content/UI` roots after cleanup:
  - `Leaderboard`
  - `Materials` with `M_UI_Glow.uasset`
  - `Minigames`
  - `Preview`
  - `Sprites`
  - `M_PixelationPostProcess.uasset`
- A post-delete source/config/data/script search found no remaining old DeletedTheme, retro wood/sky, old main-menu, old party-picker, old topbar, or old content-medal path references. Remaining `medal_*` references are the current hero-selection runtime fallback image names under `SourceAssets/UI/Reference`, not the deleted `Content/UI/Assets/Medals` folder.

Script lifecycle cleanup implemented:

- Deleted stale one-off scripts:
  - `Scripts/InspectImportLightingIssues.py`
  - `Scripts/FixCookWarningRoots.py`
- Temporary audit scripts used for this pass were removed after their audit output was written.

Verification status for this incremental pass:

- `T66Editor Win64 Development` build succeeded.
- Full standalone build/cook/package/stage succeeded through `Scripts/StageStandaloneBuild.ps1`.
- `T66 Standalone.lnk` and the pinned taskbar shortcut both resolve to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged executable smoke boot reached `BP_FrontendGameMode_C` and `Engine is initialized` with no fatal/error markers in `Saved/StandaloneLogs/T66_CleanupSmoke.log`.
- Generated support folders were cleared again after verification, preserving `Saved/StagedBuilds` and `Saved/StandaloneLogs`.
