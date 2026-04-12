# Secondary Buff Icon Prompt Pack

This file is the canonical prompt reference pack for generating the live shop secondary-buff icons used in the Power Up single-use tab.

## Project Context

- Generate icons for 28 live secondary-buff shop entries total.
- These are not rarity-based item icons. Each entry needs exactly 1 square icon.
- The icons should look like premium Dota 2 item art: painterly, readable, game-ready, polished, and centered.
- Every icon should use a medically inspired outer shell tied to its parent stat row:
  - Damage: syringe
  - Attack Speed: pill capsule
  - Attack Scale: IV bag
  - Accuracy: eyedrop bottle
  - Armor: trauma pouch
  - Evasion: inhaler
  - Luck: vial
- The shell family should remain visually consistent within each row. The unique identity should come from the inner emblem and small row-appropriate detailing.
- Background for every icon must be a flat neutral grey. No rarity colors. No scene.
- Deprecated secondary stats `HP Regen`, `Life Steal`, and `Alchemy` are intentionally excluded.
- Speed is intentionally excluded. It should not receive a shop icon.

## Master Context Prompt

```text
You are generating square UI shop icons for a fantasy action roguelike. The icons must look like premium Dota 2 item art: polished hand-painted fantasy MOBA rendering, strong silhouette, centered object, rich materials, dramatic but controlled studio lighting, high readability at small size, and a finished game-UI feel.

These icons are medically inspired buff icons, not literal modern hospital photos. Each icon should depict a stylized medical object shell containing or displaying a symbolic inner emblem. The outer shell should be large, readable, and consistent for its category. The inner emblem should be bold, simple, and unmistakable.

Hard visual rules:
- exactly one square icon
- centered object
- object fills most of the frame
- front-facing or near-front-facing product-shot view preferred
- flat mid neutral grey background only
- no gradient background
- no vignette
- no edge darkening
- no spotlighting in the background
- no scenery
- no text, numbers, letters, UI frames, borders, labels, hands, characters, or clutter
- no cast shadow on the background
- no drop shadow
- no dark grounding shadow
- no pixel-art look
- no goofy cartoon look
- keep magical energy restrained and readable

The background should read as a clean uniform mid-grey field behind the object. The object should feel premium, game-ready, mythic-medical, and readable at icon size.
```

## Icon Index

| Row | Stat | Shell | Inner Emblem |
| --- | --- | --- | --- |
| Damage | AOE Damage | Syringe | blast ring |
| Damage | Bounce Damage | Syringe | ricochet arrow |
| Damage | Pierce Damage | Syringe | spear tip through line |
| Damage | DOT Damage | Syringe | dripping drop |
| Attack Speed | AOE Speed | Pill capsule | pulse ring with speed lines |
| Attack Speed | Bounce Speed | Pill capsule | ricochet chevron trail |
| Attack Speed | Pierce Speed | Pill capsule | dart streak |
| Attack Speed | DOT Speed | Pill capsule | timer plus droplet |
| Attack Scale | AOE Scale | IV bag | expanding circles |
| Attack Scale | Bounce Scale | IV bag | enlarged rebound arc |
| Attack Scale | Pierce Scale | IV bag | long needle lance |
| Attack Scale | DOT Scale | IV bag | spreading spill cloud |
| Accuracy | Crit Damage | Eyedrop bottle | shattered star |
| Accuracy | Crit Chance | Eyedrop bottle | crosshair sparkle |
| Accuracy | Range | Eyedrop bottle | distance arrow with ruler marks |
| Accuracy | Accuracy | Eyedrop bottle | aligned reticle and laser dot |
| Armor | Taunt | Trauma pouch | aggro eye siren |
| Armor | Damage Reduction | Trauma pouch | shield plate |
| Armor | Damage Reflection | Trauma pouch | mirror shard |
| Armor | Crush | Trauma pouch | cracked shield |
| Evasion | Dodge | Inhaler | sidestep slash |
| Evasion | Counter Attack | Inhaler | return-blade arrow |
| Evasion | Invisibility | Inhaler | faded eye |
| Evasion | Assassinate | Inhaler | dagger plus skull |
| Luck | Treasure Chest | Vial | treasure chest |
| Luck | Cheating | Vial | loaded dice |
| Luck | Stealing | Vial | lifted coin purse |
| Luck | Loot Crate | Vial | reward crate burst |

