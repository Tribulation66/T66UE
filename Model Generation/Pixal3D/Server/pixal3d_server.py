"""
Pixal3D API server for the T66 model-generation production pipeline.

This server mirrors the existing T66 TRELLIS server shape:

- GET /health
- POST /generate with raw image bytes and X-* tuning headers

It is intended to live on RunPod at /workspace/Pixal3D/pixal3d_server.py.
"""

from __future__ import annotations

import io
import json
import math
import os
import subprocess
import sys
import tempfile
import time
import traceback
from pathlib import Path

os.environ.setdefault("OPENCV_IO_ENABLE_OPENEXR", "1")
os.environ.setdefault("PYTORCH_CUDA_ALLOC_CONF", "expandable_segments:True")
os.environ.setdefault("ATTN_BACKEND", os.environ.get("PIXAL3D_ATTN_BACKEND", "flash_attn_3"))
os.environ.setdefault("SPARSE_ATTN_BACKEND", os.environ.get("PIXAL3D_SPARSE_ATTN_BACKEND", os.environ["ATTN_BACKEND"]))
os.environ.setdefault("SPARSE_CONV_BACKEND", os.environ.get("PIXAL3D_SPARSE_CONV_BACKEND", "flex_gemm"))
os.environ.setdefault(
    "FLEX_GEMM_AUTOTUNE_CACHE_PATH",
    str(Path(__file__).resolve().with_name("autotune_cache.json")),
)
os.environ.setdefault("FLEX_GEMM_AUTOTUNER_VERBOSE", "1")

import cv2  # noqa: E402
import numpy as np  # noqa: E402
import o_voxel  # noqa: E402
import torch  # noqa: E402
from flask import Flask, jsonify, request, send_file  # noqa: E402
from PIL import Image  # noqa: E402

from pixal3d.pipelines import Pixal3DImageTo3DPipeline  # noqa: E402


MODEL_PATH = os.environ.get("PIXAL3D_MODEL_PATH", "TencentARC/Pixal3D")
MOGE_MODEL_NAME = os.environ.get("PIXAL3D_MOGE_MODEL", "Ruicheng/moge-2-vitl")
PORT = int(os.environ.get("PIXAL3D_PORT", "18001"))
LOW_VRAM = os.environ.get("PIXAL3D_LOW_VRAM", "0").strip().lower() in {"1", "true", "yes"}
EAGER_LOAD = os.environ.get("PIXAL3D_EAGER_LOAD", "1").strip().lower() not in {"0", "false", "no"}

IMAGE_COND_CONFIGS = {
    "ss": {
        "model_name": "camenduru/dinov3-vitl16-pretrain-lvd1689m",
        "image_size": 512,
        "grid_resolution": 16,
    },
    "shape_512": {
        "model_name": "camenduru/dinov3-vitl16-pretrain-lvd1689m",
        "image_size": 512,
        "grid_resolution": 32,
        "use_naf_upsample": True,
        "naf_target_size": 512,
    },
    "shape_1024": {
        "model_name": "camenduru/dinov3-vitl16-pretrain-lvd1689m",
        "image_size": 1024,
        "grid_resolution": 64,
        "use_naf_upsample": True,
        "naf_target_size": 512,
    },
    "tex_1024": {
        "model_name": "camenduru/dinov3-vitl16-pretrain-lvd1689m",
        "image_size": 1024,
        "grid_resolution": 64,
        "use_naf_upsample": True,
        "naf_target_size": 1024,
    },
}

app = Flask(__name__)
pipeline = None
moge_model = None
loaded_at = None


def header_int(name: str, default: int, min_value: int | None = None, max_value: int | None = None) -> int:
    value = int(request.headers.get(name, str(default)))
    if min_value is not None:
        value = max(min_value, value)
    if max_value is not None:
        value = min(max_value, value)
    return value


def header_float(name: str, default: float, min_value: float | None = None, max_value: float | None = None) -> float:
    value = float(request.headers.get(name, str(default)))
    if min_value is not None:
        value = max(min_value, value)
    if max_value is not None:
        value = min(max_value, value)
    return value


def header_bool(name: str, default: bool) -> bool:
    value = request.headers.get(name)
    if value is None:
        return default
    return value.strip().lower() in {"1", "true", "yes", "on"}


