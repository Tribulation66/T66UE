# T66 Master Map Design

**Last updated:** 2026-06-10
**Scope:** Single-source handoff for the live tower-only Tribulation map runtime, stage-space flow, traversal structure, minimap behavior, miasma/blood pressure design, and the historical implementation plan that led to the tower layout.  
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Gameplay/World/T66_LIGHTING_REFERENCE.md`  
**Maintenance rule:** Update this file after every material map-layout, terrain-generation, stage-space, minimap, traversal-gate, floor-transition, miasma/blood-pressure, or preset-selection change.

## 1. Executive Summary

- The live Tribulation runtime is now `Tower`-only.
- Runtime map-layout selection has been removed, including the old non-tower layout variants and config/console overrides.
- New runs and save-load restore now coerce the main gameplay map to the tower layout.
- The active gameplay map family is the tower generator and floor-based traversal path, not the old one-board terrain preset selection flow.
- Target normal tower pacing uses five floors, but the current parity tuning default in `Config/DefaultT66TowerTuning.ini` preserves the live 4-floor runtime until a tuning pass explicitly changes it:
  - `Floor 1 - Start`: weapon altar only; no enemies and no normal interactable scatter
  - `Floor 2` through `Floor 4`: gameplay floors with enemies and normal floor interactables
  - `Floor 5 - Boss`: boss flow only
- Boss-rush finale is a separate two-floor special case; it skips normal tower enemy and interactable population and must not be treated as the normal map-design floor-count contract.

### 1.1 Current tower implementation status

- The runtime now hard-locks Tribulation gameplay to the tower layout.
- Save/load compatibility still persists a layout field, but load paths sanitize back to `Tower`.
- The first playable tower pass currently does all of the following:
  - now uses a `400m x 400m` tower footprint so each gameplay floor has enough room for a real maze layout without the oversized first expansion pass
  - now uses a square tower shell with square floor footprints so the walls and floors meet directly without polygon-gap cleanup work
  - now clamps tower floor tiles all the way to the shell bounds so gameplay floors no longer leave a visible perimeter gap away from the surrounding wall
  - builds tower floors from tiled floor blocks instead of stretched slab bands, so the tower now reuses the regular flat-ground material look instead of the striped placeholder look
  - uses a lower tower floor spacing and smaller square descent holes so players cannot trivially double-jump back up through the previous floor opening
  - now snaps tower drop holes to the floor tile grid and removes exactly one square floor tile, so the descent opening reads as a true `1x1` square instead of an offset rectangle
  - randomizes each floor's square descent-hole position from the run seed instead of keeping the holes on a fixed shared pattern
  - adds full-height internal maze walls on gameplay floors and teaches tower surface placement to treat those wall volumes as blocked space
  - pushes maze-wall spans all the way into the outer shell so gameplay floors no longer keep an open perimeter ring around the tower walls
  - keeps `Floor 1 - Start` on the generic dungeon-tower scaffold again instead of the later enclosed-room pass, while still keeping tree props out of Stage 1
  - restores the original taller Stage 1 tower read by using the roof cap only at the top of the tower instead of adding the later low ceiling caps between floors
  - keeps `Floor 1 - Start` as a clean start floor with the weapon altar and no guaranteed start-area interactable scatter
  - keeps the stage-entry idol altar out of the current tower bootstrap path so the start floor remains weapon-altar-only
  - no longer spawns the old idol-area dummy enemy test targets
  - starts real stage combat/timer pressure once the player reaches the gameplay floors
  - builds blood-style miasma coverage across gameplay floors only
  - keeps tower-only combat rules isolated from the classic main-map rules:
    - no lava patch stage effects on tower floors
    - some enemies are pre-populated onto gameplay floors before the player reaches them
    - reinforcement enemies now emerge from tower wall surfaces instead of appearing from thin air
    - reinforcement enemies are now restricted to the outer tower shell walls, not interior maze walls
  - scopes minimap/full-map display to the current floor with floor-local reveal memory and polygon floor bounds
  - removes the old non-tower visual-navigation pillar from tower runs
  - removes obsolete traversal-pad interactables from stage population
  - allows compact casino/vendor utility opportunities to roll on gameplay floors only
  - can spawn a rare Backrooms entrance door on a tower wall when the run does not already own the Backrooms Quick Revive item; the Backrooms maze is spawned with the stage as a hidden pocket space, not loaded later
  - spawns exactly one saint on one gameplay floor per tower stage
  - spawns exactly one difficulty totem on every gameplay floor using floor-specific `T66_Tower_DifficultyTotem_##` tags, independent of the randomized saint floor selection
  - guarantees tower chest/crate rules by floor instead of using the old global stage scatter for those two interactables:
    - `Floor 1 - Start`: no chest, no crate
    - current parity config floors `2-3`: `1-3` chests and `1-3` crates per floor
    - target five-floor tuning floors `2-4`: `1-3` chests and `1-3` crates per floor
    - boss floor: no chest, no crate
  - now rejects cross-floor tower placement traces and retries tower floor placement more aggressively, so gameplay-floor chest/crate/casino/utility placement stays on the intended floor instead of bleeding through floor gaps to another floor
  - snaps tower NPC/interactable spawns to an explicit requested floor and tags them with explicit tower-floor identity so floor-local placement and safety logic do not rely only on raw actor Z
  - re-snaps tower chest/crate/totem/wheel placements after rarity/configuration work so floor-local mesh swaps cannot pull those actors back onto the wrong floor
  - tower boss clears now spawn the next-stage gate at the dedicated tower boss exit location instead of trusting the exact boss death point
  - tower visuals are now stage-aware:
    - `Stage 1`: dungeon floor/wall/roof materials and no runtime rock/tree scatter props
    - `Stage 2`: forest ground/roof materials plus theme wall replacements for both inner maze walls and the outer perimeter shell
  - old tower-only collidable rock/deco scatter on gameplay floors has been disabled; keep floor dressing out of the map/runtime terrain path unless a new generated-prop pass is explicitly approved
  - uses tower-only dungeon material instances for floor, wall, and roof surfaces instead of the inherited green/brown legacy terrain look
  - uses dedicated tower descent-hole trigger actors for floor-to-floor progression while keeping the existing boss-kill `StageGate` portal for actual stage-to-stage travel
  - gates the `Floor 2`, `Floor 3`, and `Floor 4` descent holes behind fixed placed miniboss encounters: a scaled rich `Slime` spawns when the player enters each floor, and the descent hole cannot be opened until that miniboss is defeated
  - replaces the legacy boss-threshold gate on tower with final-hole boss entry, so dropping from `Floor 4` into `Floor 5 - Boss` pauses normal wave spawning and starts boss flow
  - keeps `Floor 5 - Boss` as the terminal floor with no further descent hole
  - uses tower-specific fall-rescue logic so normal hole descent does not snap players back onto upper floors
  - uses tighter floor-classification tolerance so falling between floors is not misread as standing on a gameplay floor
  - snaps tower bosses after boss initialization so tall boss meshes do not sink into the floor
  - resets run-state and damage-log state on pause-menu restart so tower restarts follow the same reset path as the run-summary restart
  - no longer relies on stale static raw mesh pointers for tower prop decoration, fixing the packaged tower reload/restart crash that could happen while rebuilding the tower after a gameplay map restart
