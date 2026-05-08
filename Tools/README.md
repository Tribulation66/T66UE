# Tools

`Tools` is for durable operator tools that are not Unreal runtime code and are not editor Python scripts. It is not a scratch folder.

## Layout

- `Release/Steam/`: release and Steam upload helpers.
- `ArtPipeline/Items/`: item sprite and item-art processing tools.
- `ArtPipeline/Minigames/`: shared minigame sprite-sheet tools.
- `ArtPipeline/UI/PowerUp/`: power-up UI mask and statue reveal helpers.

## Lifecycle Rule

Keep master tools small, reusable, and documented. If a task-specific helper teaches a durable improvement, move that improvement into the relevant master tool, README, or process doc, then delete the task-specific helper.

Generated logs, temp folders, Python caches, and one-off outputs should stay outside this durable tree or be deleted after review.

## Current Master Tools

- `Release/Steam/UploadToSteam.ps1`
- `ArtPipeline/Items/T66ProcessReimaginedItemSheets.py`
- `ArtPipeline/Minigames/T66MiniBuildWalkComparison.py`
- `ArtPipeline/Minigames/T66MiniNormalizeWalkSheet.py`
- `ArtPipeline/Minigames/T66MiniPrepareEnemyBossAssets.py`
- `ArtPipeline/Minigames/T66MiniSplitHeroSheets.py`
- `ArtPipeline/Minigames/T66MiniSplitSheets.py`
- `ArtPipeline/UI/PowerUp/powerup_auto_mask.py`
- `ArtPipeline/UI/PowerUp/powerup_statue_masks.py`
