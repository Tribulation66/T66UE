# Item Icon Prompt Pack

This file is meant to be uploaded directly into ChatGPT as a prompt reference pack for item icon generation.

## Project Context

- Generate icons for 34 live items total: 33 regular items plus Gambler's Token.
- Every item needs 4 rarity variants: Black, Red, Yellow, and White.
- Total image count: 136 icons.
- Fountain of Life is not an item and should not receive icon prompts. In-game it is a world interactable that fully heals the player and grants +1 max heart.
- The art direction is changing away from pixel art and goofy props.
- The icons should look more intuitive, more equipment-like, more mythic, and closer to premium Dota 2 item icon style.
- The same core object must remain consistent across all 4 rarities of a given item. Higher rarities should look like upgraded versions of the same item, not a different item.

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

| Category | Item | Object |
| --- | --- | --- |
| Damage | AOE Damage | Titan Hammer |
| Damage | Bounce Damage | Moon Shuriken |
| Damage | Pierce Damage | Falchion |
| Damage | DOT Damage | Arcane Wand |
| Damage | Crit Damage | Executioner Axe |
| Damage | Close Range Damage | Boxing Gloves |
| Damage | Long Range Damage | Sniper Rifle |
| Attack Speed | AOE Speed | War Drum |
| Attack Speed | Bounce Speed | Trickster Bracer |
| Attack Speed | Pierce Speed | Repeater Crossbow |
| Attack Speed | DOT Speed | Ritual Censer |
| Attack Speed | Crit Chance | Duelist Gloves |
| Attack Scale | AOE Scale | Earthshaker Totem |
| Attack Scale | Bounce Scale | Storm Ring |
| Attack Scale | Pierce Scale | Dragon Lance |
| Attack Scale | DOT Scale | Venom Orb |
| Attack Scale | Range | Sniper Scope |
| Armor | Taunt | Red Matador Cape |
| Armor | Reflect Damage | Mirror Shield |
| Armor | HP Regen | Blessed Cuirass |
| Armor | Crush | Spiked Tower Shield |
| Armor | Damage Reduction | Bulwark Plate |
| Evasion | Invisibility | Veil Cloak |
| Evasion | Counter Attack | Parrying Dagger |
| Evasion | Life Steal | Vampiric Mask |
| Evasion | Assassinate | Assassin Dagger |
| Evasion | Evasion Chance | Feathered Mantle |
| Luck | Spin Wheel | Spin Wheel |
| Luck | Loot Crate | Loot Crate |
| Luck | Treasure Chest | Treasure Compass |
| Luck | Cheating | Loaded Dice |
| Luck | Stealing | Wallet |
| Speed | Movement Speed | Hermes Sandals |
| Special | Gambler's Token | Infernal Casino Token |

## Prompt Blocks

### AOE Damage

```text
Using the style rules above, generate 4 separate square item icons for AOE Damage.

Core object: a massive Titan Hammer.
This item represents raw area damage. It should feel brutal, weighty, ancient, and mythic.

Rarity variants:
1. Black rarity: dark iron titan hammer, rough forged head, simple leather grip, minimal ornament.
2. Red rarity: same hammer upgraded with crimson runes, heated metal seams, heavier silhouette, stronger glow.
3. Yellow rarity: same hammer upgraded into a gilded thunder hammer, engraved head, ornate gold fittings, premium craftsmanship.
4. White rarity: same hammer upgraded into a divine celestial hammer, ivory metal, radiant core, holy light, highest prestige.

Requirements:
- same hammer concept across all 4 icons
- each rarity should feel like an upgrade, not a different weapon class
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy or toy-like style
```

### Bounce Damage

```text
Using the style rules above, generate 4 separate square item icons for Bounce Damage.

Core object: a razor-edged Moon Shuriken.
This item represents ricochet damage. It should feel fast, lethal, precise, and mythic.

Rarity variants:
1. Black rarity: dark steel shuriken, simple four-blade silhouette, restrained detail, subtle edge shine.
2. Red rarity: same shuriken upgraded with crimson inlays, hotter edge glow, sharper blade profile.
3. Yellow rarity: same shuriken upgraded into a gilded masterwork, gold hub, ornate engravings, richer materials.
4. White rarity: same shuriken upgraded into a celestial relic, ivory-silver metal, radiant center, holy energy accents.

Requirements:
- same shuriken concept across all 4 icons
- each rarity should feel like a premium upgrade
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Pierce Damage

```text
Using the style rules above, generate 4 separate square item icons for Pierce Damage.