def build_image_cond_model(config: dict):
    from pixal3d.trainers.flow_matching.mixins.image_conditioned_proj import DinoV3ProjFeatureExtractor

    model = DinoV3ProjFeatureExtractor(**config)
    model.eval()
    return model


def load_moge_model(device: str = "cuda", model_name: str = MOGE_MODEL_NAME):
    from moge.model.v2 import MoGeModel

    model = MoGeModel.from_pretrained(model_name).to(device)
    model.eval()
    return model


def init_models() -> None:
    global loaded_at, moge_model, pipeline
    if pipeline is not None and moge_model is not None:
        return

    print(f"[PIXAL3D] Loading pipeline from {MODEL_PATH} low_vram={LOW_VRAM}", flush=True)
    pipe = Pixal3DImageTo3DPipeline.from_pretrained(MODEL_PATH)

    print("[PIXAL3D] Building image-conditioning models", flush=True)
    pipe.image_cond_model_ss = build_image_cond_model(IMAGE_COND_CONFIGS["ss"])
    pipe.image_cond_model_shape_512 = build_image_cond_model(IMAGE_COND_CONFIGS["shape_512"])
    pipe.image_cond_model_shape_1024 = build_image_cond_model(IMAGE_COND_CONFIGS["shape_1024"])
    pipe.image_cond_model_tex_1024 = build_image_cond_model(IMAGE_COND_CONFIGS["tex_1024"])

    pipe.low_vram = LOW_VRAM
    pipe.cuda()
    if not LOW_VRAM:
        pipe.image_cond_model_ss.cuda()
        pipe.image_cond_model_shape_512.cuda()
        pipe.image_cond_model_shape_1024.cuda()
        pipe.image_cond_model_tex_1024.cuda()

    print("[PIXAL3D] Preloading NAF upsamplers", flush=True)
    for attr in [
        "image_cond_model_ss",
        "image_cond_model_shape_512",
        "image_cond_model_shape_1024",
        "image_cond_model_tex_1024",
    ]:
        model = getattr(pipe, attr, None)
        if model is not None and getattr(model, "use_naf_upsample", False):
            model._load_naf()

    print(f"[PIXAL3D] Loading MoGe camera model {MOGE_MODEL_NAME}", flush=True)
    moge = load_moge_model(device="cuda")

    pipeline = pipe
    moge_model = moge
    loaded_at = time.time()
    print("[PIXAL3D] Models loaded", flush=True)


def compute_f_pixels(camera_angle_x: float, resolution: int) -> float:
    focal_length = 16.0 / torch.tan(torch.tensor(camera_angle_x / 2.0))
    f_pixels = focal_length * resolution / 32.0
    return float(f_pixels.item())


def distance_from_fov(camera_angle_x, grid_point, target_point, mesh_scale, image_resolution):
    rotation_matrix = torch.tensor(
        [[1.0, 0.0, 0.0], [0.0, 0.0, -1.0], [0.0, 1.0, 0.0]],
        device=grid_point.device,
    )
    gp = grid_point.to(torch.float32) @ rotation_matrix.T
    gp = gp / mesh_scale / 2
    xw, yw = gp[0].item(), gp[1].item()
    xt = float(target_point[0].item())
    f_pixels = compute_f_pixels(camera_angle_x, image_resolution)
    x_ndc = xt - image_resolution / 2.0
    distance_x = f_pixels * xw / x_ndc - yw
    return float(distance_x)


def camera_params_from_image(image_path: str, mesh_scale: float, extend_pixel: int, image_resolution: int) -> dict:
    pil_image = Image.open(image_path).convert("RGB")
    width, _height = pil_image.size
    image_np = np.array(pil_image).astype(np.float32) / 255.0
    image_tensor = torch.from_numpy(image_np).permute(2, 0, 1).to("cuda")
    with torch.no_grad():
        output = moge_model.infer(image_tensor)
    intrinsics = output["intrinsics"].squeeze().cpu().numpy()
    fx_normalized = intrinsics[0, 0]
    fx = fx_normalized * width
    camera_angle_x = 2 * math.atan(width / (2 * fx))
    distance = distance_from_fov(
        camera_angle_x,
        torch.tensor([-1.0, 0.0, 0.0], device="cuda"),
        torch.tensor([0 - extend_pixel, image_resolution - 1 + extend_pixel], device="cuda"),
        mesh_scale,
        image_resolution,
    )
    return {"camera_angle_x": camera_angle_x, "distance": distance, "mesh_scale": mesh_scale}


