# Fall Guys Map Analysis — what makes it read, and what T66 is missing

**Date:** 2026-06-11. **Scope:** deep analysis of Fall Guys map/level design
principles (externally sourced + reverse-engineered) mapped against the T66 tower
state after Phase 2 passes 1-4 (lifts, walk-under decks, shape kit + rhythm, gate
clearance + capsule launch). Output: a prioritized improvement program that the
section 1.7 room-template proposal plugs into.

**Companion docs:** `T66_MAP_DESIGN_REFERENCE.md` sections 1.2-1.7 (design
authority), `ART_DIRECTION.md` (look), `Saved/Reference/MegabonkTerrainGenerator/`
(constructive connectivity reference).

---

## 1. The principles (distilled from sources + frame analysis)

**P1 — One gimmick per space.** Every Fall Guys round is ONE star mechanic
explored thoroughly (Jump Club = a spinning rope; Big Fans = fans), not many
mechanics mixed. Mediatonic explicitly reuses mechanics across levels rather than
inventing per level. A space that tries to say three things says nothing.

**P2 — Constant flow + directional read.** You always know where "forward" is;
race rounds maintain motion with no waiting. Obstacle timing is tuned to player
arrival so you commit at speed rather than stand and wait.

**P3 — Chokepoints and width rhythm.** Narrow → wide → narrow. Funnels create
chaos and tension; open plazas give relief. The alternation IS the pacing tool.

