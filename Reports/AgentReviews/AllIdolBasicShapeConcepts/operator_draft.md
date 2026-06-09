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
- The CSV sprite/icon columns are inventory UI icons; this plan targets in-world placeholder VFX shapes.
- DOT effects are body-locked for Fire, Ice, Electricity, Nature, and Wind.
- For Wind AOE, every tier uses an orbiting tornado; the movement circle is larger than the damage radius.
- Bounce visuals should shrink later hops to match per-hit falloff readability.
- If real gameplay hit count differs from a visible count, especially Fire's approved four-ember read, decide before implementation whether extra paths are cosmetic or gameplay-backed.
