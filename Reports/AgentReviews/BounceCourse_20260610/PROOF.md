# Bounce Course + Rising Lava + Doorway Arches — Implementation Proof (2026-06-10)

Working task: Fall Guys-style obstacle-course layer for tower gameplay floors (two-tier
bouncy platforms + ramps), rising-lava hazard mode with a guaranteed dry platform chain
from arrival to exit, and inflatable baffle-tube arches over maze doorways.

## What landed

| Piece | Where |
|---|---|
| Platform/ramp/safe-chain generation + validation | `Source/T66/Gameplay/T66TowerMapTerrain.cpp` (`T66BuildFloorBouncePlatforms`, BFS `T66BuildBounceSafeChainPath`) |
| Platform spawn: hidden box proxies + batched baffle-tube visuals (1 ISM actor/floor) | `T66SpawnBounceCourseForFloor` |
| Doorway arches (half-ellipse tube arcs, visual-only, lintel fallback) | `T66SpawnDoorwayArchTubes` |
| Content placement keeps off platforms | `T66IsWalkableTowerLocation` -> `T66IsLocationInsideBounceObstacle` |
| Rising-lava mode (per-floor sheets, hero-floor rise clock, submersion DOT) | `Source/T66/Gameplay/T66MiasmaManager.cpp/.h` |
| Tuning + jump-math invariant clamps | `Source/T66/Core/T66TowerTuningConfig.*`, `Config/DefaultT66TowerTuning.ini` |
| Multi-seed audit commandlet | `Source/T66/Gameplay/T66BounceCourseAuditCommandlet.*` |
| Docs | `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` §1.2, `T66_TUNING_SURFACE.md`, `pending_issues_Gameplay.md` |

## Traversability guarantee (the core math)

Hero jump: JumpZ `1600`, GravityScale `4.5` -> max step `~290uu`, safe flat gap `~420uu`
at minimum walk speed. Sanitizer-enforced invariants: Tier1 <= `260`, Tier2-Tier1 <= `260`,
chain gap <= `420`, `LavaMaxHeight <= Tier2 - 60`. Defaults: Tier1 `200`, Tier2 `400`,
chain footprint `700` in `1000` cells (gap `300`), lava cap `320`. Tier 2 chain spans a
BFS arrival->exit path on every gameplay floor; at full flood the base floor and Tier 1
are submerged, the Tier 2 chain stays dry to the descent hole. Lava damage is
feet-below-surface DOT (`20`/s default), never instant death.

## Verification performed

1. **Focused compile**: `Build.bat T66Editor Win64 Development` — `Result: Succeeded`
   (final run 2026-06-10 ~09:06 local, after batching platform tubes into one ISM actor
   per floor). One pre-existing latent unity-build name collision surfaced by the new
   files (`CoinSize` in `T66HUDPresentationController.cpp` vs `T66CoinFlipGameWidget.cpp`)
   was fixed by renaming the HUD local to `CoinDrawSize`.
2. **Multi-seed layout audit** (headless commandlet, `-nullrhi`):
   `UnrealEditor-Cmd.exe T66.uproject -run=T66BounceCourseAudit -seeds=40`

   ```text
   [T66Proof][BounceCourseAudit] Result=PASS layouts=200 mobFloors=400 failedFloors=0
   failedLayouts=0 worstChainGap=280 worstHoleReach=150 totalPlatforms=64690
   totalRamps=800 seedsPerDifficulty=40
   ```

   200 layouts = 5 difficulties x 40 seeds. Worst chain gap `280` (limit `350`,
   jump range `~420`); worst final-platform-to-hole reach `150`. Zero failures.
   Per-floor `[T66Proof][BounceCourseSummary]` lines also log on every live build.