- The tower preset is still not complete:
  - broader tower-specific casino/vendor/NPC pacing beyond saint is still open

### 1.2 Bouncy obstacle-course layer and rising-lava hazard (2026-06-10)

Gameplay floors now generate a Fall Guys-style obstacle-course layer on top of the
dungeon-room maze, plus a rising-lava hazard mode that replaces the spreading blood
coverage when the course validates. All three systems are config-driven through
`Config/DefaultT66TowerTuning.ini` (`BounceCoursePlatforms`, `TowerLavaRise`,
`DoorwayArches` master switches).

- **Bounce platforms** (`T66TowerMapTerrain::FBouncePlatform`, built in
  `T66BuildFloorBouncePlatforms`):
  - Two tiers: Tier 1 tops at `+200` (one jump from the base floor), Tier 2 tops at
    `+400` (one jump from Tier 1). Tier steps and chain gaps are clamped in
    `UT66TowerTuningConfig::Sanitize()` against hero jump reach (JumpZ `1600`,
    gravity `4.5` -> `~290uu` max step, `~420uu` safe flat gap).
  - **Safe chain guarantee:** a BFS path from the floor arrival cell to the descent
    hole gets a Tier 2 platform in every path cell (`700uu` footprints in `1000uu`
    cells -> `~300uu` gaps). Tier 1 hop-on stones beside every third chain platform
    keep the chain mountable mid-route. Each floor logs
    `[T66Proof][BounceCourseSummary] Floor=N Result=PASS/FAIL ... MaxChainGap= HoleReach= LavaClearance=`;
    a FAIL clears the floor's chain markers, which disables lava-rise for the stage.
  - Room interiors get scattered Tier 1/2 platforms (about one per 8 room tiles, up
    to 4 per room); isolated Tier 2 platforms always receive an adjacent Tier 1
    buddy stone or are demoted. Up to two walkable ramps per floor connect the base
    floor onto scattered Tier 1 platforms (~21 degree slope).
  - Collision is hidden `UBoxComponent` prisms (floor surface to platform top), per
    the world collision contract. Visuals are baffle tubes: a horizontal tube deck
    at the top plus inflated side skirts that bulge slightly past the collision
    footprint; themed cube prisms are the no-baffle fallback. Platform surfaces
    carry no `T66_NoSurfaceBounce` tag, so hero surface-bounce impulses apply.
  - Content placement (`T66IsWalkableTowerLocation`) now rejects platform/ramp
    footprints, so chests, crates, NPCs, minibosses, and obstacle traps keep
    spawning on the base floor only.
- **Rising lava** (`AT66MiasmaManager` lava-rise mode, `IsLavaRiseModeActive()`):
  - Active only when the tower layout is live, `TowerLavaRise=1`, and every
    gameplay floor passed chain validation; otherwise the legacy blood spread runs
    unchanged.
  - One lava sheet instance per `WalkableFloorBoxes` rectangle, parked `80uu` below
    the floor surface. The rise clock advances only on the hero's current floor:
    after `LavaGraceSeconds` (default 25) the sheet rises over `LavaRiseSeconds`
    (default 150) to `LavaMaxHeight` (default `320` - above Tier 1, below Tier 2).
    Abandoned floors freeze; the visual is the original animated lava palette, not
    the blood recolor.
  - Damage is submersion-based: the hero takes `LavaDamagePerTick` (default 20) on
    the existing 1s miasma cadence only while their feet are below the lava surface
    (15uu ankle grace). Standing on any platform above the surface is fully safe -
    the old XY-tile check with the `1600uu` Z tolerance does not apply in this mode,
    and lava is damage-over-time pressure, never an instant kill.
  - At full flood the base floor and Tier 1 platforms are submerged, while the Tier
    2 safe chain stays dry all the way to the descent hole.
- **Inflatable doorway arches** (`T66SpawnDoorwayArchTubes`): in baffle mode every
  doorway header box spawns a half-ellipse arc of baffle-tube segments (default 10
  segments, `110uu` tube diameter, apex `~1100uu`) instead of the flat lintel cube.
  Visual-only, matching the lintel's no-collision behavior; the lintel remains the
  fallback when arch assets are unavailable.
- Known v1 limitations (deliberate scope):
  - Enemies ignore lava (hero-only damage, same as legacy miasma); grounded enemies
    can wade through flooded floors.
  - Ramp visuals are rotated themed cubes, not baffle tubes.
  - Per-floor lava rise clocks are transient: save/load restarts a floor's lava at
    zero on re-entry (same behavior class as the legacy elapsed-time rebuild).
  - Minimap does not yet render platform footprints.

### 1.3 Tier accessibility infrastructure (2026-06-10 second pass — design contract)

User direction after the first bounce-course playtest: the floor reads as one giant
same-size cylinder everywhere, and dense per-cell chain platforms make spawns feel
boxed-in with no climb route. The next infrastructure is accessibility-first
multi-tier terrain, built slowly and verified mathematically.

**Reference processes (user-supplied):**

- `MegabonkTerrainGenerator` Unity project (extracted to
  `Saved/Reference/MegabonkTerrainGenerator/`): accessibility is CONSTRUCTED, not
  patched — the map grows as a single connected tree from the start cell; elevation
  changes only by +1 via a slope cell created along the growth direction (direction
  locks through the slope so a ramp always connects flat ground to flat ground);
  never branch sideways off a slope; downhill drops are always-free shortcuts.
  Every cell is born with a walkable path to the start.
- Fall Guys Tail Tag layout philosophy: two altitude tiers — open ground plus a
  central raised platform reached by MULTIPLE ramps spread around its perimeter
  (some trap-guarded), perimeter hop-up platforms, beam slaloms, and playable
  space beneath bridges.

**Phase 1 (landed with this pass):**

- Per-room mesas: every room with enough interior gets a central tier-1 mesa at
  `TierHeight` (+500uu — deliberately above jump reach so ramps/lifts matter),
  inset so a walkable ground ring always remains inside the room. Corridors and
  small rooms stay tier 0.