Core object: a war Falchion with a strong thrusting point.
This item represents penetration and deadly armor-piercing strikes. It should feel disciplined, sharp, and dangerous.

Rarity variants:
1. Black rarity: dark steel falchion, battle-worn blade, wrapped grip, minimal ornament.
2. Red rarity: same falchion upgraded with crimson fuller lines, hotter metal highlights, more aggressive guard.
3. Yellow rarity: same falchion upgraded into a gilded officer's blade, engraved spine, jewel-set hilt, premium finish.
4. White rarity: same falchion upgraded into a divine relic sword, ivory-gold hilt, radiant edge, holy glow.

Requirements:
- same falchion concept across all 4 icons
- the weapon should remain clearly readable at icon size
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### DOT Damage

```text
Using the style rules above, generate 4 separate square item icons for DOT Damage.

Core object: an Arcane Wand.
This item represents lingering damage over time. It should feel cursed, alchemical, magical, and dangerous.

Rarity variants:
1. Black rarity: dark wood wand, dim toxic crystal tip, subtle corrosion, minimal ornament.
2. Red rarity: same wand upgraded with crimson veins, stronger poison glow, more ritual metal bands.
3. Yellow rarity: same wand upgraded into an ornate plague scepter, gilded handle, refined crystal, premium magical finish.
4. White rarity: same wand upgraded into a divine blight relic, ivory shaft, radiant venom core, bright holy-corrupted glow.

Requirements:
- same wand concept across all 4 icons
- it should look like a real premium MOBA item icon
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Crit Damage

```text
Using the style rules above, generate 4 separate square item icons for Crit Damage.

Core object: an Executioner Axe.
This item represents devastating burst damage and fatal strikes. It should feel heavy, brutal, and elite.

Rarity variants:
1. Black rarity: dark steel executioner axe, broad blade, plain haft, minimal ornament.
2. Red rarity: same axe upgraded with crimson runes, hotter cutting edge, reinforced fittings, stronger menace.
3. Yellow rarity: same axe upgraded into a gilded headsman's weapon, ornate carvings, polished gold trim, premium metalwork.
4. White rarity: same axe upgraded into a celestial executioner's relic, ivory-metal haft, radiant blade, divine intensity.

Requirements:
- same axe concept across all 4 icons
- each rarity should feel nastier and more prestigious
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Close Range Damage

```text
Using the style rules above, generate 4 separate square item icons for Close Range Damage.

Core object: mythic Boxing Gloves.
This item represents brutal close-quarters power. The gloves should feel like serious combat equipment, not sports gear.

Rarity variants:
1. Black rarity: dark leather boxing gloves with metal seams, compact silhouette, minimal ornament.
2. Red rarity: same gloves upgraded with crimson stitching, heavier plating, stronger impact glow.
3. Yellow rarity: same gloves upgraded into gilded war gloves, ornate cuffs, luxury materials, premium finish.
4. White rarity: same gloves upgraded into divine champion gloves, ivory leather, radiant trim, celestial glow.

Requirements:
- same glove concept across all 4 icons
- they should read as dangerous fantasy equipment
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Long Range Damage

```text
Using the style rules above, generate 4 separate square item icons for Long Range Damage.

Core object: a Sniper Rifle.
This item represents deadly ranged precision and long-distance lethality. It should feel elite, engineered, and high-impact.

Rarity variants:
1. Black rarity: dark wood-and-steel sniper rifle, long barrel, restrained detail, practical silhouette.
2. Red rarity: same rifle upgraded with crimson glow lines, reinforced scope mounts, hotter barrel accents.
3. Yellow rarity: same rifle upgraded into a gilded marksman's masterpiece, engraved stock, gold trim, premium optics.
4. White rarity: same rifle upgraded into a celestial deadeye rifle, ivory-metal body, radiant barrel core, divine precision.

