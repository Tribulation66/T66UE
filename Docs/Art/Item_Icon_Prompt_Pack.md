# Item Icon Prompt Pack

This file is the canonical prompt reference pack for generating the live item icon set through the local ChatGPT bridge workflow.

## Project Context

- Generate icons for 29 live item entries total: 28 regular items plus Gambler's Token.
- Every item needs 4 rarity variants: Black, Red, Yellow, and White.
- Total image count: 116 icons.
- Deprecated compatibility items `HP Regen`, `Life Steal`, and `Alchemy` are intentionally excluded.
- Legacy non-live targets `Close Range Damage`, `Long Range Damage`, `Spin Wheel`, and `Movement Speed` are intentionally excluded.
- The icons should look intuitive, mythic, premium, and closer to high-end Dota 2 item art than pixel-art or goofy props.
- The same core object must remain consistent across all 4 rarities of a given item. Higher rarities should look like upgraded versions of the same item, not a different item class.

## Master Context Prompt

```text
You are generating square item icons for a fantasy action roguelike. The icons must look like premium Dota 2 item icons: painterly fantasy MOBA item art, high readability, strong silhouette, centered object, dramatic lighting, rich materials, polished game UI finish. Do not make pixel art. Do not make cartoon-goofy props. Do not add text, letters, numbers, borders, UI frames, hands, characters, or cluttered scenes.

Each request is for 4 rarity variants of the same item. Keep the same core object in all 4 images. As rarity increases, only upgrade the design, materials, ornamentation, glow, prestige, and visual complexity.

The Accuracy family items should all feel like elite precision gear, target-acquisition tools, and marksman equipment. Keep a consistent premium visual language across that family while still making each item easy to distinguish.

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
| Damage | AOE Damage | Item_AoeDamage | Titan Hammer |
| Damage | Bounce Damage | Item_BounceDamage | Moon Shuriken |
| Damage | Pierce Damage | Item_PierceDamage | Falchion |
| Damage | DOT Damage | Item_DotDamage | Arcane Wand |
| Accuracy | Crit Damage | Item_CritDamage | Killshot Prism |
| Attack Speed | AOE Speed | Item_AoeSpeed | War Drum |
| Attack Speed | Bounce Speed | Item_BounceSpeed | Trickster Bracer |
| Attack Speed | Pierce Speed | Item_PierceSpeed | Repeater Crossbow |
| Attack Speed | DOT Speed | Item_DotSpeed | Ritual Censer |
| Accuracy | Crit Chance | Item_CritChance | Duelist Monocle |
| Attack Scale | AOE Scale | Item_AoeScale | Earthshaker Totem |
| Attack Scale | Bounce Scale | Item_BounceScale | Storm Ring |
| Attack Scale | Pierce Scale | Item_PierceScale | Dragon Lance |
| Attack Scale | DOT Scale | Item_DotScale | Venom Orb |
| Accuracy | Range | Item_AttackRange | Longshot Scope |
| Accuracy | Accuracy | Item_Accuracy | Laser Sight |
| Armor | Taunt | Item_Taunt | Red Matador Cape |
| Armor | Damage Reduction | Item_DamageReduction | Bulwark Plate |
| Armor | Damage Reflection | Item_ReflectDmg | Mirror Shield |
| Armor | Crush | Item_Crush | Spiked Tower Shield |
| Evasion | Evasion Chance | Item_EvasionChance | Feathered Mantle |
| Evasion | Counter Attack | Item_CounterAttack | Parrying Dagger |
| Evasion | Invisibility | Item_Invisibility | Veil Cloak |
| Evasion | Assassinate | Item_Assassinate | Assassin Dagger |
| Luck | Treasure Chest | Item_TreasureChest | Treasure Compass |
| Luck | Cheating | Item_Cheating | Loaded Dice |
| Luck | Stealing | Item_Stealing | Wallet |
| Luck | Loot Crate | Item_LootCrate | Loot Crate |
| Special | Gambler's Token | Item_GamblersToken | Infernal Casino Token |

## Prompt Blocks

### AOE Damage

```text
Using the style rules above, generate 4 separate square item icons for AOE Damage.

