#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Heroes/ChadStacySourceImageBatch01"
IN="$BASE/Inputs/approved_source_images"
OUT="$BASE/Raw/Trellis"
LOG="$BASE/Notes/trellis_chad_stacy_batch01.log"
FORCE_REGEN="${FORCE_REGEN:-0}"

mkdir -p "$IN" "$OUT" "$BASE/Notes"
: > "$LOG"

run_one() {
  local src="$1"
  local label="$2"
  local dst="$OUT/${label}_S1337_D80000_Trellis2.glb"

  if [[ ! -f "$src" ]]; then
    echo "[$(date -Is)] SKIP missing input $src" | tee -a "$LOG"
    return 0
  fi

  if [[ -f "$dst" && "$FORCE_REGEN" != "1" ]]; then
    size="$(stat -c%s "$dst")"
    echo "[$(date -Is)] SKIP existing output $label $size bytes -> $dst" | tee -a "$LOG"
    return 0
  fi

  echo "[$(date -Is)] START $label seed=1337 texture=2048 decimation=80000" | tee -a "$LOG"
  local start
  start="$(date +%s)"

  curl --fail --show-error --silent \
    -X POST http://127.0.0.1:8000/generate \
    -H 'Content-Type: image/png' \
    -H 'X-Seed: 1337' \
    -H 'X-Texture-Size: 2048' \
    -H 'X-Decimation: 80000' \
    --data-binary "@$src" \
    -o "$dst"

  local end size
  end="$(date +%s)"
  size="$(stat -c%s "$dst")"
  echo "[$(date -Is)] DONE $label $size bytes duration=$((end-start))s -> $dst" | tee -a "$LOG"
}

run_one "$IN/RoyalChad_Source_02.png" "RoyalChad_Source_02"
run_one "$IN/ChineseChad_Source_01.png" "ChineseChad_Source_01"
run_one "$IN/FoundingChad_Source_01.png" "FoundingChad_Source_01"
run_one "$IN/RoboChad_Source_01.png" "RoboChad_Source_01"
run_one "$IN/BillyChad_Source_01.png" "BillyChad_Source_01"
run_one "$IN/RabbitChad_Source_02.png" "RabbitChad_Source_02"
run_one "$IN/CSChad_Source_02.png" "CSChad_Source_02"
run_one "$IN/GambaChad_Source_02.png" "GambaChad_Source_02"
run_one "$IN/MonotoneChad_Source_02.png" "MonotoneChad_Source_02"
run_one "$IN/BaldChad_Source_02.png" "BaldChad_Source_02"
run_one "$IN/RoachChad_Source_02.png" "RoachChad_Source_02"
run_one "$IN/BoxerStacy_Source_01.png" "BoxerStacy_Source_01"
run_one "$IN/RoyalStacy_Source_01.png" "RoyalStacy_Source_01"
run_one "$IN/ChineseStacy_Source_01.png" "ChineseStacy_Source_01"
run_one "$IN/FoundingStacy_Source_01.png" "FoundingStacy_Source_01"
run_one "$IN/RoboStacy_Source_01.png" "RoboStacy_Source_01"
run_one "$IN/BillyStacy_Source_01.png" "BillyStacy_Source_01"
run_one "$IN/RabbitStacy_Source_01.png" "RabbitStacy_Source_01"
run_one "$IN/CSStacy_Source_01.png" "CSStacy_Source_01"
