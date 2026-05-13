# Pixal3D Visibility Characterization - Goblin Test

## Pixal3D configuration

```yaml
model_path: TencentARC/Pixal3D
pod_gpu: NVIDIA A40
server_remote_url: http://127.0.0.1:18001
attention_backend: flash_attn_3
sparse_attention_backend: flash_attn_3
sparse_conv_backend: flex_gemm
low_vram: false
request_method: POST /generate
request_content_type: image/png
X-Seed: 1337
X-Resolution: 1024
X-Texture-Size: 2048
X-Decimation: 30000
X-Remesh: 1
X-Remesh-Band: 1.0
X-Remesh-Project: 0.0
X-Extend-Pixel: 0
X-Image-Resolution: 512
X-Max-Num-Tokens: 49152
X-Mesh-Scale: 1.0
X-SS-Steps: 12
X-SS-Guidance: 7.5
X-SS-Guidance-Rescale: 0.7
X-SS-Rescale-T: 5.0
X-Shape-Steps: 12
X-Shape-Guidance: 7.5
X-Shape-Guidance-Rescale: 0.5
X-Shape-Rescale-T: 3.0
X-Tex-Steps: 12
X-Tex-Guidance: 1.0
X-Tex-Guidance-Rescale: 0.0
X-Tex-Rescale-T: 3.0
```

Final outputs were generated with the block above for all three variants. Troubleshooting trials that failed or were superseded are retained in `Logs/pixal3d_generation.json`.

## Variant A - Maximum readability

### Source prompt

