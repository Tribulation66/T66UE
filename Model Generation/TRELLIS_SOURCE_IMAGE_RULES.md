# TRELLIS Source Image Rules

These rules apply to the current one-image, one-model Quad Retro hero pipeline.
The source image is not final game art. It is reconstruction input for TRELLIS.
The low-poly, pixelated, dithered look is applied later by the Blender pipeline.

## Active Principle

Generate a clean, boring, readable full-body character image that TRELLIS can
turn into stable geometry. Do not generate pixel art, photoreal detail, action
poses, dramatic lighting, or identity-critical facial detail at this stage.

Identity should come from silhouette, hair, clothing, equipment, and large color
blocks. Facial details are intentionally secondary and should survive only as
simple marks after bake, palette reduction, and dithering.

## Ten Hard Rules

1. Pose: use a neutral A-pose.
   Arms should angle about `30-45` degrees down from the shoulders, separated
   from the torso. Do not use T-pose, fighting stance, flexing, walking,
   running, or any action pose.

2. Camera: dead-center front view.
   The character faces the camera straight on. Use an eye-level or chest-level
   orthographic-feeling camera with no foreshortening, no head tilt, and no
   top-down or low-angle perspective.

3. Background: pure flat saturated chroma color.
   Use a solid color that does not appear in the character, usually magenta
   `#FF00FF` or chroma green `#00FF00`. Do not use white, black, gradients,
   environments, ground planes, or shadows.

4. Lighting: flat and even.
   Use ambient lighting with no dramatic key light, rim light, cast shadow,
   contact shadow, specular highlight, reflection, or glow. TRELLIS will bake
   lighting artifacts into the mesh texture.

5. Style: clean painted concept art.
   Use flat colors, clean cel-shaded forms, and low surface noise. Do not use
   pixel art or retro graphics because TRELLIS can interpret pixel edges as
   geometry. Do not use photorealism because pores, fabric weave, hair strands,
   and camera noise create messy geometry.

6. Body proportions: lock the body-family silhouette in every prompt.
   Include extremely wide shoulders, narrow waist, V-shaped torso, exaggerated
   heroic proportions, broad chest, thick muscular arms, shoulders much wider
   than hips, and small or normal head relative to body. Suppress skinny,
   lanky, slim, realistic, average-build proportions. For Stacy, use the
   separate Stacy silhouette rule below instead of the Chad inverted triangle.

7. Face: under-detail on purpose.
   Use simple stylized facial features, neutral expression, mouth closed, and
   minimal face detail. Add only one or two identity marks if needed, such as a
   small dark cheek tattoo for Boxer Chad. Avoid detailed eyes, expressive face,
   skin pores, and intricate facial anatomy.

8. Costume: spend the prompt budget here.
   Character identity must read through `3-5` large clothing, hair, equipment,
   or material regions. The design should still be recognizable as five colored
   blobs at `128x128`.

9. Resolution and framing: square full body.
   Generate at `1024x1024` minimum, `1536x1536` preferred when available.
   Character should fill about `80%` of the frame vertically with full head,
   hands, feet, and both shoes visible.

10. Negative prompts: suppress TRELLIS failure modes aggressively.
    Always block extra limbs, mutated anatomy, asymmetry, cropped body,
    headshot, portrait, action pose, complex background, ground shadow,
    photorealism, pixel art, blurry output, particle effects, glow, and
    multiple characters.

## Generic Prompt Template

Use this structure for every new Quad Retro hero source image:

```text
Positive:
Stylized male video game character, full body front view, neutral A-pose, arms slightly away from body, both hands and feet visible, [character identity], extremely wide shoulders narrow waist V-shaped torso, exaggerated heroic proportions, broad chest, thick muscular arms, shoulders much wider than hips, [head/hair rule], simple stylized face with minimal features, neutral expression mouth closed, [3-5 readable costume/equipment regions], clean painted concept art style, flat cel-shaded colors, even flat lighting no shadows no highlights, character standing facing camera straight on, orthographic flat camera no perspective distortion, solid magenta background #FF00FF, character fills 80% of frame vertically, full body visible head to feet.

Negative:
photorealistic, photoreal, 8k, cinematic lighting, dramatic shadows, rim light, specular highlights, pixel art, retro graphics, low resolution, blurry, action pose, dynamic pose, fighting stance, motion blur, walking, running, T-pose, extra limbs, multiple arms, three legs, mutated anatomy, asymmetrical body, cropped, headshot, portrait, half body, partial body, complex background, environment, scene, floor, ground plane, ground shadow, cast shadow, drop shadow, multiple characters, weapons in motion, particle effects, glow effects, magic, detailed face, expressive face, skin pores, fabric texture detail, hair strands, realistic anatomy, slim build, lanky, average build, skinny.
```

## Body Family Rules

### Chad

Chad is the masculine/default body family. The read is an exaggerated inverted
triangle:

