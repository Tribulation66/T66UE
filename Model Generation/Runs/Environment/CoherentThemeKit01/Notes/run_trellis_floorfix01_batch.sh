#!/usr/bin/env bash
set -euo pipefail

BASE="/workspace/T66/ModelGeneration/Runs/Environment/CoherentThemeKit01"
IN="$BASE/Inputs/floor_slab_seed_images"
OUT="$BASE/Raw/TrellisFloorFix01"
LOG="$BASE/Notes/trellis_floorfix01_batch.log"

mkdir -p "$OUT" "$BASE/Notes"
: > "$LOG"

modules=(
  DungeonFloor_StoneSlabs_A
  DungeonFloor_Drain_A
  DungeonFloor_Cracked_A
  DungeonFloor_Bones_A
  ForestFloor_RootMat_A
  ForestFloor_MossStone_A
  ForestFloor_LeafCrack_A
  ForestFloor_BrambleEdge_A
  OceanFloor_ReefStone_A
  OceanFloor_ShellSand_A
  OceanFloor_CoralCrack_A
  OceanFloor_TidePool_A
  MartianFloor_RuinTile_A
  MartianFloor_RegolithPlates_A
  MartianFloor_CrystalDust_A
  MartianFloor_CraterCracks_A
  HellFloor_RunePlate_A
  HellFloor_Obsidian_A
  HellFloor_EmberFissure_A
  HellFloor_BoneAsh_A
)

for module in "${modules[@]}"; do
  src="$IN/${module}_FloorFix01.png"
  dst="$OUT/${module}_FloorFix01_S1337_D80000_Trellis2.glb"
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
