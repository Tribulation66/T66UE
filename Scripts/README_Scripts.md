# T66 Scripts — What You Actually Need

## TL;DR

- **Daily / project setup:** You only need **RunFullSetup.bat** (or run **FullSetup.py** from the Editor for world models).  
- **Localization:** **AutoTranslateLocalizationArchives.py** when adding/changing UI text.  
- **Everything else** is either called by those, or run only when you change specific content (sprites, ground, music, physics, etc.).

---

## What the agent can and cannot run

- **Agent can run:** Builds (Build.bat), PowerShell/terminal commands, standalone Python that does **not** use the `unreal` module (e.g. AutoTranslateLocalizationArchives.py), UAT/UnrealEditor-Cmd (GatherText, Cook, etc.). The agent can **confirm** these by running them and checking exit codes.
- **Agent cannot run:** Scripts that use Unreal’s Editor Python API (`import unreal`, `unreal.EditorAssetLibrary`, etc.). Those only work **inside a running Unreal Editor** (Tools → Execute Python Script). The agent cannot start the Editor or execute code inside it, so imports like **ImportHeros.py**, **ImportCompanions.py**, and **ImportModels.py** must be run by you in the Editor. For a single action, use Tools → Execute Python Script → `Scripts/ImportHeros.py` or `Scripts/ImportCompanions.py`.

---

## Essential (you need these)

| Script | Why |
|--------|-----|
| **CreateAssets.py** | Creates DataTables, Blueprints, levels. Run automatically by RunFullSetup.bat first. |
| **FullSetup.py** | Wires Game Instance / PlayerController / GameModes, imports CSV into DataTables, runs **incremental world-model import** (e.g. vending machine). Run from **Editor** (Tools → Execute Python Script) so world models are imported; the .bat skips that. |
| **RunFullSetup.bat** (.sh) | Runs CreateAssets + FullSetup. Main entry point for “setup the project.” |
| **ImportModels.py** | Used by FullSetup for incremental models (e.g. vending machine). Also runnable standalone for full zip + direct-folder import. |
| **AutoTranslateLocalizationArchives.py** | Fills .archive translations (22 cultures). Required by guidelines when you add/change player-facing text. |

**Count: 5 scripts** (plus 2 batch/shell wrappers).

---

## Convenience (same work as above, different slice)

| Script | Why |
|--------|-----|
| **ImportData.py** | Refills all DataTables from CSV only (no Blueprint wiring). Use when you only changed CSV and want to refresh DTs. FullSetup already does CSV import. |
| **SetupItemsDataTable.py** | Creates DT_Items if missing, fills from Items.csv, runs T66Setup. **RunItemsSetup.bat** runs this. Use for “items only” without full setup. |
| **RunItemsSetup.bat** | Runs SetupItemsDataTable (items-only). |

**Count: 2 scripts + 1 bat** — optional if you always run FullSetup.

---

## Content pipelines (run when you change that content)

| Script | When to run |
|--------|-------------|
| **ImportHeros.py** | **Run first** when hero models/portraits are missing. Imports from `SourceAssets/Heros/` (Knight, Ninja, Cowboy, Wizard → Hero_1..Hero_4) into `/Game/Characters/Heroes/` and `/Game/UI/Sprites/Heros/` (mesh + Walking + Alert + **Running**). Run **inside Unreal Editor**: Tools → Execute Python Script → `Scripts/ImportHeros.py`. Then run SetupCharacterVisualsDataTable.py and re-import Heroes (ImportData.py or FullSetup). **Note:** LogInterchangeEngine warnings about "invalid bind poses" and "No smoothing group" are expected; the import still succeeds and bind poses are auto-corrected. |
| **ImportCompanions.py** | **Run when** companion models/portraits are missing. Imports from `SourceAssets/Companions/Companion N/` (Def_CON, Beach_CON, Portrait) into `/Game/Characters/Companions/Companion_NN/` (Default + Beach skins, mesh + Walking + Alert + **Running**) and `/Game/UI/Sprites/Companions/`. Run **inside Unreal Editor**: Tools → Execute Python Script → `Scripts/ImportCompanions.py`. Then run SetupCharacterVisualsDataTable.py to refresh DT_CharacterVisuals from CharacterVisuals.csv. |
| **ImportSpriteTextures.py** | When you add/change PNGs under `SourceAssets/Sprites/`. |
| **BuildGroundAtlas.py** | When you change ground tiles (builds atlas PNG). Run before ImportGroundAtlas. |
| **ImportGroundAtlas.py** | When you change ground atlas / materials. |
| **ImportMusicOggs.py** | When you add/change music OGGs. |
| **SetupMusicFolders.ps1** | When you set up music folders. |
| **SetupCharacterVisualsDataTable.py** | When you add/change character visuals (DT_CharacterVisuals). |
| **ValidateCharacterVisuals.py** | Validates CharacterVisuals CSV paths exist. |
| **CreatePhysicsAsset_Boss01.py** | One-off / when you change boss physics. |
| **CreatePhysicsAssetsForCharacterVisuals.py** | When you add character meshes that need physics assets. |
| **SetupIdolsDataTable.py** | When you add/change idols data. |
| **SetupLeaderboardDataTables.py** | When you set up leaderboard DTs. |
| **SetupStagesDataTable.py** | When you add/change stages (DT_Stages); CreateAssets creates the DT, this can fill/setup. |
| **GenerateStagesCsvFromStagesMd.py** | When you generate `Stages.csv` from `T66_Stages.md`. |
| **ExtractSourceAssets.ps1** | When you need to extract source asset packs. |
| **CreateButtonProceduralAssets.py** | When you want procedural UI buttons (glossy plate + 2-tone border + outline). Creates **M_ButtonProcedural** and 6 MIs; source of truth: **SourceAssets/Data/ButtonProcedural_MaterialInstances.json**. Run in Editor: Tools → Execute Python Script → `Scripts/CreateButtonProceduralAssets.py`. T66Style uses these MIs when present. |

