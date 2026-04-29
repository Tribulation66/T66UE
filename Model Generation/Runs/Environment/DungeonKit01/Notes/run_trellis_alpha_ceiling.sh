#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Environment/DungeonKit01"
MODULE="DungeonCeiling_ChainAnchor_A"
SRC="$BASE/Inputs/alpha_seed_images/${MODULE}.png"
DST="$BASE/Raw/TrellisAlpha/${MODULE}_Alpha_S1337_D80000_Trellis2.glb"
LOG="$BASE/Notes/trellis_alpha_batch.log"

echo "[$(date -Is)] START ${MODULE} isolated" | tee -a "$LOG"
start=$(date +%s)
timeout 1200 curl --fail --show-error --silent \
  -X POST http://127.0.0.1:8000/generate \
  -H 'Content-Type: image/png' \
  -H 'X-Seed: 1337' \
  -H 'X-Texture-Size: 2048' \
  -H 'X-Decimation: 80000' \
  --data-binary "@$SRC" \
  -o "$DST"
end=$(date +%s)
size=$(stat -c%s "$DST")
echo "[$(date -Is)] DONE ${MODULE} isolated $size bytes duration=$((end-start))s -> $DST" | tee -a "$LOG"