**P4 — Triangular risk/reward routes.** Fast-risky center lane vs slow-safe side
lanes at nearly every decision point ("high risk high reward short path, low risk
low reward long path"). The choice is visible at a glance before committing.

**P5 — Skill gates + difficulty layering.** Each section is a repeatable
challenge gate; sections escalate through the level (Ski Fall: empty slope →
single obstacles → flippers+fans+blockers; rotating cylinders get faster and
smaller deeper in).

**P6 — Exposure / foreshadowing.** The objective is visible early and often (Big
Fans keeps the finish line on screen the whole round). You see the hazard before
it can hit you; you see the prize before you choose a route.

**P7 — Motion is the hazard.** The danger is overwhelmingly MOVING geometry with
readable cycles — rotators, pendulums, pistons, conveyors, flippers. Static
hazards are rare. Failure is displacement (launch/fall), recovery is fast, and
the result is slapstick, not punishment.

**P8 — Rounded, chunky, oversized geometry.** Soft rounded edges on practically
everything; fat platforms; obstacles LARGE relative to the bean. Sharp corners
basically do not exist in the Blunderdome. This single trait carries a huge share
of the "Fall Guys look".

**P9 — Color is information.** Candy neon palette, but disciplined: distinct hue
per function/layer (progress layers in Slime Climb are color-coded), bold flat
colors, minimal gradients, hazards visually loud. Clarity survives 60-player
chaos because shape+color+scale never lie.

**P10 — Template-driven authoring with seeded variety.** Levels follow a
replicable design macro (one-page pitch → blockout) and ship VARIANTS of the same
layout to prevent rote memorization. Authored composition, procedural variation —
exactly the balance the user asked for.

---

## 2. T66 gap audit (post passes 1-4)

| Principle | Current state | Verdict |
|---|---|---|
| P1 one gimmick | Rooms get random scatter + random trap pool draw | **Biggest gap.** No room says one thing. Section 1.7 templates are the fix |
| P2 flow | Chain gives corridors direction; rooms are direction-less ponds | Gap in rooms; arrival→reward axis missing |
| P3 width rhythm | Doorways are natural chokepoints; room interiors uniformly open | Partial; nothing intentionally funnels |
| P4 risk/reward routes | Single chain path; scatter is optional but rewardless | **Gap.** No visible fast-risky vs slow-safe choice anywhere |
| P5 skill gates / layering | Floor trap pools exist (floors 2-3 same pool, 4 empty); no escalation curve | Gap. Floor 3 should be meaner than floor 2 by design, not by chance |
| P6 exposure | Rewards are sparse and invisible (7 chests / 2 floors); no beacons | **Gap** — the user's "what are the objectives?" complaint verbatim |
| P7 motion | Lifts + 4 obstacle traps (random placement); launch-only reactions ✓ | Foundation proven (moving collision + capsule launch), but motion density far below the read; movers aren't composed |
| P8 rounded chunky | Sharp-edged slabs/prisms everywhere; pillars are the only round mass | **Cheapest big win.** Bevels/edge rounding would transform the read |
| P9 color discipline | Candy palette ✓, per-element hues ✓ (floor blue, platform yellow, mesa violet, ramp magenta, lift mint) | Mostly there; missing: hazard signature color + tier color-coding for height-as-progress |
| P10 templates + variants | Fully procedural; shape themes (pass 3) are a first step | Section 1.7 IS this principle |

**What we already match:** candy flat-color palette, bouncy surfaces, displacement
over damage (capsule launch, lava as DOT), playable under-deck space, readable
primitive shapes, seeded variety machinery, no-softlock math.

---

## 3. The improvement program (prioritized)

### Tier A — Visual language (cheap, immediate, no design risk)

- **A1 Beveled mesh kit.** Replace sharp cubes/prisms for COURSE elements (decks,
  stones, lift slabs, ramps) with chamfered/rounded-edge versions
  (FallGuysShapeKit02: rounded-edge slab, rounded puck, beveled hex/tri; Blender,
  exact 1-hull convex collision, same 100^3 AABB contract so zero math changes).
  This is P8 — the single highest look-per-effort item on the list.
- **A2 Deck edge trims.** Thin contrast-color strip slabs along deck/mesa edges
  (white/pink). Fall Guys platforms almost always carry an edge band; also doubles
  as a depth cue for jumps (visual only, no collision).
- **A3 Hazard signature.** Everything that launches the hero (all obstacle traps)
  wears ONE signature look — the striped inflatable pattern MIs we already have —
  so "stripes = will punt you" becomes a learned rule. (P9)
- **A4 Tier color-coding.** Height as progress: ground stones one hue, Tier 2
  decks a second, mesa decks a third (already true: yellow/yellow/violet — split
  Tier 1 vs Tier 2 hues), lava-safe chain decks keep a distinct accent. (P9)

### Tier B — Composition (the section 1.7 templates, upgraded by this analysis)

- **B1 One-gimmick rule baked into templates.** Each room template = exactly one
  star mechanic (a mover, a trap, a layout trick) + supporting traversal. (P1)
- **B2 Entry→reward axis.** Templates orient off the room's main door(s): reward
  visible from the threshold (sight-line check at generation: no wall/mesa blocks
  the line from door to reward slot). (P2, P6)
- **B3 Reward beacons.** Reward slots get a pedestal + light column / floating
  marker so objectives read across the room. (P6)
- **B4 Width rhythm inside big rooms.** Templates may place funnel walls/fences
  (low rails) to create one narrow commitment moment per room. (P3)
- **B5 Floor difficulty curve.** Floor 2 templates draw from the gentle pool +
  slower mover speeds; floor 3 from the dense pool + faster cycles (tuning-table
  driven, like trap pools today). (P5)

### Tier C — Motion library (what actually makes it FEEL like Fall Guys)

Foundation exists: the lift proved riding moving collision; capsule launch proved
displacement reactions. Each mover is one actor class on that pattern, placed by
the room composer through structure/hazard anchors, not random scatter (P1):

- **C1 Rotating bar (Jump Club).** A slow horizontal bar sweeping a deck or
  ground ring; jump it or get punted. Cheapest mover (yaw rotation + the sweeper
  hazard's launch math). Mesa-top variant is a central-mesa structure hazard.
- **C2 Pendulum hammer over bridges.** The ceiling hammer already exists —
  place it from composer anchors on bridge-deck structures. Mostly placement
  work, not new code.
- **C3 Bounce pads (flippers).** A pad that launches the hero UP onto a Tier 2
  deck / mesa — deliberate trampoline as an access route. We already have
  surface-bounce physics; a pad is a tagged disc with a fixed launch. Doubles as
  a lift alternative (access variety) and pure Fall Guys slapstick.
- **C4 Conveyor strips.** Ground belts that push (base velocity add). Medium
  cost; strong gimmick for one template; defer behind C1-C3.
- **C5 Crumble tiles (Hex-A-Gone).** Stepped-on deck tiles that vanish and
  respawn. Highest cost (per-tile state), iconic payoff; candidate for a later
  dedicated room type or a boss-floor mechanic. Defer.

### Tier D — Route choice (P4)

- **D1 Corridor forks.** Where the dry chain crosses wide spaces, generate a
  parallel FAST lane (small round stones, tighter jumps, shorter) beside the SAFE
  lane (big decks, longer) that re-merge — the triangular choice, visible at a
  glance. Validation: both lanes BFS-verified, only the safe lane must satisfy
  the lava-chain guarantee.
- **D2 Trap-guarded shortcuts.** Phase 2's "trap-guarded ramp tops" item lands
  here: the short route past a mesa carries the mover; the long ring walk is
  free. (P4 + P7 together)

### Build order recommendation

1. **Tier A** (one visual pass, ~a day): A1-A4 transform the read with zero
   gameplay risk while design decisions settle.
2. **Tier B + C1-C3** (the room-course pass, after the user's section 1.7
   decisions): templates with one-gimmick rooms, axis+beacon rewards, the first
   three movers as template signatures.
3. **Tier D + C4** (the depth pass): corridor forks, conveyor template,
   floor curve tuning from playtests.
4. **C5 crumble tiles** when a dedicated room/boss mechanic is wanted.

---

## 4. Sources

- Lawrence Thorp — Fall Guys Level Design Study (macro/template method, dual-lane
  risk/reward): https://lawrence.games/fall-guys-level-design-study
- Josh Hardy — Fall Guys Level Design Techniques (chokepoints, skill gates,
  triangular paths, color-as-layers, exposure/foreshadowing):
  https://www.joshhardy.co.uk/post/fall-guys-level-design-techniques
- Nathan Tubb — The Level Design Behind Fall Guys (difficulty layering, hazard
  telegraphing, escalating rotators):
  https://www.nathantubb.co.uk/post/the-level-design-behind-fall-guys-a-junior-s-analysis
- WayTooManyGames — Anthony Pepper (Mediatonic senior designer) interview (design
  pillars, one-gimmick depth, mechanic reuse, slapstick physicality):
  https://waytoomany.games/2020/08/18/interview-with-anthony-pepper-senior-designer-behind-fall-guys/
- ArtStation Magazine — Mediatonic Fall Guys Art Blast (rounded geometry, candy
  palette, clarity discipline):
  https://magazine.artstation.com/2020/10/mediatonic-games-fall-guys-art-blast/
- SCMP — Talking level design with the creators of Fall Guys (design pillars,
  single-page pitches): https://www.scmp.com/yp/discover/entertainment/tech-gaming/article/3104705/talking-level-design-creators-fall-guys