**Count: 15** — only when touching that content.

### Getting run animations in-game

Run animations are in **SourceAssets** (`Meshy_AI_Animation_Running_withSkin.fbx`) but must be imported into Content so the game can use them. **CharacterVisuals.csv** already has RunAnimation paths; the import scripts now include Running.

1. **Open Unreal Editor** and load the T66 project.
2. **Import heroes (including Running):** Tools → Execute Python Script → `Scripts/ImportHeros.py`.  
   - Imports mesh + Walking + Alert + **Running** for each hero (Knight→Hero_1, Ninja→Hero_2, Cowboy→Hero_3, Wizard→Hero_4), Default and Beach, Type A and B.  
   - Output names: `AM_Hero_N_Default_TypeA_Running` (Unreal may expose as `AM_..._Running_Anim`; CSV uses the _Anim path).
3. **Import companions (including Running):** Tools → Execute Python Script → `Scripts/ImportCompanions.py`.  
   - Imports mesh + Walking + Alert + **Running** for Companion_01–Companion_08, Default and Beach.  
   - Output names: `AM_Companion_NN_Default_Running`, `AM_Companion_NN_Beach_Running`.
4. **Refresh DataTable from CSV (if needed):** Tools → Execute Python Script → `Scripts/SetupCharacterVisualsDataTable.py` (or run **ImportData.py** / FullSetup to refill all CSVs).  
   - Ensures DT_CharacterVisuals matches CharacterVisuals.csv RunAnimation column.
5. **Save and test:** Run the game; hero and companion should play run animation after ~1 second of movement.

**Name match:** SourceAssets FBX `Meshy_AI_Animation_Running_withSkin.fbx` → script looks for filename containing `"Running"` → imports as `AM_{id}_{skin}_Running` → C++/CSV reference `AM_{id}_{skin}_Running_Anim` (engine/fallback handles _Anim).

---

## One-shot / orchestration

| Script | Why |
|--------|-----|
| **SetupAllAssetsAndDataTables.py** | Full Editor one-shot: ImportSpriteTextures + ImportWorldModels (full) + ImportData. Overlaps with “RunFullSetup + FullSetup from Editor”; use if you want “reimport sprites + all world models + refill CSV” in one go. |

**Count: 1.**

---

## Verification / audit

| Script | Why |
|--------|-----|
| **VerifySetup.py** | Checks required assets exist. Useful after setup or for CI. |
| **AssetAudit.py** | Audits WBP_* assets (e.g. find unreferenced). Use for cleanup. |

**Count: 2.**

---

## Debug / inspection (optional)

| Script | Why |
|--------|-----|
| **DebugGroundMaterial.py** | Debug ground material. |
| **InspectGroundMaterial.py** | Inspect ground material. |

**Count: 2** — safe to skip if you don’t use them.

---

## Likely redundant / legacy

| Script | Why |
|--------|-----|
| **SetupComplete.py** | Older “complete setup” (CSV + config). Superseded by **FullSetup.py**. |
| **ConfigureAssets.py** | Not referenced anywhere; likely legacy. |
| **ConfigureMainMenu.py** | Not referenced anywhere; likely legacy. |
| **ImportSourceAssetsModels.py** | Alternate model import path; primary path is **ImportModels.py**. |

**Count: 4** — candidates to remove or archive if you confirm they’re unused.

---

## Summary counts

| Category | Count |
|----------|--------|
| **Essential** | 5 scripts + RunFullSetup.bat/.sh |
| **Convenience** | 2 scripts + RunItemsSetup.bat |
| **Content pipelines** | 15 |
| **Orchestration** | 1 |
| **Verification** | 2 |
| **Debug** | 2 |
| **Removed** | 4 |
| **Batches/PS1** | RunFullSetup, RunItemsSetup, SetupMusicFolders, ExtractSourceAssets |

**Rough answer to “how many do we really need?”**

- **Minimum:** CreateAssets, FullSetup, ImportModels, AutoTranslateLocalizationArchives + RunFullSetup (and running FullSetup from Editor for models). So **4 Python scripts + 1 bat** for normal use and localization.
- The rest are either called by these, or used only when you change specific content (sprites, ground, music, stages, etc.) or for verification/debug. You can remove or archive the “likely redundant” set after confirming nothing references them.
