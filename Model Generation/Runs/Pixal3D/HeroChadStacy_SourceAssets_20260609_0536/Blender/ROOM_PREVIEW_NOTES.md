# Bouncy-house dungeon room preview (2026-06-09)

Characters + bouncy ribbed-tube dungeon environment together, as they'd read in-game (unlit characters,
lit environment). New file; originals untouched.

## Deliverables
- Render: `RoomPreview_render.png` (1600×900, this folder)
- New scene: `HeroChadStacy_with_current_hero1_male_RoomPreview.blend` (save-as; original `.blend` and the
  `_Baked.blend` left untouched on disk)
- Live viewport left clean: camera view, rendered shading, overlays + gizmos off.

## Room dimensions
- Footprint **9 × 9 m**, walls **3 m** above the floor surface, **open top**.
- **Tubes:** chunky ribbed "baffle" tubes — cylinders, 1.0 m diameter (R=0.5), radius sine-modulated to
  give baffle bulges (9 baffles/tube; amplitude 0.06 floor / 0.09 walls). Whole-number fit.
  - Floor = **9 parallel tubes** along Y (x = -4…+4, each 9 m long), tops at Z≈1.0 (the walking surface).
  - Walls = **4 sides × 3 stacked tubes** (12 tubes) on the perimeter (±4.5), centers Z=1.5/2.5/3.5.
- Objects: `BouncyFloor` (8,064 polys), `BouncyWalls` (10,752 polys).

## Unreal dungeon textures used (traced + exported)
The dungeon floor/wall are NOT a simple code texture path — they come from the **CoherentThemeKit01
generated kit** under `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/`
(`T66TowerThemeVisuals.cpp` only sets an explicit material for the **roof** = `MI_TowerDungeonRoof`;
floor/wall visuals are carried by the kit meshes' own `*_Mat_00` materials + `*_BaseColor_00` textures).
Per-tile variants exist: floor = Bones/Cracked/Drain/**StoneSlabs**; wall = BonesNiche/Chains/**StoneBlocks**/TorchSconce.
I used the plain stone variants as the canonical dungeon look:

| Surface | UE texture asset (`.../CoherentThemeKit01/`) | Exported PNG (`./DungeonTex/`) | Size |
|---|---|---|---|
| **Floor** | `DungeonFloor_StoneSlabs_A_UnrealReady_BaseColor_00` | `DungeonFloor_StoneSlabs_A_UnrealReady_BaseColor_00.png` | 2048² |
| **Walls** | `DungeonWall_StoneBlocks_A_UnrealReady_BaseColor_00` | `DungeonWall_StoneBlocks_A_UnrealReady_BaseColor_00.png` | 2048² |

All 8 dungeon floor/wall base-color textures were exported to `./DungeonTex/` (so a different variant can be
swapped in). No source PNG/TGA existed on disk — exported from `.uasset` via UE Python
(`Scripts/ExportDungeonTexturesAndExit.py`). Applied as **Principled BSDF** (Base Color = texture, Roughness 0.9)
so the environment is lit and the tube roundness reads. Tiling UVs (~1.6 m tile) generated on the tubes.

## Lighting
- **One** light: `RoomSun` (Sun), energy **4.0**, color white, rotation **(55°, 0°, 40°)**, soft angle 2°.
- Characters are the **baked-unlit emission** objects (CurrentHero1Male / Hero1Stacy / Hero2Chad) — intentionally
  unaffected by the light (this previews unlit characters against a lit environment).
- World background = dark gray **(0.05, 0.05, 0.055)** (not white).

## Camera
- `RoomCam`: location (0, -11, 9.5), aimed at (0, 0, 2), lens **34 mm**, **~34° downward** (elevated gameplay angle),
  framing the room + the 3 characters.

## Characters
- The 3 baked-unlit characters placed standing **on the floor** (feet at Z≈1.0), spaced at x = -2.5 / 0 / +2.5,
  facing the camera. IdolProjectile/WeaponProjectile baked copies were render-hidden (room shows characters only).

## Issues / notes
- **Character facing:** the baked-unlit objects are in QUATERNION rotation mode (from the earlier `matrix_world`
  placement), so an initial `rotation_euler` flip was silently ignored; fixed with a world-space matrix rotation
  about each character's center. They now face the camera.
- **Stray black shape + old stack:** all 41 prior-scene objects (old lit/baked comparison stack, the
  `_UnrealUnlit` variants, ReviewSun/ReviewFill, ReviewCamera, and the stray dark shape from the prior view)
  were hidden from **both** viewport and render. The stray shape is gone from the preview; it was hidden as part
  of the blanket prior-scene cleanup rather than singled out by name.
- **Environment is dark** — authentic to the dungeon stone base-color; the Sun at 4.0 lifts the floor enough to read.
- Tube ends are capped; perpendicular wall tubes overlap slightly at the 4 corners (acceptable for a preview).

## Throwaway scripts (your call to keep or delete)
- `Scripts/ExportDungeonTexturesAndExit.py` — exports dungeon kit base-color textures to PNG (reusable helper).
- `Scripts/DumpHero1MaterialAndExit.py` — from the prior task (still present).
