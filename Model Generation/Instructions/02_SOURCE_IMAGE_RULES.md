# Source Image Rules

TRELLIS source images need to be reviewed as art, not just detected as files.

## Required Image Qualities

- one clear subject
- front-readable silhouette
- no cropped body parts unless the target is intentionally partial
- clean background and minimal contact shadows
- clear material/color separation
- no prompt text, watermark, UI chrome, or contact-sheet labels inside the image

## Chad/Stacy Direction

Use explicit Chad/Stacy identity prompts when generating hero sources. Keep silhouettes exaggerated enough for readable low-poly conversion, but preserve outfit color before any palette or dither pass.

## Approval Gate

Before retopo, rigging, import, or promotion:

- visually inspect the source and generated model
- verify the GLB opens and renders
- check front, side, and oblique views for silhouette, face, hands, equipment, and scale
- reject weak input early instead of trying to repair it downstream

Source PNG existence is not approval.
