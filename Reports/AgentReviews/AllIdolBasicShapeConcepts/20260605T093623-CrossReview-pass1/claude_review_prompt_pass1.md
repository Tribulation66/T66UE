You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\AllIdolBasicShapeConcepts\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\AllIdolBasicShapeConcepts\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\AllIdolBasicShapeConcepts\20260605T093423-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
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

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The roster checks out: **20 idols** = 5 elements (Fire, Ice, Electricity, Nature, Wind) × 4 categories (DOT, AOE, Pierce, Bounce), all `Traveler` delivery, all carrying Black/Red/Yellow/White sprite columns. The basic-shape plan below is internally answerable — no user decision is required to produce it.

**Shape-language rules (apply globally):**
- **Primitive kit:** UV spheres, flattened discs (squashed cylinders), torus/rings, cylinders/capsules, cones, stretched cones (lances), thin arc/curve ribbons, line segments, billboard quads, root/branch cylinders.
- **Fixed hues:** Fire red/orange, Ice ice-blue, Electricity purple, Nature green, Wind grey. Rarity never changes hue.
- **Rarity scales four levers, not color:** shape **count**, **scale**, **timing/lifetime**, and **density/arrangement**. Black = sparsest/smallest/shortest; White = densest/largest/longest, with the cleanest arrangement.
- **Geometry anchors from CSV:** AOE uses `AoeRadius 300` with `AoeDelay 0.2` (telegraph window); DOT uses `DotTickInterval 0.5 / DotDuration 3` and is body-locked to the enemy; Pierce/Bounce are fast travelers (`~2200–2400` speed), Bounce has falloff so later hops should shrink.

### Fire
- **AOE — explosion:** central sphere flash + expanding flattened disc shockwave at the 300 radius. Black: 1 sphere + 1 disc. Red: + a few cone ember spikes. Yellow: ring of cones, taller sphere. White: dense cone fan + secondary disc ripple, longest fade.
- **DOT — body burn (body-locked):** small billboard flame quads pinned to enemy mesh. Black: 2–3 quads. Red: 4–6. Yellow: full-body quad band + faint sphere glow. White: dense flicker layer, longest lifetime across the 3s.
- **Pierce — flame lance:** single stretched cone along travel. Rarity lengthens/widens the cone, adds trailing ember billboards, then a thin sphere-bead trail at White.
- **Bounce — snapping embers:** small spheres at each hop. Rarity adds more spheres per hop + short arc ribbons between hops; later hops scale down per falloff.

### Ice
- **AOE — frost nova (no crystal spikes):** expanding flattened disc + low translucent dome (hemisphere). Rarity adds concentric disc rings and a brief sphere bloom; arrangement stays smooth, never spiky.
- **DOT — frozen body:** semi-transparent capsule/sphere shell encasing enemy. Rarity thickens shell, adds faint surface facet quads, longer hold.
- **Pierce — icicle spear:** single stretched cone (sharper/narrower than Fire). Rarity lengthens cone + adds thin parallel sliver cones flanking it.
- **Bounce — shard scatter:** small cones/tetra-like cones flung at each hop. Rarity increases shard count and scatter arc; later hops smaller.

### Electricity (purple)
- **AOE — strikes from above in radius:** vertical thin cylinders/line segments dropping inside the 300 radius, brief flat impact discs at base. Rarity adds more strike columns and tighter spacing during the 0.2 telegraph.
- **DOT — shocked body:** jittering line-segment arcs clinging to enemy + faint sphere glow. Rarity adds arc count and flicker rate.
- **Pierce — forward bolt:** single zig line-segment/thin stretched cone forward. Rarity adds branching forks + thin glow capsule core.
- **Bounce — chain spark:** line segment connecting hop points, sphere node at each hit. Rarity adds fork branches off the chain; later nodes smaller.

### Nature (green)
- **AOE — branches/roots poke up in radius:** root/branch cylinders angled upward at points inside the 300 radius, small disc dirt puffs at base. Rarity adds more roots, taller/thicker, ring arrangement.
- **DOT — spore poison:** small billboard spore quads + soft sphere cloud around enemy. Rarity adds spore density and a slow-drift second cloud layer.
- **Pierce — root forward on ground:** ground-hugging branch cylinder extending forward (segmented). Rarity lengthens, adds side-shoot cylinders.
- **Bounce — seeds:** small spheres (seeds) at each hop, tiny cone sprout on landing. Rarity adds seed count + sprout size; later hops smaller.