- Constructive ramps: each mesa gets 2-4 ramp cells on different sides at creation
  time (Tail Tag multi-approach rule). A ramp occupies one ground cell, rises the
  full tier step, and is direction-locked into the mesa edge. Mesa connectivity is
  therefore built-in: ground network connectivity comes from the dungeon
  room/corridor generator, and every mesa joins it through its ramps.
- Tier-aware safe chain: the arrival->exit BFS now walks a directed graph — same
  tier moves, ramp-up edges, and drop-down edges (Megabonk "down is free" rule).
  Corridor chain platforms slim to `620uu` footprints (~190uu clear lanes on both
  sides) and hop-on stones spawn only outside rooms; room interiors stay open
  ground + mesa + ramps.
- No-softlock proof: the layout validator BFSes the full (cell, tier) walk graph
  from arrival and FAILS the floor if any walkable cell is unreachable; the audit
  commandlet asserts it across seeds (`[T66Proof][TierAccessSummary]`).
- Lava interplay: lava cap stays below both the slimmer corridor chain tops and the
  mesa surfaces, so at full flood the dry network = mesas + ramps + corridor chain.
- Visual variety contract: three distinct tube scales — fat ground baffles, thinner
  mesa-top baffles, small roller tubes packed side by side across ramps (the
  "smaller cylinders composing a ramp" rule), plus stacked-tube mesa skirts.

**Phase 2 (in order):** moving lift platforms (LANDED — section 1.4), ring mesas
with drop-in pits (lifts are the planned pit escape route), a third tier,
trap-guarded ramp placement (sweeper/hammer at ramp tops per Tail Tag), beam
slalom posts, and corridor-length tuning so halls between rooms stay short.

**Playtest fixes (2026-06-10 evening):**

- **Stateful floor membership (the real black-map root cause)**: floor membership
  used to be derived from the hero's Z every query (`FindFloorIndexForLocation`,
  nearest surface band, 900uu tolerance, floors 1224 apart) — so a high jump from a
  mesa/platform "promoted" the hero to the floor above, and `SyncTowerTrapActivation`
  reacted by hiding the current floor's terrain visuals, disabling its collision
  proxies, switching traps, and flipping the minimap (the black map + red X).
  Floor membership is now STATEFUL (`StatefulHeroTowerFloorNumber`): it changes only
  through explicit transitions — tower layout build (start floor), descent hole
  triggers, and fall-rescue snaps — and `GetCurrentTowerFloorIndex()` returns the
  stored value (Z derivation remains as a pre-spawn bootstrap fallback only). The
  rising-lava floor clock follows the same stateful value. No jump height, ceiling
  touch, or altitude can change what floor you are on.
- **Camera vs ceiling (final approach — spring-arm collision)**: jumping from a
  platform could lift the camera level with/inside the ceiling slab, which blacks
  out the scene (occlusion/backfaces) and reveals the layout. A first attempt
  clamped the camera Z + re-aimed via a camera modifier — user verdict: wrong
  approach (lost hero framing); removed. The shipped fix is physical: the hero
  boom now runs `bDoCollisionTest=true` (probe 16, ECC_Camera) and ceilings spawn
  invisible CAMERA-ONLY blocker proxies (`T66_Floor_Tower_CeilingCameraBlocker`,
  QueryOnly, all channels ignored except ECC_Camera). Walls and course geometry
  keep ignoring the camera channel (`T66.Camera.IgnoreTowerWallCameraCollision`
  default 1; occluder-fade handles see-through), so corridor camera distance stays
  stable and the boom only shortens when the view would breach a ceiling. Drop-hole
  areas are sealed for the camera too, so holes are not a peek-up vector; expect a
  brief boom shorten during floor-to-floor falls.
- **Short halls**: room scatter previously scored candidates by distance-to-nearest
  room and picked the MAXIMUM — actively maximizing hall length. Rooms now grow as a
  tight cluster: a candidate beyond `RoomMaxGapCells` (ini, default 6) edge-gap from
  the nearest existing room is rejected, with a sweet spot around 3 cells, so every
  corridor is born short (constructive rule, same philosophy as the tier ramps).
  Audit evidence: total chain platforms fell ~12% (47.7k -> 41.7k over 200 layouts)
  purely from shorter arrival->exit corridor paths, all guarantees unchanged.

### 1.4 Moving lift platforms (2026-06-10 Phase 2 item 1 — design contract)

Fall Guys-style non-trap elevators that cycle between terrain tiers inside a room.
The player steps onto a parked slab at ground level and rides it up to a mesa top.
Lifts are the first Phase 2 infrastructure piece and the future escape route for
ring-mesa center pits (Phase 2 item 2).

**Placement (constructive, seeded):**

- Lifts generate only inside `T66BuildFloorTierTerrain`, as an ALTERNATIVE to one
  of a mesa's constructive ramps: when a mesa collected at least `MesaRampsMin + 1`
  candidate ramps, a seeded roll (`LiftChance`, default 0.5) converts the last
  candidate into a lift — same cell, same ascent direction into the mesa edge.
- Access invariant: total routes (ramps + lift) never drop below the Tail Tag
  minimum, AND at least `MesaRampsMin` walkable ramps always remain. A lift has
  downtime (it cycles); static ramps stay the guaranteed any-time routes, the lift
  is bonus variety. The audit commandlet asserts >= 2 access edges per mesa.
- The lift slab footprint (`LiftFootprint`, default 600) is pushed toward the mesa
  face, leaving a fixed 20uu clearance so the moving slab never scrapes the mesa
  collision proxy; at the top the deck sits flush with the mesa surface and the
  player walks straight off across the 20uu gap.

**No-softlock integration:**

- `T66CanWalkDirectedTier` treats a lift cell like a ramp cell: uphill edges are
  legal from the lift cell into its mesa (lift edges count as up-edges in the
  `TierAccessSummary` BFS). Drops stay free, per the Megabonk rule.
- The arrival->exit dry safe chain EXCLUDES lift cells entirely: a lift parked at
  the bottom of its cycle is submerged at full lava flood, so it can never be a
  dry anchor (unlike ramp wedges, which are static dry surfaces). The chain BFS
  routes around lift cells; room ground rings are >= 2 cells wide, so a single
  lift cell cannot disconnect the chain. If exclusion ever did break the chain,
  the existing `BounceCourseSummary` FAIL path clears the floor's chain markers
  and disables lava-rise — same fallback as any other chain failure.
- Scatter platforms, hop-on stones, and content placement
  (`T66IsWalkableTowerLocation` / `IsPointInsideBounceObstacle`) all treat the
  lift footprint as occupied, so nothing spawns inside the lift's travel column.

**Runtime actor (`AT66TowerLiftPlatform`):**

