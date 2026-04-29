#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Environment/DungeonKit01"
IN="$BASE/Inputs/alpha_seed_images"
OUT="$BASE/Raw/TrellisAlpha"
LOG="$BASE/Notes/trellis_alpha_batch.log"

mkdir -p "$OUT" "$BASE/Notes"
: > "$LOG"

modules=(
  DungeonWall_Doorway_Arch
  DungeonWall_Straight_BonesNiche
  DungeonCeiling_ChainAnchor_A
)

for module in "${modules[@]}"; do
  src="$IN/${module}.png"
  dst="$OUT/${module}_Alpha_S1337_D80000_Trellis2.glb"
  echo "[$(date -Is)] START $module" | tee -a "$LOG"
  start=$(date +%s)
  curl --fail --show-error --silent \
    -X POST http://127.0.0.1:8000/generate \
    -H 'Content-Type: image/png' \
    -H 'X-Seed: 1337' \
    -H 'X-Texture-Size: 2048' \
    -H 'X-Decimation: 80000' \
    --data-binary "@$src" \
    -o "$dst"
  end=$(date +%s)
  size=$(stat -c%s "$dst")
  echo "[$(date -Is)] DONE $module $size bytes duration=$((end-start))s -> $dst" | tee -a "$LOG"
done