### Wind (grey)
- **AOE — tornado moving in a horizontal circle; damage radius smaller than movement circle:** stacked flattened discs / a tapered cone funnel orbiting the center; the funnel's travel circle is wider than its own damage footprint. Rarity adds disc stack height, debris billboard quads, faster orbit. **Keep movement-circle vs damage-radius distinction explicit in build notes.**
- **DOT — dust devil on enemy:** small body-locked tapered cone of stacked thin discs spinning at the enemy. Rarity adds disc count/height + debris quads.
- **Pierce — skinny tornado forward:** narrow stretched cone/thin disc-stack traveling forward. Rarity lengthens + adds swirl ribbon and debris.
- **Bounce — little tornadoes bouncing:** tiny cone funnels at each hop. Rarity adds count per hop + short ground-puff discs; later hops smaller.

**Rarity ladder template (reusable per idol):** Black = minimum viable silhouette (1 core shape). Red = core + 1 supporting shape type. Yellow = layered arrangement (rings/stacks, +glow). White = densest count, largest scale, longest lifetime, cleanest symmetric arrangement.

## Evidence Checked
- `Content/Data/Idols.csv` (read-only): confirmed exactly 20 idols, 5 elements × 4 categories, all `Traveler`, Black/Red/Yellow/White sprite columns present.
- CSV numeric fields cross-checked: AOE `AoeRadius 300 / AoeDelay 0.2`; DOT `DotTickInterval 0.5 / DotDuration 3`; Pierce/Bounce `ProjectileSpeed 2200–2400` with `FalloffPerHit` on Bounce — used to anchor scale/timing claims.
- Preserved concept directions from the prompt mapped 1:1 onto primitive shapes.

## Questions Or Blockers
None requiring the user. The prompt, roster, and preserved concept direction are sufficient to produce the approval-ready plan.

## Caveats
- Sprite icon columns (e.g. Fire shares `Idol_Lava`, `Idol_Steel`, `Idol_Curse`; Wind reuses `Idol_Storm` across all four) are 2D inventory icons and are **not** the in-world VFX — the shape plan correctly targets the gameplay VFX, not these sprites. Worth a one-line note to the user so the distinction is clear.
- Plan is intentionally primitive/temporary; it defines silhouette and rarity-scaling intent, not final material/Niagara work. No asset, imagegen, or Unreal edits performed — scope respected.
- The four rarity levers (count/scale/timing/density) are a consistent framework but the exact per-tier numbers (e.g. how many cones at White) are left as build-time tuning, not fixed here — flag if the user wants hard counts locked in.

</validator_independent_answer>

<codex_draft>
# Operator Draft: All Idol Basic Shape Plan

Claude / Codex process note: Claude is the read-only Validator. For this shape-only pass, Claude's independent review returned `Result: OK`.

Primitive kit: spheres, flattened discs, torus/ring shapes, capsules/cylinders, cones/stretched cones, line segments, simple curved arc ribbons, billboard quads, and root/branch cylinders. Element color applies to every cell: Fire red/orange, Ice ice-blue, Electricity purple, Nature green, Wind grey.

## Fire

| Idol | Black | Red | Yellow | White |
|---|---|---|---|---|
| AOE | Squashed sphere/dome at impact + one flat ring. | Larger dome + faster ring + a few tiny ember spheres. | Dome + two timed rings + short cone flame fan on rim. | Bright core sphere + thick rolling ring + center cone/cylinder flame column. |
| DOT | 2-3 body-locked flame billboard quads on torso. | 4-6 flame quads around torso/head + small ember spheres rising. | Full-body vertical bands of flame quads + faint glow capsule. | Dense flame quads hugging body + ember bead shower + bright inner capsule. |
| Pierce | Stretched cone/capsule flame spear + short sphere trail. | Sharper stretched cone + longer ribbon/arc trail. | Main spear + two flanking ribbon strips + ember beads on hit path. | Thin white-hot cone/capsule + crisp impact spheres at pass-through points. |
| Bounce | Four small ember spheres snap out on short arc ribbons. | Four larger ember spheres + clearer curved tails. | Four near-simultaneous arcs + hotter landing spheres. | Four fast molten spheres + secondary tiny spark spheres at each landing. |

## Ice

