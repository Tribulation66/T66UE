#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Interactables/ArcadeReplacementBatch01"
IN="$BASE/Inputs/source_images"
OUT="$BASE/Raw/Trellis"
LOG="$BASE/Notes/trellis_interactables_batch01.log"
FORCE_REGEN="${FORCE_REGEN:-0}"

mkdir -p "$OUT" "$BASE/Notes"
: > "$LOG"

run_one() {
  local src="$1"
  local module="$2"
  local decimation="$3"
  local dst="$OUT/${module}_S1337_D${decimation}_Trellis2.glb"

  if [[ ! -f "$src" ]]; then
    echo "[$(date -Is)] SKIP missing input $src" | tee -a "$LOG"
    return 0
  fi

  if [[ -f "$dst" && "$FORCE_REGEN" != "1" ]]; then
    local existing_size
    existing_size="$(stat -c%s "$dst")"
    echo "[$(date -Is)] SKIP existing output $module $existing_size bytes -> $dst" | tee -a "$LOG"
    return 0
  fi

  echo "[$(date -Is)] START $module seed=1337 texture=2048 decimation=$decimation" | tee -a "$LOG"
  local start
  start="$(date +%s)"

  curl --fail --show-error --silent \
    -X POST http://127.0.0.1:8000/generate \
    -H 'Content-Type: image/png' \
    -H 'X-Seed: 1337' \
    -H 'X-Texture-Size: 2048' \
    -H "X-Decimation: ${decimation}" \
    --data-binary "@$src" \
    -o "$dst"

  local end size
  end="$(date +%s)"
  size="$(stat -c%s "$dst")"
  echo "[$(date -Is)] DONE $module $size bytes duration=$((end-start))s -> $dst" | tee -a "$LOG"
}

run_one "$IN/GamblerDemonStand_White.png" "GamblerDemonStand_White" "80000"
run_one "$IN/ArcadeMachine_White.png" "ArcadeMachine_White" "80000"
run_one "$IN/ArcadeAmplifierPickup_White.png" "ArcadeAmplifierPickup_White" "80000"
run_one "$IN/ArcadeAmplifierPickup_Charged_White.png" "ArcadeAmplifierPickup_Charged_White" "80000"
run_one "$IN/Chest_White.png" "Chest_White" "80000"
