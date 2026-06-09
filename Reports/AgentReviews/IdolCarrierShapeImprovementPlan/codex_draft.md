Codex draft answer for cross-review:

The current four-slot profile map should be replaced by a per-idol carrier recipe layer.

Current gap:
- `T66OutgoingTravelerPoolSubsystem.cpp` maps 20 visual profile IDs to 4 mesh slots by category.
- The existing Niagara pool uploads one position, rotation, scale, color, and mesh index per logical traveler.
- This can only produce one primitive mesh per traveler, so it cannot faithfully represent 20 unique carrier silhouettes if the carriers need to look different by idol, not just color/category.

Recommended improvement:
1. Keep the outgoing traveler gameplay simulation logical and unchanged: one traveler, one target, one arrival callback, one damage/event path.
2. Add a visual-only carrier recipe layer keyed by `IdolID + Rarity`, with fallback to `IdolID`.
3. Each recipe is a compact list of primitive parts: mesh shape, local offset, local rotation, local scale, color multiplier, and optional orbit/spin hint.
4. During `UploadLiveState`, expand each logical traveler into N visual upload rows. The Niagara pool still renders primitive mesh indices, but it renders multiple parts per logical traveler. Collision and arrival stay bound to the parent traveler only.
5. Add rarity to the traveler fire params or fold it into the visual profile ID so Black/Red/Yellow/White can use different recipes later. Start with 20 base recipes and a rarity multiplier/count curve, then split to 80 explicit recipes when the shapes are approved.
6. Keep activation/impact VFX separate from carrier VFX. Carrier is the thing flying. Activation is the explosion/burn/freeze/shock/root/tornado effect at impact.

Suggested 20 basic carrier silhouettes:
- Fire AOE: compact fire bomb, center sphere plus upward cone crown and small ember satellites.
- Fire DOT: flame seed, vertical cone flame plus two small body-orbit sparks.
- Fire Pierce: fire lance, long cone/cylinder spear with ember nodes along the shaft.
- Fire Bounce: ember cluster, three small spheres/cubes staggered like snapping sparks.
- Ice AOE: snowflake puck, flat cylinder disc with cone spikes around the edge.
- Ice DOT: frost capsule, sphere plus cube crystal shell.
- Ice Pierce: icicle spear, long thin cylinder with cone point.
- Ice Bounce: shard fan, three mini cones in a V cluster.
- Electricity AOE: storm node, central sphere with three short vertical/diagonal bolt rods.
- Electricity DOT: shock cage, small core with crossing zig rods.
- Electricity Pierce: lightning bolt, segmented zig-zag cylinders/cubes.
- Electricity Bounce: chain spark, diamond cube with short rods at both ends.
- Nature AOE: seed pod burst, center sphere with radial branch/root rods.
- Nature DOT: spore pod, clustered small spheres around a core.
- Nature Pierce: ground root, low horizontal cylinder chain with bulb nodes.
- Nature Bounce: seed dart, oval seed body with small cone point.
- Wind AOE: orbit tornado seed, stacked cones/cylinders in a squat spiral.
- Wind DOT: dust devil seed, small vertical cone stack.
- Wind Pierce: skinny tornado spear, narrow stacked cone spiral stretched forward.
- Wind Bounce: mini whirlwind, tiny stacked cone spiral with side offset.

Rarity handling:
- Black: shortest/cleanest version of the silhouette.
- Red: add one secondary part group or stronger asymmetry.
- Yellow: add more shards/branches/sparks/stack layers.
- White: larger and cleaner silhouette, full recipe, not just more clutter.

Verification plan:
- Update the visual profile gate to assert 20 unique carrier recipe signatures, not just 20 profile IDs.
- Add an isolated carrier-shape capture that shows all 20 in-flight carriers in a contact sheet or short clip.
- Compile editor, refresh staged standalone, run the visual profile gate from staged exe.

Conclusion:
This gives each idol its own basic-shape projectile now, and the exact same `IdolID + Rarity` profile keys can later resolve to 20 or 80 real Niagara systems.