def gpu_payload() -> dict:
    if not torch.cuda.is_available():
        return {"cuda": False}
    return {
        "cuda": True,
        "gpu": torch.cuda.get_device_name(0),
        "vram_total_gb": round(torch.cuda.get_device_properties(0).total_memory / 1e9, 1),
        "vram_allocated_gb": round(torch.cuda.memory_allocated() / 1e9, 1),
        "vram_reserved_gb": round(torch.cuda.memory_reserved() / 1e9, 1),
    }


@app.route("/health", methods=["GET"])
def health():
    return jsonify(
        {
            "status": "ok",
            "pipeline_loaded": pipeline is not None,
            "loaded_at": loaded_at,
            "model_path": MODEL_PATH,
            "low_vram": LOW_VRAM,
            "attention_backend": os.environ.get("ATTN_BACKEND"),
            "sparse_attention_backend": os.environ.get("SPARSE_ATTN_BACKEND"),
            "sparse_conv_backend": os.environ.get("SPARSE_CONV_BACKEND"),
            **gpu_payload(),
        }
    )


def tensor_to_cpu(value):
    if isinstance(value, torch.Tensor):
        return value.detach().cpu()
    return value


def grid_size_payload(value):
    if isinstance(value, torch.Tensor):
        flat = value.detach().cpu().reshape(-1).tolist()
        if len(flat) == 1:
            return int(flat[0])
        return [int(item) for item in flat]
    if isinstance(value, np.ndarray):
        flat = value.reshape(-1).tolist()
        if len(flat) == 1:
            return int(flat[0])
        return [int(item) for item in flat]
    if isinstance(value, (list, tuple)):
        if len(value) == 1:
            return int(value[0])
        return [int(item) for item in value]
    return int(value)


def should_skip_cumesh_fill_holes_error(exc: RuntimeError) -> bool:
    text = str(exc).lower()
    if "no kernel image" in text:
        return False
    if not any(marker in text for marker in ("cumesh", "cuda error", "out of memory")):
        return False
    return any(
        marker in text
        for marker in (
            "invalid configuration argument",
            "illegal memory access",
            "out of memory",
            "error code: 9",
            "clean_up.cu",
            "connectivity.cu",
        )
    )


def install_safe_cumesh_fill_holes() -> dict:
    state = {
        "enabled": False,
        "patched_targets": [],
        "skipped": 0,
        "errors": [],
        "cpu_uv_unwraps": 0,
        "uv_cleanup_skipped": 0,
    }
    try:
        import cumesh  # noqa: PLC0415
        from cumesh.xatlas import Atlas  # noqa: PLC0415
    except Exception as exc:
        state["install_error"] = str(exc)
        return state

    for attr_name in ("CuMesh", "Mesh"):
        mesh_cls = getattr(cumesh, attr_name, None)
        original = getattr(mesh_cls, "fill_holes", None) if mesh_cls is not None else None
        if not callable(original):
            continue

        def safe_fill_holes(self, *args, _original=original, _target=attr_name, **kwargs):
            try:
                return _original(self, *args, **kwargs)
            except RuntimeError as exc:
                if not should_skip_cumesh_fill_holes_error(exc):
                    raise
                state["skipped"] += 1
                message = str(exc)
                state["errors"].append(message[-2000:])
                print(
                    "[PIXAL3D] Safe CuMesh fill_holes fallback skipped "
                    f"{_target}.fill_holes after RuntimeError: {message[-500:]}",
                    flush=True,
                )
                return None

        setattr(mesh_cls, "fill_holes", safe_fill_holes)
        state["patched_targets"].append(f"{attr_name}.fill_holes")

    cu_mesh_cls = getattr(cumesh, "CuMesh", None)
    if callable(getattr(cu_mesh_cls, "uv_unwrap", None)):
        def cpu_xatlas_uv_unwrap(
            self,
            compute_charts_kwargs: dict = {},
            xatlas_compute_charts_kwargs: dict = {},
            xatlas_pack_charts_kwargs: dict = {},
            return_vmaps: bool = False,
            verbose: bool = False,
        ):
            try:
                self.remove_degenerate_faces()
            except RuntimeError as exc:
                if not should_skip_cumesh_fill_holes_error(exc):
                    raise
                state["uv_cleanup_skipped"] += 1
                state["errors"].append(str(exc)[-2000:])
                print(
                    "[PIXAL3D] Safe CuMesh fallback skipped remove_degenerate_faces "
                    f"after RuntimeError: {str(exc)[-500:]}",
                    flush=True,
                )

            current_vertices, current_faces = self.read()
            current_vertices = current_vertices.float().cpu().contiguous()
            current_faces = current_faces.int().cpu().contiguous()
            atlas = Atlas()
            atlas.add_mesh(current_vertices, current_faces)
            chart_kwargs = dict(xatlas_compute_charts_kwargs)
            pack_kwargs = dict(xatlas_pack_charts_kwargs)
            chart_kwargs.setdefault("verbose", verbose)
            pack_kwargs.setdefault("verbose", verbose)
            atlas.compute_charts(**chart_kwargs)
            atlas.pack_charts(**pack_kwargs)
            vmaps, out_faces, out_uvs = atlas.get_mesh(0)
            vmaps = vmaps.long().cpu().contiguous()
            out = [
                current_vertices[vmaps].contiguous(),
                out_faces.int().cpu().contiguous(),
                out_uvs.float().cpu().contiguous(),
            ]
            if return_vmaps:
                out.append(vmaps)
            state["cpu_uv_unwraps"] += 1
            print("[PIXAL3D] Safe CuMesh fallback used CPU xatlas UV unwrap", flush=True)
            return tuple(out)

        setattr(cu_mesh_cls, "uv_unwrap", cpu_xatlas_uv_unwrap)
        state["patched_targets"].append("CuMesh.uv_unwrap_cpu_xatlas")

    state["enabled"] = bool(state["patched_targets"])
    return state


