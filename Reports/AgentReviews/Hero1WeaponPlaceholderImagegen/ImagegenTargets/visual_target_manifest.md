# Hero 1 Weapon Placeholder Imagegen Targets

Date: 2026-06-05
Process: account-backed Codex imagegen wrapper, no API key
Scope: Visual target mockups only. These images do not implement Niagara assets, damage geometry, or impact-context rules.

## Files

- `hero1_weapon_black_single_crescent.png`
  - Role: Black rarity visual target.
  - Shape: One thick black annular crescent slash.
  - Gameplay intent: center primary point, body secondary.
- `hero1_weapon_red_three_crescents.png`
  - Role: Red rarity visual target.
  - Shape: Three connected smaller crescent lobes.
  - Gameplay intent: three primary impact points, connected body secondary.
- `hero1_weapon_yellow_five_crescents.png`
  - Role: Yellow rarity visual target.
  - Shape: Five connected smaller crescent lobes.
  - Gameplay intent: five primary impact points, connected body secondary.
- `hero1_weapon_white_large_crescent.png`
  - Role: White rarity visual target.
  - Shape: One large thick black crescent slash.
  - Gameplay intent: whole body primary contact.

## Prompt Summary

Common constraints:

- top-down orthographic VFX concept shape,
- solid black slash only,
- plain white background,
- no labels, UI, enemies, characters, particles, glow, texture, shadows, or material assumptions.

Per-rarity prompts:

- Black: single lunar crescent slash made from a thick annular arc, based on the user's first Paint sketch.
- Red: three connected crescent lobes, overall about 50% wider than black, based on the user's second Paint sketch.
- Yellow: five connected crescent lobes, overall about 150% of black, based on the user's third Paint sketch.
- White: one large crescent slash about twice black size, based on the user's fourth Paint sketch.

## Implementation Notes For Later

- The visual construction can be represented as annular arc geometry: an outer semicircle/arc minus an inner offset arc, with rounded or tapered endpoints.
- Red and yellow can be represented as repeated annular arc lobes connected into one footprint. Their primary impact points should be declared at lobe centers or apex/contact anchors during gameplay implementation.
- White needs a different impact-context policy from the other three: any body contact should publish a primary/full-damage trigger rather than only a center point.
- Current live `Content/Data/Weapons.csv` does not yet match the requested damage multipliers or impact-point counts. That is a later gameplay/data implementation step.
