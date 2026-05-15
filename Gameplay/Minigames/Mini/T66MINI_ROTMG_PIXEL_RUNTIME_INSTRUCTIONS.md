# T66 Mini Low-Res Pixel Runtime Instructions

This is the active process for making Mini Chadpocalypse's in-run gameplay stage use a hard low-res pixel-art presentation while preserving existing gameplay and data.

## Scope

Applies to in-run Mini gameplay only:

- arena background
- player sprites and projectiles
- companion sprites
- enemies and bosses
- interactables, pickups, and loot bags
- simple gameplay VFX

Out of scope:

- Mini menu screens
- HUD layout
- shop, character select, difficulty select, idol select, save slots, and run-summary UI
- main game hero-selection visuals
- gameplay values, waves, items, idols, and save data

## Live Asset Root

The live loose runtime set is:

```text
SourceAssets/Mini/
```

Older painterly Mini source art is retained under:

```text
SourceAssets/Archive/Mini/
```

Do not restore archived painterly assets into the live `SourceAssets/Mini` root unless the visual direction changes again.

## Rebuild Command

Use a Codex/imagegen chroma-key sprite atlas as the source input, then rebuild the runtime set with:

```powershell
python .\Tools\ArtPipeline\Minigames\T66MiniBuildRotMGPixelSet.py `
  --source-atlas <path-to-imagegen-atlas.png> `
  --project-root C:\UE\T66
```

The builder deletes and recreates `SourceAssets/Mini`, copies the source atlas into `SourceAssets/Mini/Generated/ImageGen`, and writes `SourceAssets/Mini/ROTmgPixelSetManifest.json`.

Mini HUD fallback icons are copied through from `SourceAssets/Archive/Mini/HUD` unchanged so the gameplay-art rebuild does not alter UI presentation.

## Runtime Rules

- Runtime sprite PNGs must stay tiny-looking and hard-edged after scaling.
- Mini gameplay sprites use nearest-neighbor texture filtering.
- Do not add antialiasing, painterly gradients, soft shadows, smooth outlines, or high-resolution replacement art to the in-run Mini sprite set.
- Do not restyle Mini HUD/menu assets as part of this pass.
- Prefer 2-4 frame loops and simple attack flashes over rigged animation.
- Keep asset names matched to the existing Mini data IDs and visual subsystem paths.

## Verification

For a completed pass:

1. Rebuild the sprite set.
2. Verify chroma-key residue is gone from transparent sprites.
3. Compile the project.
4. Refresh the staged standalone build.
5. Verify `T66 Standalone.lnk` points to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
6. Smoke boot the staged standalone and launch a Mini battle.
