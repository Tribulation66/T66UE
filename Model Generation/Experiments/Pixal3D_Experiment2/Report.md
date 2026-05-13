# Pixal3D Visibility Characterization - Experiment 2

## Pixal3D configuration

Configuration is copied from Experiment 1 and is identical, including `X-Seed: 1337`.

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

## Variant D - Over-bright goblin luminance ceiling test

### Source prompt

Goblin character in T-pose, front view, full body, clean white background. Skin is BRIGHT vivid yellow-green (#7FE85D), clothing is medium brown leather (#A66B3A). Two colors only across the entire character. Hard color edges, no gradients, no subtle shading variations. Strong simple silhouette with no fiddly details - no rope, no belt clasps, no pouches, no jewelry. Cartoon proportions, slightly oversized head. Flat illustration appearance, no realistic lighting, no contact shadows. Looks like a 2D drawing even though it has volume. The colors should be deliberately bright and saturated.

### Source image

![Variant D source](Sources/Variant_D.png)

### Turntable renders

![Variant D front](Renders/Variant_D_front.png)
![Variant D 3qleft](Renders/Variant_D_3qleft.png)
![Variant D side](Renders/Variant_D_side.png)
![Variant D 3qright](Renders/Variant_D_3qright.png)
![Variant D back](Renders/Variant_D_back.png)

### Extracted albedo

![Variant D albedo](Textures/Variant_D_albedo.png)

### Source color analysis

- Mask method: `non-white-background-distance>28`
- Masked pixel count: 367141
- Mean luminance: 0.4413
- Mean saturation: 0.7071
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#89E84C` | 34.46% |
| 2 | `#A55B23` | 31.67% |
| 3 | `#8BE94E` | 15.45% |
| 4 | `#A85F26` | 3.29% |
| 5 | `#62BF30` | 3.20% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 5582.11 | `#90AD3A`, `#E9EFB7` |
| 3 | 859.64 | `#8BE650`, `#A45B23`, `#2A590D` |
| 4 | 354.12 | `#89E84C`, `#A45B23`, `#4E9923`, `#ECEEBA` |

### Output texture color analysis

- Mask method: `uv0-triangle-raster-mask`
- Masked pixel count: 2402467
- Mean luminance: 0.2478
- Mean saturation: 0.7076
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#5DAF34` | 52.86% |
| 2 | `#7F4221` | 19.12% |
| 3 | `#804122` | 11.41% |
| 4 | `#5DAF31` | 7.75% |
| 5 | `#7F441E` | 1.51% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 3045.06 | `#69872D`, `#F0D4C1` |
| 3 | 674.85 | `#61B037`, `#804422`, `#3C5F23` |
| 4 | 73.71 | `#5DAF34`, `#804422`, `#F0D4C1`, `#3C5F23` |

### Silhouette analysis

- Mask method: `alpha>10`
- Bounding box: x=122, y=211, w=780, h=602
- Character pixel count: 153200
- Background pixel count: 895376
- Best k by lowest within-cluster variance: 4

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 2775.74 | `#6F792A`, `#BBB4AA` |
| 3 | 276.76 | `#699F3A`, `#7D3D13`, `#2E4F1B` |
| 4 | 252.12 | `#699F3A`, `#7D3D13`, `#365D20`, `#050403` |

## Variant E - Mushroom monster organic archetype test

### Source prompt

Mushroom monster creature standing in neutral pose, front view, full body, clean white background. Body is light cream-tan (#E8D5A8), mushroom cap is bright red (#D63A3A). Two colors only across the entire creature. Hard color edges between body and cap, no gradients, no subtle shading variations. Strong silhouette with simple bipedal humanoid mushroom form, no extra appendages, no tendrils, no fiddly details. Cartoon proportions. Flat illustration appearance, no realistic lighting, no contact shadows. Looks like a 2D drawing even though it has volume.

### Source image

![Variant E source](Sources/Variant_E.png)

### Turntable renders

![Variant E front](Renders/Variant_E_front.png)
![Variant E 3qleft](Renders/Variant_E_3qleft.png)
![Variant E side](Renders/Variant_E_side.png)
![Variant E 3qright](Renders/Variant_E_3qright.png)
![Variant E back](Renders/Variant_E_back.png)

### Extracted albedo

![Variant E albedo](Textures/Variant_E_albedo.png)

### Source color analysis

- Mask method: `non-white-background-distance>28`
- Masked pixel count: 416828
- Mean luminance: 0.5197
- Mean saturation: 0.4313
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#EAD1A0` | 52.54% |
| 2 | `#D63535` | 25.49% |
| 3 | `#EBD3A2` | 14.09% |
| 4 | `#F0D9A9` | 1.40% |
| 5 | `#E5CB99` | 0.77% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 396.59 | `#E9D1A0`, `#D13735` |
| 3 | 309.27 | `#E9D0A0`, `#D43836`, `#6B1D17` |
| 4 | 111.37 | `#EAD2A1`, `#D63535`, `#B2976F`, `#681C16` |

### Output texture color analysis

- Mask method: `uv0-triangle-raster-mask`
- Masked pixel count: 2409562
- Mean luminance: 0.4072
- Mean saturation: 0.6371
- Best k by lowest within-cluster variance: 4

| Rank | Hex | Percentage |
|---:|---|---:|
| 1 | `#EFBE7F` | 46.22% |
| 2 | `#D5080C` | 18.52% |
| 3 | `#D40B0E` | 13.48% |
| 4 | `#F1BE7F` | 10.81% |
| 5 | `#EDBB7E` | 2.85% |

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 636.01 | `#EDBC7E`, `#CE0C0E` |
| 3 | 259.84 | `#EEBC7E`, `#D50A0D`, `#57412A` |
| 4 | 115.47 | `#EFBD7F`, `#D50A0D`, `#B18157`, `#312415` |

### Silhouette analysis

- Mask method: `alpha>10`
- Bounding box: x=321, y=275, w=382, h=473
- Character pixel count: 91278
- Background pixel count: 957298
- Best k by lowest within-cluster variance: 4

| k | Within-cluster variance | Centroids |
|---:|---:|---|
| 2 | 355.32 | `#C2A687`, `#C02305` |
| 3 | 137.22 | `#C2A687`, `#C52204`, `#42311E` |
| 4 | 66.06 | `#C2A788`, `#C52204`, `#AC7A59`, `#382919` |

## Comparison table

| Variant | Source mean luminance | Output mean luminance | Luminance delta | Source mean saturation | Output mean saturation | Saturation delta | Output dominant color count | Silhouette cluster count |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| A | 0.2346 | 0.1704 | -0.0642 | 0.6293 | 0.6855 | +0.0563 | 4 | 4 |
| B | 0.1728 | 0.1003 | -0.0725 | 0.6400 | 0.7289 | +0.0889 | 4 | 4 |
| C | 0.1135 | 0.0609 | -0.0526 | 0.6125 | 0.5854 | -0.0272 | 4 | 4 |
| D | 0.4413 | 0.2478 | -0.1936 | 0.7071 | 0.7076 | +0.0005 | 4 | 4 |
| E | 0.5197 | 0.4072 | -0.1125 | 0.4313 | 0.6371 | +0.2057 | 4 | 4 |

## Observations

- Variant D output/source luminance scaling factor is 0.561 (0.2478 / 0.4413). Experiment 1 scaling factors were A: 0.726, B: 0.580, C: 0.537. The over-bright source did push the absolute output luminance to 0.2478, but the proportional retention stayed in the same band as the goblin readability variants rather than removing the luminance drop entirely.
- Variant E texture k=2 centroids are `#EDBC7E`, `#CE0C0E`. The requested source colors were `#E8D5A8` and `#D63A3A`; the source image's own k=2 centroids were `#E9D1A0`, `#D13735`. The red cap remained a strong cluster, while the cream body shifted darker/tanner in the baked texture, so the duo-color discipline mostly generalized but did not preserve exact color targets.
- Variant E also introduced small facial, spot, and body-mark detail despite the two-color prompt, so its output carries more local color complexity than the intended strict organic two-color test. Variant D inherited the same eye/tusk exceptions seen in Variant A but otherwise remained a clean two-material goblin.
