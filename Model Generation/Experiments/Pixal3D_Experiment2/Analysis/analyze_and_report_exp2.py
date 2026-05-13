import importlib.util
import json
import re
from pathlib import Path


ROOT = Path(r"C:\UE\T66\Model Generation\Experiments\Pixal3D_Experiment2")
EXP1_ROOT = Path(r"C:\UE\T66\Model Generation\Experiments\Pixal3D_Goblin_Characterization")
EXP1_ANALYSIS_SCRIPT = EXP1_ROOT / "Analysis" / "analyze_and_report.py"
EXP1_REPORT = EXP1_ROOT / "Report.md"
EXP1_METRICS = EXP1_ROOT / "Analysis" / "analysis_metrics.json"

VARIANTS = {
    "D": {
        "name": "Variant D - Over-bright goblin luminance ceiling test",
        "prompt": (
            "Goblin character in T-pose, front view, full body, clean white background. "
            "Skin is BRIGHT vivid yellow-green (#7FE85D), clothing is medium brown leather (#A66B3A). "
            "Two colors only across the entire character. Hard color edges, no gradients, no subtle shading variations. "
            "Strong simple silhouette with no fiddly details - no rope, no belt clasps, no pouches, no jewelry. "
            "Cartoon proportions, slightly oversized head. Flat illustration appearance, no realistic lighting, no contact shadows. "
            "Looks like a 2D drawing even though it has volume. The colors should be deliberately bright and saturated."
        ),
        "input_colors": ["#7FE85D", "#A66B3A"],
    },
    "E": {
        "name": "Variant E - Mushroom monster organic archetype test",
        "prompt": (
            "Mushroom monster creature standing in neutral pose, front view, full body, clean white background. "
            "Body is light cream-tan (#E8D5A8), mushroom cap is bright red (#D63A3A). Two colors only across the entire creature. "
            "Hard color edges between body and cap, no gradients, no subtle shading variations. Strong silhouette with simple "
            "bipedal humanoid mushroom form, no extra appendages, no tendrils, no fiddly details. Cartoon proportions. "
            "Flat illustration appearance, no realistic lighting, no contact shadows. Looks like a 2D drawing even though it has volume."
        ),
        "input_colors": ["#E8D5A8", "#D63A3A"],
    },
}


spec = importlib.util.spec_from_file_location("exp1_analysis", EXP1_ANALYSIS_SCRIPT)
exp1 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(exp1)


def experiment1_config_block() -> str:
    text = EXP1_REPORT.read_text(encoding="utf-8")
    match = re.search(r"## Pixal3D configuration\s+```yaml\s+(.*?)\s+```", text, flags=re.S)
    if not match:
        raise RuntimeError("Could not find Experiment 1 Pixal3D configuration block")
    return match.group(1).strip()


