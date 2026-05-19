# Pixal3D Handoff - 2026-05-18

This folder is the curated source-image package for the next model-generation agent.

Use this package as the stable input surface instead of browsing the older `ImageGen` iteration folders directly.

## Primary Files

- `manifest.csv` - spreadsheet-friendly index of every copied source.
- `manifest.json` - same index in JSON form.
- `ModelReady/` - preferred one-subject source images for model generation.
- `ReviewSheets/` - contact sheets and multi-subject sheets. These must be split into one-subject images before Pixal3D.

## Model-Ready Counts

- Characters: 2
- Stage 1 dungeon enemies: 10
- Forest enemies: 10
- Ocean enemies: 10
- Hell enemies: 10
- Martian enemies: 10
- Props: 1

Total model-ready source images: 53
Additional finalized review sheets that need splitting: 9

## Current Character Finals

- `ModelReady/Characters/LuBu/Male/lubu_male_nephilim_shorts_hifigear_shading_v01.png`
- `ModelReady/Characters/LuBu/Female/lubu_female_nephilim_hifigear_shading_v01.png`

These are the latest accepted Lu Bu male/female sources. Earlier character iterations are intentionally not included in this handoff package.

## Notes For The Model Agent

- Follow `Model Generation/Instructions/02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md` before sending any image into Pixal3D.
- `ModelReady/` images are curated as the first-pass candidates.
- `ReviewSheets/Props/` and `ReviewSheets/Enemies/` are not direct Pixal3D inputs because they contain multiple subjects per image.
- The prop sheets should be split and visually checked before model generation.
- This package copies only finalized handoff images; it does not include old image iterations for finalized models.
- The original generation history still exists under `SourceAssets/ToonStyle/ImageGen/`, but the model-generation agent should consume this handoff package first.
- No Unreal content references or runtime data tables were changed by this organization pass.