def export_glb_direct(package_path: str, output_path: str, export_params: dict):
    package = torch.load(package_path, map_location="cpu", weights_only=False)
    vertices = package["vertices"].cuda()
    faces = package["faces"].cuda()
    attr_volume = package["attr_volume"].cuda()
    coords = package["coords"].cuda()
    safe_fill_holes_state = None
    if export_params.get("safe_fill_holes", False):
        safe_fill_holes_state = install_safe_cumesh_fill_holes()

    glb = o_voxel.postprocess.to_glb(
        vertices=vertices,
        faces=faces,
        attr_volume=attr_volume,
        coords=coords,
        attr_layout=package["attr_layout"],
        grid_size=package["grid_size"],
        aabb=package["aabb"],
        decimation_target=export_params["decimation"],
        texture_size=export_params["texture_size"],
        remesh=export_params["remesh"],
        remesh_band=export_params["remesh_band"],
        remesh_project=export_params["remesh_project"],
        use_tqdm=True,
    )

    rot = np.array(
        [
            [-1, 0, 0, 0],
            [0, 0, -1, 0],
            [0, -1, 0, 0],
            [0, 0, 0, 1],
        ],
        dtype=np.float64,
    )
    glb.apply_transform(rot)
    glb.export(output_path, extension_webp=True)

    report = {
        "decimation": export_params["decimation"],
        "remesh": export_params["remesh"],
        "safe_fill_holes": bool(export_params.get("safe_fill_holes", False)),
        "fill_holes_skipped": int((safe_fill_holes_state or {}).get("skipped", 0)),
        "fill_holes_patch_targets": (safe_fill_holes_state or {}).get("patched_targets", []),
        "fill_holes_errors": (safe_fill_holes_state or {}).get("errors", []),
        "cpu_uv_unwraps": int((safe_fill_holes_state or {}).get("cpu_uv_unwraps", 0)),
        "uv_cleanup_skipped": int((safe_fill_holes_state or {}).get("uv_cleanup_skipped", 0)),
        "output_path": output_path,
        "output_bytes": os.path.getsize(output_path),
    }

    del vertices, faces, attr_volume, coords, glb, package
    torch.cuda.empty_cache()
    return report


def export_worker_main(argv: list[str]) -> int:
    if len(argv) != 4:
        print("Usage: pixal3d_server.py --export-glb-worker PACKAGE OUTPUT PARAMS_JSON", file=sys.stderr)
        return 2
    package_path, output_path, params_json = argv[1], argv[2], argv[3]
    try:
        report = export_glb_direct(package_path, output_path, json.loads(params_json))
        print(json.dumps(report), flush=True)
        return 0
    except Exception:
        traceback.print_exc()
        try:
            torch.cuda.empty_cache()
        except Exception:
            pass
        return 1