Core object: a mythic Titan Hammer.
This item represents raw area damage, quake-like impact, and battlefield devastation. It should feel brutal, heavy, ancient, and catastrophic.

Rarity variants:
1. Black rarity: dark iron titan hammer, rough forged head, simple leather grip, minimal ornament.
2. Red rarity: same hammer upgraded with crimson runes, heated metal seams, heavier silhouette, stronger impact glow.
3. Yellow rarity: same hammer upgraded into a gilded thunder hammer, engraved head, ornate gold fittings, premium craftsmanship.
4. White rarity: same hammer upgraded into a divine celestial hammer, ivory metal, radiant core, holy light, highest prestige.

Requirements:
- same hammer concept across all 4 icons
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

Core object: a mythic Moon Shuriken.
This item represents ricochet damage and lethal rebound strikes. It should feel fast, sharp, disciplined, and dangerous.

Rarity variants:
1. Black rarity: dark steel shuriken, clean four-blade silhouette, restrained detail, subtle edge shine.
2. Red rarity: same shuriken upgraded with crimson inlays, hotter edges, sharper blade profile, stronger rebound energy.
3. Yellow rarity: same shuriken upgraded into a gilded masterwork, ornate engravings, richer materials, premium finish.
4. White rarity: same shuriken upgraded into a celestial ricochet relic, ivory-silver metal, radiant hub, holy precision glow.

Requirements:
- same shuriken concept across all 4 icons
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
This item represents penetration, armor-piercing strikes, and disciplined lethality. It should feel sharp, elite, and dangerous.

Rarity variants:
1. Black rarity: dark steel falchion, battle-worn blade, wrapped grip, minimal ornament.
2. Red rarity: same falchion upgraded with crimson fuller lines, hotter metal highlights, more aggressive guard.
3. Yellow rarity: same falchion upgraded into a gilded officer's blade, engraved spine, jewel-set hilt, premium finish.
4. White rarity: same falchion upgraded into a divine relic sword, ivory-gold hilt, radiant edge, holy glow.

Requirements:
- same falchion concept across all 4 icons
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
This item represents lingering curse damage, magical corrosion, and sustained harm. It should feel ritualistic, dangerous, and premium.

Rarity variants:
1. Black rarity: dark wood wand, dim toxic crystal tip, subtle corrosion, minimal ornament.
2. Red rarity: same wand upgraded with crimson veins, stronger poisonous glow, more ritual metal bands.
3. Yellow rarity: same wand upgraded into an ornate plague scepter, gilded handle, refined crystal, premium magical finish.
4. White rarity: same wand upgraded into a divine blight relic, ivory shaft, radiant venom core, bright holy-corrupted glow.

Requirements:
- same wand concept across all 4 icons
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

Core object: a mythic Killshot Prism.
This item represents fatal precision, amplified weak-point punishment, and devastating finisher damage. It should feel lethal, elite, and engineered for execution.

Rarity variants:
1. Black rarity: dark steel prism housing, compact faceted lens core, restrained detail, minimal lethal glow.
2. Red rarity: same prism upgraded with crimson fracture lines, hotter inner lens, sharper lethal silhouette, stronger kill-mark energy.
3. Yellow rarity: same prism upgraded into a gilded execution optic, ornate metal frame, premium crystal facets, masterwork precision finish.
4. White rarity: same prism upgraded into a celestial kill prism, ivory-metal housing, radiant execution crystal, holy finisher glow, highest prestige.

Requirements:
- same killshot prism concept across all 4 icons
- it must feel like precision finisher equipment, not a generic magical orb
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy or toy-like style
```

### AOE Speed

```text
Using the style rules above, generate 4 separate square item icons for AOE Speed.

Core object: a War Drum.
This item represents faster area attacks through battlefield rhythm and pressure. It should feel martial, loud, and commanding.

