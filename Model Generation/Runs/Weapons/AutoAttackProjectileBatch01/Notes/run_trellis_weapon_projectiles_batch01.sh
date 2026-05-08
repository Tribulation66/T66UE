#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Weapons/AutoAttackProjectileBatch01"
IN="$BASE/Inputs/source_images"
OUT="$BASE/Raw/Trellis"
LOG="$BASE/Notes/trellis_weapon_projectiles_batch01.log"
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
    local existing_size
    existing_size="$(stat -c%s "$dst")"
    echo "[$(date -Is)] SKIP existing output $label $existing_size bytes -> $dst" | tee -a "$LOG"
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

run_one "$IN/RoyalChad_Sword.png" "RoyalChad_Sword"
run_one "$IN/ChineseChad_Guandao.png" "ChineseChad_Guandao"
run_one "$IN/BoxerChad_Glove.png" "BoxerChad_Glove"
run_one "$IN/FoundingChad_Rapier.png" "FoundingChad_Rapier"
run_one "$IN/RoboChad_GearBlade.png" "RoboChad_GearBlade"
run_one "$IN/BillyChad_Bullet.png" "BillyChad_Bullet"
run_one "$IN/RabbitChad_Carrot.png" "RabbitChad_Carrot"
run_one "$IN/CSChad_TacticalKnife.png" "CSChad_TacticalKnife"
run_one "$IN/GoblinoChad_Cleaver.png" "GoblinoChad_Cleaver"
run_one "$IN/MonotoneChad_InkShard.png" "MonotoneChad_InkShard"
run_one "$IN/BaldChad_Hatchet.png" "BaldChad_Hatchet"
run_one "$IN/RoachChad_RustyCrown.png" "RoachChad_RustyCrown"
