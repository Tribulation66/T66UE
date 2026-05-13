# Style Lock V2 Mushroom Variants Report

## Pixal3D Config Used

Confirmed: all five variants used the same parameter block below. No variant
used a seed change or parameter override.

```yaml
X-Seed: 1337
X-Resolution: 1536
X-Texture-Size: 2048
X-Decimation: 200000
X-Remesh: 1
X-Remesh-Band: 1.0
X-Remesh-Project: 0.0
X-Extend-Pixel: 0
X-Image-Resolution: 1024
X-Max-Num-Tokens: 49152
X-Mesh-Scale: 1.0
X-SS-Steps: 25
X-SS-Guidance: 7.5
X-SS-Guidance-Rescale: 0.7
X-SS-Rescale-T: 5.0
X-Shape-Steps: 25
X-Shape-Guidance: 7.5
X-Shape-Guidance-Rescale: 0.5
X-Shape-Rescale-T: 3.0
X-Tex-Steps: 25
X-Tex-Guidance: 4.0
X-Tex-Guidance-Rescale: 0.0
X-Tex-Rescale-T: 3.0
```

All five responses returned HTTP 200 and exported on attempt 1 using the
requested settings:

| Variant | Duration | Bytes | Export |
| --- | ---: | ---: | --- |
| Variant_1_Bipedal | 122s | 6,680,076 | requested, decimation 200000, remesh 1 |
| Variant_2_Quadrupedal | 299s | 6,684,668 | requested, decimation 200000, remesh 1 |
| Variant_3_Levitating | 95s | 6,494,952 | requested, decimation 200000, remesh 1 |
| Variant_4_MultiLegged | 242s | 8,201,776 | requested, decimation 200000, remesh 1 |
| Variant_5_Cluster | 364s | 6,411,432 | requested, decimation 200000, remesh 1 |

## Source Prompts

### Variant 1 - Asymmetric Bipedal Humanoid

Bipedal humanoid mushroom monster creature in T-pose, front view, full body,
clean white background. Cap is deep bruise-purple (#6B2B5A), body and limbs are
sickly jaundiced cream-yellow (#E5D29A). Hard color edge between cap and body,
no gradients, no realistic shading, no contact shadows. Deliberately
asymmetric and deformed: cap tilted 10-15 degrees and drooping to one side,
body weight shifted to one side with a slight hunch, eyes asymmetric (one
larger and rounder, one smaller and squintier, at different heights), small
slack mouth hanging slightly open, one arm visibly longer than the other. Clean
Megabonk-style rendering - no fiddly details, no fingers, no extra appendages,
no spots on the cap, no accessories. Looks like a 2D drawing with volume.
Sickly, diseased, uncanny - not cute.

### Variant 2 - Quadrupedal Mushroom

Quadrupedal mushroom monster creature standing on four short stubby root-like
legs, front view, full body, clean white background. Cap is deep bruise-purple
(#6B2B5A), the rest of the creature is sickly jaundiced cream-yellow
(#E5D29A). Hard color edge between cap and body, no gradients, no realistic
shading. The body is low and horizontal, like an animal walking on four legs.
Deliberately asymmetric: cap tilted to one side and drooping, body slightly
twisted, one front leg shorter than the others, asymmetric eyes on the front of
the body under the cap (one larger, one smaller, at different heights), small
slack mouth. Clean Megabonk-style rendering - simple stubby legs, no claws, no
spots, no accessories. Looks like a 2D drawing with volume. Sickly and uncanny.

### Variant 3 - Levitating Mushroom With Trailing Tendrils

Levitating mushroom monster creature floating in the air with trailing tendrils
below, front view, full body, clean white background. Cap is deep bruise-purple
(#6B2B5A), the body and tendrils trailing below are sickly jaundiced
cream-yellow (#E5D29A). Hard color edge between cap and body, no gradients, no
realistic shading. The creature has a mushroom cap on top, a small bulbous body
underneath, and 4-5 uneven trailing tendrils of different lengths hanging below
it like a jellyfish. Deliberately asymmetric: cap tilted, drooping to one side,
tendrils of clearly different lengths and thicknesses, asymmetric eyes (one
larger, one smaller, at different heights), small slack mouth. Clean
Megabonk-style rendering - simple smooth tendrils, no fiddly details, no spots,
no accessories. Looks like a 2D drawing with volume. Sickly and uncanny, like a
jellyfish-mushroom hybrid.

### Variant 4 - Multi-Legged Squat Fungal-Centipede

Squat mushroom monster creature with many short stubby legs, front view, full
body, clean white background. Cap is deep bruise-purple (#6B2B5A), the rest of
the creature is sickly jaundiced cream-yellow (#E5D29A). Hard color edge
between cap and body, no gradients, no realistic shading. The creature has an
elongated horizontal squat body with a mushroom cap on top and 6 short stubby
legs of uneven length and placement underneath, like a fungal centipede.
Deliberately asymmetric: cap tilted, body slightly twisted, legs of obviously
different lengths, asymmetric eyes on the front of the body (one larger, one
smaller, at different heights), small slack mouth. Clean Megabonk-style
rendering - simple stubby legs, no fiddly details, no spots, no accessories.
Looks like a 2D drawing with volume. Sickly and uncanny.

### Variant 5 - Cluster Mushroom

Mushroom monster creature with multiple mushroom caps growing from a single
hunched squat body, front view, full body, clean white background. The caps are
deep bruise-purple (#6B2B5A), the body is sickly jaundiced cream-yellow
(#E5D29A). Hard color edge between caps and body, no gradients, no realistic
shading. The creature has 3-4 mushroom caps of clearly different sizes growing
from the top and sides of a single hunched squat body in asymmetric positions.
The body is low and rounded, no clearly visible arms or legs, sitting on the
ground. Asymmetric eyes peering out from between the caps or from the front of
the body (one larger, one smaller, at different heights), small slack mouth.
Clean Megabonk-style rendering - no fiddly details, no spots, no accessories.
Looks like a 2D drawing with volume. Sickly and uncanny.

## Blender File

`Model Generation/Experiments/Style_Lock_V2_Mushroom_Variants/Comparison.blend`

The comparison file imports all five GLBs, normalizes them to the same bounding
box height, arranges them in one row along X, aligns bases at Z=0, labels each
variant, and sets a camera to capture the full row.

Viewport screenshot exports:

- `Model Generation/Experiments/Style_Lock_V2_Mushroom_Variants/Screenshots/comparison_front.png`
- `Model Generation/Experiments/Style_Lock_V2_Mushroom_Variants/Screenshots/comparison_3q_left.png`
- `Model Generation/Experiments/Style_Lock_V2_Mushroom_Variants/Screenshots/comparison_3q_right.png`

## Anomalies

- No Pixal3D generation failed. No export fallback, parameter override, or seed
  change was needed.
- All five GLBs imported into Blender as one mesh object each with nonzero
  bounds.
- Source generation retained the intended dominant palette and white
  background, but it did not perfectly obey the no-gradient rule; each source
  has some edge shading and facial accent detail.
- Variant 5 source reads more like a cluster of visible mushroom faces/caps than
  a single low body with only one face. It is still useful for evaluating the
  cluster form factor, but it is not a perfect prompt match.