3. **Staged standalone**: `Scripts/StageStandaloneBuild.ps1` (BuildCookRun build+cook+
   stage+pak). First attempts failed with `Error_FailedToDeleteStagingDirectory` because
   the user was playing the previously staged T66.exe; the retry after the game closed
   completed with `BUILD SUCCESSFUL` / `ExitCode=0 (Success)` (UAT log, 2026-06-10
   09:12). Evidence: staging directory recreated 09:11:56, pak set written
   09:11:56-09:12:20 (`T66-Windows.ucas` 4.44 GB), staged `T66.exe` is the 09:08 game
   target built from post-batching source, `T66 Standalone.lnk` targets
   `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`, staged SaveGames preserved
   (27 files). Logs: `Saved/Logs/StageStandalone_BounceCourse*.log`.
4. **Visual capture**: SKIPPED this session — direct entry lands on the start floor
   (deliberately platform-free); no existing capture mode traverses to gameplay floors,
   and adding floor-2 capture automation was out of scope. First-look verification is
   the taskbar standalone shortcut; platforms/arches appear on floors 2-3, lava starts
   after the flood arms plus 25s grace on the current floor.

## Addendum 2026-06-10 (same day): tribulation-entry crash fix + InflatableTraps01

- **Crash**: `EXCEPTION_ACCESS_VIOLATION reading 0xffffffffffffffff` entering the
  Tribulation (crash folder `UECC-Windows-71C9F7AA4E5D0013EB2683968EF3DE54_0000`).
  Symbolicated stack: `SetStaticMesh <- T66SpawnGeneratedDungeonInstancedMeshActor <-
  T66SpawnWallBaffleTubeVisualsForBox <- ... <- T66TowerMapTerrain::Spawn`. Root cause:
  `T66GetFloorBaffleTubeMesh()` cached `SM_BaffleTube` in a function-local
  `static TObjectPtr` — invisible to the GC, so the mesh was collected after a world
  teardown and the pointer dangled on the next tower spawn. Fixed by rooting on first
  load (`AddToRoot`); the same latent pattern in `FT66VisualUtil::GetBasicShape*` and
  the new trap asset caches was fixed the same way. `SM_BaffleTube` + the trap kit are
  now in the CookGuard registry.
- **Entry proof**: `Scripts/RunLifecycleTransitionSmokeGate.ps1` on the final staged
  build — **PASS** (`Saved/LifecycleTransitionSmokeGate/20260610_111447`), 6 world
  travels with repeated tower terrain regeneration (the exact crash pattern),
  `[CookGuard] All 35 code-referenced assets resolve`, every regeneration logging
  `BounceCourseSummary ... PASS`, zero fatal lines.
- **InflatableTraps01**: the four obstacle traps now render as authored inflatable
  balloon meshes with Codex-imagegen pattern MIs of the FriendSlop master (see
  `Model Generation/Runs/Environment/InflatableTraps01/Notes/decision_log.md` and
  `Renders/textured_sheet_*.png`). Final staging `BUILD SUCCESSFUL` 11:12, shortcut
  refreshed, SaveGames preserved.

## Addendum 2026-06-10 (third pass): tier accessibility infrastructure (Phase 1)

User direction after playtest: same-size cylinders everywhere and boxed-in spawns with
no climb route; build accessibility-first multi-tier infrastructure using the
MegabonkTerrainGenerator reference ZIP (constructive connectivity) and Fall Guys
Tail Tag (two-tier rooms, multi-ramp central platforms). Design contract:
`T66_MAP_DESIGN_REFERENCE.md` section 1.3.

- **Mesas**: rooms (excluding arrival/exit rooms) get a central +500uu tier-1 mesa
  (above jump reach AND the lava cap) inset so a walkable ground ring always remains,
  committed only when 2-4 ramps on distinct sides validate first (constructive rule:
  no access, no mesa).
- **No-softlock proof**: directed BFS over the (cell, tier) walk graph (same tier /
  ramp-up / drop-down) from arrival must reach 100% of walkable cells, else the floor
  flattens and disables lava (`[T66Proof][TierAccessSummary]`). The return path is
  constructive: drops always reach the connected ground network.
- **Chain rework**: the dry chain stays on tier 0 (mesas are bonus dry zones), detours
  around ramp wedges, slims to 620uu footprints (~190uu walk lanes), and validates
  gaps over dry anchors (platforms + ramp wedges).