Rarity variants:
1. Black rarity: dark wood war drum, plain hide, iron studs, minimal ornament.
2. Red rarity: same drum upgraded with crimson straps, ember runes, stronger pulse glow.
3. Yellow rarity: same drum upgraded into a gilded battle drum, ornate frame, gold fittings, premium craftsmanship.
4. White rarity: same drum upgraded into a celestial war drum, ivory shell, radiant surface, divine energy pulse.

Requirements:
- same drum concept across all 4 icons
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
This item represents faster ricochets, agile rebound timing, and nimble execution. It should feel clever, fast, and premium.

Rarity variants:
1. Black rarity: dark steel bracer, restrained detail, compact silhouette, subtle rebound energy.
2. Red rarity: same bracer upgraded with crimson channels, hotter edge lines, quicker motion feel.
3. Yellow rarity: same bracer upgraded into a gilded trickster armguard, ornate plates, rich materials, elite finish.
4. White rarity: same bracer upgraded into a celestial rebound relic, ivory metal, radiant motion core, holy agility glow.

Requirements:
- same bracer concept across all 4 icons
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
This item represents rapid piercing volleys and relentless precision fire. It should feel engineered, lethal, and disciplined.

Rarity variants:
1. Black rarity: dark steel repeater crossbow, compact frame, restrained detail, practical silhouette.
2. Red rarity: same crossbow upgraded with crimson tension lines, hotter bolts, reinforced limbs, stronger firing energy.
3. Yellow rarity: same crossbow upgraded into a gilded marksman repeater, ornate mechanism, gold fittings, premium craftsmanship.
4. White rarity: same crossbow upgraded into a celestial piercer, ivory-metal frame, radiant string core, holy rapid-fire glow.

Requirements:
- same repeater crossbow concept across all 4 icons
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
This item represents faster ticking curses, sustained affliction, and accelerated decay. It should feel ritualistic, arcane, and dangerous.

Rarity variants:
1. Black rarity: dark metal censer, subtle toxic smoke, restrained ornament, practical chain details.
2. Red rarity: same censer upgraded with crimson vents, hotter curse glow, stronger internal burn.
3. Yellow rarity: same censer upgraded into a gilded plague censer, ornate filigree, rich materials, premium finish.
4. White rarity: same censer upgraded into a celestial affliction relic, ivory-gold body, radiant fumes, holy-corrupted glow.

Requirements:
- same censer concept across all 4 icons
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

Core object: a mythic Duelist Monocle.
This item represents timing, opportunistic strikes, and sharp-eyed critical openings. It should feel elegant, elite, and precision-focused.

Rarity variants:
1. Black rarity: dark steel monocle, compact lens frame, restrained detail, subtle targeting glint.
2. Red rarity: same monocle upgraded with crimson sight lines, hotter lens edge, sharper ornament, stronger strike-opportunity glow.
3. Yellow rarity: same monocle upgraded into a gilded duelist optic, ornate frame, premium lens glass, masterwork prestige.
4. White rarity: same monocle upgraded into a celestial deadeye monocle, ivory-metal frame, radiant lens, holy precision glow, highest prestige.

Requirements:
- same duelist monocle concept across all 4 icons
- it must feel like elite critical-opening equipment, not jewelry-first fashion
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy or toy-like style
```

### AOE Scale

```text
Using the style rules above, generate 4 separate square item icons for AOE Scale.

Core object: an Earthshaker Totem.
This item represents larger area attacks and heavier battlefield presence. It should feel primal, imposing, and mythic.

Rarity variants:
1. Black rarity: dark stone totem, simple runes, restrained detail, heavy silhouette.
2. Red rarity: same totem upgraded with crimson fissures, hotter cracks, stronger quake energy.
3. Yellow rarity: same totem upgraded into a gilded earth relic, ornate carvings, premium trim, rich materials.
4. White rarity: same totem upgraded into a celestial world-pillar, ivory stone, radiant rune core, holy seismic glow.

Requirements:
- same totem concept across all 4 icons
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
This item represents broader ricochets and larger rebound arcs. It should feel charged, elegant, and kinetic.

