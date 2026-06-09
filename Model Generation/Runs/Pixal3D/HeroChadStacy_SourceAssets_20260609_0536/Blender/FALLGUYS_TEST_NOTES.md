# Fall-Guys LIT recipe look-dev test (2026-06-09)

Tests the Fall-Guys-style lit material on the 3 characters inside an enlarged bouncy dungeon-tube room.
New scene; originals preserved.

## Deliverables
- Wide shot: `FallGuys_Wide.png` (1600×900) — room + 3 characters (judges the combination).
- Close-up: `FallGuys_Closeup.png` (1200×1500) — Hero2Chad chest/face (judges material detail).
- New scene: `HeroChadStacy_with_current_hero1_male_FallGuysTest.blend` (save-as; original `.blend`,
  `_Baked.blend`, and `_RoomPreview.blend` all left untouched on disk).
- Rendered in **Cycles (GPU/OPTIX)** for accurate subsurface + sheen, 96 samples (wide) / 128 (close-up), denoised.

## Characters (the test)
- Used the **ORIGINAL meshes with RAW albedo** — duplicated `geometry_0.005/006/007` → `FG_CurrentHero1Male`,
  `FG_Hero1Stacy`, `FG_Hero2Chad` (single-user mesh; raw albedo `Image_0.005/006/007`, 4096², shared read-only).
  The baked-lighting emission copies were **retired** (deleted in this file only). Originals untouched and hidden.
- **Normal smoothing:** shade-smooth (all faces) + **Weighted-Normal modifier** (`mode=FACE_AREA`, `keep_sharp=False`,
  `weight=50`). Shading-only — silhouette and UVs unchanged. This knocks down the Pixal3D normal noise so the
  sheen/spec don't blotch (visible in the clean close-up).

## Fall-Guys material (node group `FG_FallGuys`, exposed params)
Principled BSDF wrapped in a node group; the recipe scalars are **labeled Value nodes inside the group**
(edit once → all 3 characters update). Per-character Base Color = that character's raw albedo image.
Mapped to current Blender 4.x Principled input names:

| Recipe knob (Value node) | Principled input | Value | Intent |
|---|---|---|---|
| `FG_Subsurface_Weight` | Subsurface Weight | **0.20** | soft light wrap; shaded side stays soft (no hard dark) |
| `FG_Subsurface_Scale` | Subsurface Scale | **0.15** | small radius (subtle, not waxy) |
| (fixed) | Subsurface Radius | (0.12, 0.10, 0.09) | small neutral-ish, faint warmth |
| `FG_Sheen_Weight` | Sheen Weight | **0.35** | moderate fabric-fuzz fresnel rim |
| `FG_Sheen_Roughness` | Sheen Roughness | **0.30** | softness of the fuzz |
| (fixed) | Sheen Tint | white | neutral fuzz |
| `FG_Specular_IOR_Level` | Specular IOR Level | **0.30** | low-moderate (broad soft sheen, not a hotspot) |
| `FG_Roughness` | Roughness | **0.60** | mid-high → broad soft spec = rubber |
| (fixed) | Metallic | **0.0** | non-metal |

To tune: open the `FG_FallGuys` group in the Shader Editor and edit those Value nodes (global to all 3),
or override the group node per-material for one character.

## Lighting — one bright soft rig ("one lighting for everything")
- **Key:** one Area light `FG_Key` — `size 9 m` (soft shadows), `energy 9000 W`, color (1.0, 0.98, 0.95),
  at (5, -7, 14) aimed at room center. (Knob: energy.)
- **Fill:** strong neutral **world** ambient — Background color (0.5, 0.5, 0.5), **strength 1.6**. Bright all
  over, crevices filled, no harsh shadows. (Knob: strength — lower it for more form contrast, raise for flatter/brighter.)
- This single rig lights BOTH characters and the environment (environment material is Principled/lit).

## Room (enlarged — a knob)
- Floor **18 × 18 m** (18 parallel ribbed baffle tubes), walls **3 m** (4 sides × 3 stacked tubes), open top,
  1 m tube diameter. Floor 31,680 polys / walls 21,120 polys. Dungeon textures reused
  (`Dungeon_Floor_StoneSlabs`, `Dungeon_Wall_StoneBlocks`, Principled/lit). Characters spaced at x = -5 / 0 / +5,
  feet on floor (Z≈1.0). (Knob: `SIDE` and wall stack count.)

## Cameras
- `RoomCam_Wide`: (0, -22, 15) → (0, 0, 2.5), lens 35 mm, ~30° down — room + all 3 characters.
- `RoomCam_Close`: (-5, -5, 4.3) → (-5, 0, 4.1), lens 80 mm — Hero2Chad chest/face for material detail.

## Read of the result
- **Close-up:** the recipe reads exactly as intended — soft subsurface wrap (shaded sides stay soft), a gentle
  sheen rim on shoulders/arms, broad soft specular across the muscle forms (no sharp hotspot), clean smoothed
  surface with no Pixal3D blotching. Strong "soft toy / rubber" Fall-Guys feel.
- **Wide:** characters + bouncy environment combine well under the single rig; bright, soft, crevices filled,
  form still reads.

## Issues / notes
- The strong world fill (1.6) makes the wide-shot background read near-white at the top and slightly flattens
  form. It satisfies the brief ("bright all over, no harsh shadows"); lower world strength (~1.0–1.2) if you want
  more shaping. This is the main tuning knob.
- `FG_*` character objects are in QUATERNION rotation mode (inherited from the originals' transform), so they
  were rotated to face the camera via matrix rotation rather than euler.
- Subsurface/sheen are rendered in Cycles for fidelity; EEVEE-Next approximates them but Cycles is the reference
  for this judgment.
- Originals (`geometry_0.005/006/007`, their materials, raw albedo images) and all prior `.blend` files are untouched.

## Throwaway helper scripts (your call to keep/delete)
- `Scripts/ExportDungeonTexturesAndExit.py`, `Scripts/DumpHero1MaterialAndExit.py` (from prior tasks).
