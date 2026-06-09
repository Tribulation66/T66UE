# Original User Request

The user clarified that the previous Fire-only answer was acceptable but they wanted the full every-idol rarity breakdown.

Relevant current request:

> This is fine, but I wanted all of the elements. In that assumption is correct, black, red, yellow, white, that it increases in rarity. But I have a question about what Claude is saying about this. I see that some tokens were spent by Claude, but I don't really see if it gave the OK or what the deal is on how you two are working together. But for the next answer, give me the full every idol.

Prior user concept corrections to preserve:

- Fire AOE: explosion.
- Fire DOT: enemy body catches fire, body-locked burn, not floor.
- Fire Pierce: flame lance is fine.
- Fire Bounce: embers snapping/bouncing to nearby enemies.
- Ice AOE: frost nova, but no crystal spikes.
- Ice DOT: enemy gets frozen / body ice blue.
- Ice Pierce: icicle spear.
- Ice Bounce: ice shards shoot in different directions.
- Electricity AOE: lightning strikes from above hitting enemies within radius.
- Electricity DOT: enemy is shocked.
- Electricity Pierce: forward lightning bolt.
- Electricity Bounce: chain spark.
- Nature AOE: tree branch/root/branches poke in radius.
- Nature DOT: spore poison thing.
- Nature Pierce: root travels forward along ground.
- Nature Bounce: seeds.
- Wind AOE: tornado moving in a circular radius, not vertically; damage radius smaller than movement circle.
- Wind DOT: dust devil on enemy.
- Wind Pierce: skinny tornado moving forward.
- Wind Bounce: several little tornadoes bouncing around.

## Task Contract

Working task: Planning-only full rarity concept grid for all current idols, plus clarify Claude review status/process.
Operator: Codex
Validator: Claude
Scope: All 20 current idols, each with Black/Red/Yellow/White visual progression. No imagegen, no Unreal edits, no asset generation.
Stop condition: Return approval-ready concepts for every idol and mention Claude validation result.

## Live Roster Evidence

`Content/Data/Idols.csv` contains Fire, Ice, Electricity, Nature, Wind, each with DOT, AOE, Pierce, Bounce.

Confirmed rarity order from user: Black -> Red -> Yellow -> White increases in rarity.

## Constraints

- Keep element colors: Fire red/orange, Ice ice blue, Electricity purple, Nature green, Wind grey.
- Rarity progression should not be only 1/3/5/one-big. Use timing, shape grammar, density, motion, and impact expression.
- Keep concepts feasible for simple placeholder shapes first.
