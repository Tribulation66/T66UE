# Original User Request

The user wants planning-only suggestions for central visual concepts for each current idol before any rarity breakdown or implementation.

Relevant user request:

> Okay, cool. So now, let's go on to making the idols. I actually want to hear your suggestions. So, for each idol, describe the basic shape of it, and from the first weapon, you kind of get a concept of the idols as well, because the idea is, for example, we have the rarities for the idols, right? So, for the fire AOE, it should be an explosion, right? And let's say it's kind of like a similar pattern that, you know, it's the black one at the point of impact, it creates a circular explosion over a certain area. For the red one, it can be like a dual explosion. It should all be an explosion. I don't just want to, you know, don't just do the gimmick of it's 1, 3, 5, and then one big one. It should be more creative than that, but it should have the same nature, right? So, for this one, for your answer now, don't tell me the rarities yet. Just for each idol, what is the central idea of what it looks like when it activates? And then once I approve that, you know, what should it look like? For example, for the wind one, we should have a tornado sort of look. You know, don't be limited by the super simplified shapes now. Do keep them in mind on how you would be able to achieve it with the simplistic shapes we have. And then once I green light that, we'll move on to the rarities per idol. Okay, we'll do that first.

## Task Contract

Working task: Propose central, non-rarity visual ideas for all current idols.
Operator: Codex
Validator: Claude
Scope: Read-only planning answer only. Current roster is Fire/Ice/Electricity/Nature/Wind x DOT/AOE/Pierce/Bounce. No imagegen, no Unreal edits, no asset generation, no rarity variants yet.
Stop condition: Return concise, approval-ready concepts that respect user color/nature direction and are feasible with simple placeholder shapes later.

## Current Live Roster Evidence

`Content/Data/Idols.csv` contains:

- Fire: DOT, AOE, Pierce, Bounce
- Ice: DOT, AOE, Pierce, Bounce
- Electricity: DOT, AOE, Pierce, Bounce
- Nature: DOT, AOE, Pierce, Bounce
- Wind: DOT, AOE, Pierce, Bounce

User color direction from prior scope:

- Fire: red
- Ice: very light blue / ice blue
- Electricity: purple
- Nature: green
- Wind: grey

## Requested Review

Give an independent read-only answer or critique for the central idol visual concepts. Do not propose rarity variants yet. Focus on whether the concepts should preserve each idol's element and delivery category while remaining feasible for a simple placeholder-shape phase.