| Idol | Black | Red | Yellow | White |
|---|---|---|---|---|
| AOE | Thin flat ice-blue ring expands from impact, no spikes. | Wider ring + low transparent dome. | Two smooth concentric rings + soft mist billboard quads. | Broad layered rings + larger dome + dense low mist quads, still no spikes. |
| DOT | Transparent ice-blue capsule shell around enemy body. | Thicker body shell + small frost billboard flecks. | Shell plus flat facet panels on torso/limbs + drifting frost dots. | Full-body shell + brighter edge rings + vapor quads around body. |
| Pierce | Single stretched cone icicle with faint trail line. | Longer sharper cone + small trailing shard cones. | Main cone + two thin sliver cones flanking it. | Long bright cone + tip sphere flash + thin frost line behind it. |
| Bounce | A few small cone shards shoot outward. | More cone shards in a wider fan, each with short line trail. | Dense angular cone-shard scatter + small crack discs on contact. | Fast shard volley: bright cones + impact rings/discs at each hit. |

## Electricity, Purple

| Idol | Black | Red | Yellow | White |
|---|---|---|---|---|
| AOE | Three vertical line/cylinder bolts from above + small impact discs. | Same three bolts with base discs linked by thin ground lines. | More staggered bolt cylinders + brighter impact discs across radius. | Dense overhead bolt set + central flash sphere + flickering line web. |
| DOT | Short jitter line segments attached to enemy body. | More body arcs + faint purple glow capsule. | Branching line segments around torso/limbs + pulse spheres on ticks. | Dense full-body arc cage + bright jolt spheres at hands/head/torso. |
| Pierce | Thin forward line segment or very narrow capsule bolt. | Longer bolt + one or two branch line forks. | Thicker central capsule/line + several short fork lines. | Bright straight bolt core + persistent fork lines + tip flash sphere. |
| Bounce | One line arc between hit points + sphere node at contact. | Clearer arc link + brighter node spheres. | Fast chain line with small fork lines off each node. | Dense chain lattice: multiple line links + bright node spheres at contacts. |

## Nature, Green

| Idol | Black | Red | Yellow | White |
|---|---|---|---|---|
| AOE | Few angled branch/root cylinders poke up from ground discs. | More branch cylinders + small green spore sphere/quads at bases. | Ring of branch cylinders in sequence + poison haze billboards. | Dense branch/root cylinder eruption + larger spore cloud quads. |
| DOT | Small green spore billboard cluster on enemy body. | Thicker body-locked spore quads + soft cloud sphere. | Enemy wrapped in spore cloud sphere + falling mote spheres. | Dense poison cloud shell + pulsing green discs/quads around body. |
| Pierce | Segmented root cylinder crawling forward on ground. | Longer segmented root + dirt puff discs behind it. | Thicker root + side tendril cylinders branching off path. | Fast heavy root lane + many side tendrils + tip burst sphere. |
| Bounce | Small seed sphere hops to target, tiny landing puff. | Several seed spheres arc with leaf/fleck quads. | Seed scatter with bounce arcs + spore puff spheres on landing. | Dense fast seed spheres + green puff quads/discs at every contact. |

## Wind, Grey

| Idol | Black | Red | Yellow | White |
|---|---|---|---|---|
| AOE | Tapered stack of thin discs forms a small tornado orbiting a circle. | Taller disc stack + wider orbit path + dust quads. | Denser stacked discs + faint outer ring showing travel circle. | Heavy stacked-disc funnel + debris quads; orbit circle stays larger than damage radius. |
| DOT | Small body-locked dust devil: stacked discs around enemy torso. | Taller body swirl + a few debris billboard quads. | Tight funnel stack around enemy + visible mote spheres. | Dense sustained disc stack wrapping target + stronger debris quads. |
| Pierce | Skinny forward tornado: narrow cone/disc-stack projectile. | Longer narrow stack + ground dust line behind it. | Denser column + debris wake quads along travel path. | Fast needle funnel + long debris trail + tip gust ring. |
| Bounce | One or two tiny cone/disc-stack tornadoes hop between enemies. | Several small funnels rebound on short arcs. | Active mini-tornado cluster + landing puff discs. | Dense swarm of little funnels + puff rings at each ricochet. |

## Build Notes

- These are temporary silhouette carriers, not final Niagara/material polish.
- DOT effects are body-locked for Fire, Ice, Electricity, Nature, and Wind.
- For Wind AOE, every tier uses an orbiting tornado; the movement circle is larger than the damage radius.
- If real gameplay hit count differs from a visible count, decide before implementation whether extra paths are cosmetic or gameplay-backed.

</codex_draft>