def image_link(root: Path, path: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def write_generation_log(config_block: str, metrics: dict) -> None:
    success_path = ROOT / "Logs" / "pixal3d_generation_successful_outputs.json"
    successful = json.loads(success_path.read_text(encoding="utf-8")) if success_path.exists() else []
    payload = {
        "experiment": "Pixal3D_Experiment2",
        "configuration_source": str(EXP1_REPORT),
        "configuration_identical_to_experiment1": True,
        "configuration_block": config_block,
        "successful_outputs": successful,
        "analysis_metric_file": str(ROOT / "Analysis" / "analysis_metrics.json"),
    }
    (ROOT / "Logs" / "pixal3d_generation.json").write_text(
        json.dumps(payload, indent=2), encoding="ascii"
    )


def analyze_variants() -> dict:
    analysis = {}
    for variant in VARIANTS:
        analysis[variant] = {
            "source": exp1.color_analysis(ROOT / "Sources" / f"Variant_{variant}.png"),
            "texture": exp1.texture_analysis(ROOT, variant),
            "silhouette": exp1.silhouette_analysis(ROOT, variant),
        }
    (ROOT / "Analysis" / "analysis_metrics.json").write_text(
        json.dumps(analysis, indent=2), encoding="ascii"
    )
    return analysis


def comparison_rows(exp2_analysis: dict) -> list[tuple[str, dict]]:
    exp1_analysis = json.loads(EXP1_METRICS.read_text(encoding="ascii"))
    rows = []
    for variant in ("A", "B", "C"):
        rows.append((variant, exp1_analysis[variant]))
    for variant in ("D", "E"):
        rows.append((variant, exp2_analysis[variant]))
    return rows


def kmeans_line(kmeans: dict, key: str) -> str:
    item = kmeans[key]
    return f"k={key}, variance={item['variance']:.2f}, centroids={', '.join(item['centroids'])}"


def build_variant_section(variant: str, payload: dict) -> list[str]:
    meta = VARIANTS[variant]
    lines = [f"## {meta['name']}", "", "### Source prompt", "", meta["prompt"], ""]
    lines += [
        "### Source image",
        "",
        f"![Variant {variant} source]({image_link(ROOT, ROOT / 'Sources' / f'Variant_{variant}.png')})",
        "",
        "### Turntable renders",
        "",
    ]
    for angle in ("front", "3qleft", "side", "3qright", "back"):
        lines.append(
            f"![Variant {variant} {angle}]({image_link(ROOT, ROOT / 'Renders' / f'Variant_{variant}_{angle}.png')})"
        )
    lines += [
        "",
        "### Extracted albedo",
        "",
        f"![Variant {variant} albedo]({image_link(ROOT, ROOT / 'Textures' / f'Variant_{variant}_albedo.png')})",
        "",
        "### Source color analysis",
        "",
        f"- Mask method: `{payload['source']['mask_method']}`",
        f"- Masked pixel count: {payload['source']['sampled_pixel_count']}",
        f"- Mean luminance: {payload['source']['mean_luminance']:.4f}",
        f"- Mean saturation: {payload['source']['mean_saturation']:.4f}",
        f"- Best k by lowest within-cluster variance: {payload['source']['best_k']}",
        "",
        exp1.dominant_table(payload["source"]["dominant_colors_32_top5"]),
        "",
        exp1.kmeans_table(payload["source"]["kmeans"]),
        "",
        "### Output texture color analysis",
        "",
        f"- Mask method: `{payload['texture']['mask_method']}`",
        f"- Masked pixel count: {payload['texture']['sampled_pixel_count']}",
        f"- Mean luminance: {payload['texture']['mean_luminance']:.4f}",
        f"- Mean saturation: {payload['texture']['mean_saturation']:.4f}",
        f"- Best k by lowest within-cluster variance: {payload['texture']['best_k']}",
        "",
        exp1.dominant_table(payload["texture"]["dominant_colors_32_top5"]),
        "",
        exp1.kmeans_table(payload["texture"]["kmeans"]),
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
        exp1.kmeans_table(payload["silhouette"]["kmeans"]),
        "",
    ]
    return lines


def build_comparison_table(rows: list[tuple[str, dict]]) -> list[str]:
    lines = [
        "## Comparison table",
        "",
        "| Variant | Source mean luminance | Output mean luminance | Luminance delta | "
        "Source mean saturation | Output mean saturation | Saturation delta | "
        "Output dominant color count | Silhouette cluster count |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for variant, payload in rows:
        source = payload["source"]
        texture = payload["texture"]
        silhouette = payload["silhouette"]
        lum_delta = texture["mean_luminance"] - source["mean_luminance"]
        sat_delta = texture["mean_saturation"] - source["mean_saturation"]
        lines.append(
            f"| {variant} | {source['mean_luminance']:.4f} | {texture['mean_luminance']:.4f} | {lum_delta:+.4f} | "
            f"{source['mean_saturation']:.4f} | {texture['mean_saturation']:.4f} | {sat_delta:+.4f} | "
            f"{texture['best_k']} | {silhouette['best_k']} |"
        )
    return lines


def build_observations(rows: list[tuple[str, dict]], exp2_analysis: dict) -> list[str]:
    ratios = {
        variant: payload["texture"]["mean_luminance"] / payload["source"]["mean_luminance"]
        for variant, payload in rows
    }
    d = exp2_analysis["D"]
    e = exp2_analysis["E"]
    d_ratio = ratios["D"]
    exp1_ratio_text = ", ".join(f"{variant}: {ratios[variant]:.3f}" for variant in ("A", "B", "C"))
    e_k2 = e["texture"]["kmeans"]["2"]["centroids"]
    e_source_k2 = e["source"]["kmeans"]["2"]["centroids"]
    return [
        "## Observations",
        "",
        f"- Variant D output/source luminance scaling factor is {d_ratio:.3f} "
        f"({d['texture']['mean_luminance']:.4f} / {d['source']['mean_luminance']:.4f}). "
        f"Experiment 1 scaling factors were {exp1_ratio_text}. The over-bright source did push the absolute output "
        f"luminance to {d['texture']['mean_luminance']:.4f}, but the proportional retention stayed in the same band as "
        "the goblin readability variants rather than removing the luminance drop entirely.",
        f"- Variant E texture k=2 centroids are {', '.join(f'`{color}`' for color in e_k2)}. "
        f"The requested source colors were `#E8D5A8` and `#D63A3A`; the source image's own k=2 centroids were "
        f"{', '.join(f'`{color}`' for color in e_source_k2)}. The red cap remained a strong cluster, while the cream body "
        "shifted darker/tanner in the baked texture, so the duo-color discipline mostly generalized but did not preserve exact color targets.",
        "- Variant E also introduced small facial, spot, and body-mark detail despite the two-color prompt, so its output carries more local color complexity than the intended strict organic two-color test. Variant D inherited the same eye/tusk exceptions seen in Variant A but otherwise remained a clean two-material goblin.",
    ]


def write_report(analysis: dict) -> None:
    config_block = experiment1_config_block()
    rows = comparison_rows(analysis)
    lines = [
        "# Pixal3D Visibility Characterization - Experiment 2",
        "",
        "## Pixal3D configuration",
        "",
        "Configuration is copied from Experiment 1 and is identical, including `X-Seed: 1337`.",
        "",
        "```yaml",
        config_block,
        "```",
        "",
    ]
    for variant in ("D", "E"):
        lines.extend(build_variant_section(variant, analysis[variant]))
    lines.extend(build_comparison_table(rows))
    lines.extend([""])
    lines.extend(build_observations(rows, analysis))
    lines.append("")
    (ROOT / "Report.md").write_text("\n".join(lines), encoding="utf-8", newline="\n")
    write_generation_log(config_block, analysis)


def main() -> None:
    analysis = analyze_variants()
    write_report(analysis)


if __name__ == "__main__":
    main()