- **Visual variety**: three tube scales — ground baffles (CVar default), thinner mesa
  tops (200/180), small roller tubes packed across ramp slopes (90uu, the "smaller
  cylinders composing a ramp" rule) — all batched into one ISM actor per floor.
- **Audit (truthful anchor metric)**:

  ```text
  [T66Proof][BounceCourseAudit] Result=PASS layouts=200 mobFloors=400 failedFloors=0
  failedLayouts=0 worstChainGap=360 worstHoleReach=190 totalPlatforms=47665
  totalRamps=745 totalMesas=3200 totalTierRamps=9550 floorsWithMesas=400
  seedsPerDifficulty=40
  ```

  Every floor passed the tier-access coverage BFS (failures flatten the floor and
  would surface as failedFloors). Worst dry-anchor gap 360 <= 380 limit (jump range
  ~435 at minimum walk speed).
- Phase 2 next: moving lift platforms, ring mesas with pit lifts, third tier,
  trap-guarded ramps, beam slaloms, corridor-length tuning.

## Known v1 limitations (filed in pending_issues_Gameplay.md)

- Enemies ignore lava (hero-only damage contract preserved); grounded enemies wade.
- Ramp visuals are rotated themed cubes, not baffle tubes.
- Per-floor lava clocks are transient across save/load (restart at floor re-entry).
- Minimap does not render platform footprints.

## Addendum 2026-06-10 (fourth pass): moving lift platforms (Phase 2 item 1)

Fall Guys-style non-trap elevators cycling ground <-> mesa top inside a room.
Design contract: `T66_MAP_DESIGN_REFERENCE.md` section 1.4. New actor:
`Source/T66/Gameplay/T66TowerLiftPlatform.{h,cpp}`.

- **Constructive placement**: a mesa that gathered a SURPLUS ramp candidate beyond
  `MesaRampsMin` converts the last candidate into a lift at `LiftChance` (seeded).
  Static ramps never drop below the access minimum; the lift is a bonus route.
  The audit asserts the invariant per mesa (`mesaAccessFails`).
- **No-softlock integration**: `T66CanWalkDirectedTier` counts lift edges as
  up-edges (TierAccessSummary BFS now logs `Lifts=`); the dry safe chain EXCLUDES
  lift cells (a parked slab is submerged at full flood — never a dry anchor);
  scatter/stones/content placement treat the lift travel column as occupied.
- **Moving collision contract**: the lift actor owns a MOVING hidden box proxy
  slab (60uu, BlockAll, camera-ignored) + a separate no-collision mint-green candy
  slab visual (`MI_FallGuys_Lift`, new FallGuysKit MI, in CookGuard — 48 entries).
  The hero rides via standard character based movement. Cycle: 2s dwell bottom ->
  3s cosine-eased rise -> dwell top -> eased descent, seeded phase offsets. Deck
  parks 30uu above the floor (below 45uu step height: boarding is a walk-on) and
  tops out flush with the mesa surface (20uu lateral clearance to the mesa face).
  Both terrain sync tags + floor tag -> the stateful floor pass hides it and kills
  its collision off-floor. Lifts never change floor membership (stateful floor
  rule); a future FLOOR-crossing lift must call SetHeroTowerFloorNumber.
- **Tuning**: `TierLifts`, `LiftChance`, `LiftFootprint`, `LiftTravelSeconds`,
  `LiftDwellSeconds` in `DefaultT66TowerTuning.ini`; sanitizer clamps footprint to
  the cell with mesa clearance, floors travel time so rise speed <= ~400uu/s, and
  keeps dwell >= 0.75s boarding time.
- **Audit (truthful anchor metric)**:

  ```text
  [T66Proof][BounceCourseAudit] Result=PASS layouts=200 mobFloors=400 failedFloors=0
  failedLayouts=0 mesaAccessFails=0 worstChainGap=360 worstHoleReach=190
  totalPlatforms=41910 totalRamps=730 totalMesas=3200 totalTierRamps=8430
  totalTierLifts=1070 floorsWithMesas=400 floorsWithLifts=380 seedsPerDifficulty=40
  ```

  (`Saved/Logs/BounceCourseAudit_Lifts_20260610.log`.) Per-floor proof:
  `[T66Proof][TierAccessSummary] Floor=N Result=PASS Reached=X/X ExitReached=1
  Mesas=M TierRamps=R Lifts=L` — 100% cell coverage with lift edges in the graph.
- **Lift v1 limitations (deliberate)**: descending slab depenetrates a hero
  standing under it sideways (30uu bottom rest height prevents pinning); cycle
  phase restarts on save/load re-entry (same class as lava clocks); enemies do
  not ride or avoid lifts; riding feel needs the user's staged-build playtest.
- **Staging + entry gate**: `StageStandaloneBuild.ps1` BUILD SUCCESSFUL (both
  shortcuts refreshed); `RunLifecycleTransitionSmokeGate.ps1` **PASS**
  (`Saved/LifecycleTransitionSmokeGate/20260610_205627`, 6 travels, stress on,
  exit 0). Every tower regeneration in the staged run logged
  `[CookGuard] All 42 code-referenced assets resolve`,
  `TierAccessSummary ... Result=PASS ... Lifts=3` (floors 2 and 3), and
  `[MAP] Tier terrain spawned floor=N mesas=8 ramps=~20 lifts=3 baffles=0`.
  Two foreign-UAT collisions during staging were waited out per the parallel-
  session rule (no retried-fix churn).
- **Open verification**: the physical ride feel (standing on the slab through
  full cycles) has no automation capture route; it relies on the standard UE
  movable-base contract and needs the user's staged-build playtest via the
  refreshed `T66 Standalone` taskbar shortcut.

## Addendum 2026-06-11 (fifth pass): elevated-deck architecture + exact-collision contract

User direction: the map read as "a bunch of squares" acting as a second altitude —
solid blocks, no walking underneath, and bounces triggering without touching
anything. Design contract: `T66_MAP_DESIGN_REFERENCE.md` section 1.5.

- **Mesas are now elevated decks (bridges), not solid blocks**: a 60uu walkable
  slab proxy at `TierHeight` + round pillar grid (cylinder HISM instances carrying
  the basic shape's own exact simple collision), ground underneath open/playable/
  floodable and connected to the room ring on every side (softlock-impossible by
  construction). With the 2-4 constructive ramps each mesa reads as
  "ramp up -> deck -> ramp down with space underneath".
- **Tier 2 platforms are floating decks**: 60uu slab on one round center pillar,
  340uu of playable clearance underneath at defaults. Chain decks stay
  RECTANGULAR (dry-anchor gap proof measures box edges); scatter Tier 2 may be
  round (cylinder visual = its own exact collision).
- **Tier 1 platforms stay grounded** (200uu hero cannot fit under +200) but a
  seeded `RoundPlatformChance=0.45` share spawn as cylinders (Fall Guys pucks).
  Round stones never take bounce ramps (a wedge against a curved face leaves gaps).
- **Exact-collision contract**: rectangular elements keep byte-equal box proxies;
  round elements enable the engine cylinder's own simple convex collision
  (defensive fallback to square if a basic shape ever ships without collision);
  **the felt "bouncing off air" root cause was the bouncy-wall sweep** — a 29uu
  sphere cast 220uu ahead of a 34uu-radius capsule. Default
  `t66.HeroMovement.SurfaceBounceWallTraceDistance` dropped 220 -> 50.
- **Camera under decks**: deck slabs carry `T66_Tower_DeckVisual` + camera-channel
  block; `T66IsGameplayCameraWallComponent` accepts thin deck slabs (Z extent <=
  80) so the occluder fade keeps the hero visible under bridges; pillars/stones
  ignore the camera channel.
- **Audit (truthful anchor metric)**:

  ```text
  [T66Proof][BounceCourseAudit] Result=PASS layouts=200 mobFloors=400 failedFloors=0
  failedLayouts=0 mesaAccessFails=0 worstChainGap=360 worstHoleReach=190
  totalPlatforms=41845 totalRamps=565 totalMesas=3200 totalTierRamps=8430
  totalTierLifts=1070 floorsWithMesas=400 floorsWithLifts=380 seedsPerDifficulty=40
  ```

  (`Saved/Logs/BounceCourseAudit_DeckArch_20260610.log`.) Bounce ramps 730 -> 565:
  round stones no longer take ramps, everything else identical. BFS semantics
  deliberately unchanged (tier cells = "deck above"; under-deck ground is open
  flat floor connected by construction).
- **Staging + entry gate (fifth pass)**: `StageStandaloneBuild.ps1` BUILD
  SUCCESSFUL (shortcuts refreshed); `RunLifecycleTransitionSmokeGate.ps1` **PASS**
  (`Saved/LifecycleTransitionSmokeGate/20260611_050733`, 6 travels, exit 0, zero
  fatal lines). Staged-game markers per regeneration: `[CookGuard] All 42
  code-referenced assets resolve`, `BounceCourseSummary Result=PASS` and
  `TierAccessSummary Result=PASS ... Lifts=3-4` on floors 2+3, deck-architecture
  terrain spawning (`mesas=8 ramps=~20 lifts=3-4`). Note: three earlier staging
  attempts failed on the PARALLEL session's in-flight shop/casino edits (UAT
  mutex, RunStateSubsystem link race, CasinoVendorTabWidget C2665) — foreign
  lane, waited out per the parallel-session rule; nothing in this pass touched
  those files.
- **Open verification**: visual/feel judgment of the new architecture (deck
  read, pillar density, round-shape mix, under-bridge camera fade, the new 50uu
  wall-bounce contact feel) is the user's staged-build playtest.

## Addendum 2026-06-11 (sixth pass): shape language + pattern variety

User verdict on the deck pass: structure right, but "too uniform, too many squares,
platforms one after the other with no logic", and big decks should get holes.
Design contract: `T66_MAP_DESIGN_REFERENCE.md` section 1.6.

- **FallGuysShapeKit01** (`Model Generation/Runs/Environment/FallGuysShapeKit01/`,
  Blender CLI -> GLB -> `Scripts/ImportFallGuysShapeKitAndExit.py`): SM_FGShape_Hex
  (squashed-regular hexagonal prism) + SM_FGShape_Tri (triangular prism), both with
  EXACT 100^3 AABBs (import JSON verified extent 50/50/50) and 1-hull convex
  collision (a convex prism's hull IS the mesh — exact-collision contract). In
  CookGuard: 44/44 resolve in the staged build.
- **Shape policy**: per-ROOM seeded shape themes (Mixed/Rounds/HexField/SquareTri)
  at `RoundPlatformChance` (now = total non-square share); chains mix square/round/
  hex (axis hops contact every kit shape's AABB face exactly — box-gap proof
  unchanged); triangles scatter-only; seeded 90-degree yaws; shaped stones never
  take bounce ramps; first/last chain platforms pinned square.
- **Chain rhythm**: seeded big/small footprint alternation (pair-sum invariant:
  adjacent footprints sum >= 2*(pitch - PlatformChainMaxGap), anchors and their
  neighbors pinned big) + cross-axis meander up to +-170uu on STRAIGHT path
  segments only (perpendicular offsets cannot widen axis-hop gaps; corners skip).
- **Ring mesas** (Phase 2 item 2 pulled forward): >=5x5 mesas roll
  `RingMesaChance=0.5` for a 1-2 cell center drop hole; hole cells revert to tier
  0 (legal drop into the open under-deck ground — escape built into the deck
  architecture); deck spawns as 4 frame slabs; pillars skip the hole and rim its
  corners; the dry-chain BFS skips mesa-rect tier-0 cells so no chain platform
  can spawn under a deck.
- **Audit (truthful anchor metric)**:

  ```text
  [T66Proof][BounceCourseAudit] Result=PASS layouts=200 mobFloors=400 failedFloors=0
  failedLayouts=0 mesaAccessFails=0 worstChainGap=369 worstHoleReach=190
  totalPlatforms=41925 totalRamps=565 totalMesas=3200 totalRingMesas=1615
  totalTierRamps=8470 totalTierLifts=1075 floorsWithMesas=400 floorsWithLifts=390
  seedsPerDifficulty=40
  ```

  (`Saved/Logs/BounceCourseAudit_ShapeKit_20260611.log`.) worstChainGap=369
  matches the pair-sum invariant's predicted ceiling (~370 < 380 cap); 1615/3200
  mesas ring (~RingMesaChance).
- **Staging + entry gate**: BUILD SUCCESSFUL (one retry — the user's live playtest
  session held the staged folder; waited for game close, no force-delete);
  `RunLifecycleTransitionSmokeGate.ps1` **PASS**
  (`Saved/LifecycleTransitionSmokeGate/20260611_062720`, 6 travels, 0 fatals);
  staged markers: CookGuard 44/44, BounceCourseSummary PASS floors 2+3 with LIVE
  variable gaps (MaxChainGap 363/359 — the rhythm is in the real build),
  TierAccessSummary 100% coverage.
- **Open verification**: shape-mix/meander/ring read is the user's playtest;
  knobs: `RoundPlatformChance` (non-square share), `RingMesaChance`.

## Addendum 2026-06-11 (seventh pass): gate clearance + capsule launch + cross-floor visibility

User playtest findings on the shape build, all root-caused and fixed:

- **Obstacle visible from the floor above (the dark slab)**: the ceiling-hammer's
  mount disc hung at Height(1280)+26 = ~1306uu while floors are 1224 apart — the
  disc topped out ABOVE the next floor's surface. Fixed in two layers:
  `ObstacleCeilingHammer Height` 1280 -> 1060 in `DefaultT66TrapTuning.ini` (mount
  stays under its OWN 1200uu ceiling, with an ini comment recording the invariant),
  and `UT66TrapSubsystem::SetActiveTowerFloor` now HIDES off-floor tower traps
  instead of only disabling them (a tall trap piece can never be seen cross-floor
  again).
- **Gate clearance on both floors**: `TryGetObstacleTrapSpawnLocation` rejects
  candidates within max(2000, footprint+900)uu of the floor's ARRIVAL point
  (the landing zone under the previous floor's gate) with retry loops; scatter
  platforms, buddy stones, and chain hop-on stones skip arrival +-1 cells. The
  hole side keeps its existing 2450-2550uu trap paddings and exit +-1 platform
  exclusions.
- **Obstacles launch, never disable**: `AT66ObstacleTrapBase::TryApplyObstacleReaction`
  now calls `LaunchCharacter` directly (no ragdoll state, no knockdown window, no
  jump-mash recovery). The tuned ini LaunchXY/LaunchZ ragdoll-impulse magnitudes
  map onto capsule velocity via `t66.Trap.ObstacleCapsuleLaunchScaleXY` (0.15)
  and `t66.Trap.ObstacleCapsuleLaunchScaleZ` (1.1), clamped to 600-2600 planar /
  650-1600 vertical. The hero physics ragdoll system is untouched for its other
  sources. Log marker: `[ObstacleTrapReaction] ... Mode=CapsuleLaunch`.
- **"Interactables not spawning" diagnosis**: they spawn (gate evidence: chests
  7/7, crates 7/7, vendors 2/2, totems 2/2) but at ~7 chests per two 10-20-cell
  floors the density is imperceptible — folded into the room-course redesign
  (PROPOSAL, design ref section 1.7, pending user direction on templates /
  rewards-per-room / trap pooling).
- **Verification**: focused compile clean; audit PASS 200 layouts (0 fails,
  mesaAccessFails=0, worstChainGap=369, ringMesas 1615) in
  `Saved/Logs/BounceCourseAudit_GateClear_20260611.log`; staging BUILD SUCCESSFUL
  (4th attempt — the parallel session fired UAT runs back-to-back; a 45s
  quiet-window retry loop won the slot); gate **PASS**
  (`Saved/LifecycleTransitionSmokeGate/20260611_072348`, CookGuard 44/44,
  BounceCourseSummary + TierAccessSummary PASS both floors, 0 fatals).

## Addendum 2026-06-11 (eighth pass): the Fall Guys program — all four analysis solutions

User approved "do all 4 solutions" from `FALLGUYS_MAP_ANALYSIS.md` (defaults:
proposed template set, 1-2 rewards/room, traps in templates). Design record:
`T66_MAP_DESIGN_REFERENCE.md` section 1.7 (now IMPLEMENTED).

- **Tier A — visual language**: FallGuysShapeKit01 v2 (Blender): BevelCube +
  BevelPuck new, Hex/Tri re-authored with 8uu bevels — all 1-hull convex, bevels
  cut only INWARD from the AABB so every gap proof is untouched (import JSON:
  extents verified, convex_elems=1 each). White edge-band trims on rectangular
  decks/mesa frames (batched HISM, 6uu decorative class). Tier 2 decks recolored
  coral (`MI_FallGuys_Deck`); `MI_FallGuys_Trim` white. Hazard signature: every
  punting surface (sweeper arm, both bumpers, hammer mallet, bounce pads) wears
  the diagonal stripes. CookGuard 42 -> 48 entries.
- **Tier B — room courses**: rooms 6-10 cells / 13-16 per floor (was 10-20 / 10),
  arrival room 6-8 (was 3-4 — the cramped landing), exit chooser biased to big
  far rooms, room-count proof moved to a RANGE contract. Five templates
  (CenterKeep / RingRun / SteppingCourt / BridgeCross / OpenArena) assigned
  per-room, seeded; each registers 1-2 REWARD SLOTS at its payoff point; the
  interactable population fills slots FIRST (+ bonus chest on two-slot rooms);
  every slot gets a white beacon column.
- **Tier C — movers**: trap subsystem consumes template anchors before random
  placement (anchor Z preserved — the CenterKeep sweeper spins ON the deck;
  the BridgeCross hammer swings over the bridge). New `AT66BouncePadObstacle`:
  striped walk-on disc, capsule launch (`t66.Trap.BouncePadLaunchZ`=2200 clears
  the mesa rim), cooldown, never ragdolls — RingRun pit escape.
- **Tier D — route choice**: chain detours through rooms grow a FAST lane of
  small round stones cutting the corner; at full flood the low fork drowns while
  the Tier 2 chain stays dry.
- **Verification**: compile clean; audit PASS 200 layouts
  (`BounceCourseAudit_FGProgram3_20260611.log`: 0 failed floors/layouts,
  mesaAccessFails=0, worstChainGap=369, totalPlatforms=68790 — court/bridge/fork
  composition, totalMesas=1705, totalRingMesas=160 — rings are big-room specials
  at the new room sizes, totalTierLifts=655, RoomLayout FAILs=0); staging BUILD
  SUCCESSFUL (quiet-window retry strategy vs the parallel session's bursts);
  gate **PASS** (`Saved/LifecycleTransitionSmokeGate/20260611_101502`):
  CookGuard 48/48, `TowerRoomContentSummary PASS ContentRooms=29` (every room
  rewarded, was ~10), `TowerRoomLayoutSummary PASS`, per-floor markers
  `beacons=11/15`, `pads=` live, 0 fatals.
- **Open verification**: the look/feel of all four tiers is the user's staged
  playtest (bevel read, trim bands, coral decks, stripes rule, template
  legibility, beacon visibility, pad launch feel, fork temptation). Knobs:
  RoundPlatformChance, RingMesaChance, room sizes, t66.Trap.BouncePadLaunchZ,
  t66.Trap.ObstacleCapsuleLaunchScaleXY/Z.