Requirements:
- same rifle concept across all 4 icons
- clear silhouette and readable scope profile
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### AOE Speed

```text
Using the style rules above, generate 4 separate square item icons for AOE Speed.

Core object: a War Drum.
This item represents faster area attacks through tempo and battlefield rhythm. It should feel martial, loud, and commanding.

Rarity variants:
1. Black rarity: dark wood war drum, plain hide, iron studs, minimal ornament.
2. Red rarity: same drum upgraded with crimson straps, ember runes, stronger pulse glow.
3. Yellow rarity: same drum upgraded into a gilded battle drum, ornate frame, gold fittings, premium craftsmanship.
4. White rarity: same drum upgraded into a celestial war drum, ivory shell, radiant surface, divine energy pulse.

Requirements:
- same drum concept across all 4 icons
- it should feel like a premium fantasy relic
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Bounce Speed

```text
Using the style rules above, generate 4 separate square item icons for Bounce Speed.

Core object: a Trickster Bracer.
This item represents faster ricochet attacks. It should feel agile, clever, and lightly arcane.

Rarity variants:
1. Black rarity: dark leather-and-steel bracer, clean silhouette, subtle etched lines, minimal ornament.
2. Red rarity: same bracer upgraded with crimson gems, more kinetic glow, sharper layered plates.
3. Yellow rarity: same bracer upgraded into an ornate trickster relic, gilded trim, premium gem setting, elegant detail.
4. White rarity: same bracer upgraded into a celestial ricochet bracer, ivory metals, radiant core, holy energy streaks.

Requirements:
- same bracer concept across all 4 icons
- keep it readable as an arm equipment piece
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Pierce Speed

```text
Using the style rules above, generate 4 separate square item icons for Pierce Speed.

Core object: a Repeater Crossbow.
This item represents rapid piercing fire. It should feel mechanical, deadly, and efficient.

Rarity variants:
1. Black rarity: dark steel repeater crossbow, compact magazine shape, plain limbs, restrained detail.
2. Red rarity: same crossbow upgraded with crimson bolts, hotter energy rails, more aggressive mechanics.
3. Yellow rarity: same crossbow upgraded into a gilded marksman device, ornate stock, premium metalwork, refined mechanism.
4. White rarity: same crossbow upgraded into a celestial repeater, ivory-metal frame, radiant limbs, divine energy chamber.

Requirements:
- same crossbow concept across all 4 icons
- it should remain the same weapon class as rarity increases
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### DOT Speed

```text
Using the style rules above, generate 4 separate square item icons for DOT Speed.

Core object: a Ritual Censer.
This item represents faster application of lingering damage. It should feel occult, smoky, ceremonial, and dangerous.

Rarity variants:
1. Black rarity: dark iron censer, thin smoke trails, simple chains, minimal ornament.
2. Red rarity: same censer upgraded with crimson smoke glow, hotter vents, more ritual engravings.
3. Yellow rarity: same censer upgraded into a gilded plague censer, ornate body, premium chainwork, rich magical detail.
4. White rarity: same censer upgraded into a celestial ritual relic, ivory metal, radiant fumes, holy-corrupted glow.

Requirements:
- same censer concept across all 4 icons
- keep the smoke secondary to the object silhouette
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Crit Chance

```text
Using the style rules above, generate 4 separate square item icons for Crit Chance.

Core object: Duelist Gloves.
This item represents precision, timing, and higher critical hit chance. It should feel elegant, sharp, and high-skill.

Rarity variants:
1. Black rarity: dark leather duelist gloves, slim silhouette, subtle stitching, minimal ornament.
2. Red rarity: same gloves upgraded with crimson lining, sharper wrist guards, stronger precision glow.
3. Yellow rarity: same gloves upgraded into gilded fencing gloves, ornate cuffs, premium fabric, jewel accents.
4. White rarity: same gloves upgraded into celestial duelist gloves, ivory leather, radiant trim, refined divine finish.

Requirements:
- same glove concept across all 4 icons
- the design should read as precision gear, not bulky armor
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### AOE Scale

```text
Using the style rules above, generate 4 separate square item icons for AOE Scale.

Core object: an Earthshaker Totem.
This item represents larger area coverage. It should feel ancient, heavy, primal, and powerful.

