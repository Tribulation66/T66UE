# Pixal3D Hero Model Generation Decision Gate

## Blocker

`SourceAssets/Hero1Stacy.png` and `SourceAssets/Hero2Chad.png` fail the current T66 Pixal3D source-image gate.

## Evidence

- `Hero1Stacy.png`
  - Size: `1024x1536`
  - Visual issue: black background instead of clean white background
  - Mean linear luminance: `0.1003`
- `Hero2Chad.png`
  - Size: `1024x1536`
  - Visual issue: black background instead of clean white background
  - Mean linear luminance: `0.1351`

The current repo gate in `Model Generation/Instructions/02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md` requires a clean white background and treats source luminance below `0.45` as a hard reject. It also says to stop before staging or generation and not manually repair the source unless Pablo explicitly asks for a correction/regeneration pass.

## Decision Needed

Choose one path:

1. Provide replacement PNGs on clean white backgrounds that meet the source gate.
2. Explicitly approve a separate source-correction pass for these two PNGs, after which the corrected sources can be reviewed and then sent to Pixal3D.
3. Explicitly override the source gate and run Pixal3D on the current black-background images, accepting higher risk of bad silhouettes/material contamination.

Until one of these is chosen, generation and Blender setup for these two images is paused.
