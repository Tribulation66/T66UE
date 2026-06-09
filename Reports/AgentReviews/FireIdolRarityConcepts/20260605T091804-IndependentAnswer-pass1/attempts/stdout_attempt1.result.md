Result: OK

## Independent Answer

All four Fire idols are confirmed in the roster (`Idol_Fire_DOT`, `Idol_Fire_AOE`, `Idol_Fire_Pierce`, `Idol_Fire_Bounce`), so this planning-only breakdown is well-grounded. Below is an approval-ready Fire rarity concept set. Treat rarity as an ascending progression Black → Red → Yellow → White, with each tier changing **shape grammar, timing, density, and impact expression** rather than just count. All tiers stay red/orange fire-themed.

### Fire AOE — red ground burst (user: "red AOE, that's fine")
- **Black** — single low red dome-sphere that expands once and fades; dull ember crackle, slow rise. Squat, heavy, "thump."
- **Red** — dome plus a fast outward fire-ring (expanding torus) that races to the edge; brighter core, quicker bloom. Adds a crescent scorch arc on the leading edge.
- **Yellow** — staggered double pulse: inner dome snaps, then a second ring overtakes it; a few upward flame billboards lick off the rim. Punchier timing, more vertical energy.
- **White** — sharp instant flash-core, ring snaps outward with a thin trailing ribbon, and a brief column of upward flame sprites at center. Fastest onset, hottest white-orange core fading to red edges.

### Fire DOT — burn ON the enemy body (user: VFX on enemy, body catches fire, not floor)
- **Black** — small flame billboard parented to the enemy torso; gentle flicker, sparse embers drifting up. Reads as "smoldering."
- **Red** — flames climb to cover torso + head region (capsule-fitted flame sprites); steadier, denser flicker with a faint heat-shimmer feel via opacity pulse.
- **Yellow** — full-body wrap of flame billboards with rhythmic flare-ups (timed intensity pulses) and a steady stream of rising embers. Clearly "engulfed."
- **White** — tight white-orange core flame hugging the body, faster aggressive flicker, embers snapping off in bursts on each damage tick. Most violent, hottest read while still body-locked.

### Fire Pierce — lance through enemies (user: "Pierce is fine")
- **Black** — short red line/ribbon trail behind the projectile; small spark on each pass-through.
- **Red** — longer, brighter ribbon with a thin leading flame cone; pass-through pops a small ember crescent.
- **Yellow** — twin-ribbon trail with a hotter cone tip; each pierce leaves a brief lingering ember streak at the contact point. Faster, sharper.
- **White** — concentrated white-hot lance with a crisp cone tip and a snapping spark burst at every enemy it passes through; trail is thinner but more intense (a "cutting beam" feel rather than a fat trail).

### Fire Bounce — snapping embers (user: hits and flicks ~4 embers to 4 close enemies)
- **Black** — impact flicks 2 slow embers to the 2 nearest enemies; soft snap, short ember arcs.
- **Red** — flicks ~4 embers in quick succession to nearby enemies, each a small bouncing spark with a short ribbon tail; crisper snap timing.
- **Yellow** — the 4 embers snap out near-simultaneously, each leaving a brief crescent arc and a small flame pop on landing. More energetic, tighter spacing.
- **White** — a fast radial snap of 4 hot embers that chain-flick with a sharp white-orange flash on each landing and a quick secondary spark; reads as electric-fast ember scatter while staying fire-colored.

Note the rarity escalation deliberately varies *expression* (dome→ring→pulse→flash-column for AOE; smolder→engulf for DOT; trail thickness/heat for Pierce; ember count→sync→snap-speed for Bounce) instead of a flat 1/3/5/one-big count ramp.

## Evidence Checked
- `Content/Data/Idols.csv` lines 2–5: all four Fire rows present; sprite refs map to Black/Red/Yellow/White columns, confirming the four-rarity structure.
- Original prompt directives (AOE red ok, DOT burn-on-body, Pierce ok, Bounce 4-ember flick) and constraints (stay fire-themed, avoid mechanical count pattern, placeholder-shape feasible).

## Questions Or Blockers
None requiring the user. One thing for Codex to confirm in its draft: the **rarity ordering direction** (I assumed Black=lowest → White=highest). The user only listed the label order "black, red, yellow, white" and didn't state which is rarest. If the intended hierarchy differs, the escalation should be re-mapped, but the per-tier concepts themselves still hold.

## Caveats
- Concepts are intentionally placeholder-shape friendly (spheres, rings, capsules, cones, crescents, ribbons, ember billboards) per the constraint; no asset/VFX implementation implied.
- DOT explicitly avoids any floor decal — all flame is body/capsule-parented as the user required.
- Bounce ember count is held at the user's "~4 to 4 close enemies"; I scaled lower tiers down to 2 to give rarity headroom — flag if the user wants 4 fixed across all tiers with only style changing.