- Collision is a MOVING hidden box proxy: a movable `UBoxComponent` slab (60uu
  thick, BlockAll, camera channel ignored like all course geometry) owned by the
  lift actor. The visual is a separate no-collision cube slab in the FallGuysKit
  candy look (`MI_FallGuys_Lift`, mint green — distinct from the sunny-yellow
  static platforms so moving surfaces read at a glance). The hero rides via the
  standard character based-movement path (movable base + block).
- Cycle: dwell at bottom (`LiftDwellSeconds`) -> cosine-eased rise
  (`LiftTravelSeconds`) -> dwell at top -> eased descent. Each lift gets a seeded
  phase offset so lifts in one room desynchronize. Bottom deck top rests 30uu
  above the floor surface (below the 45uu step height, so boarding is a walk-on);
  top deck top lands exactly on the mesa surface (`TierHeight`).
- The deck stays bouncy (no `T66_NoSurfaceBounce` tag): boarding and riding never
  trigger bounce impulses (no landing impact), but jumping down onto the deck
  bounces like any course surface — intended Fall Guys feel.
- The actor carries both terrain sync tags (`T66_MainMapTerrain_Visual` +
  `T66_MainMapTerrain_CollisionProxy`) plus the floor tag, so the existing
  stateful floor visibility/collision pass hides it and disables its collision
  when the hero is on another floor. Lifts keep ticking while hidden (cheap, and
  the cycle position stays seeded-deterministic per phase offset).
- STATEFUL FLOOR RULE: a lift cycling between tiers WITHIN a floor never touches
  floor membership — `StatefulHeroTowerFloorNumber` is unchanged by any ride. If
  a future lift ever crosses FLOORS, that arrival is an explicit transition and
  MUST call `SetHeroTowerFloorNumber` (same contract as descent holes). Never
  derive membership from hero Z.

**Tuning (`Config/DefaultT66TowerTuning.ini`, sanitizer-clamped):** `TierLifts`
master switch, `LiftChance` [0..1], `LiftFootprint` [400 .. cell-100],
`LiftTravelSeconds` (floored so rise speed stays <= ~400uu/s — comfortably
rideable), `LiftDwellSeconds` [0.75..10] (enough to board at minimum).

**Known v1 limitations (deliberate scope):**

- A descending slab over a hero depenetrates them sideways (Fall Guys-style
  shove); the 30uu bottom rest height means nobody can be pinned underneath.
- Lift cycle clocks are transient: save/load re-entry restarts phase offsets
  (same class as the per-floor lava clocks).
- Enemies do not ride or avoid lifts (no nav impact; same hero-only scope as
  lava).

### 1.5 Elevated-deck architecture + exact-collision contract (2026-06-10 Phase 2 second pass)

User direction after the lift pass: the map reads as "a bunch of squares you jump on
top of" — solid blocks acting as a second altitude, not professional Fall Guys map
design. Two mandates: (1) the second altitude must be REAL architecture — ramp up,
walkable deck, ramp down, with the ground UNDERNEATH the deck playable (Tail Tag
bridges), plus circular shapes, not only rectangles; (2) collision must match the
visible mesh EXACTLY — no bouncing off air.

**Architecture rework (what each element is now):**

- **Mesas = elevated decks (bridges), not solid blocks.** The mesa is a 60uu-thick
  walkable slab at `TierHeight`, held up by round pillars; the ground beneath stays
  open, walkable, floodable by lava, and connected to the room's ground ring on
  every side (no walls under decks — softlock-impossible by construction). With its
  2-4 constructive ramps the mesa now reads exactly as "ramp up -> deck -> ramp
  down with space underneath". Pillars: cylinder visuals carrying their own exact
  simple collision, corner-inset plus a grid pitch so spans stay believable.
- **Tier 2 platforms = floating decks.** Chain and scatter Tier 2s are 60uu slabs
  at `PlatformTier2Height` on one round center pillar — walkable underneath
  (underside 340uu at defaults vs the 200uu hero). Chain decks stay RECTANGULAR
  (the dry-anchor gap proof measures box edges; round corners would silently widen
  diagonal gaps past jump range). Scatter Tier 2 decks may be round.
- **Tier 1 platforms = grounded stepping stones (unchanged height, new shapes).**
  At +200 the hero (200uu) cannot fit underneath, so Tier 1 stays ground-touching
  by design — but a seeded share (`RoundPlatformChance`, default 0.45) becomes
  CYLINDERS instead of cubes, so the floor reads as Fall Guys stools/pucks rather
  than boxes.
- Ramps and moving lifts are unchanged (already slabs with exact proxies).

**Exact-collision contract (binding):**

1. Every course element's collision matches its rendered mesh dimensions exactly.
   Rectangular elements: hidden box proxy == visual box. Round elements: the
   engine BasicShape cylinder's OWN simple convex collision is enabled on the
   visual itself (exact by definition). This AMENDS the box-proxy-only rule:
   round course elements may carry engine-primitive simple collision; box proxies
   remain the rule for everything rectangular. Defensive rule: if a basic shape
   ever ships without simple collision, the spawner falls back to the square
   variant rather than spawning a walk-through visual.
2. The 2uu floor-seam sink remains ONLY on ground-touching prisms (z-fight guard,
   below perception); floating decks need no sink (no coplanar floor face).
3. **Wall-bounce trace fix (the felt "bouncing off air")**: the hero's bouncy-wall
   sweep ran a 29uu sphere 220uu ahead of the hero center while the capsule radius
   is 34uu — bounces triggered ~2 hero-widths before visual contact. The default
   (`t66.HeroMovement.SurfaceBounceWallTraceDistance`) drops 220 -> 50 so the
   sweep reaches ~79uu from center: contact-feel anticipation of a few cm, not a
   body length.

**Camera under decks:** deck slabs carry a `T66_Tower_DeckVisual` tag and block
ONLY the camera channel (QueryOnly on no-collision visuals, response-only on
collision-bearing round decks) — the spring-arm boom shortens below a deck the
hero walks under (same physical contract as ceilings) and the occluder-fade pass
accepts deck-tagged components (thin-slab shape exemption to the wall filter), so
the hero never plays blind under a bridge. Pillars and Tier 1 stones keep ignoring
the camera channel.

**Validation model (unchanged on purpose):** CellTiers/BFS semantics are identical
— tier 1 cells mean "deck surface above", ramps/lifts are the up-edges, drops stay
free. The ground under a deck needs no BFS membership: it is open flat floor
connected to the ring on all sides by construction. The dry-chain proof, lava
clearances (deck tops unchanged), audit commandlet asserts, and content-placement
exclusions all hold without semantic changes.

### 1.6 Shape language + pattern variety (2026-06-11 Phase 2 third pass)

