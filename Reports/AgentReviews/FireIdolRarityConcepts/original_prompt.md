# Original User Request

The user approved/revised the central idol concepts and now wants Fire only, broken down by category and rarity.

Relevant current request:

> Fire red AOE, that's fine. DOT should be a burn thing: the VFX takes place on the enemy body, not on the floor; hit the enemy and the body catches fire. Pierce is fine. Bounce should be embers snapping and bouncing: it hits and flicks like four embers to four close enemies. Next, go one by one. First, Fire, all four rarities, how they change. Break it down by category and rarity: black, red, yellow, white.

## Task Contract

Working task: Planning-only Fire idol rarity breakdown.
Operator: Codex
Validator: Claude
Scope: Fire AOE, Fire DOT, Fire Pierce, Fire Bounce only. For each, propose Black/Red/Yellow/White rarity visual changes. No imagegen, no Unreal edits, no asset generation.
Stop condition: Return approval-ready rarity concepts for Fire idols.

## Live Roster Evidence

`Content/Data/Idols.csv` contains:

- `Idol_Fire_DOT`
- `Idol_Fire_AOE`
- `Idol_Fire_Pierce`
- `Idol_Fire_Bounce`

## Constraints

- Fire activation VFX should stay red/orange/fire-themed regardless of rarity label.
- Avoid a mechanical 1/3/5/one-big pattern. Rarity should change shape grammar, timing, density, and impact expression while preserving category identity.
- Keep the concepts feasible for simple placeholder shapes first: spheres, rings, capsules, cones, crescent/arc shapes, short line/ribbon trails, and billboard flame/ember sprites.