## Prompt Blocks

### AOE Damage

```text
Generate 1 square icon for AOE Damage.

Core object: a premium fantasy-medical syringe.
Inner emblem: a bold blast ring suspended in the fluid chamber.
This icon should feel brutal, volatile, and powerful.

Requirements:
- keep the syringe large and centered
- the blast ring emblem should read clearly at small size
- thick metal needle, reinforced glass chamber, subtle engraved fittings
- restrained luminous fluid and destructive energy
- flat neutral grey background only
```

### Bounce Damage

```text
Generate 1 square icon for Bounce Damage.

Core object: a premium fantasy-medical syringe.
Inner emblem: a ricochet arrow symbol suspended in the fluid chamber.
This icon should feel kinetic, sharp, and rebound-focused.

Requirements:
- keep the syringe large and centered
- the ricochet emblem should be simple and highly readable
- elegant metalwork and sturdy glass chamber
- restrained energized fluid with a hint of ricochet motion
- flat neutral grey background only
```

### Pierce Damage

```text
Generate 1 square icon for Pierce Damage.

Core object: a premium fantasy-medical syringe.
Inner emblem: a spear tip piercing through a horizontal line inside the chamber.
This icon should feel precise, lethal, and penetrating.

Requirements:
- keep the syringe large and centered
- emblem should clearly communicate penetration
- sharp premium silhouette, polished metal needle, durable glass
- subtle piercing energy accents only
- flat neutral grey background only
```

### DOT Damage

```text
Generate 1 square icon for DOT Damage.

Core object: a premium fantasy-medical syringe.
Inner emblem: a dripping drop symbol suspended in the chamber.
This icon should feel toxic, lingering, and dangerous.

Requirements:
- keep the syringe large and centered
- the dripping emblem must read immediately
- elegant medical-fantasy construction, vivid fluid readability
- restrained poison-like or arcane corrosion accents
- flat neutral grey background only
```

### AOE Speed

```text
Generate 1 square icon for AOE Speed.

Core object: a premium fantasy pill capsule.
Inner emblem: a pulse ring with speed lines embedded in the capsule core.
This icon should feel rhythmic, accelerated, and energetic.

Requirements:
- keep the capsule large and centered
- the pulse emblem should remain simple and readable
- polished shell, premium materials, slight medical-fantasy ornament
- restrained motion accents only
- flat neutral grey background only
```

### Bounce Speed

```text
Generate 1 square icon for Bounce Speed.

Core object: a premium fantasy pill capsule.
Inner emblem: a ricochet chevron trail embedded in the capsule core.
This icon should feel springy, fast, and agile.

Requirements:
- keep the capsule large and centered
- the chevron trail emblem should read in one glance
- clean silhouette and premium finish
- restrained kinetic accents only
- flat neutral grey background only
```

### Pierce Speed

```text
Generate 1 square icon for Pierce Speed.

Core object: a premium fantasy pill capsule.
Inner emblem: a dart streak symbol embedded in the capsule core.
This icon should feel swift, focused, and surgical.

Requirements:
- keep the capsule large and centered
- the dart streak emblem should feel directional and fast
- polished premium materials with minimal ornament
- restrained speed accents only
- flat neutral grey background only
```

### DOT Speed

```text
Generate 1 square icon for DOT Speed.

Core object: a premium fantasy pill capsule.
Inner emblem: a timer plus droplet symbol embedded in the capsule core.
This icon should feel paced, timed, and persistent.

Requirements:
- keep the capsule large and centered
- timer and droplet should still read clearly at small size
- premium medical-fantasy finish
- restrained temporal energy accents only
- flat neutral grey background only
```

### AOE Scale

```text
Generate 1 square icon for AOE Scale.

Core object: a premium fantasy IV bag.
Inner emblem: expanding circles visible in the fluid reservoir.
This icon should feel enlarging, expansive, and area-focused.

Requirements:
- keep the IV bag large and centered
- expanding-circle emblem must read clearly through the bag
- elegant hooks, fittings, and premium translucent materials
- restrained fluid glow only
- flat neutral grey background only
```