User verdict on the deck pass: structure is right (walk-under confirmed good), but
the read is "too uniform, too many squares, platforms one after the other with no
logic", and big decks should get holes. This pass gives the course a Fall Guys
shape language without touching any traversal guarantee.

**Shape kit (`FallGuysShapeKit01`, Blender-built, exact collision):**

- `SM_FGShape_Hex` — hexagonal prism (the Fall Guys Hex-A-Gone icon), squashed to
  an exact 100x100x100 AABB (corners at +-50 X, flats at +-50 Y) so the cube
  scale math and the box gap proof both stay EXACT: contact distances on the hop
  centerlines equal the AABB faces in both axes.
- `SM_FGShape_Tri` — triangular prism, vertices (-50,-50)(50,-50)(0,50), 100 AABB.
- Both flat-shaded, one material slot, and given a 1-hull convex collision at
  import (a convex prism's hull IS the mesh — exact-collision contract holds).
  Engine cylinder remains the round shape. Defensive square fallback stays.

**Shape policy (seeded, `RoundPlatformChance` now = total non-square share):**

- Chain platforms: per-platform mix square / round / hex. Safe because chain hops
  are ALWAYS axis-aligned 4-neighbor steps and every kit shape's centerline
  contact distance equals its AABB face — the box-gap proof measures the true
  jump. First and last chain platforms stay square (ramp/hole anchor edges).
- Scatter + stones: per-ROOM shape theme (Mixed / Rounds / HexField / SquareTri)
  so each room reads intentionally designed, not sprinkled; triangles are
  scatter-only (their pointy sides never carry the chain), with seeded 90-degree
  yaw steps. AABBs recorded in `FBouncePlatform.Bounds` keep content exclusion
  and the audit conservative-correct.

**Chain rhythm (kills the conveyor read):**

- Footprint alternation widens to a seeded big/small rhythm (even cells 700-860,
  odd cells 560-660 — worst adjacent pair 1260 -> 370uu gap, still under the 380
  cap and the ~435 jump range).
- Cross-axis MEANDER: each mid-chain platform shifts up to +-170uu perpendicular
  to its local path direction. Perpendicular offsets never widen the hop gap
  (boxes keep overlapping in the cross axis, so the gap stays the axis gap); the
  first/last two platforms stay centered for the ramp wedge and descent-hole
  anchors.

**Ring mesas (the "hole in the middle", Phase 2 item 2 pulled forward):**

- Mesas spanning >= 5x5 cells roll `RingMesaChance` (default 0.5) to become RING
  decks: a 1-2 cell center hole, deck spawned as 4 frame slabs (N/S/E/W), hole
  cells revert to tier 0. Falling through the hole is a legal drop into the open
  under-deck ground — with the deck architecture the pit escape is built-in (walk
  out between pillars), which is exactly why item 2 needed lifts BEFORE decks
  existed and does not anymore.
- Guards: the dry-chain BFS skips tier-0 cells inside any mesa rect (a chain
  platform inside the hole would sit under the deck slab); pillars skip the hole
  interior and rim its corners instead; ramps/lifts keep attaching to OUTER mesa
  edges only; content placement already excludes mesa bounds.

> **Deep analysis companion (2026-06-11):** `FALLGUYS_MAP_ANALYSIS.md` — ten
> sourced Fall Guys design principles, the T66 gap audit, and the prioritized
> Tier A-D improvement program that this proposal plugs into.

### 1.7 Room courses: "miniature Fall Guys maps" (2026-06-11 — IMPLEMENTED, all four analysis solutions)

**Status: LANDED, then infrastructure-renamed on 2026-06-11.** The first pass
kept the existing mechanics but moved the design vocabulary away from named room
templates. Rooms now carry composition profiles and reusable structure IDs; the
offensive obstacle layer is described as hazards.

- **Tier A visual language**: beveled course kit (SM_FGShape_BevelCube/BevelPuck
  + re-authored beveled Hex/Tri, exact 1-hull convex collision, bevels only cut
  inward from the AABB so all gap math is untouched); white edge-band trims on
  rectangular decks and mesa frames (batched HISM, 6uu decorative protrusion —
  same accepted class as the 2uu sink); Tier 2 decks recolored CORAL
  (`MI_FallGuys_Deck`) so height-as-progress reads at a glance; every surface
  that punts the hero (sweeper arm, bumpers, pads, hammer mallet, bounce pads)
  wears the diagonal-stripes hazard signature.
- **Tier B room composition**: rooms 6-10 cells (was 10-20), 13-16 per floor
  (was 10), arrival room 6-8 (was 3-4), exit room biased big (area term in the
  exit chooser, range-contract room-count proof). Each eligible room now receives
  a composition profile plus one or more reusable structures: central mesa, ring
  mesa, stepping stones, bridge deck, or scatter stones. Those are infrastructure
  pieces, not named room identities; later passes should add more structures and
  let combinations carry the variety. Every structured room registers 1-2 REWARD
  SLOTS; the interactable population fills slots FIRST (vendor/totem/loot
  bag/chest/crate + bonus chest for two-slot rooms), and every slot gets a white
  BEACON column.
- **Tier C movers**: structure-anchored hazards (the hazard spawn path consumes
  composer anchors before random placement, anchor Z preserved for deck-top sweepers);
  new `AT66BouncePadObstacle` — striped walk-on disc, capsule-launch straight up
  (`t66.Trap.BouncePadLaunchZ` 2200 clears the mesa deck), never ragdolls.
- **Tier D route choice**: where the dry chain detours through a room, a FAST
  lane of small round stones cuts the corner — at full lava flood the low fork
  drowns while the Tier 2 chain stays dry (risk/reward made literal).

**Original proposal (for reference):**

User direction after the shape pass: every room should feel like a MINIATURE Fall
Guys map with 1-2 REWARDS (NPC or interactable) as its objectives — you enter a
room, read the objectives, and traverse/jump/dodge to reach them. Not a pure
obstacle course, not random objects on a floor: a guided middle. Current state
fails this because rooms are huge (10-20 cells), content density is ~7 chests
across two whole floors, and scatter is per-cell random with no composition.

**Original proposal (superseded by structure/hazard composition):**

1. **Smaller rooms**: combat rooms drop from 10-20 to ~6-10 cells per side, room
   count rises to compensate (10 -> 12-16). A room then reads in ONE camera view —
   the precondition for "miniature map".
2. **Bigger gate rooms**: the rooms containing the ARRIVAL (drop landing) and the
   EXIT hole get a dedicated larger room rule (~10-12 cells) so gate ceremony
   (idol altar, guardian miniboss, lift queue) breathes.
3. **Room structures (the heart)**: each eligible room receives a composition
   profile and a small set of reusable structures, composed from the existing
   verified pieces (mesa deck / ring deck / lifts / ramps / stones / floating
   decks / one hazard). Structures place their pieces RELATIVE to room bounds
   (parametric, seeded variation in counts/shapes/yaws), so every instance differs
   without depending on named room archetypes. Placement keeps every existing
   invariant: constructive access, BFS coverage, content exclusions, gate
   clearances.
4. **Rewards as objectives**: each templated room reserves 1-2 REWARD SLOTS at
   its designed payoff points (deck top, pit floor, bridge end) and the
   interactable/NPC population FILLS those slots first (chests, crates, loot
   wheels, NPCs), instead of free-roaming random placement. Room rule
   RewardContentSlots becomes the slot count source.
5. **Corridor identity**: corridors keep the safe chain + stones only — rooms own
   the composition, halls stay transit.

**Open direction:** future content work should add structures and hazards to the
libraries rather than adding named room archetypes.

**Look flip (2026-06-10, same day):** after the first tier playtest the user parked
the baffle-tube look entirely until the map design is nailed — overlapping tubes
also z-fight in the light. The tower now renders the full Fall Guys read: clean
slab/box geometry in bright solid candy colors via
`/Game/World/Terrain/FallGuysKit/MI_FallGuys_*` instances of the FriendSlop master
(floor sky blue, walls bubblegum pink, ceiling lavender, platforms sunny yellow,
ramps magenta-purple, mesas violet). Switches: `t66.Tower.FallGuysLook` (default 1,
0 restores themed materials) and `t66.Tower.FloorBaffles` (default now 0; 1 brings
the inflated tube visuals back). Course prisms sink 2uu into the floor so coplanar
faces cannot z-fight. Trap meshes keep their inflatable balloon kit — only map
surfaces flipped to slabs.

## 2. Current Map Runtime Spine

### 2.1 Preset selection and persistence

- `Source/T66/Gameplay/T66ProceduralLandscapeParams.h`
  - defines `ET66MainMapLayoutVariant`
  - current live value is:
    - `Tower`
- `Source/T66/Core/T66GameInstance.cpp`
  - no longer reads map-layout config
  - no longer supports a map-layout console override
  - applies the finalized tower layout directly to runtime run state
- Save/load already persists the chosen layout variant:
  - `Source/T66/Core/T66RunSaveGame.h`
  - `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`
  - `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
- Important limitation:
  - saves still carry a layout field for compatibility
  - saves do **not** know a concept like `current floor inside a tower stage`

### 2.2 Current main-stage geometry model

- `Source/T66/Gameplay/T66MainMapTerrain.cpp`
  - is the active generator for the normal gameplay stage
  - builds a single square board with reserved regions:
    - `StartArea`
    - `StartPath`
    - `MainBoard`
    - `BossPath`
    - `BossArea`
- `Generate(...)`
  - expects one contiguous grid footprint
  - not stacked floors
  - not cylindrical walls
  - not floor-to-floor descent holes

### 2.3 Current stage flow

- `Source/T66/Gameplay/T66GameMode.cpp`
  - boots one stage as one play space
  - current normal-stage sequence is:
    1. spawn terrain
    2. place tower start-floor content
    3. player crosses start threshold
    4. stage timer starts
    5. miasma + enemy director activate
    6. player reaches boss threshold
    7. boss awakens
    8. boss dies
    9. stage gate spawns
    10. stage transition reloads the gameplay map for the next stage
- Current special flow actors:
  - `AT66StartGate`
  - `AT66BossGate`
  - `AT66StageGate`
  - `AT66CowardiceGate`
  - `AT66IdolAltar`

### 2.4 Current timing and pressure model

- `Source/T66/Core/T66RunStateSubsystem.h`
  - stage timer duration is currently fixed:
    - `420s`
    - `7:00`
- `Source/T66/Gameplay/T66MiasmaManager.cpp`
  - normal layouts still build coverage over the stage footprint
  - tower default is now the rising-lava mode over gameplay floors (see section 1.2);
    the blood-style spread coverage remains the fallback when the bounce-course
    chain validation fails or `TowerLavaRise=0`
  - tower hazard timing is floor-aware, but altar-driven blood escalation is still not fully authored

### 2.5 Current minimap model

- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - normal-layout minimap is still a generic XY snapshot over the current stage
  - tower minimap/full-map are now active-floor views
  - tower reveal memory is floor-local and resets visually when moving to another floor
  - tower map rendering now uses the current floor polygon instead of the old square placeholder
  - full map uses broad stage areas:
    - `START`
    - `MAIN`
    - `BOSS`
  - markers come from actor positions, not explored floor tiles
- Current limitation:
  - normal layouts do not have per-floor reveal memory because they are still single-board stages
  - tower reveal is marker- and floor-memory-based; it no longer depends on a lighting or fog visibility mask

### 2.6 Current NPC / interactable placement

- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`
  - `SpawnWorldInteractablesForStage()`
    - scatters world interactables across the stage, excluding reserved start/boss zones
  - `SpawnSupportVendorAtStartIfNeeded()`
    - optional start-area vendor
  - `SpawnCircusInteractableIfNeeded()`
    - current casino shell
- Important current reality:
  - the "casino" is not a separate Gambler NPC in stage placement
  - it is `AT66CircusInteractable`
  - current circus/casino spawns at most once per stage
  - current circus safe zone radius is large (`1100`)
  - current circus mesh is `/Game/World/Interactables/Casino/Casino.Casino`
- Backrooms entrance doors are tower-wall interactables placed by the tower GameMode bootstrap. The spawned pocket uses `/Game/World/Backrooms/Textures/T_Backrooms_Wall`, `T_Backrooms_Floor`, and `T_Backrooms_Door`, with a closed entrance door behind the player and a separate exit door inside the maze.
- Entering the Backrooms pauses stage pressure, timers, enemies, traps, projectiles, miasma, and save-and-return. The hero and Backrooms chaser remain active; inventory and weapon state are temporarily hidden and restored on exit or death resolution.
- A successful Backrooms exit grants the reward-only `Item_BackroomsQuickRevive` item. Owning that item makes later Backrooms door rolls ineligible.
- Non-shipping QA uses `T66.Backrooms.ForceSpawn 1` plus `-T66BackroomsAutoQA=Exit|Death|Consume`. `-T66BackroomsAutoQAScreenshot=<path>` is requested by `AT66GameMode` only after the entry interaction has made `bBackroomsChallengeActive` true, so the proof image captures the active hidden pocket rather than the pre-entry tower wall.
- `SpawnIdolAltarForPlayer()` / `SpawnIdolAltarAtLocation(...)`
  - currently supports:
    - stage-start altar
    - post-boss altar near gate spawn

### 2.7 Current enemy spawn model

- `Source/T66/Gameplay/T66EnemyDirector.cpp`
  - normal layouts still spawn around player world positions using min/max distance rings and safe-zone exclusion
  - tower layout now filters by the active floor and spawns from the shell perimeter instead of below-ground rise locations
- `Source/T66/Gameplay/T66GameMode.cpp`
  - legacy visual-navigation and traversal-pad runtime paths have been removed
  - tower terrain safety rescue now uses tower-local traces and a below-bottom-floor threshold so falling through holes is not treated as an out-of-bounds recovery
- `AT66BossGate`
  - currently pauses the enemy director when the player enters the boss threshold

## 3. Current Design Constraints That Matter For The Tower Preset

### 3.1 The current system assumes one continuous stage board

- Start flow, combat activation, boss activation, miasma coverage, minimap, and interactable scatter all assume one board for one stage.

### 3.2 Save/load needs new floor-aware state

- Layout variant is already saved.
- Floor index, current-floor hazard state, and per-floor reveal state are not.

### 3.3 The current minimap is too stage-global

- Tower requires the minimap to become a current-floor system.

### 3.4 The current miasma system is too footprint-global

- Tower blood pressure needs to care about the active floor only.

### 3.5 The current casino implementation is already a shell/interactable, not a gambler house NPC

- Recommended path:
  - keep the casino as a variant of `AT66CircusInteractable`
  - shrink it for the tower preset
  - do **not** revive a separate Gambler NPC unless that is a deliberate broader design change

## 4. Target Tower Preset Design

### 4.1 Core fantasy

- One gameplay stage remains one runtime stage and one timer.
- Inside that stage, the player descends through multiple vertically stacked floors.
- Each floor is part of the same stage, not a separate stage.
- The geometry theme is:
  - connected polygon shell with straight walls for the first practical implementation
  - walkable floor footprint that can evolve toward a rounder silhouette later if needed
  - side-wall enemy emergence
  - one descent hole leading to the next floor
  - final floor is the boss floor

### 4.2 Recommended first playable structure

- Current normal tower structure:
  - `Floor 1 - Start`: start floor only, modeled after the current start area, with the weapon altar and no normal enemy/wave gameplay
  - `Floor 2`: gameplay floor with monsters, world interactables, and side-wall enemy spawning
  - `Floor 3`: gameplay floor with monsters, world interactables, and side-wall enemy spawning
  - `Floor 4`: gameplay floor with monsters, world interactables, and side-wall enemy spawning
  - `Floor 5 - Boss`: boss floor
- Boss-rush finale is an explicit two-floor exception and skips the normal enemy/interactable population rules.
- Reason for this recommendation:
  - `7:00` total stage time is already established in runtime/UI/save logic
  - `3:00` boss budget leaves `4:00` for non-boss gameplay
  - separating the top floor as a start-only space preserves the current game feel
  - `5` total floors keeps the descent readable without the old seven-floor confusion

### 4.3 Traversal rules

- `Floor 1 - Start` should own:
  - the stage-start weapon altar
  - current start-area feel and breathing room
  - no standard monster spawning
  - no normal world-interactable scatter pass
  - the first descent hole into real gameplay
- Each gameplay floor (`Floor 2` through `Floor 4`) should own:
  - one local exploration space
  - one descent hole
  - side-wall enemy spawn anchors
  - optional small casino / vendor opportunities
- `Floor 5 - Boss` should own:
  - boss entry point
  - boss combat space
  - no further descent hole
- Live implementation note:
  - between floors in the same stage, traversal is handled by the floor hole
  - after the boss dies, progression to the next stage still uses the existing `AT66StageGate` portal

### 4.3.1 Placed miniboss descent gates

- Normal tower stages use deliberate placed miniboss encounters instead of random per-wave miniboss promotion.
- The eligible floor exits are:
  - `Floor 2 -> Floor 3`
  - `Floor 3 -> Floor 4`
  - `Floor 4 -> Floor 5 - Boss`
- `Floor 1 -> Floor 2` has no miniboss gate.
- Boss-rush finale stages are a two-floor exception and have no placed miniboss gates.
- The placeholder miniboss is the basic `Slime` mob row, spawned as a rich `AT66EnemyBase` and scaled with the current miniboss multipliers (`3.0x` HP, `2.0x` damage, `1.75x` actor scale).
- The same slime placeholder appears across all stages until unique miniboss creatures are authored.
- The placed miniboss spawns when the player enters its floor, is assigned to that floor's `AT66TowerDescentHole`, and must be killed before the hole can open.
- The existing tower guardian death behavior still runs on defeat, including the idol altar side effect; changing miniboss rewards is a separate design pass.
- Random miniboss promotion in normal trickle waves is disabled, which keeps basic-mob performance captures free of random rich miniboss routes.

### 4.4 Hazard rules

- Replace the current lava presentation with blood / miasma for the tower preset.
- Recommended first-pass behavior:
  - interacting with the floor altar arms that floor's blood pressure
  - blood spreads only on the current floor
  - the HUD timer turns red once current-floor blood is active
  - when the player descends, the previous floor can be discarded from active hazard simulation

### 4.5 Minimap rules

- Minimap should show only the active floor.
- Floor starts mostly dark.
- The visible region is revealed through exploration.
- When the player descends:
  - old floor map is no longer the active display
  - new floor starts dark again
- Recommended first-pass rule:
  - no stacked tower overview yet
  - current-floor-only map is enough

### 4.6 NPC / interactable rules for tower preset

- Casino:
  - use a smaller circus/casino version
  - chance to spawn on each gameplay floor
- Saint:
  - exactly once per stage
  - only on one gameplay floor
- Baby gate:
  - placed beside the descent hole before the boss floor

### 4.7 Visual and asset rules for tower preset

- The first tower prototype should reuse existing assets only.
- Terrain materials:
  - reuse the same texture/material family already used by the current main map
  - do not block the preset on new texture work
- Props:
  - do not reuse old tree, dirt, or rock clutter from the legacy map/prop pipeline
  - use only current approved generated visual props or a future explicitly reviewed generated-prop pass
- Practical implication:
  - the tower preset should be shippable with current tower terrain textures and latest approved generated visual props
  - no new environment-art dependency is required for the first playable version

## 5. Recommended Technical Approach

### 5.1 Keep preset selection shared, but split geometry implementation

- Keep one shared live preset enum value:
  - `Tower`
- Do **not** force tower geometry into a long chain of `if (LayoutVariant == Tower)` inside `T66MainMapTerrain::Generate(...)`.
- Recommended implementation:
  - keep tower geometry in the tower runtime generator

### 5.2 Use one runtime world, stacked vertically

- Recommended first implementation:
  - all floors exist inside the same gameplay world
  - floors are separated by Z distance
  - descending through a hole moves the player to the next floor in the same map
- Reason:
  - preserves current stage model
  - avoids per-floor streaming as the first implementation burden
  - keeps one stage timer and one stage save context

### 5.3 Add floor-aware runtime state

- Add state for:
  - current tower floor index
  - total floors in this stage
  - current-floor blood active state
  - current-floor reveal data
- This should be available to:
  - `GameMode`
  - minimap/HUD
  - save/load
  - enemy spawning

### 5.4 Convert spawning from area-random to floor-anchor-driven

- Current `EnemyDirector` chooses random world positions around players.
- Tower preset needs authored/runtime-generated wall anchors.
- Recommended first-pass architecture:
  - tower generator emits wall spawn anchors per floor
  - enemy director selects from anchors on the active floor
  - anchors must respect min distance and visibility fairness

### 5.5 Make hazard coverage provider-driven

- Current `AT66MiasmaManager` derives coverage from the whole main board.
- Tower preset needs:
  - current floor footprint
  - blood visual profile
  - floor-local damage only
- Recommended path:
  - keep the manager actor
  - add coverage/profile modes rather than writing a second unrelated hazard actor

### 5.6 Keep the casino as the circus shell

- The project already routes casino behavior through `AT66CircusInteractable`.
- Recommended tower approach:
  - create a smaller tower-specific circus/casino spawn mode
  - reduce safe-zone radius
  - reduce visual footprint
  - preserve the existing circus overlay flow

## 6. Required File Surface

### 6.1 Preset plumbing

- `Source/T66/Gameplay/T66ProceduralLandscapeParams.h`
  - add `Tower`
- `Source/T66/Core/T66GameInstance.cpp`
  - parse `Tower`
  - extend console override handling beyond `0/1`
- `Config/DefaultGame.ini`
  - update valid-values comment

### 6.2 New tower runtime generator

- Recommended new files:
  - `Source/T66/Gameplay/T66TowerMapTerrain.h`
  - `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- Responsibilities:
  - build stacked floor descriptors
  - emit traversal/boss floor anchors
  - emit side-wall spawn anchors
  - emit hole locations
  - emit minimap bounds per floor

### 6.3 Game flow / floor transitions

- `Source/T66/Gameplay/T66GameMode.h/.cpp`
  - route to tower preparation when `LayoutVariant == Tower`
  - replace start/boss-board assumptions for that preset
  - manage active floor transitions
- Recommended new actor(s):
  - `AT66TowerDescentHole`
  - `AT66TowerBabyGate`

### 6.4 Enemy spawning

- `Source/T66/Gameplay/T66EnemyDirector.h/.cpp`
  - either:
    - add spawn-provider support
    - or split tower spawning into a dedicated tower director

### 6.5 Hazard / timer warning

- `Source/T66/Gameplay/T66MiasmaManager.h/.cpp`
  - add tower floor coverage mode
  - add blood visual profile
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
  - timer warning turns red when current-floor blood is active

### 6.6 Minimap

- `Source/T66/UI/T66GameplayHUDWidget.h/.cpp`
  - active-floor-only map mode
  - per-floor reveal state
  - tower minimap bounds instead of current stage-global board framing

### 6.7 Save/load

- `Source/T66/Core/T66RunStateSubsystem.h/.cpp`
  - export/import tower floor state
- `Source/T66/Core/T66RunSaveGame.h`
  - persist current tower floor data
- `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`
- `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp`
  - save/load new tower state

## 7. Recommended Implementation Order

### Phase 1. Lock the preset and floor model

- Keep the live layout locked to `Tower`.
- Lock initial stage shape:
  - `1` start floor
  - `4` gameplay floors
  - `1` boss floor
  - existing textures/materials only
  - current terrain/material assets only; no old tree, dirt, or rock clutter props

### Phase 2. Get tower geometry playable without full feature parity

- Build the stacked tower floors.
- Add descent holes.
- Add active floor tracking.
- Let the player move from floor to floor.
- Keep `Floor 1 - Start` as a start-only, weapon-altar-only floor.

### Phase 3. Move combat spawning to tower rules

- Add side-wall spawn anchors.
- Make enemy spawns floor-aware.
- Restrict spawns to the active floor.
- Keep normal monster spawning disabled on `Floor 1 - Start`.

### Phase 4. Move stage systems to tower rules

- Replace boss-threshold assumptions with boss-floor entry behavior.
- Move baby gate to final traversal floor.

### Phase 5. Move hazard pressure to tower rules

- Convert tower preset from lava presentation to blood presentation.
- Make hazard floor-local.
- Arm it from altar interaction.
- Turn timer red when active.

### Phase 6. Move the minimap to floor-local reveal

- Current floor only
- dark-until-explored
- reset visibility state on descent

### Phase 7. Save/load and standalone validation

- Save current floor index
- save active floor hazard state
- save map reveal state if needed
- validate in packaged Development standalone, not just editor

## 8. Recommended Defaults For First Pass

- Preset enum label:
  - `Tower`
- Start floor count:
  - `1`
- Gameplay floor count:
  - `3` (`Floor 2` through `Floor 4`)
- Boss floor count:
  - `1`
- Circus/casino:
  - smaller tower-specific variant
  - chance per gameplay floor
- Saint:
  - exactly once per stage
- Baby gate:
  - not part of the current normal five-floor tower contract
- Art source:
  - reuse current terrain textures/materials
  - do not reuse old tree, dirt, or rock clutter props

## 9. Open Questions Still To Lock

1. Do we want the preset name to stay plain `Tower`, or should it be something more thematic like `Descent` while keeping the same design?
2. Does altar interaction immediately arm blood pressure, or should blood begin after a short grace delay on that floor?
3. Should the timer remain frozen on `Floor 1 - Start` and only begin once the player descends into `Floor 2`, matching the current start-area philosophy?
4. Should the casino be allowed on `Floor 4`, or should that floor stay focused on pre-boss setup?
5. Do we want one altar on every gameplay floor, or only on selected floors after pacing tests?

## 10. Bottom Line

- The current project already has the right place to add a third preset.
- The current project does **not** have a tower-ready runtime model yet.
- The major shift is not just terrain shape; it is a change from:
  - one continuous stage board
  - one global minimap
  - one global hazard footprint
  - area-random enemy spawning
- The safest path is:
  - add `Tower` as a third preset
  - implement tower geometry as a separate map family
  - then migrate minimap, hazard, spawn, and NPC rules onto a floor-aware runtime model in phases