Rarity variants:
1. Black rarity: dark stone totem, cracked surface, rough carving, minimal ornament.
2. Red rarity: same totem upgraded with crimson fissures, stronger inner glow, heavier rune cuts.
3. Yellow rarity: same totem upgraded into a gilded earth relic, refined carvings, gold bands, premium craft.
4. White rarity: same totem upgraded into a celestial world-totem, pale stone, radiant core, divine aura.

Requirements:
- same totem concept across all 4 icons
- the object should feel monumental even at icon scale
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Bounce Scale

```text
Using the style rules above, generate 4 separate square item icons for Bounce Scale.

Core object: a Storm Ring.
This item represents larger ricochet reach and bounce influence. It should feel energetic, elegant, and arcane.

Rarity variants:
1. Black rarity: dark metal ring, subtle crackling energy, simple silhouette, restrained detail.
2. Red rarity: same ring upgraded with crimson lightning veins, stronger charge glow, sharper setting.
3. Yellow rarity: same ring upgraded into a gilded storm relic, ornate bezel, premium metalwork, richer energy accents.
4. White rarity: same ring upgraded into a celestial tempest ring, ivory-gold body, radiant charge core, divine electricity.

Requirements:
- same ring concept across all 4 icons
- keep it centered and clearly readable as a ring-shaped artifact
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Pierce Scale

```text
Using the style rules above, generate 4 separate square item icons for Pierce Scale.

Core object: a Dragon Lance.
This item represents greater reach and larger piercing impact. It should feel noble, lethal, and elite.

Rarity variants:
1. Black rarity: dark steel lance tip and shaft, restrained detail, practical war silhouette.
2. Red rarity: same lance upgraded with crimson scale motifs, hotter metal seams, stronger prestige.
3. Yellow rarity: same lance upgraded into a gilded dragon lance, ornate guard, premium engravings, jewel accents.
4. White rarity: same lance upgraded into a celestial dragon relic, ivory shaft, radiant spearhead, divine glow.

Requirements:
- same lance concept across all 4 icons
- keep the silhouette long and unmistakable
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### DOT Scale

```text
Using the style rules above, generate 4 separate square item icons for DOT Scale.

Core object: a Venom Orb.
This item represents a larger damage-over-time effect footprint. It should feel toxic, magical, and dangerous.

Rarity variants:
1. Black rarity: dark glass orb, murky poison core, restrained glow, minimal ornament.
2. Red rarity: same orb upgraded with crimson toxin currents, stronger internal light, richer casing detail.
3. Yellow rarity: same orb upgraded into a gilded venom relic, ornate frame, premium crystal clarity, bright poison energy.
4. White rarity: same orb upgraded into a celestial plague orb, ivory-gold casing, radiant toxic core, divine-corrupted light.

Requirements:
- same orb concept across all 4 icons
- keep the orb itself dominant in the frame
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Range

```text
Using the style rules above, generate 4 separate square item icons for Range.

Core object: a Sniper Scope.
This item represents increased attack range. It should feel engineered, precise, and premium.

Rarity variants:
1. Black rarity: dark steel scope, clear glass lens, practical mounts, restrained detail.
2. Red rarity: same scope upgraded with crimson lens glow, sharper fittings, stronger precision feel.
3. Yellow rarity: same scope upgraded into a gilded marksman optic, engraved body, premium lens housing, luxury finish.
4. White rarity: same scope upgraded into a celestial deadeye scope, ivory-metal casing, radiant lens core, divine clarity.

Requirements:
- same scope concept across all 4 icons
- keep the lens readable and central
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Taunt

```text
Using the style rules above, generate 4 separate square item icons for Taunt.

Core object: a Red Matador Cape.
This item represents drawing enemy attention. It should feel dramatic, elegant, and confrontational.

Rarity variants:
1. Black rarity: dark crimson cape, heavy cloth, simple folds, minimal ornament.
2. Red rarity: same cape upgraded with richer scarlet fabric, stronger motion shape, ornate trim.
3. Yellow rarity: same cape upgraded into a gilded duelist cape, gold embroidery, premium cloth, noble finish.
4. White rarity: same cape upgraded into a celestial champion cape, ivory-red fabric, radiant edging, divine prestige.

Requirements:
- same cape concept across all 4 icons
- the cape should feel like a premium fantasy item, not costume cloth
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Reflect Damage

```text
Using the style rules above, generate 4 separate square item icons for Reflect Damage.

