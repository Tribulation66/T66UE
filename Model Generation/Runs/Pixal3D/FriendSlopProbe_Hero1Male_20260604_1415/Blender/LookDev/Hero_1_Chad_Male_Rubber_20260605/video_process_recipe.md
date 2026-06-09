# Hero 1 Male Video Process Material Comparison

- Scene: `Transcript_Process_Comparison`
- Blend: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`
- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Comparison grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_video_process_comparison.png`
- Preview grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_video_process_comparison_preview_1300.png`

Each version follows one transcript/video process. They are deliberately not collapsed into one combined shader.

| ID | Label | Video | Process Principle | Key Parameters |
|---|---|---|---|---|
| P01_fall_guys_rough_diffuse | Fall Guys Rough Diffuse | Make your own 3D Fall Guy in Blender 2.9 | Bright fun color, high roughness, diffuse soft character read. | roughness 0.72; specular 0.24; coat 0.0; noise scale 140.0; bump distance 0.004 |
| P02_kirby_toon_black_outline | Kirby Toon Black Outline | Achieving Kirby's Stylized Material in Blender [Shader Tutorial] | Flat/toon color behavior with a black Fresnel outline mixed into the material. | black Fresnel outline threshold 0.14; roughness 0.86; no coat |
| P03_cycles_rubber_node_group | Cycles Rubber Node Group | Blender Cycles Rubber Shader and Node Group Tutorial | Diffuse plus translucent softness, then only a small rough glossy component. | translucent factor 0.55; gloss factor 0.08; gloss roughness 0.4 |
| P04_principled_fine_bump_rubber | Principled Rubber Fine Bump | Blender Rubber Material Shader using Principled BSDF Texture Node | Principled BSDF rubber defined by roughness, high-scale noise, and very low bump distance. | roughness 0.5; specular 0.32; coat 0.0; noise scale 500.0; bump distance 0.01 |
| P05_octane_vinyl_roughness_grunge | Octane Vinyl Roughness Grunge | Cinema 4D Tutorial - Creating Plastic Vinyl Toy Textures with Octane Render | Adapted Octane glossy vinyl: roughness channel variation plus subtle grunge/bump imperfections. | roughness 0.34; specular 0.44; coat 0.06; noise scale 75.0; bump distance 0.008 |
| P06_procedural_plastic_noise_bump | Procedural Plastic Noise Bump | Creating Procedural Plastic Materials in Blender (Tutorial) | Procedural plastic controls: noise texture, bump, material roughness, and clearcoat kept controllable. | roughness 0.38; specular 0.36; coat 0.0; noise scale 100.0; bump distance 0.02 |

## Notes

- `P01_fall_guys_rough_diffuse` is the closest to the Fall Guys material transcript: bright, diffuse, high roughness, no coat.
- `P02_kirby_toon_black_outline` is the only version that intentionally follows the Kirby NPR material and includes the requested black outline.
- `P05_octane_vinyl_roughness_grunge` is an adapted Blender mapping of the C4D/Octane vinyl process, not a literal Octane node reproduction.
- `P06_procedural_plastic_noise_bump` follows the procedural plastic/latex control idea and may be useful as a boundary, not necessarily as the final rubber target.
