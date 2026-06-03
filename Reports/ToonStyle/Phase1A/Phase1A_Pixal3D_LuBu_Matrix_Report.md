# Phase 1A Pixal3D Lu Bu Matrix Report

## Endpoint

Pixal3D was running on the RunPod pod reachable via:

- SSH: `ssh root@69.30.85.138 -p 22082 -i ~/.ssh/id_ed25519`
- In-pod server: `http://127.0.0.1:18001`
- Endpoint used: `POST /generate`

Health check:

- `/health` returned `status: ok`
- GPU: NVIDIA A40
- model path: `TencentARC/Pixal3D`
- pipeline loaded: true
- attention backend: `flash_attn_3`
- sparse convolution backend: `flex_gemm`

Source image:

- `C:\UE\T66\SourceAssets\ToonStyle\ImageGen\Phase1A\LuBu\lubu_front_v01.png`

## Pixal3D Submissions

All six submissions completed with HTTP 200. Each curl request was executed from inside the RunPod pod.

Common headers:

- `X-Seed: 1337`
- `X-Decimation: 200000`
- `X-Remesh: 1`
- `X-Export-Fallback: 1`
- `X-Fallback-Decimation: 30000`

Default sampling used server defaults:

- `X-SS-Steps`: default 12
- `X-Shape-Steps`: default 12
- `X-Tex-Steps`: default 12
- `X-SS-Guidance`: default 7.5
- `X-Shape-Guidance`: default 7.5
- `X-Tex-Guidance`: default 1.0

High sampling used:

- `X-SS-Steps: 24`
- `X-Shape-Steps: 24`
- `X-Tex-Steps: 24`
- `X-SS-Guidance: 8.0`
- `X-Shape-Guidance: 8.0`
- `X-Tex-Guidance: 1.25`

| Variant | Resolution | Texture | Sampling | Duration | Size | Raw GLB |
| --- | ---: | ---: | --- | ---: | ---: | --- |
| 1 | 1024 | 2048 | default | 64s | 9309176 | `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Raw\lubu_r1024_t2048_default.glb` |
| 2 | 1536 | 2048 | default | 166s | 9746384 | `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Raw\lubu_r1536_t2048_default.glb` |
| 3 | 1024 | 4096 | default | 100s | 11742284 | `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Raw\lubu_r1024_t4096_default.glb` |
| 4 | 1536 | 4096 | default | 149s | 13488068 | `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Raw\lubu_r1536_t4096_default.glb` |
| 5 | 1024 | 2048 | high | 86s | 9806676 | `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Raw\lubu_r1024_t2048_high.glb` |
| 6 | 1536 | 4096 | high | 206s | 13649312 | `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Raw\lubu_r1536_t4096_high.glb` |

Generation summary log:

- `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Logs\local_generation_summary.txt`

## Normalization

Created:

- `C:\UE\T66\ToonStyle\BlenderScripts\normalize_pixal3d_glb.py`

The script imports GLB, joins meshes, centers XY, places the lowest Z vertex at floor height, scales height to 180 UU, applies transforms, and exports GLB. It does not retopo, bake, downsample, dither, recalculate normals, strip vertex colors, or alter materials beyond normal glTF export.

Normalized GLBs:

| File | Height | Feet at Z | Meshes | Materials |
| --- | ---: | ---: | ---: | ---: |
| `lubu_r1024_t2048_default_normalized.glb` | 180.0 | 0.0 | 1 | 1 |
| `lubu_r1536_t2048_default_normalized.glb` | 180.0 | 0.0 | 1 | 1 |
| `lubu_r1024_t4096_default_normalized.glb` | 180.0 | 0.0 | 1 | 1 |
| `lubu_r1536_t4096_default_normalized.glb` | 179.99998 | 0.0 | 1 | 1 |
| `lubu_r1024_t2048_high_normalized.glb` | 180.0 | 0.0 | 1 | 1 |
| `lubu_r1536_t4096_high_normalized.glb` | 180.0 | 0.0 | 1 | 1 |

Normalized output directory:

- `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\Normalized\`

## UE Import

Created:

- `C:\UE\T66\ToonStyle\Source\ImportLuBuMatrixStaticMeshesAndExit.py`

Intent was direct GLB import using the existing `Scripts/ImportStaticMeshes.py` path. In full-editor execution, normalized GLB import returned zero object paths for all six files. I did not force the issue inside the shared import helper during this pass.

Fallback used:

- Converted normalized GLBs to FBX through Blender, preserving mesh/material/texture data.
- Imported FBX with normals and tangents preserved.
- Saved static meshes to `/Game/ToonStyle/TestAssets/LuBu_Matrix/`.

FBX fallback files are staged under:

- `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Phase1A\LuBu_Matrix\FBX\`

UE assets:

| Label | Asset |
| --- | --- |
| R1024 T2048 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T2048_Default` |
| R1536 T2048 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T2048_Default` |
| R1024 T4096 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T4096_Default` |
| R1536 T4096 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T4096_Default` |
| R1024 T2048 High | `/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T2048_High` |
| R1536 T4096 High | `/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T4096_High` |

Import log notes:

- Import completed with `DONE success=6 failed=0`.
- The editor then hit the existing shutdown issue: `Object is not packaged: ModeManagerInteractiveToolsContext None`.
- Texture warning repeated during import: `Texture2D /Game/ToonStyle/TestAssets/LuBu_Matrix/Image_0 contains no miplevels`.
- Some static mesh warnings reported near-zero tangents. This should be considered during visual review but was not corrected in this pass because the purpose is to compare Pixal3D output, not repair it.

I added a pending issue under `Scripts/pending_issues_Scripts.md` for the GLB zero-object-path behavior.

## TestRoom Placement

Updated:

- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_TestRoom.cpp`

