# Source Image Rules

Source images need to be reviewed as art inputs, not just detected as files.
Pixal3D and TRELLIS preserve broad silhouette better than they preserve weak
value structure, so source approval is primarily a silhouette, composition, and
color-readability gate.

## Required Image Qualities

- one clear subject
- T-pose or neutral standing pose unless the target is intentionally non-character
- front view and full body
- front-readable silhouette
- no cropped body parts unless the target is intentionally partial
- clear separation between the subject and background
- no cluttered environment elements, proxy redraws, or alpha cutouts
- no prompt text, watermark, UI chrome, or contact-sheet labels inside the image

## Color Discipline

These are the default source-image color rules for non-humanoid characters,
creatures, props, and interactables. Humanoid heroes and companions have a
family-specific color-section rule in
`10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md`; use that file after this one for
humanoid prompts.

Use two primary colors across the character. The two colors must have hard
edges between them, no gradients between regions, and clear separation on both
hue and value axes.

Small accent details are allowed: eyes, mouth, fangs, and simple equipment can
use additional colors when they do not break the main silhouette or turn the
character into a noisy multi-material read. Spots, marks, weathering, jewelry,
clasps, pouches, rope, and other small details are rejected when they fragment
the silhouette or compete with the two main colors.

Anything beyond two main colors plus minimal accents is rejected before model
generation. Do not send noisy sources downstream and expect retopo, baking,
pixelation, or normalization to repair them.

## Source Image Stop Rule

When any source image fails a technical or visual gate, stop the model-generation
workflow before staging or generation. Do not manually edit the pixels, do not
crop, brighten, repaint, clean up, split, or otherwise repair the image, and do
not regenerate a corrected source with imagegen unless Pablo explicitly asks for
that in a follow-up instruction.

The agent response must report the failing image, the exact gate or visible
problem, and why the issue blocks generation. Wait for Pablo to provide a
replacement image or explicitly approve a separate correction/regeneration pass.
Keep the original source unchanged.

## Style Direction

Style is not locked. Cartoon proportions, grounded proportions, angular
ULTRAKILL-style forms, Chad/Stacy exaggeration, and other readable visual
directions are all valid.

The technical rules are about composition, color count, edge clarity, and
subject readability. Do not treat the cartoon proportions from the goblin and mushroom
experiments as a required house style.

## Chad/Stacy Direction

Use `10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md` for current Chad/Stacy hero and
companion source images. It owns the locked A-pose 1.5, forward-feet stance,
thigh-gap rule, detail-parity rule, demo clothing rule, and current hero and
companion identity slots.

## Detail Leakage Caveat

Image generators often add unrequested details even when the prompt forbids
them. Common leakage includes skin spots, cloth texture, pouches, seams,
jewelry, scratches, and subtle body shading.

Mitigate this with direct negative prompt language:

- no spots
- no texture noise
- no gradients
- no weathering
- no pouches, clasps, rope, jewelry, or small props

Some residual variance is acceptable when the two-color read remains intact.
Pipeline normalization and post-bake processing can smooth small variance; they
cannot rescue a source with fragmented color or a muddy silhouette.

## Approval Gate

Before retopo, rigging, import, or promotion:

- visually inspect the source and generated model
- verify duo-color discipline on the source image
- reject sources outside the color-discipline or readability requirements
- for source-image failures, stop and report the failing gate to Pablo instead
  of repairing, splitting, or regenerating the image
- verify the GLB opens and renders
- check front, side, and oblique views for silhouette, face, hands, equipment, and scale
- reject weak input early instead of trying to repair it downstream

Source PNG existence is not approval. A source passes only when it is readable
and disciplined enough for generation.