Core object: a Mirror Shield.
This item represents turning damage back on attackers. It should feel defensive, polished, and magical.

Rarity variants:
1. Black rarity: dark steel mirror shield, muted reflective center, simple rim, minimal ornament.
2. Red rarity: same shield upgraded with crimson glow lines, stronger mirror sheen, sharper rim detailing.
3. Yellow rarity: same shield upgraded into a gilded reflective bulwark, ornate frame, gold relief work, premium polish.
4. White rarity: same shield upgraded into a celestial mirror relic, ivory-gold frame, radiant reflective core, divine light.

Requirements:
- same shield concept across all 4 icons
- the mirror center should stay obvious in every rarity
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### HP Regen

```text
Using the style rules above, generate 4 separate square item icons for HP Regen.

Core object: a Blessed Cuirass.
This item represents durability and steady healing. It should feel protective, noble, and restorative.

Rarity variants:
1. Black rarity: dark steel cuirass, simple breastplate silhouette, restrained detail, minimal ornament.
2. Red rarity: same cuirass upgraded with crimson lining, warmer glow, stronger plate shaping.
3. Yellow rarity: same cuirass upgraded into a gilded blessed armor piece, ornate chest engravings, premium finish.
4. White rarity: same cuirass upgraded into a celestial breastplate, ivory-gold armor, radiant core, holy restoration glow.

Requirements:
- same cuirass concept across all 4 icons
- it should read as a chest armor item, not a full character
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Crush

```text
Using the style rules above, generate 4 separate square item icons for Crush.

Core object: a Spiked Tower Shield.
This item represents brutal reflected impact and lethal punishment. It should feel imposing, defensive, and vicious.

Rarity variants:
1. Black rarity: dark iron tower shield, heavy spikes, plain face, restrained detail.
2. Red rarity: same shield upgraded with crimson spike glow, stronger metal seams, more intimidating silhouette.
3. Yellow rarity: same shield upgraded into a gilded war bulwark, ornate spikes, premium metalwork, engraved surface.
4. White rarity: same shield upgraded into a celestial crushing shield, ivory-metal body, radiant spikes, divine power.

Requirements:
- same tower shield concept across all 4 icons
- the spikes should remain prominent but readable
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Damage Reduction

```text
Using the style rules above, generate 4 separate square item icons for Damage Reduction.

Core object: Bulwark Plate.
This item represents direct damage reduction. It should feel dense, reliable, elite, and extremely protective.

Rarity variants:
1. Black rarity: dark heavy plate armor piece, thick layered metal, restrained detail, minimal ornament.
2. Red rarity: same plate upgraded with crimson reinforcement lines, hotter seams, stronger defensive presence.
3. Yellow rarity: same plate upgraded into a gilded fortress plate, ornate layering, premium steel, gold accents.
4. White rarity: same plate upgraded into a celestial bulwark relic, ivory-gold plate, radiant core, divine protection glow.

Requirements:
- same armor plate concept across all 4 icons
- it should feel heavier and more protective with each rarity
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Invisibility

```text
Using the style rules above, generate 4 separate square item icons for Invisibility.

Core object: a Veil Cloak.
This item represents stealth and concealment. It should feel mysterious, elegant, and magical.

Rarity variants:
1. Black rarity: dark cloak, soft folds, faint shimmer, minimal ornament.
2. Red rarity: same cloak upgraded with crimson inner lining, stronger fade effect, richer trim.
3. Yellow rarity: same cloak upgraded into a gilded stealth mantle, premium fabric, ornate clasp, refined magical sheen.
4. White rarity: same cloak upgraded into a celestial veil, ivory fabric, radiant translucence, divine stealth aura.

Requirements:
- same cloak concept across all 4 icons
- keep the cloak itself readable and centered
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Counter Attack

