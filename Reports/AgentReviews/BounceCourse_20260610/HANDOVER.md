# Map Infrastructure Program — Handover (2026-06-10, end of session)

One day of work on the tower map redesign: Fall Guys direction, accessibility-first.
Everything below is implemented, compiled, multi-seed audited, staged to the taskbar
standalone, and lifecycle-gate verified. All work is UNCOMMITTED on `main` (the tree
also carries the user's parallel Codex lanes — MotionRig, Fire Pierce; do not triage).

## Read first

1. `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` — section 1.2 (bounce course + lava),
   section 1.3 (tier accessibility design contract + playtest fixes + look flip).
   This is the design authority; update it after every map change (maintenance rule).
2. `Reports/AgentReviews/BounceCourse_20260610/PROOF.md` — full evidence trail.
3. `Gameplay/World/WORLD_AGENTS.md` + root `AGENTS.md` — process contracts.
4. Claude project memory `project_t66.md` entries 12-17 mirror this state.

## What landed today (all verified)

| Piece | Where | Proof |
|---|---|---|
| Tribulation-entry crash fix (GC-dangling static mesh caches -> AddToRoot) | `T66TowerMapTerrain.cpp` `T66GetFloorBaffleTubeMesh`, `T66VisualUtil.cpp` basic shapes | gate PASS, symbolicated stack in PROOF.md |
| Bounce course: Tier1/2 jump platforms + dry safe chain + rising lava (DOT, never insta-kill) | `T66TowerMapTerrain.{h,cpp}`, `T66MiasmaManager.{h,cpp}` | `[T66Proof][BounceCourseSummary]` per floor |
| Inflatable trap kit (Blender lathe meshes + Codex imagegen patterns, FriendSlop MIs) | `Traps/T66ObstacleTrap.cpp`, `/Game/World/Traps/Inflatable/`, run folder `Model Generation/Runs/Environment/InflatableTraps01/` | renders in run folder |
| Tier terrain: per-room +500 mesas, 2-4 constructive ramps each, no-softlock BFS proof | `T66BuildFloorTierTerrain`, `T66ValidateTierAccess` | `[T66Proof][TierAccessSummary]` 100% coverage |
| Fall Guys slab look (tubes parked) | `T66TowerThemeVisuals.cpp` `t66.Tower.FallGuysLook`, `/Game/World/Terrain/FallGuysKit/MI_FallGuys_*`, `t66.Tower.FloorBaffles` default 0 | staged build |
| Spring-arm camera + ceiling camera-only blockers (whole-map reveal fix) | `T66HeroBase.cpp` boom `bDoCollisionTest=true`, `T66SpawnGeneratedDungeonFloorUndersideTiles` camera blockers | gate PASS |
| Short halls: gap-banded room clustering (old scatter MAXIMIZED spread) | `T66TryFindScatteredDungeonRoom`, `RoomMaxGapCells` ini (default 6) | audit: chain platforms −12% |
| STATEFUL floor membership (true black-map root cause, user-diagnosed) | `StatefulHeroTowerFloorNumber` on GameMode; transitions: layout build / descent hole / fall rescue | gate PASS 20260610_195457 |

## Architecture rules established today (do not violate)

1. **Constructive connectivity**: accessibility is built at generation time (mesa
   commits only after its ramps validate; rooms cluster within `RoomMaxGapCells`),
   then verified by BFS — never generate-then-patch.
2. **Stateful floors**: any floor-scoped system consumes
   `AT66GameMode::GetCurrentTowerFloorIndex()` (stateful). NEVER classify hero Z
   directly — Z-band flipping was the black-map bug.
3. **Collision = hidden box proxies** (WORLD_AGENTS rule); visuals separate. Course
   surfaces carry no `T66_NoSurfaceBounce` tag (bouncy by default).
4. **Function-local static asset caches must `AddToRoot()`** or they dangle after
   world-teardown GC (crash class).
5. **Every C++ LoadObject literal path goes in `T66CodeReferencedAssets.cpp`**
   (CookGuard) — currently 47 entries, all resolving.
6. **Tuning lives in `Config/DefaultT66TowerTuning.ini`** via `T66TowerTuningConfig`
   (sanitizer clamps enforce jump-math invariants: tier steps <= 260, chain gap <= 420,
   lava cap < Tier2).

## Verification recipes

- Focused compile: `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -project="C:\UE\T66\T66.uproject" -waitmutex`
- Multi-seed layout audit (200 layouts, asserts every guarantee):
  `UnrealEditor-Cmd.exe T66.uproject -run=T66BounceCourseAudit -seeds=40 -unattended -nop4 -nosplash -nullrhi`
- Stage: `Scripts/StageStandaloneBuild.ps1` (build+cook+stage+shortcut)
- Entry/regression gate: `Scripts/RunLifecycleTransitionSmokeGate.ps1` (6 world travels)
- Grep markers: `BounceCourseSummary`, `TierAccessSummary`, `TowerRoomLayoutSummary`,
  `CookGuard`, `Hero tower floor ->`

## Operational gotchas (cost real time today)

- **PowerShell `*>` redirects write UTF-16 logs** — bash `grep`/monitors never match
  them; use `Select-String` for log checks.
- **Parallel sessions contend for the single UAT slot** ("conflicting instance" /
  transient SDKNotFound / LNK1104 DLL locks) — check for foreign
  dotnet/AutomationTool/UnrealEditor-Cmd processes before diagnosing, then retry.
- Stale "Live Coding active" UBT block with no editor running — transient, retry.
- Editor screenshots: `-ExecutePythonScript` never ticks (callbacks dead) and
  `-ExecCmds="py ..."` gives black pre-shader-warmup frames — use Blender renders
  for mesh QA; in-game judgment via the staged build.
- Adding .cpp files re-bins unity chunks and can surface latent cross-file name
  collisions (C4459) — fix the collision, it is not your change.
- The staged game writes logs/screenshots to `Saved/StagedBuilds/Windows/T66/Saved/`.

## Phase 2 queue (user priorities, in order)

1. **Moving lift platforms** (task: non-trap elevators cycling between tiers;
   player rides up; bouncy look). Design note: a lift arriving at a tier/floor is an
   explicit transition — integrate with `SetHeroTowerFloorNumber` if lifts ever cross
   FLOORS (tier-to-tier within a floor needs no floor-state change). Keep collision
   as a moving hidden box proxy + slab visual; reuse `FBounceRamp`-style tuning.
2. Ring mesas with center drop pits (pit needs a lift or ramp out — no-softlock rule).
3. Third terrain tier (the lava-to-second-tier vision: lava cap between tier 1 and 2).
4. Trap-guarded ramp tops (sweeper/hammer at ramp crests, per Tail Tag).
5. Beam slalom posts (vertical obstacles, Tail Tag perimeter pattern).
6. Corridor-length fine tuning (`RoomMaxGapCells`) after playtest feedback.

Reference projects: `Saved/Reference/MegabonkTerrainGenerator/` (constructive
connectivity: grow connected, slope cells direction-locked, drops free) and Fall Guys
Tail Tag (two tiers, central platform, multi-side ramps, some trap-guarded).

## Known open issues (filed in `Source/T66/Gameplay/pending_issues_Gameplay.md`)

- Enemies ignore lava (wade through flooded floors) — needs an enemy hazard rule.
- Ramp visuals are plain slabs (roller tubes only render in baffle mode, now off).
- Per-floor lava clocks reset on save/load re-entry.
- Minimap does not render platforms/mesas/tiers.
- Trap quality verdict: procedural Blender fine for geometric inflatables; Pixal3D
  recommended for organic shapes (rolling boulder) — user gates access on request.
