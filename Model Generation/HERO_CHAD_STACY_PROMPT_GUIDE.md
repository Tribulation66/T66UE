# Hero Chad/Stacy Prompt Guide

This is the current prompt and roster guide for the Quad Retro hero direction.
Use it with [TRELLIS_SOURCE_IMAGE_RULES.md](C:/UE/T66/Model%20Generation/TRELLIS_SOURCE_IMAGE_RULES.md) and [RETRO_CHARACTER_PIPELINE.md](C:/UE/T66/Model%20Generation/RETRO_CHARACTER_PIPELINE.md).

## Current Naming

These names replace the old direct first-name/source-likeness direction.

| Hero ID | Old source cue | New public name | Male source image status |
| --- | --- | --- | --- |
| Hero_1 | Arthur | Royal Chad | Needs male image variants |
| Hero_2 | Lu Bu | Chinese Chad | Needs male image variants |
| Hero_3 | Mike | Boxer Chad | Reference candidate selected |
| Hero_4 | George | Founding Chad | Needs male image variants |
| Hero_5 | Robo | Robo Chad | Needs male image variants |
| Hero_6 | Billy | Billy Chad | Needs male image variants |
| Hero_7 | Rabbit | Rabbit Chad | Male image candidate selected |
| Hero_8 | Shroud | CS Chad | Male image candidate selected |
| Hero_9 | xQc | Gamba Chad | Male image candidate selected |
| Hero_10 | Moist | Monotone Chad | Male image candidate selected |
| Hero_11 | North | Bald Chad | Male image candidate selected |
| Hero_12 | Asmon | Roach Chad | Male image candidate selected |

Do not use the old source cues as public-facing names. They are internal visual
memory only.

## Boxer Chad Reference

Use Boxer Chad as the male silhouette calibration reference:

- Source silhouette reference: [BoxerChad_ExaggeratedV_04.png](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroSourceExploration01/Inputs/imagegen_candidates/BoxerChad_ExaggeratedV_04.png)
- Approved source copy: [BoxerChad_ExaggeratedV_04_Source.png](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroModelTest01/Inputs/approved_seed_images/BoxerChad_ExaggeratedV_04_Source.png)
- Best current pipeline render: [BoxerChad_Medium_front.png](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake/Renders/Front/BoxerChad_Medium_front.png)

For the male batch, push the same body read harder than a normal athletic
character: very wide shoulders, oversized chest and traps, thick arms, very
thin waist, dramatic V taper, hips much narrower than shoulders, small or normal
head, simple low-detail face, strong blocky jaw.

## Source Image Rules In Short

Every source image for TRELLIS should be:

- full-body front view
- neutral A-pose with arms angled about `30-45` degrees down and separated from the torso
- orthographic or flat camera, no perspective distortion
- flat magenta `#FF00FF` or chroma green `#00FF00` background
- flat ambient lighting with no shadow, glow, rim light, specular highlight, or floor
- clean painted/cel-shaded concept art
- not pixel art, not photoreal, not a cinematic render
- identity carried by clothing, hair, equipment, and large color blocks
- face intentionally simple and low-detail

Generate `4` variants per hero for the next male pass. Pick by silhouette,
costume readability, and TRELLIS safety, not by face appeal.

## Male Prompt Template

