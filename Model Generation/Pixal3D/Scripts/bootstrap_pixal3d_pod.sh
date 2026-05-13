#!/usr/bin/env bash
set -euo pipefail

# Bootstrap a RunPod pod into a separate Pixal3D environment for T66 research.
# Run this on the pod as root. It intentionally does not replace the existing
# TRELLIS.2 environment.

POD_ROOT="${POD_ROOT:-/workspace}"
REPO_DIR="${PIXAL3D_REPO_DIR:-${POD_ROOT}/Pixal3D}"
CONDA_ROOT="${CONDA_ROOT:-/opt/conda}"
ENV_NAME="${PIXAL3D_ENV_NAME:-pixal3d}"
LOCKED_COMMIT="${PIXAL3D_COMMIT:-d42dcaad99ba07d35a02fa62a23e1cd6f2f61da1}"

apt-get update
apt-get install -y wget curl git cmake libjpeg-dev zlib1g-dev pkg-config build-essential

if [[ ! -d "${CONDA_ROOT}" ]]; then
  wget -q https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O /tmp/miniforge.sh
  bash /tmp/miniforge.sh -b -p "${CONDA_ROOT}"
fi

source "${CONDA_ROOT}/etc/profile.d/conda.sh"

if ! conda env list | awk '{print $1}' | grep -qx "${ENV_NAME}"; then
  conda create -y -n "${ENV_NAME}" python=3.10 pip
fi

conda activate "${ENV_NAME}"

python -m pip install --upgrade pip

if [[ -d "${REPO_DIR}" && ! -d "${REPO_DIR}/.git" ]]; then
  BACKUP_DIR="${REPO_DIR}.preclone.$(date +%Y%m%d_%H%M%S)"
  echo "Existing non-git Pixal3D directory found; moving it to ${BACKUP_DIR}"
  mv "${REPO_DIR}" "${BACKUP_DIR}"
fi

if [[ ! -d "${REPO_DIR}/.git" ]]; then
  git clone https://github.com/TencentARC/Pixal3D.git "${REPO_DIR}"
fi

cd "${REPO_DIR}"
git fetch origin
git checkout "${LOCKED_COMMIT}"

# The upstream regular requirements assume a TRELLIS.2 base install. The
# demo requirements include the CUDA 12.4 wheels needed by a fresh pod.
python -m pip install -r requirements-hfdemo.txt
python -m pip install --force-reinstall --no-deps \
  https://github.com/LDYang694/Storages/releases/download/20260430/utils3d-0.0.2-py3-none-any.whl
python -m pip install flask psutil

DETECTED_CUDA_ARCH="$(python - <<'PY'
import torch
if torch.cuda.is_available():
    major, minor = torch.cuda.get_device_capability(0)
    print(f"{major}.{minor}")
PY
)"

NATTEN_SOURCE_ARCH="${PIXAL3D_NATTEN_SOURCE_ARCH:-}"
if [[ -z "${NATTEN_SOURCE_ARCH}" && "${DETECTED_CUDA_ARCH}" == "8.6" ]]; then
  # The upstream hfdemo wheel can miss A40/Ampere kernels. Building NATTEN once
  # for sm_86 fixes the NAF upsampler path used by Pixal3D.
  NATTEN_SOURCE_ARCH="8.6"
fi

if [[ -n "${NATTEN_SOURCE_ARCH}" && "${PIXAL3D_SKIP_NATTEN_SOURCE_BUILD:-0}" != "1" ]]; then
  echo "Building NATTEN from source for CUDA arch ${NATTEN_SOURCE_ARCH}"
  set +u
  conda install -y -c conda-forge \
    cuda-cudart-static_linux-64=12.4.127 \
    cuda-cudart-dev_linux-64=12.4.127 \
    cuda-cudart_linux-64=12.4.127 \
    cuda-crt=12.4.131 \
    cuda-crt-dev_linux-64=12.4.131 \
    cuda-nvcc=12.4.131
  set -u
  python -m pip install --upgrade cmake==4.1.0
  export CUDA_HOME="${CONDA_PREFIX}"
  export CUDA_PATH="${CONDA_PREFIX}"
  export PATH="${CONDA_PREFIX}/bin:${PATH}"
  export CPATH="${CONDA_PREFIX}/targets/x86_64-linux/include:${CPATH:-}"
  export LIBRARY_PATH="${CONDA_PREFIX}/targets/x86_64-linux/lib:${CONDA_PREFIX}/lib:${LIBRARY_PATH:-}"
  export LD_LIBRARY_PATH="${CONDA_PREFIX}/targets/x86_64-linux/lib:${CONDA_PREFIX}/lib:${LD_LIBRARY_PATH:-}"
  export NATTEN_CUDA_ARCH="${NATTEN_SOURCE_ARCH}"
  export NATTEN_N_WORKERS="${PIXAL3D_NATTEN_N_WORKERS:-16}"
  export MAX_JOBS="${PIXAL3D_NATTEN_N_WORKERS:-16}"
  python -m pip install --no-cache-dir --no-build-isolation --no-deps --no-binary=natten --force-reinstall "natten==0.21.0"
fi

if [[ -n "${HF_TOKEN:-}" ]]; then
  python -c "from huggingface_hub import login; login(token='${HF_TOKEN}')"
else
  echo "HF_TOKEN not set; skipping Hugging Face login"
fi

export PYTHONPATH="${REPO_DIR}"
export ATTN_BACKEND="${PIXAL3D_ATTN_BACKEND:-flash_attn_3}"
export SPARSE_ATTN_BACKEND="${PIXAL3D_SPARSE_ATTN_BACKEND:-${ATTN_BACKEND}}"
export SPARSE_CONV_BACKEND="${PIXAL3D_SPARSE_CONV_BACKEND:-flex_gemm}"
export OPENCV_IO_ENABLE_OPENEXR=1
export PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True

python - <<'PY'
import torch
import cv2
import o_voxel
import nvdiffrast
import nvdiffrec_render
import flex_gemm
import cumesh
import natten
import flash_attn_interface
from pixal3d.pipelines import Pixal3DImageTo3DPipeline
from moge.model.v2 import MoGeModel

print("torch", torch.__version__)
print("cuda", torch.version.cuda)
print("gpu", torch.cuda.get_device_name(0) if torch.cuda.is_available() else "none")
print("cv2 OK")
print("o_voxel OK")
print("nvdiffrast OK")
print("nvdiffrec_render OK")
print("flex_gemm OK")
print("cumesh OK")
print("natten OK")
print("flash_attn_interface OK")
print("pixal3d pipeline import OK")
print("moge import OK")
PY

echo
echo "Bootstrap complete."
echo "Upload pixal3d_server.py to ${REPO_DIR}/pixal3d_server.py, then start with:"
echo "source ${CONDA_ROOT}/etc/profile.d/conda.sh && conda activate ${ENV_NAME} && export PYTHONPATH=${REPO_DIR} && export PIXAL3D_PORT=18001 && cd ${REPO_DIR} && python -u ${REPO_DIR}/pixal3d_server.py"