```text
Using the style rules above, generate 4 separate square item icons for Counter Attack.

Core object: a Parrying Dagger.
This item represents timing, defense, and retaliatory strikes. It should feel quick, elegant, and lethal.

Rarity variants:
1. Black rarity: dark steel parrying dagger, simple guard, practical silhouette, minimal ornament.
2. Red rarity: same dagger upgraded with crimson edge glow, stronger hand guard, richer detail.
3. Yellow rarity: same dagger upgraded into a gilded duelist relic, ornate guard, premium grip, engraved blade.
4. White rarity: same dagger upgraded into a celestial counterblade, ivory-gold hilt, radiant edge, divine precision.

Requirements:
- same dagger concept across all 4 icons
- keep the parrying guard visible
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Life Steal

```text
Using the style rules above, generate 4 separate square item icons for Life Steal.

Core object: a Vampiric Mask.
This item represents draining life from enemies. It should feel sinister, aristocratic, and supernatural.

Rarity variants:
1. Black rarity: dark metal-and-leather mask, dim red accents, restrained detail, minimal ornament.
2. Red rarity: same mask upgraded with stronger blood glow, richer surface carving, more dangerous presence.
3. Yellow rarity: same mask upgraded into a gilded vampire relic, ornate cheek plates, premium finish, jewel accents.
4. White rarity: same mask upgraded into a celestial blood relic, ivory mask, radiant red core, divine-corrupted light.

Requirements:
- same mask concept across all 4 icons
- it should feel iconic and readable at small size
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Assassinate

```text
Using the style rules above, generate 4 separate square item icons for Assassinate.

Core object: an Assassin Dagger.
This item represents lethal finishing potential. It should feel sleek, elite, and merciless.

Rarity variants:
1. Black rarity: dark assassin dagger, narrow blade, practical grip, minimal ornament.
2. Red rarity: same dagger upgraded with crimson linework, stronger edge glow, sharper profile.
3. Yellow rarity: same dagger upgraded into a gilded killer's relic, ornate hilt, premium steel, refined menace.
4. White rarity: same dagger upgraded into a celestial execution blade, ivory-gold hilt, radiant edge, divine lethality.

Requirements:
- same dagger concept across all 4 icons
- it should feel more surgical than the parrying dagger
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Evasion Chance

```text
Using the style rules above, generate 4 separate square item icons for Evasion Chance.

Core object: a Feathered Mantle.
This item represents improved dodge chance. It should feel light, graceful, and elite.

Rarity variants:
1. Black rarity: dark feathered mantle, subtle layered feathers, restrained detail, minimal ornament.
2. Red rarity: same mantle upgraded with crimson feathers, stronger motion curve, richer trim.
3. Yellow rarity: same mantle upgraded into a gilded evasive mantle, premium feather arrangement, ornate clasp, noble finish.
4. White rarity: same mantle upgraded into a celestial winged mantle, ivory feathers, radiant highlights, divine agility.

Requirements:
- same mantle concept across all 4 icons
- it should feel light and prestigious rather than bulky
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Spin Wheel

```text
Using the style rules above, generate 4 separate square item icons for Spin Wheel.

Core object: an actual Spin Wheel.
This item should literally be a spin wheel, but it should still feel like a premium fantasy casino artifact rather than a toy prop.

Rarity variants:
1. Black rarity: dark wood-and-iron spin wheel, simple wedges, restrained detail, minimal ornament.
2. Red rarity: same wheel upgraded with crimson lacquer, glowing divider lines, richer metal trim.
3. Yellow rarity: same wheel upgraded into a gilded casino relic, ornate spokes, premium finish, jewel accents.
4. White rarity: same wheel upgraded into a celestial fortune wheel, ivory-gold body, radiant center, divine luck glow.

Requirements:
- same spin wheel concept across all 4 icons
- the wheel should stay centered and readable at icon scale
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Loot Crate

```text
Using the style rules above, generate 4 separate square item icons for Loot Crate.

Core object: an actual Loot Crate.
This item should literally be a loot crate, but it should feel like a premium fantasy reward container, not a cheap box.