Rarity variants:
1. Black rarity: dark metal storm ring, restrained detail, subtle inner glow, compact silhouette.
2. Red rarity: same ring upgraded with crimson channels, hotter arc energy, sharper ornament.
3. Yellow rarity: same ring upgraded into a gilded tempest band, ornate trim, premium lightning core, richer materials.
4. White rarity: same ring upgraded into a celestial storm halo, ivory-metal body, radiant arc core, holy rebound glow.

Requirements:
- same ring concept across all 4 icons
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
This item represents larger piercing attacks and extended impalement reach. It should feel aristocratic, dangerous, and premium.

Rarity variants:
1. Black rarity: dark steel lance, restrained detail, practical shaft wrapping, minimal ornament.
2. Red rarity: same lance upgraded with crimson grooves, hotter point, stronger draconic inlays.
3. Yellow rarity: same lance upgraded into a gilded dragoon lance, ornate fittings, premium finish, richer materials.
4. White rarity: same lance upgraded into a celestial wyrm lance, ivory metal, radiant spearhead, holy piercing glow.

Requirements:
- same lance concept across all 4 icons
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
This item represents larger pools of lingering damage and stronger corruption spread. It should feel unstable, toxic, and premium.

Rarity variants:
1. Black rarity: dark glass venom orb, restrained corruption glow, minimal ornament.
2. Red rarity: same orb upgraded with crimson fractures, hotter poison core, stronger spread energy.
3. Yellow rarity: same orb upgraded into a gilded plague sphere, ornate casing, rich materials, masterwork finish.
4. White rarity: same orb upgraded into a celestial blight orb, ivory-metal frame, radiant toxin core, holy-corrupted glow.

Requirements:
- same orb concept across all 4 icons
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

Core object: a mythic Longshot Scope.
This item represents sightline extension, disciplined reach, and long-distance target control. It should feel elite, engineered, and purpose-built for distance.

Rarity variants:
1. Black rarity: dark steel longshot scope, practical housing, restrained detail, subtle range lens glow.
2. Red rarity: same scope upgraded with crimson emitter lines, hotter objective lens, reinforced fittings, stronger long-range energy.
3. Yellow rarity: same scope upgraded into a gilded marksman's scope, ornate precision housing, gold trim, premium lens glass, masterwork craftsmanship.
4. White rarity: same scope upgraded into a celestial horizon scope, ivory-metal body, radiant lens core, holy distance glow, highest prestige.

Requirements:
- same longshot scope concept across all 4 icons
- it must feel clearly longer-range than the accuracy item
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy or toy-like style
```

### Accuracy

```text
Using the style rules above, generate 4 separate square item icons for Accuracy.

Core object: a mythic Laser Sight.
This item represents precision, sight alignment, target acquisition, and reliable head-targeting. It should feel elite, engineered, sharp, and premium.

Rarity variants:
1. Black rarity: dark steel laser sight module, compact lens, restrained detail, practical housing, faint targeting glow.
2. Red rarity: same laser sight upgraded with crimson emitter lines, hotter lens core, sharper housing silhouette, stronger targeting energy.
3. Yellow rarity: same laser sight upgraded into a gilded marksman's sight, ornate precision housing, gold fittings, premium lens glass, masterwork craftsmanship.
4. White rarity: same laser sight upgraded into a celestial deadeye sight, ivory-metal housing, radiant targeting crystal, holy precision glow, highest prestige.

Requirements:
- same laser sight concept across all 4 icons
- it must feel like high-end accuracy equipment, not a generic flashlight or full rifle scope
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy or toy-like style
```

### Taunt

```text
Using the style rules above, generate 4 separate square item icons for Taunt.

Core object: a Red Matador Cape.
This item represents aggro control, bravado, and drawing enemy attention. It should feel theatrical, dangerous, and premium.

Rarity variants:
1. Black rarity: dark battle cape, restrained folds, minimal trim, subtle motion.
2. Red rarity: same cape upgraded with richer crimson fabric, hotter lining, stronger dramatic sweep.
3. Yellow rarity: same cape upgraded into a gilded matador mantle, ornate embroidery, luxury materials, premium prestige.
4. White rarity: same cape upgraded into a celestial provocateur cloak, ivory-red fabric, radiant trim, holy theatrical glow.

