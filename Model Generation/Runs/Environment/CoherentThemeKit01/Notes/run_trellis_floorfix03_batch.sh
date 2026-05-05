#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Environment/CoherentThemeKit01"
IN="$BASE/Inputs/floor_slab_seed_images_fix03"
OUT="$BASE/Raw/TrellisFloorFix03"
LOG="$BASE/Notes/trellis_floorfix03_batch.log"

mkdir -p "$OUT" "$BASE/Notes"
: > "$LOG"

modules=(
  HellFloor_Obsidian_A
  HellFloor_EmberFissure_A
)

for module in "${modules[@]}"; do
  src="$IN/${module}_FloorFix03.png"
  dst="$OUT/${module}_FloorFix03_S1337_D80000_Trellis2.glb"
  echo "[$(date -Is)] START $module" | tee -a "$LOG"
  start="$(date +%s)"
  curl --fail --show-error --silent \
    -X POST http://127.0.0.1:8000/generate \
    -H 'Content-Type: image/png' \
    -H 'X-Seed: 1337' \
    -H 'X-Texture-Size: 2048' \
    -H 'X-Decimation: 80000' \
    --data-binary "@$src" \
    -o "$dst"
  end="$(date +%s)"
  size="$(stat -c%s "$dst")"
  echo "[$(date -Is)] DONE $module $size bytes duration=$((end-start))s -> $dst" | tee -a "$LOG"
done
