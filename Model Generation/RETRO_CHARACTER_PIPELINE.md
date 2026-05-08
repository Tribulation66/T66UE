# T66 Quad Retro Character Pipeline

This is the current process for the one-image, one-model hero direction.

## Goal

Generate a clean full-body Trellis character, reduce it with Quad Remesher, bake
the Trellis diffuse identity back onto the low-poly mesh, then apply the
pixelated/dithered texture treatment.

## Source Image Rules

Read [TRELLIS_SOURCE_IMAGE_RULES.md](C:/UE/T66/Model%20Generation/TRELLIS_SOURCE_IMAGE_RULES.md)
before generating any new hero source image.

The current source-image rule is:

- one full-body source image per character
- A-pose, straight-on, orthographic-feeling camera
- flat saturated chroma background such as `#FF00FF`, not white or black
- clean painted/cel-shaded concept art, not pixel art and not photoreal
- identity carried by silhouette, costume, hair, and equipment, not detailed face
- generate `4-6` variants and approve the cleanest reconstruction source before
  sending anything to TRELLIS

Do not pre-pixelate the source image. The source image should be clean enough
for TRELLIS to reconstruct; the pixelated/dithered look is applied after
remesh, unwrap, bake, and palette reduction.

## Main Script

- [t66_quad_retro_character_pipeline.py](C:/UE/T66/Model%20Generation/Scripts/t66_quad_retro_character_pipeline.py)
- [RunQuadRetroCharacterPipeline.ps1](C:/UE/T66/Model%20Generation/Scripts/RunQuadRetroCharacterPipeline.ps1)

For actual command execution, foreground-Blender rules, Medium baseline values,
and failure recovery, use
[QUAD_RETRO_DO_THIS_RUNBOOK.md](C:/UE/T66/Model%20Generation/QUAD_RETRO_DO_THIS_RUNBOOK.md).

## Adjustable Values

The report JSON writes the exact values used under `adjustable_values`.

- `target_quads`: Quad Remesher target quad count.
- `adaptive_size`: how strongly small quads concentrate around curved/detail areas.
- `adapt_quad_count`: whether Quad Remesher may exceed the target to keep detail.
- `use_materials`: whether existing material borders guide edge loops.
- `use_normals`: whether split normals guide edge loops.
- `autodetect_hard_edges`: whether geometry angles create hard-edge guides.
- `bake_size`: high-to-low diffuse bake size before pixelation.
- `texture_size`: final pixelated texture resolution.
- `palette_mode`: `none`, `kmeans`, or `per-channel`.
  - Use `none` for identity-preserving passes. It preserves baked outfit/skin
    colors and only applies nearest-neighbor texture resampling.
  - Use `kmeans` or `per-channel` only after a character still reads clearly in
    no-palette passes.
- `palette_size`: number of extracted colors when `palette_mode=kmeans`.
- `palette_steps`: per-channel levels when `palette_mode=per-channel`.
- `dither_type`: `none`, `bayer4`, or `bayer8`. Dither only has visible effect
  when palette reduction is enabled.
- `dither_strength`: Bayer threshold strength. Keep this at `0` when
  `palette_mode=none`.

## Preset Ladder Rules

The first preset test was too destructive because Low still used color
quantization and a `256` texture. That removed boxing gloves and outfit color,
which are now the identity carriers. Future agents should not use that ladder
as an art baseline.

For character approval passes, start with color-preserving presets:

- Low: `target_quads=30000`, `texture_size=1024`, `palette_mode=none`,
  `dither_type=none`, `dither_strength=0`.
- Medium: `target_quads=12000`, `texture_size=512`, `palette_mode=none`,
  `dither_type=none`, `dither_strength=0`.
- High: `target_quads=3000`, `texture_size=256`, `palette_mode=none`,
  `dither_type=none`, `dither_strength=0`.

Only introduce palette reduction after the model is readable at those geometry
and texture settings. If the outfit colors stop reading, the pass has failed
even if the silhouette survived.

Quad Remesher may occasionally undershoot or overshoot a target count on a
specific mesh. Always compare `retopo_quads` in the report before sending
screenshots. The successful Boxer Chad Medium pass used `target_quads=12000`
and produced about `13.7k` quads / `27.7k` tris.

## Smoke Command

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Model Generation\Scripts\RunQuadRetroCharacterPipeline.ps1" `
  -InputModel "C:\UE\T66\Model Generation\Runs\Arthur\Raw\Arthur_HeroReference_Full_White_S1337_D80000_Trellis2.glb" `
  -OutputDir "C:\UE\T66\Model Generation\Runs\Heroes\QuadRetroPipelineSmoke01" `
  -Label "ArthurFullBody" `
  -TargetQuads 12000 `
  -AdaptiveSize 50 `
  -TextureSize 512 `
  -PaletteMode "none" `
  -DitherType "none" `
  -DitherStrength 0
```

## Notes

- Quad Remesher trial/license activation must be completed once through
  `xrLicenseManager.exe`.
- Run this pipeline through a normal Blender host window. In testing the
  Quad Remesher engine returned a host-app communication error when launched as
  a bare external process, while the Blender add-on host path succeeded.
- Do not use background Blender for Quad Remesher. Do not launch the Blender
  process hidden, detached, or redirected through `Start-Process`. Let the
  foreground wrapper command block until the visible Blender process exits.
- Use PowerShell colon booleans in generated commands:
  `-RenderQA:$true -Background:$false`. Loose `$true` / `$false` tokens can
  become strings when a command is generated and launched through another
  shell.
- QA renders should be generated from the exported GLB after the pipeline pass.
  The pipeline report is written before any optional QA rendering so a slow
  render cannot invalidate a successful mesh/texture output.
- With the Quad Remesher FBX axis rotation cleared before export, the front QA
  render uses `yaw=0` and a small pitch such as `2`.
- Quad Remesher's returned FBX can import into Blender with a `-90` degree Z
  object rotation. Clear that rotation before UV unwrap and selected-to-active
  baking. If this is missed, the low-poly mesh is sideways relative to the
  source mesh and costume areas bake onto the wrong polygons, usually visible as
  lost glove/costume colors and diagonal projection streaks in the baked PNG.
- The script writes new outputs only. It does not overwrite source GLBs.
- The old Blender voxel rebake and direct decimate prototype scripts were
  removed because they did not preserve character identity well enough.
- The old split head/body Type A workflow is legacy for this direction. New
  Quad Retro heroes should start from one approved full-body TRELLIS source
  image unless a specific future exception is documented.