- extremely wide shoulders, pushed beyond realistic bodybuilder proportions
- oversized broad chest and traps
- thick muscular arms
- very narrow waist
- dramatic V-shaped torso taper from shoulders to waist
- hips much narrower than shoulders
- blocky giga-chad jaw/head read

### Stacy

Stacy is the feminine body family. The read is athletic, powerful, and stylized,
but not the same inverted triangle as Chad:

- narrow waist
- visibly larger bust within a full-coverage athletic outfit
- wider hips than waist
- powerful thighs and glutes
- toned arms
- athletic shoulders that are readable but not wider than Chad shoulders
- exaggerated hourglass or heroic S-curve silhouette
- simple stylized face with hair and outfit carrying identity

Stacy should still be reconstruction-safe. Make the silhouette more exaggerated
through body proportions and large clothing shapes, not through posing or
sexualized framing. Do not prompt for fashion-pose hips, contrapposto, pinup
posing, high heels, long loose cloth, tiny ankles, hair strands, cleavage focus,
lingerie, or complex jewelry chains. Keep the pose neutral, symmetrical, and
full-body.

## Boxer Chad Test Prompt

Use Boxer Chad as the first calibration source for this process.

```text
Positive:
Stylized male video game character, full body front view, A-pose, arms slightly away from body, both hands and feet visible, dark-skinned muscular boxer, extremely wide shoulders narrow waist V-shaped torso, exaggerated heroic proportions, shaved head, simple stylized face with minimal features, small dark tattoo marking on left cheek, red leather boxing gloves with white laces on both hands, plain white athletic tank top, black boxing trunks with white waistband, black boxing boots, neutral expression mouth closed, clean painted concept art style, flat cel-shaded colors, even flat lighting no shadows no highlights, character standing facing camera straight on, orthographic flat camera no perspective distortion, magenta solid background #FF00FF, character fills 80% of frame vertically, full body visible head to feet.

Negative:
photorealistic, photoreal, 8k, cinematic lighting, dramatic shadows, rim light, specular highlights, pixel art, retro graphics, low resolution, blurry, action pose, dynamic pose, fighting stance, motion blur, walking, running, T-pose, extra limbs, mutated anatomy, asymmetrical, cropped, headshot, portrait, half body, partial body, complex background, environment, scene, ground shadow, cast shadow, multiple characters, weapons, particle effects, glow effects, magic, detailed face, expressive face, skin pores, fabric texture detail, hair strands, realistic anatomy, slim build, lanky, average build, skinny.
```

## Boxer Stacy Test Prompt

Use Boxer Stacy to calibrate the feminine body-family silhouette against the
same Mike-inspired boxer archetype.

```text
Positive:
Stylized adult female video game character, full body front view, A-pose, arms slightly away from body, both hands and feet visible, dark-skinned athletic female boxer, powerful feminine heroic proportions, narrow waist, wider hips than waist, strong legs, toned arms, athletic shoulders, clean hourglass silhouette, simple stylized face with minimal features, short tied-back hair, small dark tattoo marking on left cheek, red leather boxing gloves with white laces on both hands, cropped white athletic boxing top, black high-waist boxing trunks with white waistband, black boxing boots, neutral expression mouth closed, clean painted concept art style, flat cel-shaded colors, even flat lighting no shadows no highlights, character standing facing camera straight on, orthographic flat camera no perspective distortion, magenta solid background #FF00FF, character fills 80% of frame vertically, full body visible head to feet.

Negative:
photorealistic, photoreal, 8k, cinematic lighting, dramatic shadows, rim light, specular highlights, pixel art, retro graphics, low resolution, blurry, action pose, dynamic pose, fighting stance, motion blur, walking, running, T-pose, fashion pose, contrapposto, pinup pose, extra limbs, mutated anatomy, asymmetrical, cropped, headshot, portrait, half body, partial body, complex background, environment, scene, ground shadow, cast shadow, multiple characters, weapons, particle effects, glow effects, magic, detailed face, expressive face, skin pores, fabric texture detail, hair strands, realistic anatomy, skinny, fragile, tiny ankles, high heels, long dress, loose cape, chains, complex jewelry.
```

## Selection Gate

Generate `4-6` variants and choose the one with:

- cleanest full-body silhouette
- clearly separated arms and legs
- largest readable costume/equipment regions
- least facial and surface noise
- no visible shadow, floor, card, or background geometry
- no cropping of head, hands, gloves, feet, or boots

Do not choose based on face appeal. The face is intentionally not the primary
identity carrier and will be reduced heavily by the pixel/dither pass.

## Naming Direction

Use archetype names for the new direction instead of first-name likeness cues:

- `Boxer Chad`
- `Chinese Chad`
- `Founding Chad`
- `Regal Chad`
- `Boxer Stacy`
- `Regal Stacy`
- `Founding Stacy`

Prompts may be inspired by broad archetypes, historical clothing categories, or
sport/equipment language, but should avoid direct likeness replication.