Rarity variants:
1. Black rarity: dark wood crate with iron bands, simple silhouette, restrained detail, minimal ornament.
2. Red rarity: same crate upgraded with crimson panels, stronger lock glow, richer metal reinforcements.
3. Yellow rarity: same crate upgraded into a gilded reward chest-crate hybrid, ornate corners, premium craftsmanship, treasure feel.
4. White rarity: same crate upgraded into a celestial vault crate, ivory-gold frame, radiant seams, divine reward glow.

Requirements:
- same crate concept across all 4 icons
- the object should remain clearly a crate in every rarity
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Treasure Chest

```text
Using the style rules above, generate 4 separate square item icons for Treasure Chest.

Core object: a Treasure Compass.
This item represents finding treasure and better fortune. It should feel adventurous, valuable, and mystical.

Rarity variants:
1. Black rarity: dark metal compass, simple casing, muted glow, restrained detail.
2. Red rarity: same compass upgraded with crimson inlays, stronger needle glow, richer casework.
3. Yellow rarity: same compass upgraded into a gilded treasure relic, ornate bezel, jewel accents, premium finish.
4. White rarity: same compass upgraded into a celestial fortune compass, ivory-gold housing, radiant face, divine guidance glow.

Requirements:
- same compass concept across all 4 icons
- keep the compass clearly readable at icon size
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Cheating

```text
Using the style rules above, generate 4 separate square item icons for Cheating.

Core object: Loaded Dice.
This item represents rigged luck and casino manipulation. It should feel clever, dangerous, and expensive.

Rarity variants:
1. Black rarity: dark carved dice, subtle glow in pips, restrained detail, minimal ornament.
2. Red rarity: same dice upgraded with crimson pips, hotter edge glow, richer carved markings.
3. Yellow rarity: same dice upgraded into gilded casino dice, premium polish, jewel-like pips, luxury finish.
4. White rarity: same dice upgraded into celestial loaded dice, ivory-gold faces, radiant pips, divine luck glow.

Requirements:
- same dice concept across all 4 icons
- keep the dice large and readable in frame
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Stealing

```text
Using the style rules above, generate 4 separate square item icons for Stealing.

Core object: a Wallet.
This item represents thief-like luck and stealing power. The wallet should feel like a premium fantasy rogue item, not a modern everyday accessory.

Rarity variants:
1. Black rarity: dark leather wallet, simple folded form, sturdy clasp, restrained detail.
2. Red rarity: same wallet upgraded with crimson stitching, richer leather, subtle coin glow, sharper trim.
3. Yellow rarity: same wallet upgraded into a gilded thief's wallet, gold clasp, ornate embossing, premium finish.
4. White rarity: same wallet upgraded into a celestial rogue wallet, ivory leather, radiant clasp, divine-luck accents.

Requirements:
- same wallet concept across all 4 icons
- it should feel like a fantasy relic carried by a master thief
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Movement Speed

```text
Using the style rules above, generate 4 separate square item icons for Movement Speed.

Core object: Hermes Sandals.
This item represents speed and mobility. It should feel mythological, agile, and iconic.

Rarity variants:
1. Black rarity: dark leather winged sandals, simple silhouette, restrained detail, minimal ornament.
2. Red rarity: same sandals upgraded with crimson straps, stronger motion glow, richer wing accents.
3. Yellow rarity: same sandals upgraded into gilded messenger sandals, ornate buckles, premium leather, noble finish.
4. White rarity: same sandals upgraded into celestial Hermes sandals, ivory leather, radiant wings, divine speed glow.

Requirements:
- same sandal concept across all 4 icons
- the wing motif should stay obvious and elegant
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Gambler's Token

```text
Using the style rules above, generate 4 separate square item icons for Gambler's Token.

Core object: an Infernal Casino Token.
This item represents high-risk gambling power. It should feel rare, dangerous, luxurious, and slightly sinister.

Rarity variants:
1. Black rarity: dark metal casino token, simple embossing, restrained glow, minimal ornament.
2. Red rarity: same token upgraded with crimson energy lines, richer engraving, hotter infernal glow.
3. Yellow rarity: same token upgraded into a gilded high-roller token, ornate edge detail, premium shine, jewel accents.
4. White rarity: same token upgraded into a celestial infernal token, ivory-gold body, radiant core, divine-luxury glow.

Requirements:
- same token concept across all 4 icons
- it should feel like a premium legendary casino artifact
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```