Goblin character in T-pose, front view, full body, clean white background. Skin is bright vivid green (#4FA84C), clothing is brown leather (#8B5A2B). Two colors only across the entire character. Hard color edges, no gradients, no subtle shading variations. Strong simple silhouette with no fiddly details - no rope, no belt clasps, no pouches, no jewelry. Cartoon proportions, slightly oversized head. Flat illustration appearance, no realistic lighting, no contact shadows. Looks like a 2D drawing even though it has volume.

### Source image

![Variant A source](Sources/Variant_A.png)

### Turntable renders

![Variant A front](Renders/Variant_A_front.png)
![Variant A 3qleft](Renders/Variant_A_3qleft.png)
![Variant A side](Renders/Variant_A_side.png)
![Variant A 3qright](Renders/Variant_A_3qright.png)
![Variant A back](Renders/Variant_A_back.png)

### Extracted albedo

![Variant A albedo](Textures/Variant_A_albedo.png)

### Source color analysis

- Mask method: `non-white-background-distance>28`
- Masked pixel count: 318362
- Mean luminance: 0.2346
- Mean saturation: 0.6293
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#64A844` | 33.76% |
| 2 | `#7E4A21` | 25.09% |
| 3 | `#66AA46` | 15.51% |
| 4 | `#804C23` | 8.28% |
| 5 | `#4C8C33` | 2.06% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 2746.37 | `#6B8136`, `#E3DFBE` |
| 3 | 1154.16 | `#6BAB4C`, `#7D4A21`, `#41742B` |
| 4 | 241.75 | `#65A845`, `#7D4A21`, `#40732B`, `#E4E0BE` |

### Output texture color analysis

- Mask method: `uv0-triangle-raster-mask`
- Masked pixel count: 2401280
- Mean luminance: 0.1704
- Mean saturation: 0.6855
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#559031` | 58.83% |
| 2 | `#52341A` | 18.79% |
| 3 | `#5F3810` | 12.70% |
| 4 | `#603912` | 2.29% |
| 5 | `#588F32` | 1.71% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 2072.56 | `#556F27`, `#FCF0AA` |
| 3 | 1025.44 | `#5A9334`, `#583616`, `#1C3811` |
| 4 | 1012.60 | `#5A9334`, `#583616`, `#1A3610`, `#516C27` |

### Silhouette analysis

- Mask method: `alpha>10`
- Bounding box: x=54, y=188, w=916, h=648
- Character pixel count: 187970
- Background pixel count: 860606
- Best k by lowest within-cluster variance: 4

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 2293.16 | `#54661E`, `#BEBAA2` |
| 3 | 410.54 | `#5B8A2C`, `#4E2E0B`, `#515F18` |
| 4 | 64.61 | `#59892A`, `#512F0B`, `#132807`, `#C0BBA3` |

## Variant B - Moderate readability

### Source prompt

Goblin character in T-pose, front view, full body, clean white background. Skin is bright green (#4FA84C), clothing is brown leather (#8B5A2B). Predominantly two colors with minor accents. Some natural cloth folds and organic shape variation allowed. Cartoon proportions. Soft but visible color separation. No realistic lighting.

### Source image

![Variant B source](Sources/Variant_B.png)

### Turntable renders

![Variant B front](Renders/Variant_B_front.png)
![Variant B 3qleft](Renders/Variant_B_3qleft.png)
![Variant B side](Renders/Variant_B_side.png)
![Variant B 3qright](Renders/Variant_B_3qright.png)
![Variant B back](Renders/Variant_B_back.png)

### Extracted albedo

![Variant B albedo](Textures/Variant_B_albedo.png)

### Source color analysis

- Mask method: `non-white-background-distance>28`
- Masked pixel count: 281430
- Mean luminance: 0.1728
- Mean saturation: 0.6400
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#5F3F21` | 7.49% |
| 2 | `#6C4724` | 7.21% |
| 3 | `#53371C` | 6.16% |
| 4 | `#729933` | 6.11% |
| 5 | `#84AE3E` | 5.81% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 1845.13 | `#53481F`, `#81A242` |
| 3 | 1332.11 | `#50411D`, `#6E8731`, `#97B958` |
| 4 | 865.20 | `#636028`, `#7FA43D`, `#463718`, `#D7D7C1` |

### Output texture color analysis

- Mask method: `uv0-triangle-raster-mask`
- Masked pixel count: 2434709
- Mean luminance: 0.1003
- Mean saturation: 0.7289
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#4C3416` | 9.66% |
| 2 | `#523816` | 8.76% |
| 3 | `#778928` | 8.26% |
| 4 | `#6F8021` | 7.16% |
| 5 | `#4A5812` | 6.73% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 554.45 | `#493F15`, `#6D7A23` |
| 3 | 361.44 | `#483815`, `#556018`, `#758328` |
| 4 | 282.12 | `#4B3417`, `#5C681B`, `#778529`, `#424E0F` |

### Silhouette analysis

- Mask method: `alpha>10`
- Bounding box: x=83, y=187, w=858, h=648
- Character pixel count: 152153
- Background pixel count: 896423
- Best k by lowest within-cluster variance: 4

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 478.93 | `#6F7610`, `#48320D` |
| 3 | 303.49 | `#482F0D`, `#777F16`, `#5F6709` |
| 4 | 255.87 | `#462E0B`, `#787F16`, `#606807`, `#574223` |

## Variant C - Current style baseline

### Source prompt

Goblin character in T-pose, front view, full body, clean white background. Realistic skin texturing in green tones, varied leather clothing with weathered details. Natural lighting with realistic shadows falling on the body. Stylized representation.

### Source image

![Variant C source](Sources/Variant_C.png)

### Turntable renders

![Variant C front](Renders/Variant_C_front.png)
![Variant C 3qleft](Renders/Variant_C_3qleft.png)
![Variant C side](Renders/Variant_C_side.png)
![Variant C 3qright](Renders/Variant_C_3qright.png)
![Variant C back](Renders/Variant_C_back.png)

### Extracted albedo

![Variant C albedo](Textures/Variant_C_albedo.png)

### Source color analysis

- Mask method: `non-white-background-distance>28`
- Masked pixel count: 324726
- Mean luminance: 0.1135
- Mean saturation: 0.6125
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#2B1B0A` | 5.87% |
| 2 | `#422B17` | 5.80% |
| 3 | `#342416` | 5.79% |
| 4 | `#51371E` | 5.68% |
| 5 | `#897937` | 4.53% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 2079.35 | `#453318`, `#95824C` |
| 3 | 1307.61 | `#3C2A13`, `#756030`, `#AE9F6A` |
| 4 | 820.48 | `#352510`, `#644E27`, `#988549`, `#D8D2C3` |

### Output texture color analysis

- Mask method: `uv0-triangle-raster-mask`
- Masked pixel count: 2405520
- Mean luminance: 0.0609
- Mean saturation: 0.5854
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#604F26` | 10.44% |
| 2 | `#655429` | 9.39% |
| 3 | `#6A592C` | 9.03% |
| 4 | `#5A4A22` | 8.78% |
| 5 | `#291C11` | 6.32% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 377.89 | `#625129`, `#2E2113` |
| 3 | 259.96 | `#2D1F13`, `#584723`, `#6C5A2F` |
| 4 | 201.81 | `#5A4924`, `#6D5A2F`, `#261A10`, `#392817` |

### Silhouette analysis

- Mask method: `alpha>10`
- Bounding box: x=135, y=220, w=754, h=586
- Character pixel count: 101488
- Background pixel count: 947088
- Best k by lowest within-cluster variance: 4

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 687.90 | `#614C23`, `#2C1C0E` |
| 3 | 339.65 | `#5F4920`, `#2B1C0D`, `#9C9179` |
| 4 | 211.75 | `#665022`, `#28190C`, `#4F3B1B`, `#9D927A` |

## Comparison table

| Variant | Source mean luminance | Output mean luminance | Source mean saturation | Output mean saturation | Output dominant color count | Silhouette cluster count |
|---|---:|---:|---:|---:|---:|---:|
| A | 0.2346 | 0.1704 | 0.6293 | 0.6855 | 4 | 4 |
| B | 0.1728 | 0.1003 | 0.6400 | 0.7289 | 4 | 4 |
| C | 0.1135 | 0.0609 | 0.6125 | 0.5854 | 4 | 4 |

## Observations

- Variant A: output luminance delta -0.0642 (0.2346 -> 0.1704); saturation delta +0.0563 (0.6293 -> 0.6855). The top five 32-color buckets cover 84.70% of the source mask versus 94.32% of the output texture mask. Texture best-k is 4 and front-silhouette best-k is 4.
- Variant B: output luminance delta -0.0725 (0.1728 -> 0.1003); saturation delta +0.0889 (0.6400 -> 0.7289). The top five 32-color buckets cover 32.78% of the source mask versus 40.57% of the output texture mask. Texture best-k is 4 and front-silhouette best-k is 4.
- Variant C: output luminance delta -0.0526 (0.1135 -> 0.0609); saturation delta -0.0272 (0.6125 -> 0.5854). The top five 32-color buckets cover 27.67% of the source mask versus 43.96% of the output texture mask. Texture best-k is 4 and front-silhouette best-k is 4.
- Across these three outputs, lower export decimation was needed for stability: the final controlled outputs use `X-Decimation: 30000`. The retained generation log records the failed/superseded `80000` trials.
