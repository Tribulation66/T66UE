#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Heroes/TypeA_Masculine_Batch01"
IN="$BASE/Inputs/approved_seed_images"
OUT="$BASE/Raw/Trellis"
LOG="$BASE/Notes/trellis_typea_batch01.log"
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
    size="$(stat -c%s "$dst")"
    echo "[$(date -Is)] SKIP existing output $module $size bytes -> $dst" | tee -a "$LOG"
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

run_one "$IN/Hero_1_Arthur_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_1_Arthur_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_1_Arthur_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_1_Arthur_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_3_LuBu_TypeA_Standard_BodyOnly_Green.png" "Hero_3_LuBu_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_3_LuBu_TypeA_Standard_HeadOnly_Green.png" "Hero_3_LuBu_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_3_LuBu_TypeA_Standard_WeaponOnly_Green.png" "Hero_3_LuBu_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_3_LuBu_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_3_LuBu_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_3_LuBu_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_3_LuBu_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_4_Mike_TypeA_Standard_BodyOnly_Green.png" "Hero_4_Mike_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_4_Mike_TypeA_Standard_HeadOnly_Green.png" "Hero_4_Mike_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_4_Mike_TypeA_Standard_WeaponOnly_Green.png" "Hero_4_Mike_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_4_Mike_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_4_Mike_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_4_Mike_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_4_Mike_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_5_George_TypeA_Standard_BodyOnly_Green.png" "Hero_5_George_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_5_George_TypeA_Standard_HeadOnly_Green.png" "Hero_5_George_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_5_George_TypeA_Standard_WeaponOnly_Green.png" "Hero_5_George_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_5_George_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_5_George_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_5_George_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_5_George_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_7_Robo_TypeA_Standard_BodyOnly_Green.png" "Hero_7_Robo_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_7_Robo_TypeA_Standard_HeadOnly_Green.png" "Hero_7_Robo_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_7_Robo_TypeA_Standard_WeaponOnly_Green.png" "Hero_7_Robo_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_7_Robo_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_7_Robo_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_7_Robo_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_7_Robo_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_8_Billy_TypeA_Standard_BodyOnly_Green.png" "Hero_8_Billy_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_8_Billy_TypeA_Standard_HeadOnly_Green.png" "Hero_8_Billy_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_8_Billy_TypeA_Standard_WeaponOnly_Green.png" "Hero_8_Billy_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_8_Billy_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_8_Billy_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_8_Billy_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_8_Billy_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_9_Rabbit_TypeA_Standard_BodyOnly_Green.png" "Hero_9_Rabbit_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_9_Rabbit_TypeA_Standard_HeadOnly_Green.png" "Hero_9_Rabbit_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_9_Rabbit_TypeA_Standard_WeaponOnly_Green.png" "Hero_9_Rabbit_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_9_Rabbit_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_9_Rabbit_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_9_Rabbit_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_9_Rabbit_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_11_Shroud_TypeA_Standard_BodyOnly_Green.png" "Hero_11_Shroud_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_11_Shroud_TypeA_Standard_HeadOnly_Green.png" "Hero_11_Shroud_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_11_Shroud_TypeA_Standard_WeaponOnly_Green.png" "Hero_11_Shroud_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_11_Shroud_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_11_Shroud_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_11_Shroud_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_11_Shroud_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_12_xQc_TypeA_Standard_BodyOnly_Green.png" "Hero_12_xQc_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_12_xQc_TypeA_Standard_HeadOnly_Green.png" "Hero_12_xQc_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_12_xQc_TypeA_Standard_WeaponOnly_Green.png" "Hero_12_xQc_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_12_xQc_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_12_xQc_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_12_xQc_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_12_xQc_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_13_Moist_TypeA_Standard_BodyOnly_Green.png" "Hero_13_Moist_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_13_Moist_TypeA_Standard_HeadOnly_Green.png" "Hero_13_Moist_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_13_Moist_TypeA_Standard_WeaponOnly_Green.png" "Hero_13_Moist_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_13_Moist_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_13_Moist_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_13_Moist_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_13_Moist_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_14_North_TypeA_Standard_BodyOnly_Green.png" "Hero_14_North_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_14_North_TypeA_Standard_HeadOnly_Green.png" "Hero_14_North_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_14_North_TypeA_Standard_WeaponOnly_Green.png" "Hero_14_North_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_14_North_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_14_North_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_14_North_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_14_North_TypeA_BeachGoer_WeaponOnly_Green" "200000"
run_one "$IN/Hero_15_Asmon_TypeA_Standard_BodyOnly_Green.png" "Hero_15_Asmon_TypeA_Standard_BodyOnly_Green" "80000"
run_one "$IN/Hero_15_Asmon_TypeA_Standard_HeadOnly_Green.png" "Hero_15_Asmon_TypeA_Standard_HeadOnly_Green" "120000"
run_one "$IN/Hero_15_Asmon_TypeA_Standard_WeaponOnly_Green.png" "Hero_15_Asmon_TypeA_Standard_WeaponOnly_Green" "200000"
run_one "$IN/Hero_15_Asmon_TypeA_BeachGoer_BodyOnly_Green.png" "Hero_15_Asmon_TypeA_BeachGoer_BodyOnly_Green" "80000"
run_one "$IN/Hero_15_Asmon_TypeA_BeachGoer_WeaponOnly_Green.png" "Hero_15_Asmon_TypeA_BeachGoer_WeaponOnly_Green" "200000"
