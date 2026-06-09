# Tools

`Tools` is for durable operator tools that are not Unreal runtime code and are not editor Python scripts. It is not a scratch folder.

## Layout

- `Release/Steam/`: release and Steam upload helpers.
- `ArtPipeline/Items/`: item sprite and item-art processing tools.
- `ArtPipeline/UI/PowerUp/`: power-up UI mask and statue reveal helpers.

## Lifecycle Rule

Keep master tools small, reusable, and documented. If a task-specific helper teaches a durable improvement, move that improvement into the relevant master tool, README, or process doc, then delete the task-specific helper.

Generated logs, temp folders, Python caches, and one-off outputs should stay outside this durable tree or be deleted after review.

## Current Master Tools

- `Release/Steam/UploadToSteam.ps1`
- `ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
- `ArtPipeline/Items/ITEM_SPRITE_RETRO_PROCESS.md`
- `ArtPipeline/UI/PowerUp/powerup_auto_mask.py`
- `ArtPipeline/UI/PowerUp/powerup_statue_masks.py`