Spawn gate:

- CVar: `t66.TestRoom.SpawnLuBuMatrix`
- Default: `1`
- Set to `0` later to enter the TestRoom without the matrix.

Placement:

| Label | Location |
| --- | --- |
| R1024 T2048 Default | `(1500, -1000, 0)` |
| R1536 T2048 Default | `(1500, -600, 0)` |
| R1024 T4096 Default | `(1500, -200, 0)` |
| R1536 T4096 Default | `(1500, 200, 0)` |
| R1024 T2048 High | `(1500, 600, 0)` |
| R1536 T4096 High | `(1500, 1000, 0)` |

Labels:

- `ATextRenderActor`
- Location Z: `400`
- World size: `65`
- Color: `FColor(255, 235, 120)`
- Text faces the player-side target `(0, 0, 400)`

Matrix actors are tagged:

- `T66_TestRoom`
- `T66_TestRoom_LuBuMatrix`

They are also tagged `T66_AtmosphereSpared` through the TestRoom tag helper.

Follow-up after first visual check:

- The imported UE static mesh bounds were 100x larger than the normalized source files because the FBX fallback import interpreted units differently.
- The normalized GLB/FBX files remain the source of truth and still measure approximately 180 units tall in Blender.
- TestRoom placement now applies actor scale `0.01` to the six matrix meshes, making the displayed variants approximately 180 UU tall.
- This is a display-path correction only. The import pipeline still needs a proper direct-GLB fix before this becomes a reusable production import path.

Follow-up after second visual check:

- The variants were the correct size but appeared untextured in the staged TestRoom.
- The FBX fallback extracted each Pixal3D texture as an extensionless WebP file named `Image_0` under `SourceAssets/ToonStyle/Pixal3D/Phase1A/LuBu_Matrix/FBX/*_normalized.fbm/`.
- Unreal did not bind those extracted WebP payloads as usable material textures, producing the earlier `Image_0 contains no miplevels` warning and the blue/teal untextured look.
- I converted the six WebP payloads to PNG, imported them as real texture assets, created six explicit material instances from `/Game/Materials/M_GLB_Unlit`, and assigned one texture-backed material instance to each matrix static mesh.

Texture-backed UE assets:

| Label | Texture | Material |
| --- | --- | --- |
| R1024 T2048 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/T_LuBu_R1024_T2048_Default` | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/MI_LuBu_R1024_T2048_Default` |
| R1536 T2048 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/T_LuBu_R1536_T2048_Default` | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/MI_LuBu_R1536_T2048_Default` |
| R1024 T4096 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/T_LuBu_R1024_T4096_Default` | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/MI_LuBu_R1024_T4096_Default` |
| R1536 T4096 Default | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/T_LuBu_R1536_T4096_Default` | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/MI_LuBu_R1536_T4096_Default` |
| R1024 T2048 High | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/T_LuBu_R1024_T2048_High` | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/MI_LuBu_R1024_T2048_High` |
| R1536 T4096 High | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Textures/T_LuBu_R1536_T4096_High` | `/Game/ToonStyle/TestAssets/LuBu_Matrix/Materials/MI_LuBu_R1536_T4096_High` |

## Verification

- Pixal3D health check succeeded before generation.
- All 6 raw GLBs exist and are non-zero.
- All 6 normalized GLBs exist and have Blender sanity reports showing mesh present, material present, height approximately 180 UU, and feet at Z=0.
- All 6 UE static mesh assets exist under `Content/ToonStyle/TestAssets/LuBu_Matrix/`.
- `Saved/LuBuTextureBindingVerify.json` confirms all six static mesh slot-0 assignments point to the expected `MI_LuBu_*` material instances and those material instances point to the expected `T_LuBu_*` textures.
- Cook verification confirmed the ToonStyle TestAssets packages exist under `Saved/Cooked/Windows/T66/Content/ToonStyle/TestAssets/LuBu_Matrix/`.
- Cook verification after the texture fix confirmed all six material instance packages and all six texture packages are present in the staged cook.
- Game build succeeded.
- Standalone stage/cook/package succeeded after adding `/Game/ToonStyle` to always-cook.
- Staged standalone smoke boot exited with code 0 before and after the scale/texture follow-ups.

Final visual validation remains Pablo's eyeball pass in the TestRoom.
