# Original User Request

The user accepted the all-idol rarity concepts and now wants the same full format, but focused on how each idol/raring VFX would be built with the temporary basic-shape approach.

Relevant current request:

> Okay, all that's fine, but, okay, so as long as you're getting the okay from Claude, that's fine. Now what I want is in the same format, but what basic shapes, how you're gonna build out all of these with the basic shape approach. That's my question.

## Task Contract

Working task: Planning-only basic-shape construction plan for every current idol.
Operator: Codex
Validator: Claude
Scope: All 20 current idols, each with Black/Red/Yellow/White primitive-shape construction notes. No imagegen, no Unreal edits, no asset generation.
Stop condition: Return approval-ready shape plan and report Claude validation status.

## Live Roster Evidence

`Content/Data/Idols.csv` contains Fire, Ice, Electricity, Nature, Wind, each with DOT, AOE, Pierce, Bounce.

Confirmed rarity order from user: Black -> Red -> Yellow -> White increases in rarity.

## Preserved Concept Direction

- Fire AOE: explosion.
- Fire DOT: enemy body burn, body-locked.
- Fire Pierce: flame lance.
- Fire Bounce: snapping embers.
- Ice AOE: frost nova, no crystal spikes.
- Ice DOT: frozen body.
- Ice Pierce: icicle spear.
- Ice Bounce: shard scatter.
- Electricity AOE: lightning strikes from above in radius.
- Electricity DOT: shocked body.
- Electricity Pierce: forward lightning bolt.
- Electricity Bounce: chain spark.
- Nature AOE: tree branches/roots poke in radius.
- Nature DOT: spore poison.
- Nature Pierce: root forward on ground.
- Nature Bounce: seeds.
- Wind AOE: tornado moving in horizontal circle; damage radius smaller than movement circle.
- Wind DOT: dust devil on enemy.
- Wind Pierce: skinny tornado forward.
- Wind Bounce: little tornadoes bouncing.

## Constraints

- Focus on temporary primitive/basic shapes: spheres, flattened discs, torus/rings, cylinders/capsules, cones, stretched cones, simple arcs/curves, line segments, billboard quads, and simple root/branch cylinders.
- Element hue stays fixed: Fire red/orange, Ice ice-blue, Electricity purple, Nature green, Wind grey.
- Rarity changes shape count, scale, timing, density, and arrangement, not just color.