Requirements:
- same cape concept across all 4 icons
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

Core object: a Bulwark Plate.
This item represents raw mitigation, steadfast defense, and durable protection. It should feel heavy, disciplined, and elite.

Rarity variants:
1. Black rarity: dark steel chest plate, practical silhouette, restrained detail, solid mass.
2. Red rarity: same plate upgraded with crimson seams, hotter metal accents, reinforced armor lines.
3. Yellow rarity: same plate upgraded into a gilded bulwark cuirass, ornate trim, premium metalwork, rich materials.
4. White rarity: same plate upgraded into a celestial ward plate, ivory-gold armor, radiant defensive core, holy resilience glow.

Requirements:
- same plate concept across all 4 icons
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```

### Damage Reflection

```text
Using the style rules above, generate 4 separate square item icons for Damage Reflection.

Core object: a Mirror Shield.
This item represents reflected punishment and retaliatory defense. It should feel noble, dangerous, and premium.

Rarity variants:
1. Black rarity: dark steel mirror shield, restrained detail, polished reflective face, minimal trim.
2. Red rarity: same shield upgraded with crimson inlays, hotter edge light, stronger retaliatory energy.
3. Yellow rarity: same shield upgraded into a gilded reflective bulwark, ornate frame, rich materials, masterwork finish.
4. White rarity: same shield upgraded into a celestial mirror aegis, ivory-gold body, radiant mirror core, holy counter-glow.

Requirements:
- same mirror shield concept across all 4 icons
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
This item represents staggering impact, armor breaking, and brutal front-line pressure. It should feel oppressive, heavy, and dangerous.

Rarity variants:
1. Black rarity: dark steel tower shield, restrained spikes, practical silhouette, minimal ornament.
2. Red rarity: same shield upgraded with crimson spikes, hotter seams, stronger crushing menace.
3. Yellow rarity: same shield upgraded into a gilded war bastion, ornate faceplate, premium metalwork, rich materials.
4. White rarity: same shield upgraded into a celestial breaker shield, ivory-gold body, radiant spike core, holy impact glow.

Requirements:
- same spiked shield concept across all 4 icons
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
This item represents graceful dodging, elusive movement, and untouchable poise. It should feel light, elegant, and premium.

Rarity variants:
1. Black rarity: dark feathered mantle, restrained detail, subtle shimmer, graceful silhouette.
2. Red rarity: same mantle upgraded with crimson lining, hotter feather accents, stronger evasive energy.
3. Yellow rarity: same mantle upgraded into a gilded evasive cloak, ornate trim, premium fabric, rich materials.
4. White rarity: same mantle upgraded into a celestial wind mantle, ivory feathers, radiant drift glow, holy lightness.

Requirements:
- same mantle concept across all 4 icons
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
This item represents perfect timing, retaliation, and sharp defensive offense. It should feel elegant, lethal, and disciplined.

Rarity variants:
1. Black rarity: dark steel parrying dagger, restrained guard, minimal ornament, precise silhouette.
2. Red rarity: same dagger upgraded with crimson fuller lines, hotter edge, stronger riposte energy.
3. Yellow rarity: same dagger upgraded into a gilded duelist blade, ornate guard, rich materials, premium finish.
4. White rarity: same dagger upgraded into a celestial riposte relic, ivory-gold hilt, radiant edge, holy counter-glow.

Requirements:
- same parrying dagger concept across all 4 icons
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
This item represents concealment, quiet movement, and vanishing from sight. It should feel mysterious, elegant, and premium.

Rarity variants:
1. Black rarity: dark veil cloak, restrained folds, subtle shimmer, minimal trim.
2. Red rarity: same cloak upgraded with crimson inner light, stronger concealment glow, richer fabric.
3. Yellow rarity: same cloak upgraded into a gilded shadow mantle, ornate trim, premium weave, refined luxury.
4. White rarity: same cloak upgraded into a celestial phantom cloak, ivory fabric, radiant translucence, holy concealment glow.

