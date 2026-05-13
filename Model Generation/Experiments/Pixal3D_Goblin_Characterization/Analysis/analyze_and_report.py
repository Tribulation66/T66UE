import argparse
import json
import math
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw


VARIANTS = {
    "A": {
        "name": "Variant A - Maximum readability",
        "prompt": (
            "Goblin character in T-pose, front view, full body, clean white background. "
            "Skin is bright vivid green (#4FA84C), clothing is brown leather (#8B5A2B). "
            "Two colors only across the entire character. Hard color edges, no gradients, "
            "no subtle shading variations. Strong simple silhouette with no fiddly details - "
            "no rope, no belt clasps, no pouches, no jewelry. Cartoon proportions, slightly "
            "oversized head. Flat illustration appearance, no realistic lighting, no contact "
            "shadows. Looks like a 2D drawing even though it has volume."
        ),
    },
    "B": {
        "name": "Variant B - Moderate readability",
        "prompt": (
            "Goblin character in T-pose, front view, full body, clean white background. "
            "Skin is bright green (#4FA84C), clothing is brown leather (#8B5A2B). "
            "Predominantly two colors with minor accents. Some natural cloth folds and "
            "organic shape variation allowed. Cartoon proportions. Soft but visible color "
            "separation. No realistic lighting."
        ),
    },
    "C": {
        "name": "Variant C - Current style baseline",
        "prompt": (
            "Goblin character in T-pose, front view, full body, clean white background. "
            "Realistic skin texturing in green tones, varied leather clothing with "
            "weathered details. Natural lighting with realistic shadows falling on the body. "
            "Stylized representation."
        ),
    },
}


CONFIG_BLOCK = """model_path: TencentARC/Pixal3D
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
X-Tex-Rescale-T: 3.0"""


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True)
    return parser.parse_args()


def srgb_to_linear(rgb):
    rgb = np.clip(rgb, 0.0, 1.0)
    return np.where(rgb <= 0.04045, rgb / 12.92, ((rgb + 0.055) / 1.055) ** 2.4)


def mean_luminance(rgb_u8):
    rgb = rgb_u8.astype(np.float32) / 255.0
    linear = srgb_to_linear(rgb)
    lum = 0.2126 * linear[:, 0] + 0.7152 * linear[:, 1] + 0.0722 * linear[:, 2]
    return float(lum.mean())


def mean_saturation(rgb_u8):
    rgb = rgb_u8.astype(np.float32) / 255.0
    maxc = rgb.max(axis=1)
    minc = rgb.min(axis=1)
    sat = np.where(maxc <= 1e-8, 0.0, (maxc - minc) / maxc)
    return float(sat.mean())


def hex_color(center):
    values = np.clip(np.rint(center), 0, 255).astype(np.uint8)
    return f"#{values[0]:02X}{values[1]:02X}{values[2]:02X}"


def load_rgba(path):
    return np.array(Image.open(path).convert("RGBA"))


def subject_mask_from_rgba(rgba):
    alpha = rgba[:, :, 3] > 10
    rgb = rgba[:, :, :3].astype(np.int16)
    white_distance = np.abs(rgb - 255).sum(axis=2)
    non_white = white_distance > 28
    if alpha.sum() < alpha.size * 0.98:
        mask = alpha
        method = "alpha>10"
    else:
        mask = non_white
        method = "non-white-background-distance>28"
    return mask, method


def uv_mask_from_metadata(metadata_path, image_size):
    if not metadata_path.exists():
        return None
    metadata = json.loads(metadata_path.read_text(encoding="ascii"))
    triangles = metadata.get("uv_triangles") or []
    if not triangles:
        return None
    width, height = image_size
    mask_image = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(mask_image)
    for tri in triangles:
        points = []
        for u, v in tri:
            x = max(0, min(width - 1, int(round(u * (width - 1)))))
            y = max(0, min(height - 1, int(round((1.0 - v) * (height - 1)))))
            points.append((x, y))
        draw.polygon(points, fill=255)
    mask = np.array(mask_image) > 0
    return mask if mask.any() else None


