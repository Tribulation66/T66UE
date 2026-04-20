#!/usr/bin/env bash
set -euo pipefail

# Bootstrap a fresh RunPod pod into the locked TRELLIS.2 baseline used by T66.
# Run this script on the pod after SSH'ing in as root.
#
# Notes:
# - This script intentionally locks transformers==5.2.0.
# - It applies the rembg and cfg_strength fixes.
# - It writes trellis_server.py directly on the pod.
# - If HF_TOKEN is set, it will log into Hugging Face automatically.

POD_ROOT="/workspace"
REPO_DIR="${POD_ROOT}/TRELLIS.2"
CONDA_ROOT="/opt/conda"
ENV_NAME="trellis2"
LOCKED_COMMIT="5565d240c4a494caaf9ece7a554542b76ffa36d3"
SYSTEM_CUDA_HOME="/usr/local/cuda"

apt-get update
apt-get install -y wget curl git cmake libjpeg-dev zlib1g-dev pkg-config build-essential

if [[ ! -d "${CONDA_ROOT}" ]]; then
  wget -q https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O /tmp/miniforge.sh
  bash /tmp/miniforge.sh -b -p "${CONDA_ROOT}"
fi

source "${CONDA_ROOT}/etc/profile.d/conda.sh"

if [[ ! -d "${REPO_DIR}" ]]; then
  cd "${POD_ROOT}"
  git clone --recursive https://github.com/microsoft/TRELLIS.2.git
fi

cd "${REPO_DIR}"
git fetch origin
git checkout "${LOCKED_COMMIT}"
git submodule update --init --recursive

if ! conda env list | awk '{print $1}' | grep -qx "${ENV_NAME}"; then
  conda create -y -n "${ENV_NAME}" python=3.10
fi

conda activate "${ENV_NAME}"

python -m pip install --upgrade pip
python -m pip install --index-url https://download.pytorch.org/whl/cu124 torch==2.6.0 torchvision==0.21.0
python -m pip install imageio imageio-ffmpeg tqdm easydict opencv-python-headless ninja trimesh "transformers==5.2.0" gradio==6.0.1 tensorboard pandas lpips zstandard
python -m pip install git+https://github.com/EasternJournalist/utils3d.git@9a4eb15e4021b67b12c460c7057d642626897ec8
python -m pip install kornia timm flask onnxruntime rembg psutil

export NVCC_PREPEND_FLAGS=
export CUDA_HOME="${SYSTEM_CUDA_HOME}"
export PATH="${CUDA_HOME}/bin:${PATH}"
export PYTHONPATH="${REPO_DIR}"

cd "${REPO_DIR}"
rm -rf /tmp/extensions
. ./setup.sh --cumesh
. ./setup.sh --o-voxel
. ./setup.sh --flexgemm
. ./setup.sh --nvdiffrast
. ./setup.sh --nvdiffrec
python -m pip install flash-attn==2.7.3 --no-build-isolation

cat > "${REPO_DIR}/trellis2/pipelines/rembg/BiRefNet.py" <<'PYEOF'
from typing import *
from PIL import Image
from rembg import remove


class BiRefNet:
    def __init__(self, model_name: str = "ZhengPeng7/BiRefNet"):
        pass

    def to(self, device: str):
        return self

    def cuda(self):
        return self

    def cpu(self):
        return self

    def __call__(self, image: Image.Image) -> Image.Image:
        return remove(image)
PYEOF

python - <<'PY'
from pathlib import Path

path = Path("/workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py")
text = path.read_text()
old = '        return model(x_t, t, cond, **kwargs)'
new = '        kwargs.pop("cfg_strength", None); return model(x_t, t, cond, **kwargs)'
if new not in text:
    if old not in text:
        raise SystemExit("Expected flow_euler target line not found")
    text = text.replace(old, new, 1)
    path.write_text(text)
print("Patched", path)
PY

cat > "${REPO_DIR}/trellis_server.py" <<'PYEOF'
"""
TRELLIS.2 API server for the T66 character-generation workflow.
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
PYEOF

if [[ -n "${HF_TOKEN:-}" ]]; then
  python -c "from huggingface_hub import login; login(token='${HF_TOKEN}')"
else
  echo "HF_TOKEN not set; skipping Hugging Face login"
fi

python - <<'PY'
import transformers
import torch
import nvdiffrast
import cumesh
import o_voxel
import flash_attn
from trellis2.pipelines import Trellis2ImageTo3DPipeline

print("transformers", transformers.__version__)
print("torch", torch.__version__)
print("nvdiffrast OK")
print("cumesh OK")
print("o_voxel OK")
print("flash_attn OK")
print("pipeline import OK")
PY

echo
echo "Bootstrap complete."
echo "Start the server with:"
echo "source ${CONDA_ROOT}/etc/profile.d/conda.sh && conda activate ${ENV_NAME} && export NVCC_PREPEND_FLAGS= && export CUDA_HOME=${SYSTEM_CUDA_HOME} && export PATH=${SYSTEM_CUDA_HOME}/bin:\$PATH && export PYTHONPATH=${REPO_DIR} && cd ${REPO_DIR} && nohup python -u ${REPO_DIR}/trellis_server.py > ${REPO_DIR}/trellis_server.log 2>&1 < /dev/null &"
