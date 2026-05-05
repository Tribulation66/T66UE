# T66 Archive Asset Cleanup Ledger

**Date:** 2026-04-12  
**Purpose:** First-pass audit of `SourceAssets/Archive` before any destructive asset cleanup.  
**Rule:** Nothing under `SourceAssets/Archive` should be deleted until it is classified here.

## 1. Current Archive Surface

Top-level entries under `SourceAssets/Archive`:

- `FinalPortraits/`
- `Fonts/`
- `HeroPortraits/`
- `Import/`
- `ItemSprites/`
- `Skies/`
- `UI/`
- `OuterWallTexture.png`

## 2. Confirmed Live Or Blocking References

These are the archive items that are still directly referenced by code or are the only remaining copy of a path that scripts expect.

### 2.1 `SourceAssets/Archive/OuterWallTexture.png`

Status:

- `Migrated`

Reason:

- The file has been moved to `SourceAssets/OuterWallTexture.png`.
- `Source/T66/T66.Build.cs` now stages the non-archive path.
- `Scripts/SetupOuterWallTexture.py` already expects the non-archive path.

Implication:

- The archive copy is no longer needed.

### 2.2 `SourceAssets/Archive/UI/MainMenuArcaneFill/`

Status:

- `Migrated`

Reason:

- The source textures have been moved to `SourceAssets/UI/MainMenuArcaneFill`.
- Runtime fallback files already exist under `RuntimeDependencies/T66/UI/MainMenuArcaneFill`.
- `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp` still contains a compatibility remap for the old archive-relative path.

Implication:

- The archive source folder is no longer needed.
- The legacy remap can be cleaned later once we decide whether to keep compatibility code for old relative paths.

### 2.3 `SourceAssets/Archive/UI/RetroWoodTrimV2/`

Status:

- `Migrated`

Reason:

- Current tooling expects `SourceAssets/UI/RetroWoodTrimV2`.
- The source textures have been moved to that live path.

Implication:

- The archive source folder is no longer needed.

### 2.4 `SourceAssets/Archive/Skies/QuakeSky/`

Status:

- `Migrated`

Reason:

- `Scripts/SetupQuakeSky.py` expects `SourceAssets/Skies/QuakeSky`.
- The source textures have been moved to that live path.

Implication:

- The archive source folder is no longer needed.

## 3. Likely Delete Candidates

These look like true historical archive content rather than active source-of-truth assets.

### 3.1 `SourceAssets/Archive/FinalPortraits/`

Status:

- `Deleted`

Reason:

- The live non-archive folder `SourceAssets/FinalPortraits` exists.
- `Scripts/ImportHeroPortraits.py` reads from `SourceAssets/FinalPortraits`.
- The archived folder matched the live file set and has been removed.

### 3.2 `SourceAssets/Archive/ItemSprites/`

Status:

- `Likely delete`

Reason:

- The live non-archive folder `SourceAssets/ItemSprites` exists.
- `Scripts/ImportItemSprites.py` reads from `SourceAssets/ItemSprites`.

### 3.3 `SourceAssets/Archive/HeroPortraits/`

Status:

- `Likely delete, but verify no unique assets need to be preserved`

Reason:

- `Scripts/ImportHeroPortraits.py` looks for `SourceAssets/HeroPortraits`, not the archive path.
- The archive folder is not part of the current import workflow.
- There is no live non-archive `SourceAssets/HeroPortraits` folder at the moment, which suggests this archive may just be old material rather than active input.

### 3.4 `SourceAssets/Archive/Import/`

Status:

- `Likely delete`

Reason:

- Current live import workflows point at other source locations under `SourceAssets/Import`.
- The archived import content appears to be old staging material, not the current import source.

## 4. Hold For Later Review

These are not yet clear enough to delete in the first asset pass:

- `SourceAssets/Archive/Fonts/`
  - likely historical source copies, but worth checking against `RuntimeDependencies/T66/ThemeFonts`
- `SourceAssets/Archive/UI/`
  - mixed state: some parts look removable, others are the only remaining source copies

## 5. Recommended Asset Cleanup Order

1. Migrate the archive items that are still acting as de facto source files:
   - `OuterWallTexture.png` `DONE`
   - `UI/MainMenuArcaneFill` `DONE`
   - `UI/RetroWoodTrimV2` `DONE`
   - `Skies/QuakeSky` `DONE`
2. Update code/build/script references to the new non-archive paths where needed.
   - `OuterWallTexture` build path `DONE`
   - legacy `MainMenuArcaneFill` remap still present for compatibility
3. Delete the duplicate archive folders:
   - `FinalPortraits` `DONE`
   - `ItemSprites` `DONE`
   - `HeroPortraits` `DONE`
   - `Import` `DONE`
   - `Fonts` `DONE`
   - legacy empty `UI` and `Skies` archive folders `DONE`
4. Re-run one verification sweep for broken references before touching the remaining archive folders.

## 6. Current State

- `SourceAssets/Archive` has been fully removed.
- Live source assets moved to non-archive locations:
  - `SourceAssets/OuterWallTexture.png`
  - `SourceAssets/Skies/QuakeSky`
  - `SourceAssets/UI/MainMenuArcaneFill`
  - `SourceAssets/UI/RetroWoodTrimV2`
- The stale archive-only compatibility remap in `Source/T66/UI/Style/T66LegacyUITextureAccess.cpp` has been removed.