def run_export_worker(package_path: str, output_path: str, export_params: dict, timeout_seconds: int) -> dict:
    command = [
        sys.executable,
        str(Path(__file__).resolve()),
        "--export-glb-worker",
        package_path,
        output_path,
        json.dumps(export_params, sort_keys=True),
    ]
    result = subprocess.run(
        command,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=timeout_seconds,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "GLB export worker failed "
            f"(exit={result.returncode}, decimation={export_params['decimation']}, "
            f"remesh={export_params['remesh']}, "
            f"safe_fill_holes={export_params.get('safe_fill_holes', False)}):\n"
            f"{result.stdout}"
        )
    if not os.path.exists(output_path) or os.path.getsize(output_path) <= 0:
        raise RuntimeError(
            "GLB export worker completed without a usable output "
            f"(decimation={export_params['decimation']}, remesh={export_params['remesh']}, "
            f"safe_fill_holes={export_params.get('safe_fill_holes', False)}):\n"
            f"{result.stdout}"
        )
    lines = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    for line in reversed(lines):
        try:
            return json.loads(line)
        except json.JSONDecodeError:
            continue
    return {
        "decimation": export_params["decimation"],
        "remesh": export_params["remesh"],
        "safe_fill_holes": bool(export_params.get("safe_fill_holes", False)),
        "fill_holes_skipped": 0,
        "cpu_uv_unwraps": 0,
        "uv_cleanup_skipped": 0,
        "output_path": output_path,
        "output_bytes": os.path.getsize(output_path),
    }


def export_attempt_plan(
    decimation: int,
    remesh: bool,
    remesh_band: float,
    remesh_project: float,
    texture_size: int,
    fallback_decimation: int,
    fallback_enabled: bool,
    safe_fill_holes_fallback: bool,
) -> list[dict]:
    base = {
        "texture_size": texture_size,
        "remesh_band": remesh_band,
        "remesh_project": remesh_project,
        "safe_fill_holes": False,
    }
    attempts = [{**base, "label": "requested", "decimation": decimation, "remesh": remesh}]
    if fallback_enabled:
        safe_decimation = min(decimation, fallback_decimation)
        fallback_candidates = [
            {**base, "label": "safe_decimation", "decimation": safe_decimation, "remesh": remesh},
            {**base, "label": "safe_decimation_no_remesh", "decimation": safe_decimation, "remesh": False},
        ]
        if safe_fill_holes_fallback:
            fallback_candidates.append(
                {
                    **base,
                    "label": "safe_decimation_no_remesh_skip_fill_holes",
                    "decimation": safe_decimation,
                    "remesh": False,
                    "safe_fill_holes": True,
                }
            )
        seen = {(attempts[0]["decimation"], attempts[0]["remesh"], attempts[0]["safe_fill_holes"])}
        for candidate in fallback_candidates:
            key = (candidate["decimation"], candidate["remesh"], candidate["safe_fill_holes"])
            if key not in seen:
                attempts.append(candidate)
                seen.add(key)
    return attempts