Requirements:
- same cloak concept across all 4 icons
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
This item represents execution, stealth lethality, and elite kill pressure. It should feel sharp, sinister, and high-status.

Rarity variants:
1. Black rarity: dark steel assassin dagger, restrained detail, clean lethal silhouette, minimal ornament.
2. Red rarity: same dagger upgraded with crimson channels, hotter edge, stronger kill intent.
3. Yellow rarity: same dagger upgraded into a gilded assassin's fang, ornate fittings, premium metalwork, rich materials.
4. White rarity: same dagger upgraded into a celestial execution blade, ivory-gold hilt, radiant edge, holy lethal glow.

Requirements:
- same assassin dagger concept across all 4 icons
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
This item represents better treasure discovery and guided loot hunting. It should feel adventurous, valuable, and premium.

Rarity variants:
1. Black rarity: dark brass treasure compass, restrained detail, practical casing, subtle glint.
2. Red rarity: same compass upgraded with crimson inlays, hotter needle glow, richer metal finish.
3. Yellow rarity: same compass upgraded into a gilded treasure finder, ornate housing, premium glass, luxury craftsmanship.
4. White rarity: same compass upgraded into a celestial fortune compass, ivory-gold body, radiant needle core, holy treasure glow.

Requirements:
- same compass concept across all 4 icons
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
This item represents manipulation, bent odds, and illicit luck control. It should feel mischievous, premium, and dangerous.

Rarity variants:
1. Black rarity: dark loaded dice, restrained shine, minimal ornament, clear readable pips.
2. Red rarity: same dice upgraded with crimson edge glow, hotter trickster energy, richer finish.
3. Yellow rarity: same dice upgraded into gilded cheater's dice, ornate bevels, premium materials, luxury craft.
4. White rarity: same dice upgraded into celestial fate dice, ivory-gold body, radiant pips, holy chance glow.

Requirements:
- same dice concept across all 4 icons
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

Core object: a premium Wallet.
This item represents theft, lifted spoils, and sneaky gain. It should feel stylish, greedy, and premium rather than mundane.

Rarity variants:
1. Black rarity: dark leather wallet, restrained detail, practical silhouette, subtle coin glint.
2. Red rarity: same wallet upgraded with crimson lining, hotter metal accents, richer finish.
3. Yellow rarity: same wallet upgraded into a gilded pickpocket purse, ornate trim, luxury materials, premium craft.
4. White rarity: same wallet upgraded into a celestial thief's purse, ivory leather, radiant clasp, holy-greedy glow.

Requirements:
- same wallet concept across all 4 icons
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

Core object: a Loot Crate.
This item represents explosive reward delivery and extra item payoff. It should feel exciting, premium, and desirable.

Rarity variants:
1. Black rarity: dark metal loot crate, restrained detail, practical latch, minimal ornament.
2. Red rarity: same crate upgraded with crimson seams, hotter inner reward glow, stronger anticipation.
3. Yellow rarity: same crate upgraded into a gilded reward chest, ornate frame, rich materials, premium prestige.
4. White rarity: same crate upgraded into a celestial prize crate, ivory-gold body, radiant reward core, holy jackpot glow.

Requirements:
- same loot crate concept across all 4 icons
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
This item represents risky profit, gambling power, and sinister jackpot energy. It should feel premium, dangerous, and unmistakably special.

Rarity variants:
1. Black rarity: dark infernal casino token, restrained glow, engraved edges, subtle menace.
2. Red rarity: same token upgraded with crimson infernal light, hotter core, richer carvings, stronger risk aura.
3. Yellow rarity: same token upgraded into a gilded high-roller token, ornate edgework, premium materials, lavish prestige.
4. White rarity: same token upgraded into a celestial jackpot relic, ivory-gold body, radiant center, holy-luck glow.

Requirements:
- same infernal token concept across all 4 icons
- it must read as a premium special item, not a generic coin
- centered object
- clean rarity-colored background only
- no text
- no border
- no pixel-art look
- no goofy style
```
