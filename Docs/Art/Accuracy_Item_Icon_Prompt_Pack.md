# Accuracy Item Icon Prompt Pack

This file is a targeted prompt pack for generating the new live `Item_Accuracy` art set after Accuracy became a primary stat.

## Project Context

- Generate icons for exactly 1 live item: `Item_Accuracy`.
- This item needs 4 rarity variants: Black, Red, Yellow, and White.
- Total image count: 4 icons.
- The item should fit the existing premium Dota 2 item-icon direction already used by the live item set.
- `Item_AttackRange` remains the range-focused item.
- `Item_Accuracy` should be a distinct object, not a reskin of the range item.

## Master Context Prompt

```text
You are generating square item icons for a fantasy action roguelike. The icons must look like premium Dota 2 item icons: painterly fantasy MOBA item art, high readability, strong silhouette, centered object, dramatic lighting, rich materials, polished game UI finish. Do not make pixel art. Do not make cartoon-goofy props. Do not add text, letters, numbers, borders, UI frames, hands, characters, or cluttered scenes.

Each request is for 4 rarity variants of the same item. Keep the same core object in all 4 images. As rarity increases, only upgrade the design, materials, ornamentation, glow, prestige, and visual complexity.

The background must stay rarity-colored and simple:
- Black rarity: deep charcoal, obsidian, dark ash
- Red rarity: crimson, blood red
- Yellow rarity: gold, amber
- White rarity: pale ivory, holy white

The item should fill most of the frame and remain readable at small icon size. The object must be centered with a clean background and no scenery. Export as 4 separate square icons, one for each rarity.
```

## Item Object Index

| Category | Item | ItemID | Object |
| --- | --- | --- | --- |
| Accuracy | Accuracy | Item_Accuracy | Laser Sight |

## Prompt Blocks

### Accuracy

```text
Using the style rules above, generate 4 separate square item icons for Accuracy.

Core object: a mythic Laser Sight.
This item represents precision, sight alignment, target acquisition, and reliable head-targeting. It should feel elite, engineered, sharp, and premium.

The object should read as a fantasy-tech sighting module rather than a modern cheap attachment. Think precision lens housing, disciplined silhouette, expensive materials, and a distinct targeting identity. It must be clearly different from a long sniper scope.

Rarity variants:
1. Black rarity: dark steel laser sight module, compact lens, restrained detail, practical housing, faint targeting glow.
2. Red rarity: same laser sight upgraded with crimson emitter lines, hotter lens core, sharper housing silhouette, stronger targeting energy.
3. Yellow rarity: same laser sight upgraded into a gilded marksman's sight, ornate precision housing, gold fittings, premium lens glass, masterwork craftsmanship.
4. White rarity: same laser sight upgraded into a celestial deadeye sight, ivory-metal housing, radiant targeting crystal, holy precision glow, highest prestige.

Requirements:
- same laser sight concept across all 4 icons
- it must feel like high-end accuracy equipment, not a generic flashlight or full rifle scope
- clear central lens / targeting element
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy or toy-like style
```
