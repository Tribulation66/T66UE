#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/EnemyBosses/EnemyBossBatch01"
IN="$BASE/Inputs/TrellisImages/Enemies"
OUT="$BASE/Raw/Trellis/Enemies"
LOG="$BASE/Notes/enemies_trellis_pod_batch.log"
SEED="${SEED:-1337}"
TEXTURE_SIZE="${TEXTURE_SIZE:-2048}"
DECIMATION="${DECIMATION:-80000}"
FORCE_LABELS="${FORCE_LABELS:-Dungeon_Slime Dungeon_WebSpider}"

labels=(
  Dungeon_Slime
  Dungeon_Skeleton
  Dungeon_WebSpider
  Dungeon_RabidRat
  Dungeon_Bat
  Forest_MushroomBrute
  Forest_TreantSapling
  Forest_ThornImp
  Forest_Boar
  Forest_Wasp
  Ocean_CrabGuard
  Ocean_DrownedSailor
  Ocean_Jellyfish
  Ocean_SharkPup
  Ocean_GhostRay
  Martian_DroneGrunt
  Martian_CrystalCrawler
  Martian_PlasmaSpitter
  Martian_RocketLeaper
  Martian_SaucerDrone
  Hell_Imp
  Hell_BoneKnight
  Hell_FireSkull
  Hellhound
  Hell_Gargoyle
)

mkdir -p "$IN" "$OUT" "$(dirname "$LOG")"
: > "$LOG"

log() {
  echo "[$(date -Is)] $*" | tee -a "$LOG"
}

is_forced() {
  local label="$1"
  for forced in $FORCE_LABELS; do
    if [[ "$forced" == "$label" ]]; then
      return 0
    fi
  done
  return 1
}

log "enemy-stage01 start total=${#labels[@]} seed=$SEED texture=$TEXTURE_SIZE decimation=$DECIMATION"

if pgrep -f "/workspace/t66_enemyboss_bosses_stage01/run_bosses.sh" >/dev/null 2>&1; then
  log "waiting for boss stage01 job to finish before enemy Trellis requests"
fi
while pgrep -f "/workspace/t66_enemyboss_bosses_stage01/run_bosses.sh" >/dev/null 2>&1; do
  sleep 20
done

for i in "${!labels[@]}"; do
  label="${labels[$i]}"
  src="$IN/${label}.png"
  dst_dir="$OUT/$label"
  dst="$dst_dir/${label}_Trellis.glb"
  tmp="$dst.tmp"

  if [[ ! -f "$src" ]]; then
    log "skip $((i + 1))/${#labels[@]} $label missing input $src"
    continue
  fi

  mkdir -p "$dst_dir"

  if [[ -f "$dst" ]] && ! is_forced "$label"; then
    bytes="$(stat -c%s "$dst")"
    log "skip $((i + 1))/${#labels[@]} $label existing bytes=$bytes"
    continue
  fi

  rm -f "$tmp"
  log "generate $((i + 1))/${#labels[@]} $label"
  start="$(date +%s)"
  curl --fail --silent --show-error --max-time 3600 \
    -X POST \
    -H "Content-Type: image/png" \
    -H "X-Seed: $SEED" \
    -H "X-Texture-Size: $TEXTURE_SIZE" \
    -H "X-Decimation: $DECIMATION" \
    --data-binary "@$src" \
    http://127.0.0.1:8000/generate \
    -o "$tmp"
  mv "$tmp" "$dst"
  end="$(date +%s)"
  bytes="$(stat -c%s "$dst")"
  log "done $((i + 1))/${#labels[@]} $label bytes=$bytes duration=$((end - start))s"
done

log "enemy-stage01 complete"
