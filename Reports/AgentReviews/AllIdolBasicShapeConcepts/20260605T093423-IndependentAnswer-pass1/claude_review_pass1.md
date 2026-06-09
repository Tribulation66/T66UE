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