def sample_rows(rgb_u8, max_rows, seed):
    if len(rgb_u8) <= max_rows:
        return rgb_u8
    rng = np.random.default_rng(seed)
    indices = rng.choice(len(rgb_u8), size=max_rows, replace=False)
    return rgb_u8[indices]


def kmeans_rgb(rgb_u8, k, seed=1337, sample_limit=120000, iterations=40):
    data_u8 = sample_rows(rgb_u8, sample_limit, seed + k)
    data = data_u8.astype(np.float32)
    if len(data) == 0:
        return {"k": k, "variance": 0.0, "centroids": [], "counts": []}
    unique = np.unique(data, axis=0)
    rng = np.random.default_rng(seed + k * 17)
    if len(unique) >= k:
        centers = unique[rng.choice(len(unique), size=k, replace=False)].astype(np.float32)
    else:
        centers = unique.astype(np.float32)
        while len(centers) < k:
            centers = np.vstack([centers, centers[-1:]])
    labels = np.zeros(len(data), dtype=np.int32)
    for _ in range(iterations):
        distances = ((data[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
        new_labels = distances.argmin(axis=1)
        if np.array_equal(new_labels, labels):
            labels = new_labels
            break
        labels = new_labels
        for idx in range(k):
            points = data[labels == idx]
            if len(points):
                centers[idx] = points.mean(axis=0)
            else:
                centers[idx] = data[rng.integers(0, len(data))]
    distances = ((data[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
    labels = distances.argmin(axis=1)
    variance = float(distances[np.arange(len(data)), labels].mean())
    counts = np.bincount(labels, minlength=k)
    order = np.argsort(-counts)
    return {
        "k": k,
        "variance": variance,
        "centroids": [hex_color(centers[idx]) for idx in order],
        "centroid_rgb": [[float(v) for v in centers[idx]] for idx in order],
        "counts": [int(counts[idx]) for idx in order],
    }


def dominant_colors_32(rgb_u8):
    result = kmeans_rgb(rgb_u8, 32, seed=7331, sample_limit=180000, iterations=45)
    centers = np.array(result["centroid_rgb"], dtype=np.float32)
    if len(centers) == 0:
        return []
    counts = np.zeros(len(centers), dtype=np.int64)
    data = rgb_u8.astype(np.float32)
    chunk_size = 200000
    for start in range(0, len(data), chunk_size):
        chunk = data[start : start + chunk_size]
        distances = ((chunk[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
        labels = distances.argmin(axis=1)
        counts += np.bincount(labels, minlength=len(centers))
    order = np.argsort(-counts)[:5]
    total = max(1, int(counts.sum()))
    return [
        {"hex": hex_color(centers[idx]), "percentage": round(float(counts[idx]) * 100.0 / total, 2)}
        for idx in order
    ]


def color_analysis(path, mask=None, mask_method=None):
    rgba = load_rgba(path)
    if mask is None:
        mask, inferred_method = subject_mask_from_rgba(rgba)
        mask_method = mask_method or inferred_method
    rgb = rgba[:, :, :3][mask]
    if len(rgb) == 0:
        rgb = rgba[:, :, :3].reshape(-1, 3)
        mask_method = "fallback-all-pixels"
    kmeans = {str(k): kmeans_rgb(rgb, k) for k in (2, 3, 4)}
    best_key = min(kmeans, key=lambda key: kmeans[key]["variance"])
    return {
        "path": str(path),
        "mask_method": mask_method,
        "sampled_pixel_count": int(len(rgb)),
        "mean_luminance": mean_luminance(rgb),
        "mean_saturation": mean_saturation(rgb),
        "dominant_colors_32_top5": dominant_colors_32(rgb),
        "kmeans": kmeans,
        "best_k": int(best_key),
        "best_k_centroids": kmeans[best_key]["centroids"],
    }


def texture_analysis(root, variant):
    path = root / "Textures" / f"Variant_{variant}_albedo.png"
    rgba = load_rgba(path)
    metadata_path = root / "Analysis" / f"Variant_{variant}_blender_metadata.json"
    mask = uv_mask_from_metadata(metadata_path, (rgba.shape[1], rgba.shape[0]))
    if mask is not None:
        return color_analysis(path, mask=mask, mask_method="uv0-triangle-raster-mask")
    return color_analysis(path)


def silhouette_analysis(root, variant):
    path = root / "Renders" / f"Variant_{variant}_front.png"
    rgba = load_rgba(path)
    mask, method = subject_mask_from_rgba(rgba)
    ys, xs = np.where(mask)
    if len(xs) == 0:
        bbox = {"x": 0, "y": 0, "w": 0, "h": 0}
        rgb = rgba[:, :, :3].reshape(-1, 3)
    else:
        x0, x1 = int(xs.min()), int(xs.max())
        y0, y1 = int(ys.min()), int(ys.max())
        bbox = {"x": x0, "y": y0, "w": x1 - x0 + 1, "h": y1 - y0 + 1}
        rgb = rgba[:, :, :3][mask]
    kmeans = {str(k): kmeans_rgb(rgb, k) for k in (2, 3, 4)}
    best_key = min(kmeans, key=lambda key: kmeans[key]["variance"])
    return {
        "path": str(path),
        "mask_method": method,
        "bounding_box": bbox,
        "character_pixel_count": int(mask.sum()),
        "background_pixel_count": int(mask.size - mask.sum()),
        "kmeans": kmeans,
        "best_k": int(best_key),
        "best_k_centroids": kmeans[best_key]["centroids"],
    }


def format_float(value, places=4):
    return f"{value:.{places}f}"


def image_link(root, path):
    return Path(path).resolve().relative_to(root.resolve()).as_posix()


def dominant_table(colors):
    lines = ["| Rank | Hex | Percentage |", "|---:|---|---:|"]
    for index, item in enumerate(colors, start=1):
        lines.append(f"| {index} | `{item['hex']}` | {item['percentage']:.2f}% |")
    return "\n".join(lines)


def kmeans_table(kmeans):
    lines = ["| k | Within-cluster variance | Centroids |", "|---:|---:|---|"]
    for key in ("2", "3", "4"):
        item = kmeans[key]
        lines.append(
            f"| {key} | {item['variance']:.2f} | "
            f"{', '.join(f'`{color}`' for color in item['centroids'])} |"
        )
    return "\n".join(lines)


def build_observations(analysis):
    lines = []
    for variant, payload in analysis.items():
        source = payload["source"]
        texture = payload["texture"]
        sil = payload["silhouette"]
        lum_delta = texture["mean_luminance"] - source["mean_luminance"]
        sat_delta = texture["mean_saturation"] - source["mean_saturation"]
        source_top = sum(item["percentage"] for item in source["dominant_colors_32_top5"])
        output_top = sum(item["percentage"] for item in texture["dominant_colors_32_top5"])
        lines.append(
            f"- Variant {variant}: output luminance delta {lum_delta:+.4f} "
            f"({source['mean_luminance']:.4f} -> {texture['mean_luminance']:.4f}); "
            f"saturation delta {sat_delta:+.4f} "
            f"({source['mean_saturation']:.4f} -> {texture['mean_saturation']:.4f}). "
            f"The top five 32-color buckets cover {source_top:.2f}% of the source mask "
            f"versus {output_top:.2f}% of the output texture mask. Texture best-k is "
            f"{texture['best_k']} and front-silhouette best-k is {sil['best_k']}."
        )
    lines.append(
        "- Across these three outputs, lower export decimation was needed for stability: "
        "the final controlled outputs use `X-Decimation: 30000`. The retained generation "
        "log records the failed/superseded `80000` trials."
    )
    return "\n".join(lines)


def write_report(root, analysis):
    lines = ["# Pixal3D Visibility Characterization - Goblin Test", ""]
    lines += ["## Pixal3D configuration", "", "```yaml", CONFIG_BLOCK, "```", ""]
    lines.append(
        "Final outputs were generated with the block above for all three variants. "
        "Troubleshooting trials that failed or were superseded are retained in "
        "`Logs/pixal3d_generation.json`."
    )
    lines.append("")

    for variant, meta in VARIANTS.items():
        payload = analysis[variant]
        lines += [f"## {meta['name']}", "", "### Source prompt", "", meta["prompt"], ""]
        lines += [
            "### Source image",
            "",
            f"![Variant {variant} source]({image_link(root, root / 'Sources' / f'Variant_{variant}.png')})",
            "",
            "### Turntable renders",
            "",
        ]
        render_names = ("front", "3qleft", "side", "3qright", "back")
        lines.extend(
            [
                f"![Variant {variant} {name}]({image_link(root, root / 'Renders' / f'Variant_{variant}_{name}.png')})"
                for name in render_names
            ]
        )
        lines += [
            "",
            "### Extracted albedo",
            "",
            f"![Variant {variant} albedo]({image_link(root, root / 'Textures' / f'Variant_{variant}_albedo.png')})",
            "",
            "### Source color analysis",
            "",
            f"- Mask method: `{payload['source']['mask_method']}`",
            f"- Masked pixel count: {payload['source']['sampled_pixel_count']}",
            f"- Mean luminance: {payload['source']['mean_luminance']:.4f}",
            f"- Mean saturation: {payload['source']['mean_saturation']:.4f}",
            f"- Best k by lowest within-cluster variance: {payload['source']['best_k']}",
            "",
            dominant_table(payload["source"]["dominant_colors_32_top5"]),
            "",
            kmeans_table(payload["source"]["kmeans"]),
            "",
            "### Output texture color analysis",
            "",
            f"- Mask method: `{payload['texture']['mask_method']}`",
            f"- Masked pixel count: {payload['texture']['sampled_pixel_count']}",
            f"- Mean luminance: {payload['texture']['mean_luminance']:.4f}",
            f"- Mean saturation: {payload['texture']['mean_saturation']:.4f}",
            f"- Best k by lowest within-cluster variance: {payload['texture']['best_k']}",
            "",
            dominant_table(payload["texture"]["dominant_colors_32_top5"]),
            "",
            kmeans_table(payload["texture"]["kmeans"]),
            "",
            "### Silhouette analysis",
            "",
            f"- Mask method: `{payload['silhouette']['mask_method']}`",
            f"- Bounding box: x={payload['silhouette']['bounding_box']['x']}, "
            f"y={payload['silhouette']['bounding_box']['y']}, "
            f"w={payload['silhouette']['bounding_box']['w']}, "
            f"h={payload['silhouette']['bounding_box']['h']}",
            f"- Character pixel count: {payload['silhouette']['character_pixel_count']}",
            f"- Background pixel count: {payload['silhouette']['background_pixel_count']}",
            f"- Best k by lowest within-cluster variance: {payload['silhouette']['best_k']}",
            "",
            kmeans_table(payload["silhouette"]["kmeans"]),
            "",
        ]

    lines += ["## Comparison table", ""]
    lines.append(
        "| Variant | Source mean luminance | Output mean luminance | "
        "Source mean saturation | Output mean saturation | "
        "Output dominant color count | Silhouette cluster count |"
    )
    lines.append("|---|---:|---:|---:|---:|---:|---:|")
    for variant in VARIANTS:
        payload = analysis[variant]
        lines.append(
            f"| {variant} | {payload['source']['mean_luminance']:.4f} | "
            f"{payload['texture']['mean_luminance']:.4f} | "
            f"{payload['source']['mean_saturation']:.4f} | "
            f"{payload['texture']['mean_saturation']:.4f} | "
            f"{payload['texture']['best_k']} | {payload['silhouette']['best_k']} |"
        )
    lines += ["", "## Observations", "", build_observations(analysis), ""]
    (root / "Report.md").write_text("\n".join(lines), encoding="utf-8", newline="\n")


def main():
    args = parse_args()
    root = Path(args.root)
    analysis = {}
    for variant in VARIANTS:
        analysis[variant] = {
            "source": color_analysis(root / "Sources" / f"Variant_{variant}.png"),
            "texture": texture_analysis(root, variant),
            "silhouette": silhouette_analysis(root, variant),
        }
    (root / "Analysis" / "analysis_metrics.json").write_text(
        json.dumps(analysis, indent=2), encoding="ascii"
    )
    write_report(root, analysis)


if __name__ == "__main__":
    main()
