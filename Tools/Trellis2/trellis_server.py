"""
TRELLIS.2 API server for the T66 character-generation workflow.

This is the canonical server template that should be copied to the RunPod
workspace as `/workspace/TRELLIS.2/trellis_server.py`.
"""

import io
import os
import time

import cv2
import torch
from PIL import Image
from flask import Flask, jsonify, request, send_file

from trellis2.pipelines import Trellis2ImageTo3DPipeline
from trellis2.renderers import EnvMap
import o_voxel


os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"
os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "expandable_segments:True"

app = Flask(__name__)

print("[TRELLIS] Loading pipeline...")
pipeline = Trellis2ImageTo3DPipeline.from_pretrained("microsoft/TRELLIS.2-4B")
pipeline.cuda()
print("[TRELLIS] Pipeline loaded!")

envmap = EnvMap(
    torch.tensor(
        cv2.cvtColor(
            cv2.imread("assets/hdri/forest.exr", cv2.IMREAD_UNCHANGED),
            cv2.COLOR_BGR2RGB,
        ),
        dtype=torch.float32,
        device="cuda",
    )
)
print("[TRELLIS] Environment map loaded!")


@app.route("/health", methods=["GET"])
def health():
    return jsonify(
        {
            "status": "ok",
            "gpu": torch.cuda.get_device_name(0),
            "vram_total_gb": round(
                torch.cuda.get_device_properties(0).total_memory / 1e9, 1
            ),
            "vram_used_gb": round(torch.cuda.memory_allocated() / 1e9, 1),
        }
    )


@app.route("/generate", methods=["POST"])
def generate():
    start_time = time.time()

    image_data = request.get_data()
    if not image_data:
        return jsonify({"error": "No image data received"}), 400

    image = Image.open(io.BytesIO(image_data)).convert("RGBA")

    seed = int(request.headers.get("X-Seed", str(torch.randint(0, 2**31, (1,)).item())))
    texture_size = int(request.headers.get("X-Texture-Size", "2048"))
    decimation = int(request.headers.get("X-Decimation", "200000"))

    print(
        f"[TRELLIS] Generating: seed={seed}, tex={texture_size}, decim={decimation}"
    )

    outputs, latents = pipeline.run(
        image,
        seed=seed,
        preprocess_image=True,
        return_latent=True,
        sparse_structure_sampler_params={
            "steps": 20,
        },
        shape_slat_sampler_params={
            "steps": 20,
        },
        tex_slat_sampler_params={
            "steps": 20,
            "guidance_strength": 2.0,
        },
    )
    mesh = outputs[0]
    res = latents[2]
    mesh.simplify(16777216)

    job_id = f"{int(time.time())}_{seed}"
    tmp_path = f"/tmp/{job_id}.glb"

    glb = o_voxel.postprocess.to_glb(
        vertices=mesh.vertices,
        faces=mesh.faces,
        attr_volume=mesh.attrs,
        coords=mesh.coords,
        attr_layout=pipeline.pbr_attr_layout,
        grid_size=res,
        aabb=[[-0.5, -0.5, -0.5], [0.5, 0.5, 0.5]],
        decimation_target=decimation,
        texture_size=texture_size,
        remesh=True,
        remesh_band=1,
        remesh_project=0,
        verbose=True,
    )
    glb.export(tmp_path, extension_webp=True)
    torch.cuda.empty_cache()

    elapsed = time.time() - start_time
    print(f"[TRELLIS] Done in {elapsed:.1f}s")

    response = send_file(
        tmp_path,
        mimetype="model/gltf-binary",
        as_attachment=True,
        download_name=f"trellis_{job_id}.glb",
    )

    @response.call_on_close
    def cleanup():
        try:
            os.remove(tmp_path)
        except OSError:
            pass

    return response


if __name__ == "__main__":
    print("[TRELLIS] Starting server on port 8000...")
    app.run(host="0.0.0.0", port=8000, threaded=False)
