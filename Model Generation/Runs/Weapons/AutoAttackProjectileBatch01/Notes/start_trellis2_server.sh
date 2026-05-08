#!/usr/bin/env bash
set -euo pipefail

source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2

export NVCC_PREPEND_FLAGS=
export CUDA_HOME=/usr/local/cuda
export PATH="$CONDA_PREFIX/bin:/usr/local/cuda/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
export PYTHONPATH=/workspace/TRELLIS.2
export OPENCV_IO_ENABLE_OPENEXR=1
export PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True

cd /workspace/TRELLIS.2
exec python -u /workspace/TRELLIS.2/trellis_server.py
