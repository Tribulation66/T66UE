# Operator Draft: Fire Idol Rarity Concepts

Assumption: Black -> Red -> Yellow -> White escalates in intensity, with White the strongest. If the hierarchy changes later, the same concepts can be remapped. Fire stays red/orange across all rarities; rarity changes silhouette, timing, density, and impact expression, not element color.

## Fire AOE

Central identity: impact explosion.

- Black: compact circular burst. A low red fire dome expands from the impact point, with one simple ring fading out at the edge.
- Red: pressure-ring explosion. The center blooms, then a fast outward fire ring races across the radius, making the blast read wider and sharper.
- Yellow: staggered detonation. A central bloom hits first, then a second delayed fire wave erupts from the rim with a few upward flame tongues.
- White: inferno bloom. The impact flashes white-orange at the center, a clean circular flame wall rolls outward, and the center leaves a brief rising flame column.

## Fire DOT

Central identity: burn on the enemy body, not the floor.

- Black: smoldering body burn. Small torso-anchored flames flicker on the enemy with a few embers drifting upward.
- Red: climbing burn. Flames wrap more clearly around torso and head, with a steady body-locked flicker.
- Yellow: engulfed burn. Flame bands wrap the full enemy silhouette, pulsing on damage ticks with a stronger ember stream.
- White: white-hot body blaze. A tight white-orange core hugs the enemy body, with aggressive flicker and snap-off embers each tick.

## Fire Pierce

Central identity: flame lance.

- Black: simple fire spear. A narrow red bolt/capsule shoots forward with a short ember trail.
- Red: cone-tipped lance. The forward tip becomes sharper and brighter, with a longer trailing ribbon.
- Yellow: drilling lance. Two small flame ribbons flank the main spear, and each pierced enemy gets a brief ember streak.
- White: white-hot cutting lance. Thin, fast, and very bright, with crisp pass-through spark bursts instead of a fat trail.

## Fire Bounce

Central identity: snapping embers that flick from the first hit toward nearby enemies.

- Black: small ember flick. The hit spits four tiny red cinders in short arcing paths, with a soft pop on landing.
- Red: four clear snapping embers. The embers leave readable curved tails and land with small flame pops.
- Yellow: simultaneous ember scatter. The embers snap out almost at once, with sharper crescent trails and hotter landing flares.
- White: molten cinder volley. The embers launch with a white-orange flash, rebound in very fast arcs, and each landing creates a tiny secondary spark burst.

## Implementation Note

Fire Bounce assumes four snapping ember paths as the core read across rarities. If the implemented gameplay target count differs for a tier, we should decide before wiring whether to tune the gameplay count or make any extra ember paths cosmetic. The concept is not "more rarity means mechanically 1/3/5"; the tier controls speed, clarity, heat, and landing behavior.