### Bounce Scale

```text
Generate 1 square icon for Bounce Scale.

Core object: a premium fantasy IV bag.
Inner emblem: an enlarged rebound arc visible in the fluid reservoir.
This icon should feel elastic, broad, and rebound-enhancing.

Requirements:
- keep the IV bag large and centered
- emblem should clearly imply larger rebounds
- premium translucent reservoir and controlled lighting
- restrained energy accents only
- flat neutral grey background only
```

### Pierce Scale

```text
Generate 1 square icon for Pierce Scale.

Core object: a premium fantasy IV bag.
Inner emblem: a long needle lance visible in the fluid reservoir.
This icon should feel elongated, piercing, and reach-enhancing.

Requirements:
- keep the IV bag large and centered
- the lance emblem should feel long and directional
- premium translucent materials and polished metal fittings
- restrained scale energy only
- flat neutral grey background only
```

### DOT Scale

```text
Generate 1 square icon for DOT Scale.

Core object: a premium fantasy IV bag.
Inner emblem: a spreading spill cloud visible in the fluid reservoir.
This icon should feel widening, contaminated, and dangerous.

Requirements:
- keep the IV bag large and centered
- the spreading cloud emblem should read immediately
- premium medical-fantasy construction
- restrained corruption-like energy only
- flat neutral grey background only
```

### Crit Damage

```text
Generate 1 square icon for Crit Damage.

Core object: a premium fantasy-medical eyedrop bottle.
Inner emblem: a shattered star suspended in the bottle core.
This icon should feel lethal, high-impact, and surgically devastating.

Requirements:
- keep the eyedrop bottle large and centered
- the shattered star emblem should read immediately at small size
- premium glass, metal collar, and precision-crafted silhouette
- restrained impact energy only
- flat neutral grey background only
```

### Crit Chance

```text
Generate 1 square icon for Crit Chance.

Core object: a premium fantasy-medical eyedrop bottle.
Inner emblem: a crosshair sparkle suspended in the bottle core.
This icon should feel sharp-eyed, opportunistic, and precision-focused.

Requirements:
- keep the eyedrop bottle large and centered
- the crosshair sparkle emblem should be very readable and unmistakable
- premium polished shell with subtle targeting detail
- restrained sparkle energy only
- flat neutral grey background only
```

### Range

```text
Generate 1 square icon for Range.

Core object: a premium fantasy-medical eyedrop bottle.
Inner emblem: a long distance arrow with ruler marks suspended in the bottle core.
This icon should feel extended, disciplined, and sightline-focused.

Requirements:
- keep the eyedrop bottle large and centered
- the distance emblem should communicate longer reach in one glance
- elegant premium materials and clear silhouette
- restrained long-range energy only
- flat neutral grey background only
```

### Accuracy

```text
Generate 1 square icon for Accuracy.

Core object: a premium fantasy-medical eyedrop bottle.
Inner emblem: an aligned reticle with a laser dot suspended in the bottle core.
This icon should feel exact, disciplined, and dead-center precise.

Requirements:
- keep the eyedrop bottle large and centered
- the aligned reticle and laser dot should read clearly at small size
- premium precision-themed silhouette with subtle sighting details
- restrained laser energy only
- flat neutral grey background only
```

### Taunt

```text
Generate 1 square icon for Taunt.

Core object: a premium fantasy trauma pouch.
Inner emblem: an aggro eye siren suspended in the pouch centerpiece.
This icon should feel provocative, loud, and threat-drawing.

Requirements:
- keep the trauma pouch large and centered
- the aggro eye emblem should read instantly
- rugged premium materials, reinforced stitching, and field-ready silhouette
- restrained alarm energy only
- flat neutral grey background only
```

### Damage Reduction

```text
Generate 1 square icon for Damage Reduction.

Core object: a premium fantasy trauma pouch.
Inner emblem: a shield plate suspended in the pouch centerpiece.
This icon should feel durable, stabilizing, and protective.

Requirements:
- keep the trauma pouch large and centered
- the shield emblem should be obvious at icon size
- rugged premium materials and clear front-facing silhouette
- restrained defensive glow only
- flat neutral grey background only
```

