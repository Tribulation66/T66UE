#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Heroes/Hero_1_Arthur"
IN="$BASE/Inputs"
OUT="$BASE/Raw/Trellis"
LOG="$BASE/Notes/trellis_arthur_six_batch.log"

mkdir -p "$OUT" "$BASE/Notes"
: > "$LOG"

run_one() {
  local src="$1"
  local module="$2"
  local decimation="$3"
  local dst="$OUT/${module}_S1337_D${decimation}_Trellis2.glb"

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

run_one "$IN/ModelReference_Green.png" "ModelReference_Green" "80000"
run_one "$IN/Hero_1_Arthur_01_Apose_NoWeapon_Green.png" "Hero_1_Arthur_01_Apose_NoWeapon_Green" "80000"
run_one "$IN/Hero_1_Arthur_02_Apose_RightHandWeapon_Green.png" "Hero_1_Arthur_02_Apose_RightHandWeapon_Green" "80000"
run_one "$IN/Hero_1_Arthur_03_WeaponOnly_Green.png" "Hero_1_Arthur_03_WeaponOnly_Green" "200000"
run_one "$IN/Hero_1_Arthur_04_HeadOnly_Green.png" "Hero_1_Arthur_04_HeadOnly_Green" "120000"
run_one "$IN/Hero_1_Arthur_05_BodyOnly_Green.png" "Hero_1_Arthur_05_BodyOnly_Green" "80000"