```text
Positive:
Stylized male video game character, full body front view, neutral A-pose, arms angled slightly downward and away from body, both hands and feet visible, [identity block], extremely wide shoulders, oversized broad chest and traps, very narrow waist, dramatic V-shaped torso, shoulders much wider than hips, thick muscular arms, exaggerated heroic proportions, small or normal head relative to body, blocky giga-chad jawline, simple stylized low-detail face, neutral expression mouth closed, [3-5 readable costume/equipment regions], clean painted concept art style, flat cel-shaded colors, even flat lighting no shadows no highlights, character standing facing camera straight on, orthographic flat camera no perspective distortion, solid magenta background #FF00FF, character fills 80% of frame vertically, full body visible head to feet.

Negative:
photorealistic, photoreal, 8k, cinematic lighting, dramatic shadows, rim light, specular highlights, pixel art, retro graphics, low resolution, blurry, action pose, dynamic pose, fighting stance, motion blur, walking, running, T-pose, extra limbs, multiple arms, three legs, mutated anatomy, asymmetrical body, cropped, headshot, portrait, half body, partial body, complex background, environment, scene, floor, ground plane, ground shadow, cast shadow, drop shadow, multiple characters, weapons in motion, particle effects, glow effects, magic, detailed face, expressive face, skin pores, fabric texture detail, hair strands, realistic anatomy, slim build, lanky, average build, skinny.
```

## Male Identity Blocks

Use these as the `[identity block]` and costume/equipment section for the next
male image batch.

Royal Chad:
`regal knight champion, ornate golden crown, royal red velvet shoulder mantle with gold trim, polished silver chestplate, dark leather belt with crown buckle, armored boots, royal blue and crimson accents`

Chinese Chad:
`ancient Chinese warlord champion, black and red lamellar armor with gold trim, high plume helmet, broad armored shoulder guards, red waist sash, heavy black boots, ceremonial halberd strapped vertically behind back and not in motion`

Founding Chad:
`founding-era commander champion, navy colonial military coat with gold epaulettes, cream waistcoat, white trousers, black riding boots, tricorn hat, leather belt, parchment and brass accents`

Robo Chad:
`bulky humanoid robot champion, broad mechanical shoulders, silver and gunmetal armor plates, simple cyan chest core, heavy mechanical forearms, segmented metal boots, clean readable hard-surface shapes`

Billy Chad:
`western outlaw champion, wide-brim cowboy hat, tan duster cropped short enough to reveal the torso silhouette, dark vest, red neckerchief, leather holsters, blue trousers, brown boots, brass belt buckle`

Rabbit Chad:
`actual anthropomorphic rabbit trench-coat champion, rabbit head and upright rabbit ears as simple solid shapes, cropped tan trench coat ending above the knees, dark vest, lavender tie or scarf tucked close, compact belt, dark trousers, brown boots, simple gloves, compact baton or cane held naturally`

CS Chad:
`unmasked tactical sniper champion, short black hair, rectangular glasses, dark tactical vest, charcoal shirt, gray cargo pants, knee pads, gloves, boots, muted green and black equipment blocks, sniper rifle held above waist`

Gamba Chad:
`gambling-inspired champion, medium-long yellow-blond hair in a simple solid hair mass, black sleeveless jacket cropped close to the body, white shirt, red and gold vest panels, casino chip belt buckle, playing-card suit symbols as large simple shapes, dark trousers, sneakers, one oversized die held in each hand`

Monotone Chad:
`deadpan necromancer champion, long straight black hair as one simple solid mass, cropped black necromancer coat ending above the thighs, dark gray armor vest, white shirt panel, bone-white belt buckle, black trousers, dark boots, simple black gloves, compact skull-topped scepter`

Bald Chad:
`bald berserker champion, shaved head, simple rectangular glasses, cropped fur-lined berserker vest close to the torso, leather harness straps as broad bands, slate-gray trousers, heavy boots, ice-blue scarf tucked close at neck, metal bracers, large battle axe`

Roach Chad:
`roach king scavenger champion, long dark brown hair as one simple solid mass, smooth dusty black chest armor with broad roach-shell brown shoulder plates, simple roach emblem on the center chest, tarnished gold belt buckle, dark gloves, rugged boots, broad sword held naturally`

## Stacy Later

Do not start Stacy variants until the male forms are approved. Stacy uses the
hourglass body-family rule in [TRELLIS_SOURCE_IMAGE_RULES.md](C:/UE/T66/Model%20Generation/TRELLIS_SOURCE_IMAGE_RULES.md), not the Chad inverted-triangle rule.
