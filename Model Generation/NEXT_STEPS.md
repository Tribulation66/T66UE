# Next Steps

This is the current recommended work order as of `2026-05-06`.

## Priority Order

1. Generate male source-image candidates for the remaining `11` Chad heroes.
   Use [NEXT_CHAT_HERO_MALE_IMAGEGEN_PROMPT.md](C:/UE/T66/Model%20Generation/NEXT_CHAT_HERO_MALE_IMAGEGEN_PROMPT.md).
2. Review the `4` variants per hero and approve one source image per hero.
   Judge by exaggerated Chad silhouette, separated limbs, costume readability,
   no shadows/floors/background geometry, and TRELLIS safety.
3. Only after male source images are approved, run TRELLIS and the Quad Retro
   Blender pipeline on approved candidates.
4. Use the second-pass color-preserving pipeline settings first:
   - Low: `target_quads=30000`, `texture_size=1024`, `palette_mode=none`,
     `dither_type=none`, `dither_strength=0`
   - Medium: `target_quads=10000`, `texture_size=512`, `palette_mode=none`,
     `dither_type=none`, `dither_strength=0`
   - High: `target_quads=3000`, `texture_size=256`, `palette_mode=none`,
     `dither_type=none`, `dither_strength=0`
5. Do not start Stacy variants until the male forms are approved.

## Active Hero Direction

The old split head/body Type A direction is legacy for this workstream. The
current Quad Retro direction is:

- one approved full-body source image per character
- one TRELLIS model per character
- Quad Remesher for low-poly topology
- selected-to-active diffuse bake back onto the low-poly mesh
- color-preserving pixelation first
- palette reduction and dithering only after the outfit/color identity survives

Read these before touching this workstream:

- [TRELLIS_SOURCE_IMAGE_RULES.md](C:/UE/T66/Model%20Generation/TRELLIS_SOURCE_IMAGE_RULES.md)
- [HERO_CHAD_STACY_PROMPT_GUIDE.md](C:/UE/T66/Model%20Generation/HERO_CHAD_STACY_PROMPT_GUIDE.md)
- [RETRO_CHARACTER_PIPELINE.md](C:/UE/T66/Model%20Generation/RETRO_CHARACTER_PIPELINE.md)
- [QUAD_RETRO_DO_THIS_RUNBOOK.md](C:/UE/T66/Model%20Generation/QUAD_RETRO_DO_THIS_RUNBOOK.md)

## Current Hero Names

| Hero ID | New name |
| --- | --- |
| Hero_1 | Royal Chad |
| Hero_2 | Chinese Chad |
| Hero_3 | Boxer Chad |
| Hero_4 | Founding Chad |
| Hero_5 | Robo Chad |
| Hero_6 | Billy Chad |
| Hero_7 | Rabbit Chad |
| Hero_8 | CS Chad |
| Hero_9 | Gamba Chad |
| Hero_10 | Monotone Chad |
| Hero_11 | Bald Chad |
| Hero_12 | Roach Chad |

## Reference Artifacts

Use Boxer Chad as the male silhouette calibration reference:

- [BoxerChad_ExaggeratedV_04.png](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroSourceExploration01/Inputs/imagegen_candidates/BoxerChad_ExaggeratedV_04.png)
- [BoxerChad_Medium_front.png](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake/Renders/Front/BoxerChad_Medium_front.png)

Keep the successful second-pass pipeline output:

- [QuadRetroPipelinePresetTest02_FixedBake](C:/UE/T66/Model%20Generation/Runs/Heroes/QuadRetroPipelinePresetTest02_FixedBake)

## Defer Until Later

- Stacy source images and body-family tuning
- TRELLIS runs for unapproved source images
- palette reduction and dithering beyond the color-preserving baseline
- Unreal import, DataTable reload, rigging, and staged executable verification
- CoherentThemeKit01 environment review
- legacy Type A masculine/beach-goer rerolls