@app.route("/generate", methods=["POST"])
def generate():
    start_time = time.time()
    try:
        init_models()
        image_data = request.get_data()
        if not image_data:
            return jsonify({"error": "No image data received"}), 400

        seed = header_int("X-Seed", int(torch.randint(0, 2**31, (1,)).item()))
        texture_size = header_int("X-Texture-Size", 2048, 128, 4096)
        decimation = header_int("X-Decimation", 200000, 1000, 2000000)
        remesh = header_bool("X-Remesh", True)
        remesh_band = header_float("X-Remesh-Band", 1.0, 0.1, 8.0)
        remesh_project = header_float("X-Remesh-Project", 0.0, 0.0, 1.0)
        export_fallback = header_bool("X-Export-Fallback", True)
        fallback_decimation = header_int("X-Fallback-Decimation", 80000, 1000, 2000000)
        safe_fill_holes_fallback = header_bool("X-Safe-Fill-Holes-Fallback", True)
        export_timeout = header_int("X-Export-Timeout", 900, 60, 7200)
        resolution = header_int("X-Resolution", 1024)
        if resolution not in {1024, 1536}:
            return jsonify({"error": "X-Resolution must be 1024 or 1536"}), 400

        mesh_scale = header_float("X-Mesh-Scale", 1.0, 0.05, 10.0)
        extend_pixel = header_int("X-Extend-Pixel", 0, -512, 512)
        image_resolution = header_int("X-Image-Resolution", 512, 128, 2048)
        max_num_tokens = header_int("X-Max-Num-Tokens", 49152, 4096, 131072)

        ss_sampler = {
            "steps": header_int("X-SS-Steps", 12, 1, 80),
            "guidance_strength": header_float("X-SS-Guidance", 7.5, 0.0, 20.0),
            "guidance_rescale": header_float("X-SS-Guidance-Rescale", 0.7, 0.0, 2.0),
            "rescale_t": header_float("X-SS-Rescale-T", 5.0, 0.0, 20.0),
        }
        shape_sampler = {
            "steps": header_int("X-Shape-Steps", 12, 1, 80),
            "guidance_strength": header_float("X-Shape-Guidance", 7.5, 0.0, 20.0),
            "guidance_rescale": header_float("X-Shape-Guidance-Rescale", 0.5, 0.0, 2.0),
            "rescale_t": header_float("X-Shape-Rescale-T", 3.0, 0.0, 20.0),
        }
        tex_sampler = {
            "steps": header_int("X-Tex-Steps", 12, 1, 80),
            "guidance_strength": header_float("X-Tex-Guidance", 1.0, 0.0, 20.0),
            "guidance_rescale": header_float("X-Tex-Guidance-Rescale", 0.0, 0.0, 2.0),
            "rescale_t": header_float("X-Tex-Rescale-T", 3.0, 0.0, 20.0),
        }

        print(
            "[PIXAL3D] Generating "
            f"seed={seed} resolution={resolution} texture={texture_size} decimation={decimation} "
            f"remesh={remesh} remesh_band={remesh_band} remesh_project={remesh_project} "
            f"export_fallback={export_fallback} fallback_decimation={fallback_decimation} "
            f"safe_fill_holes_fallback={safe_fill_holes_fallback} "
            f"ss={ss_sampler} shape={shape_sampler} tex={tex_sampler}",
            flush=True,
        )

        image = Image.open(io.BytesIO(image_data))
        image_preprocessed = pipeline.preprocess_image(image)

        tmp_preprocessed = tempfile.NamedTemporaryFile(prefix="pixal3d_preprocessed_", suffix=".png", delete=False)
        tmp_preprocessed.close()
        image_preprocessed.save(tmp_preprocessed.name)
        try:
            camera_params = camera_params_from_image(
                tmp_preprocessed.name,
                mesh_scale=mesh_scale,
                extend_pixel=extend_pixel,
                image_resolution=image_resolution,
            )
        finally:
            try:
                os.remove(tmp_preprocessed.name)
            except OSError:
                pass

        torch.manual_seed(seed)
        mesh_list, (shape_slat, tex_slat, res) = pipeline.run(
            image_preprocessed,
            camera_params=camera_params,
            seed=seed,
            sparse_structure_sampler_params=ss_sampler,
            shape_slat_sampler_params=shape_sampler,
            tex_slat_sampler_params=tex_sampler,
            preprocess_image=False,
            return_latent=True,
            pipeline_type=f"{resolution}_cascade",
            max_num_tokens=max_num_tokens,
        )
        mesh = mesh_list[0]

        job_id = f"{int(time.time())}_{seed}"
        tmp_path = f"/tmp/pixal3d_{job_id}.glb"

        export_package = tempfile.NamedTemporaryFile(prefix=f"pixal3d_{job_id}_export_", suffix=".pt", delete=False)
        export_package.close()
        torch.save(
            {
                "vertices": tensor_to_cpu(mesh.vertices),
                "faces": tensor_to_cpu(mesh.faces),
                "attr_volume": tensor_to_cpu(mesh.attrs),
                "coords": tensor_to_cpu(mesh.coords),
                "attr_layout": pipeline.pbr_attr_layout,
                "grid_size": grid_size_payload(res),
                "aabb": [[-0.5, -0.5, -0.5], [0.5, 0.5, 0.5]],
            },
            export_package.name,
        )

        del mesh_list, shape_slat, tex_slat, mesh
        torch.cuda.empty_cache()

        export_attempts = []
        export_report = None
        attempt_plan = export_attempt_plan(
            decimation=decimation,
            remesh=remesh,
            remesh_band=remesh_band,
            remesh_project=remesh_project,
            texture_size=texture_size,
            fallback_decimation=fallback_decimation,
            fallback_enabled=export_fallback,
            safe_fill_holes_fallback=safe_fill_holes_fallback,
        )
        try:
            for index, attempt in enumerate(attempt_plan, start=1):
                try:
                    print(
                        "[PIXAL3D] Export attempt "
                        f"{index}/{len(attempt_plan)} label={attempt['label']} "
                        f"decimation={attempt['decimation']} remesh={attempt['remesh']} "
                        f"safe_fill_holes={attempt['safe_fill_holes']}",
                        flush=True,
                    )
                    export_report = run_export_worker(export_package.name, tmp_path, attempt, export_timeout)
                    export_report["label"] = attempt["label"]
                    export_report["attempt_index"] = index
                    export_attempts.append({**export_report, "status": "ok"})
                    break
                except Exception as export_exc:
                    message = str(export_exc)
                    print(f"[PIXAL3D] Export attempt failed: {message}", flush=True)
                    export_attempts.append(
                        {
                            "label": attempt["label"],
                            "decimation": attempt["decimation"],
                            "remesh": attempt["remesh"],
                            "safe_fill_holes": attempt["safe_fill_holes"],
                            "status": "failed",
                            "error": message[-4000:],
                        }
                    )
                    try:
                        torch.cuda.empty_cache()
                    except Exception:
                        pass
            if export_report is None:
                return jsonify({"error": "All GLB export attempts failed", "export_attempts": export_attempts}), 500
        finally:
            try:
                os.remove(export_package.name)
            except OSError:
                pass

        elapsed = time.time() - start_time
        print(
            "[PIXAL3D] Done "
            f"in {elapsed:.1f}s -> {tmp_path} "
            f"export_decimation={export_report['decimation']} export_remesh={export_report['remesh']} "
            f"export_label={export_report['label']} "
            f"safe_fill_holes={export_report.get('safe_fill_holes', False)} "
            f"fill_holes_skipped={export_report.get('fill_holes_skipped', 0)} "
            f"cpu_uv_unwraps={export_report.get('cpu_uv_unwraps', 0)}",
            flush=True,
        )

        response = send_file(
            tmp_path,
            mimetype="model/gltf-binary",
            as_attachment=True,
            download_name=f"pixal3d_{job_id}.glb",
        )
        response.headers["X-Pixal3D-Export-Attempt"] = str(export_report["attempt_index"])
        response.headers["X-Pixal3D-Export-Label"] = str(export_report["label"])
        response.headers["X-Pixal3D-Export-Decimation"] = str(export_report["decimation"])
        response.headers["X-Pixal3D-Export-Remesh"] = "1" if export_report["remesh"] else "0"
        response.headers["X-Pixal3D-Export-Attempts"] = str(len(export_attempts))
        response.headers["X-Pixal3D-Export-Safe-Fill-Holes"] = (
            "1" if export_report.get("safe_fill_holes", False) else "0"
        )
        response.headers["X-Pixal3D-Export-Fill-Holes-Skipped"] = str(
            export_report.get("fill_holes_skipped", 0)
        )
        response.headers["X-Pixal3D-Export-CPU-UV-Unwraps"] = str(
            export_report.get("cpu_uv_unwraps", 0)
        )

        @response.call_on_close
        def cleanup():
            try:
                os.remove(tmp_path)
            except OSError:
                pass

        return response
    except Exception as exc:
        traceback.print_exc()
        try:
            torch.cuda.empty_cache()
        except Exception:
            print("[PIXAL3D] CUDA cleanup failed after generation error", flush=True)
        return jsonify({"error": str(exc), "traceback": traceback.format_exc()}), 500


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--export-glb-worker":
        raise SystemExit(export_worker_main(sys.argv[1:]))
    if EAGER_LOAD:
        init_models()
    print(f"[PIXAL3D] Starting server on port {PORT}", flush=True)
    app.run(host="0.0.0.0", port=PORT, threaded=False)
