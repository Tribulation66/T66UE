# T66 Scripts — What You Actually Need

## TL;DR

- **Daily / project setup:** You only need **RunFullSetup.bat** (or run **FullSetup.py** from the Editor for world models).  
- **Localization:** **AutoTranslateLocalizationArchives.py** when adding/changing UI text.  
- **Everything else** is either called by those, or run only when you change specific content (sprites, ground, music, physics, etc.).

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

**Count: 14** — only when touching that content.

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
| **Content pipelines** | 14 |
| **Orchestration** | 1 |
| **Verification** | 2 |
| **Debug** | 2 |
| **Removed** | 4 |
| **Batches/PS1** | RunFullSetup, RunItemsSetup, SetupMusicFolders, ExtractSourceAssets |

**Rough answer to “how many do we really need?”**

- **Minimum:** CreateAssets, FullSetup, ImportModels, AutoTranslateLocalizationArchives + RunFullSetup (and running FullSetup from Editor for models). So **4 Python scripts + 1 bat** for normal use and localization.
- The rest are either called by these, or used only when you change specific content (sprites, ground, music, stages, etc.) or for verification/debug. You can remove or archive the “likely redundant” set after confirming nothing references them.
