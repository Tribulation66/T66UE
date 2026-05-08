#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Heroes/ChadStacySourceImageBatch01"
IN="/tmp/T66_ChadStacyRemainingStacyInputs"
OUT="$BASE/Raw/Trellis"
LOG="$BASE/Notes/trellis_remaining_stacy_batch01.log"
FORCE_REGEN="${FORCE_REGEN:-0}"

mkdir -p "$OUT" "$BASE/Notes"
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

run_one "$IN/GambaStacy_Source_01.png" "GambaStacy_Source_01"
run_one "$IN/MonotoneStacy_Source_01.png" "MonotoneStacy_Source_01"
run_one "$IN/BaldStacy_Source_01.png" "BaldStacy_Source_01"
run_one "$IN/RoachStacy_Source_01.png" "RoachStacy_Source_01"