### Damage Reflection

```text
Generate 1 square icon for Damage Reflection.

Core object: a premium fantasy trauma pouch.
Inner emblem: a mirror shard suspended in the pouch centerpiece.
This icon should feel retaliatory, sharp, and dangerous.

Requirements:
- keep the trauma pouch large and centered
- the mirror-shard emblem should feel reflective and readable
- reinforced pouch materials and premium fittings
- restrained rebound energy only
- flat neutral grey background only
```

### Crush

```text
Generate 1 square icon for Crush.

Core object: a premium fantasy trauma pouch.
Inner emblem: a cracked shield suspended in the pouch centerpiece.
This icon should feel heavy, armor-breaking, and oppressive.

Requirements:
- keep the trauma pouch large and centered
- the cracked-shield emblem should read at a glance
- rugged premium materials and strong silhouette
- restrained crushing energy only
- flat neutral grey background only
```

### Dodge

```text
Generate 1 square icon for Dodge.

Core object: a premium fantasy inhaler.
Inner emblem: a sidestep slash symbol embedded in the inhaler core.
This icon should feel evasive, reactive, and nimble.

Requirements:
- keep the inhaler large and centered
- the sidestep emblem should read in one glance
- premium shell materials and clean silhouette
- restrained agility accents only
- flat neutral grey background only
```

### Counter Attack

```text
Generate 1 square icon for Counter Attack.

Core object: a premium fantasy inhaler.
Inner emblem: a return-blade arrow embedded in the inhaler core.
This icon should feel reactive, retaliatory, and sharp.

Requirements:
- keep the inhaler large and centered
- the return-blade emblem should be immediately legible
- premium shell materials and controlled lighting
- restrained retaliatory accents only
- flat neutral grey background only
```

### Invisibility

```text
Generate 1 square icon for Invisibility.

Core object: a premium fantasy inhaler.
Inner emblem: a faded eye embedded in the inhaler core.
This icon should feel hidden, quiet, and elusive.

Requirements:
- keep the inhaler large and centered
- the faded-eye emblem should feel stealthy but readable
- premium shell materials and subtle silhouette refinement
- restrained concealment accents only
- flat neutral grey background only
```

### Assassinate

```text
Generate 1 square icon for Assassinate.

Core object: a premium fantasy inhaler.
Inner emblem: a dagger plus skull embedded in the inhaler core.
This icon should feel lethal, surgical, and sinister.

Requirements:
- keep the inhaler large and centered
- the dagger-skull emblem should read immediately
- premium shell materials and strong silhouette
- restrained lethal energy only
- flat neutral grey background only
```

### Treasure Chest

```text
Generate 1 square icon for Treasure Chest.

Core object: a premium fantasy vial.
Inner emblem: a treasure chest suspended in the liquid.
This icon should feel rewarding, wealthy, and desirable.

Requirements:
- keep the vial large and centered
- the treasure-chest emblem should be bold and recognizable
- premium glass and cap details
- restrained treasure shimmer only
- flat neutral grey background only
```

### Cheating

```text
Generate 1 square icon for Cheating.

Core object: a premium fantasy vial.
Inner emblem: loaded dice suspended in the liquid.
This icon should feel sly, rigged, and mischievous.

Requirements:
- keep the vial large and centered
- dice emblem should read instantly
- premium vial materials and clear silhouette
- restrained mischievous energy only
- flat neutral grey background only
```

### Stealing

```text
Generate 1 square icon for Stealing.

Core object: a premium fantasy vial.
Inner emblem: a lifted coin purse suspended in the liquid.
This icon should feel sneaky, greedy, and opportunistic.

Requirements:
- keep the vial large and centered
- the coin-purse emblem should be simple and readable
- premium glass and cap details
- restrained thieving accents only
- flat neutral grey background only
```

### Loot Crate

```text
Generate 1 square icon for Loot Crate.

Core object: a premium fantasy vial.
Inner emblem: a reward crate burst suspended in the liquid.
This icon should feel rewarding, explosive, and prize-focused.

Requirements:
- keep the vial large and centered
- the crate emblem should be bold and recognizable
- premium vial materials with restrained inner shimmer
- restrained reward energy only
- flat neutral grey background only
```
