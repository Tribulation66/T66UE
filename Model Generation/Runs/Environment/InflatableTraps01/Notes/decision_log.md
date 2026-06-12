# InflatableTraps01 — decision log

2026-06-10. User-directed experiment: make the four tower obstacle traps read as
inflatable balloons (shape + themed pattern), authored in Blender/Unreal first;
Pixal3D access is gated on this attempt's quality.

## Method

- Shapes: procedural Blender 5.1 (`Scripts/build_inflatable_traps.py` in this run folder,
  CLI `--background --factory-startup`). Lathe builder revolves balloon profiles with
  pinch seams + scallop lobes and assigns cylindrical UVs (u = angle, v = arc length) so
  band/stripe textures wrap as rings/barber-poles. The wall pad is a subdivided cube with
  analytic mattress inflation + quilt seam lines + perimeter weld pinch.
- Canonical envelope: every mesh fits the engine basic-shape 100uu native size so the
  existing dimension-driven component scale math in `T66ObstacleTrap.cpp` is unchanged.
- Textures: Codex account-backed imagegen worker
  (`Saved/Codex/World/InflatableTraps01/w1_patterns/`), five 1024 seamless flat-albedo
  patterns in the hellfire palette (lava red/cream stripes, gold/charcoal bands, ember
  chevrons, gold stars on charcoal-purple, ember dots on cream).
- Unreal: GLBs (exported `export_materials='NONE'`) imported via
  `Scripts/ImportInflatableTrapsAndExit.py` + `Scripts/FinishInflatableTrapsAndExit.py`
  (Interchange nests GLB imports under `<Name>/StaticMeshes/<Name>`; the finisher
  flattens to `/Game/World/Traps/Inflatable/SM_*` and binds slot-default MIs).
  Pattern MIs are instances of `M_FriendSlop_FallGuys` (BaseColorTexture set).
- Runtime: `T66ObstacleTrap.cpp` prefers the kit meshes + pattern MIs with the legacy
  basic-shape + flat-color path as fallback; caches rooted (AddToRoot) against the
  world-teardown GC dangling-pointer crash class. CookGuard registry lists every path.

## Mesh inventory (LOD0 triangles after import)

| Mesh | Tris | Pattern | Used by |
|---|---:|---|---|
| SM_Inflatable_SweeperArm | 1286 | StripesDiag | sweeper arm |
| SM_Inflatable_Hub | 506 | BandsHoriz | sweeper hub + hammer ceiling mount (Dots) |
| SM_Inflatable_Bumper | 668 | BandsHoriz | floor bumper |
| SM_Inflatable_Pad | 818 | Chevrons | wall bumper plate |
| SM_Inflatable_Mallet | 710 | Stars | hammer head |
| SM_Inflatable_Tube | 1238 | BandsHoriz | hammer cable |
| SM_Inflatable_SpikeBall | 1568 | Dots | quality probe / future spike-ball trap |

## Open judgments

- Bumper native silhouette has a slight rubber-duck read at 1:1 proportions; the in-game
  6.4 x 6.4 x 2.1 squash flattens it into a proper donut bumper. Revisit only if the
  squashed read fails in capture.
- Quality gate vs Pixal3D: procedural lathe shapes carry seams/lobes well; organic or
  asymmetric shapes (boulder, mascot-like obstacles) would benefit from Pixal3D.
